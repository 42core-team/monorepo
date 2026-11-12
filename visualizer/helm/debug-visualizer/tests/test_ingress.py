# /// script
# requires-python = ">=3.10"
# dependencies = ["PyYAML>=6,<7"]
# ///
"""Run without cluster access: uv run --script tests/test_ingress.py."""

from pathlib import Path
import subprocess
import unittest

import yaml

CHART = Path(__file__).resolve().parents[1]
ENVIRONMENTS = {
    "dev": ("development", "debug-visualizer-dev", "dev.visualizer.coregame.sh", "dev-visualizer-coregame-sh-tls"),
    "prod": ("production", "debug-visualizer", "visualizer.coregame.sh", "visualizer-coregame-sh-tls"),
}


def render(environment, overlay=None, extra=(), release=None):
    namespace, expected_release, _, _ = ENVIRONMENTS[environment]
    command = ["helm", "template", release or expected_release, str(CHART), "--namespace", namespace,
               "--values", str(CHART / f"values-{environment}.yaml")]
    if overlay:
        command.extend(["--values", str(CHART / overlay)])
    result = subprocess.run(command + list(extra), text=True, capture_output=True)
    return result, [document for document in yaml.safe_load_all(result.stdout) if document]


def ingresses(documents):
    return [document for document in documents if document["kind"] == "Ingress"]


