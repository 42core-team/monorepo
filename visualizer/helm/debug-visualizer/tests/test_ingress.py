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
    "dev": ("development", "debug-visualizer-dev", "dev.visualizer.coregame.sh", "dev-visualizer-coregame-sh-tls", 1),
    "prod": ("production", "debug-visualizer", "visualizer.coregame.sh", "visualizer-coregame-sh-tls", 2),
}


def render(environment=None, extra=()):
    namespace, release, _, _, _ = ENVIRONMENTS[environment or "prod"]
    command = ["helm", "template", release, str(CHART), "--namespace", namespace]
    if environment:
        command.extend(["--values", str(CHART / f"values-{environment}.yaml")])
    result = subprocess.run(command + list(extra), text=True, capture_output=True)
    return result, [document for document in yaml.safe_load_all(result.stdout) if document]


def resources(documents, kind):
    return [document for document in documents if document["kind"] == kind]


class IngressTests(unittest.TestCase):
    def test_default_and_environment_routes_use_traefik_with_existing_tls(self):
        for environment in [None, "dev", "prod"]:
            with self.subTest(environment=environment):
                result, documents = render(environment)
                self.assertEqual(result.returncode, 0, result.stderr)
                routes = resources(documents, "Ingress")
                self.assertEqual(len(routes), 1)
                ingress = routes[0]
                _, release, host, secret, _ = ENVIRONMENTS[environment or "prod"]
                self.assertEqual(ingress["metadata"]["name"], release)
                self.assertEqual(ingress["spec"]["ingressClassName"], "traefik")
                self.assertEqual(ingress["spec"]["tls"], [{"hosts": [host], "secretName": secret}])
                rule = ingress["spec"]["rules"][0]
                self.assertEqual(rule["host"], host)
                path = rule["http"]["paths"][0]
                self.assertEqual((path["path"], path["pathType"]), ("/", "Prefix"))
                self.assertEqual(path["backend"]["service"], {"name": release, "port": {"number": 80}})
                annotations = ingress["metadata"]["annotations"]
                self.assertEqual(annotations["cert-manager.io/cluster-issuer"], "letsencrypt-prod")
                self.assertEqual(annotations["traefik.ingress.kubernetes.io/router.entrypoints"], "websecure")
                self.assertEqual(annotations["traefik.ingress.kubernetes.io/router.tls"], "true")
                self.assertNotIn("kubernetes.io/ingress.class", annotations)
                self.assertFalse(any(key.startswith("nginx.ingress.kubernetes.io/") for key in annotations))

    def test_disabled_ingress_does_not_change_application_resources(self):
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                _, enabled = render(environment)
                result, disabled = render(environment, ("--set", "ingress.enabled=false"))
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(resources(disabled, "Ingress"), [])
                self.assertEqual([d for d in enabled if d["kind"] != "Ingress"], disabled)
                for kind in ["Deployment", "Service"]:
                    self.assertEqual(len(resources(disabled, kind)), 1)

    def test_image_overrides_and_application_configuration_are_preserved(self):
        for environment in ENVIRONMENTS:
            with self.subTest(environment=environment):
                result, documents = render(environment, ("--set-string", "image.repository=ghcr.io/42core-team/visualizer",
                                                        "--set-string", "image.tag=build-123"))
                self.assertEqual(result.returncode, 0, result.stderr)
                deployment = resources(documents, "Deployment")[0]
                _, release, _, _, replicas = ENVIRONMENTS[environment]
                self.assertEqual(deployment["metadata"]["name"], release)
                self.assertEqual(deployment["spec"]["replicas"], replicas)
                container = deployment["spec"]["template"]["spec"]["containers"][0]
                self.assertEqual(container["image"], "ghcr.io/42core-team/visualizer:build-123")
                self.assertEqual(container["ports"], [{"name": "http", "containerPort": 80, "protocol": "TCP"}])
                self.assertEqual(container["readinessProbe"]["httpGet"], {"path": "/", "port": "http"})
                service = resources(documents, "Service")[0]
                self.assertEqual(service["metadata"]["name"], release)
                self.assertEqual(service["spec"]["type"], "ClusterIP")
                self.assertEqual(service["spec"]["ports"][0]["port"], 80)

    def test_enabled_ingress_requires_traefik_class(self):
        for value in ['""', "null", '"nginx"']:
            with self.subTest(class_name=value):
                result, _ = render("dev", ("--set-json", "ingress.className=" + value))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("ingress.className must be traefik", result.stderr)

    def test_tls_configuration_is_required_for_enabled_ingress(self):
        cases = [(("--set-json", "ingress.tls=[]"), "ingress.tls must reference certificate Secrets"),
                 (("--set", "ingress.tls[0].secretName="), "requires secretName and hosts"),
                 (("--set-json", "ingress.tls[0].hosts=[]"), "requires secretName and hosts")]
        for extra, message in cases:
            with self.subTest(extra=extra):
                result, _ = render("dev", extra)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_custom_annotations_stay_on_the_single_certificate_owner(self):
        extra = ("--set-string", r"ingress.annotations.example\.com/custom=kept",
                 "--set-string", r"ingress.annotations.external-dns\.kubernetes\.io/target=192.0.2.1")
        result, documents = render("dev", extra)
        self.assertEqual(result.returncode, 0, result.stderr)
        routes = resources(documents, "Ingress")
        self.assertEqual(len(routes), 1)
        annotations = routes[0]["metadata"]["annotations"]
        self.assertEqual(annotations["example.com/custom"], "kept")
        self.assertEqual(annotations["external-dns.kubernetes.io/target"], "192.0.2.1")
        self.assertEqual(annotations["cert-manager.io/cluster-issuer"], "letsencrypt-prod")


if __name__ == "__main__":
    unittest.main(verbosity=2)