class IngressMigrationTests(unittest.TestCase):
    def assert_traefik(self, ingress, environment, preview, issuer="letsencrypt-prod"):
        _, release, host, secret = ENVIRONMENTS[environment]
        self.assertEqual(ingress["spec"]["ingressClassName"], "traefik")
        self.assertEqual(ingress["spec"]["tls"], [{"hosts": [host], "secretName": secret}])
        rule = ingress["spec"]["rules"][0]
        self.assertEqual(rule["host"], host)
        path = rule["http"]["paths"][0]
        self.assertEqual((path["path"], path["pathType"]), ("/", "Prefix"))
        self.assertEqual(path["backend"]["service"], {"name": release, "port": {"number": 80}})
        annotations = ingress["metadata"]["annotations"]
        self.assertEqual(annotations["traefik.ingress.kubernetes.io/router.entrypoints"], "websecure")
        self.assertEqual(annotations["traefik.ingress.kubernetes.io/router.tls"], "true")
        self.assertNotIn("kubernetes.io/ingress.class", annotations)
        self.assertFalse(any(key.startswith("nginx.ingress.kubernetes.io/") for key in annotations))
        if preview:
            self.assertFalse(any(key.startswith(("cert-manager.io/", "acme.cert-manager.io/")) for key in annotations))
            self.assertNotIn("kubernetes.io/tls-acme", annotations)
        else:
            self.assertEqual(annotations["cert-manager.io/cluster-issuer"], issuer)

    def test_default_routes_remain_nginx(self):
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                result, documents = render(environment)
                self.assertEqual(result.returncode, 0, result.stderr)
                routes = ingresses(documents)
                self.assertEqual(len(routes), 1)
                route = routes[0]
                self.assertEqual(route["metadata"]["name"], ENVIRONMENTS[environment][1])
                self.assertEqual(route["spec"]["ingressClassName"], "nginx")
                self.assertEqual(route["metadata"]["annotations"]["kubernetes.io/ingress.class"], "nginx")
                self.assertEqual(route["metadata"]["annotations"]["nginx.ingress.kubernetes.io/ssl-redirect"], "true")

    def test_preview_keeps_original_and_app_resources_unchanged(self):
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                _, original = render(environment)
                result, preview = render(environment, "values-traefik-preview.yaml")
                self.assertEqual(result.returncode, 0, result.stderr)
                routes = ingresses(preview)
                self.assertEqual(len(routes), 2)
                self.assertEqual(routes[0], ingresses(original)[0])
                self.assertEqual(routes[1]["metadata"]["name"], ENVIRONMENTS[environment][1] + "-traefik-preview")
                self.assert_traefik(routes[1], environment, preview=True)
                self.assertEqual([d for d in original if d["kind"] != "Ingress"],
                                 [d for d in preview if d["kind"] != "Ingress"])

    def test_final_cutover_has_one_traefik_route_and_certificate_owner(self):
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                result, documents = render(environment, "values-traefik.yaml")
                self.assertEqual(result.returncode, 0, result.stderr)
                routes = ingresses(documents)
                self.assertEqual(len(routes), 1)
                self.assertEqual(routes[0]["metadata"]["name"], ENVIRONMENTS[environment][1])
                self.assert_traefik(routes[0], environment, preview=False)

    def test_preview_removes_all_certificate_ownership_annotations(self):
        extra = ("--set-string", r"ingress.annotations.cert-manager\.io/issuer=other",
                 "--set-string", r"ingress.annotations.acme\.cert-manager\.io/http01-ingress-class=nginx",
                 "--set-string", r"ingress.annotations.kubernetes\.io/tls-acme=true",
                 "--set-string", r"ingress.annotations.example\.com/custom=kept")
        result, documents = render("dev", "values-traefik-preview.yaml", extra)
        self.assertEqual(result.returncode, 0, result.stderr)
        preview = ingresses(documents)[1]
        self.assert_traefik(preview, "dev", preview=True)
        self.assertEqual(preview["metadata"]["annotations"]["example.com/custom"], "kept")

    def test_preview_removes_dns_ownership_without_changing_primary(self):
        extra = ("--set-string", r"ingress.annotations.external-dns\.alpha\.kubernetes\.io/hostname=dev.visualizer.coregame.sh",
                 "--set-string", r"ingress.annotations.external-dns\.kubernetes\.io/target=192.0.2.1")
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                result, documents = render(environment, "values-traefik-preview.yaml", extra)
                self.assertEqual(result.returncode, 0, result.stderr)
                primary, preview = ingresses(documents)
                for key in ("external-dns.alpha.kubernetes.io/hostname", "external-dns.kubernetes.io/target"):
                    self.assertIn(key, primary["metadata"]["annotations"])
                    self.assertNotIn(key, preview["metadata"]["annotations"])
                result, documents = render(environment, "values-traefik.yaml", extra)
                self.assertEqual(result.returncode, 0, result.stderr)
                for key in ("external-dns.alpha.kubernetes.io/hostname", "external-dns.kubernetes.io/target"):
                    self.assertIn(key, ingresses(documents)[0]["metadata"]["annotations"])

    def test_migration_issuer_override_belongs_only_to_original_ingress(self):
        extra = ("--set-string", r"ingress.annotations.cert-manager\.io/cluster-issuer=letsencrypt-traefik")
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                result, documents = render(environment, "values-traefik-preview.yaml", extra)
                self.assertEqual(result.returncode, 0, result.stderr)
                primary, preview = ingresses(documents)
                self.assertEqual(primary["metadata"]["annotations"]["cert-manager.io/cluster-issuer"], "letsencrypt-traefik")
                self.assert_traefik(preview, environment, preview=True)
                result, documents = render(environment, "values-traefik.yaml", extra)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(len(ingresses(documents)), 1)
                self.assert_traefik(ingresses(documents)[0], environment, preview=False, issuer="letsencrypt-traefik")

    def test_invalid_migration_combinations_fail(self):
        cases = [(("--set", "ingress.className=traefik"), "disable ingress.traefikPreview"),
                 (("--set", "ingress.enabled=false"), "requires ingress.enabled=true"),
                 (("--set-json", "ingress.tls=[]"), "require ingress.tls"),
                 (("--set", "ingress.tls[0].secretName="), "requires secretName and hosts")]
        for extra, message in cases:
            with self.subTest(extra=extra):
                result, _ = render("dev", "values-traefik-preview.yaml", extra)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_disabled_ingress_renders_no_routes(self):
        result, documents = render("dev", extra=("--set", "ingress.enabled=false"))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(ingresses(documents), [])

    def test_long_preview_name_is_valid_and_collision_resistant(self):
        names = []
        for suffix in ["a", "b"]:
            result, documents = render("dev", "values-traefik-preview.yaml", release="visualizer-" + "x" * 41 + suffix)
            self.assertEqual(result.returncode, 0, result.stderr)
            name = ingresses(documents)[1]["metadata"]["name"]
            self.assertLessEqual(len(name), 63)
            self.assertRegex(name, r"^[a-z0-9][a-z0-9-]*[a-z0-9]$")
            self.assertTrue(name.endswith("-traefik-preview"))
            names.append(name)
        self.assertNotEqual(*names)


if __name__ == "__main__":
    unittest.main(verbosity=2)
