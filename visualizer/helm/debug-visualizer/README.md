# Visualizer ingress migration

The chart keeps its existing NGINX routes by default. Two overlays support a
staged move to Traefik without changing Services, application images, hosts,
paths, TLS Secret names, or the container's NGINX static-file server.

| Values | Primary route | Additional route | Certificate owner |
| --- | --- | --- | --- |
| Environment values only | NGINX, original name | None | Original Ingress |
| Plus `values-traefik-preview.yaml` | NGINX, original name | Traefik, `-traefik-preview` suffix | Original Ingress only |
| Plus `values-traefik.yaml` | Traefik, original name | None | Original Ingress |

Traefik routes use the HTTPS `websecure` entrypoint and existing TLS Secrets.
The controller must expose port 443 and redirect port 80 to HTTPS while allowing
cert-manager HTTP-01 challenge routes. The chart does not install a controller
or configure its redirect globally. See [Traefik's Ingress documentation](https://doc.traefik.io/traefik/reference/routing-configuration/kubernetes/ingress/).

For Traefik routes the chart removes NGINX annotations and the obsolete
`kubernetes.io/ingress.class` annotation. Preview routes additionally remove
cert-manager/ACME annotations, `kubernetes.io/tls-acme`, and both
`external-dns.alpha.kubernetes.io/*` and `external-dns.kubernetes.io/*` annotation
families. They reuse the original route's certificate Secret rather than asking
ingress-shim to own it again, and do not copy DNS ownership annotations to the
preview. Other custom annotations remain intact. Enabling preview requires TLS hosts and
Secret names, an enabled primary ingress, and a primary class other than Traefik.
A single Traefik primary route replaces the preview during final cutover.

## Prerequisites

- Use the intended cluster context and confirm the Traefik installation is ready,
  has its own load balancer, owns IngressClass `traefik`, and exposes its HTTPS
  entrypoint with the exact name `websecure`.
- Confirm the existing Certificate is Ready. Preview does not issue certificates.
- Prepare and test a Traefik HTTP-01 issuer for migrated hostnames. The live shared
  `letsencrypt-prod` issuer currently uses NGINX. After a hostname's DNS moves to
  Traefik, leaving its certificate renewal configured for NGINX will fail. A
  separate issuer such as `letsencrypt-traefik` lets development migrate while
  production continues renewing through NGINX. Keep this coordination in the
  Terraform/controller rollout; this chart does not create ClusterIssuers.
- Preserve current NGINX controller/routes until DNS caches have drained and all
  hosts and certificates have been verified on Traefik.

The live releases and environment files are:

| Environment | Namespace | Release | Values | Host | Certificate/Secret |
| --- | --- | --- | --- | --- | --- |
| Development | `development` | `debug-visualizer-dev` | `values-dev.yaml` | `dev.visualizer.coregame.sh` | `dev-visualizer-coregame-sh-tls` |
| Production | `production` | `debug-visualizer` | `values-prod.yaml` | `visualizer.coregame.sh` | `visualizer-coregame-sh-tls` |

## Development rollout

From the monorepo root, set the kubeconfig explicitly. The examples below update
an existing Helm release and reuse its saved configuration, retaining the image
repository/tag set by `.github/workflows/visualizer.yml`.

The examples use Helm 3's `--atomic`, matching the workflow's pinned Helm 3.
With Helm 4, replace `--atomic` with `--rollback-on-failure` in each upgrade
command; keep `--wait` and the timeout. Check `helm version --short` first.
See the [Helm upgrade reference](https://helm.sh/docs/helm/helm_upgrade/).

```bash
export KUBECONFIG=/Users/paul/.kube/kcoregame
CHART=visualizer/helm/debug-visualizer
kubectl config current-context
helm status debug-visualizer-dev -n development
kubectl get certificate dev-visualizer-coregame-sh-tls -n development
helm history debug-visualizer-dev -n development

helm upgrade debug-visualizer-dev "$CHART" -n development \
  --reuse-values -f "$CHART/values-traefik-preview.yaml" \
  --wait --atomic --timeout 3m
```

Record the pre-migration Helm revision. Inspect both Ingresses and check their
class, hosts, backend, and Secret references:

```bash
kubectl get ingress debug-visualizer-dev debug-visualizer-dev-traefik-preview \
  -n development -o yaml
```

Point test requests at the new load balancer without changing public DNS:

```bash
TRAEFIK_IP=<traefik-load-balancer-ip>
curl --resolve "dev.visualizer.coregame.sh:443:$TRAEFIK_IP" \
  -I https://dev.visualizer.coregame.sh/
curl --resolve "dev.visualizer.coregame.sh:443:$TRAEFIK_IP" \
  -I https://dev.visualizer.coregame.sh/dev/
```

Also exercise the browser replay loader and embedded version paths, and compare
cache/security headers. Do not use `curl -k`: the existing certificate should
validate. Check HTTP-to-HTTPS redirection at the new IP separately.

DNS is managed outside Terraform following removal of Cloudflare DNS management.
Make DNS changes through the current DNS management process; these Helm overlays
and Terraform will not update the records. All live application subdomains
currently CNAME to `coregame.sh`. To migrate only
development, first give `dev.visualizer.coregame.sh` an independent DNS target
pointing to Traefik; do not change the apex A record yet. Keep both routes while
the previous CNAME and apex TTLs expire. Validate through normal public DNS and
check both old/new load balancer traffic before removing the NGINX route.

Once DNS is reaching Traefik and the infrastructure rollout's migration issuer
`letsencrypt-traefik` is Ready, switch the primary route. The explicit annotation
override below selects that issuer on the original certificate-owning Ingress:

```bash
helm upgrade debug-visualizer-dev "$CHART" -n development \
  --reuse-values -f "$CHART/values-traefik.yaml" \
  --set-string 'ingress.annotations.cert-manager\.io/cluster-issuer=letsencrypt-traefik' \
  --wait --atomic --timeout 3m
kubectl get ingress -n development
kubectl get certificate dev-visualizer-coregame-sh-tls -n development
kubectl get orders,challenges -n development
```

Changing the issuer may trigger a certificate reissue. Wait for the Certificate
to be Ready with the intended issuer and validate the served certificate; Helm's
`--wait` does not check ACME issuance or public routing.

For production use release `debug-visualizer`, namespace `production`, and host
`visualizer.coregame.sh` throughout the same sequence. Coordinate its DNS cutover
with the website/API/wiki/www hosts before changing `coregame.sh`.

## CI persistence and rollback

The deployment workflow currently passes only the environment values file.
Manual migration flags will be reset on a later deployment. During preview,
append `-f .../values-traefik-preview.yaml` to the workflow's Helm command; during
final cutover append `-f .../values-traefik.yaml` and the approved issuer override,
or persist equivalent settings in the selected environment values file. Keep the
existing release name, namespace, image repository/tag, and other pipeline flags.
Make those stage-specific changes deliberately; this PR leaves ordinary CI
behavior on NGINX until the controller/DNS rollout is ready.

To cancel preview before DNS changes, rerun the previous release configuration
with `--set ingress.traefikPreview.enabled=false`. To roll back after DNS cutover,
first restore parallel routing with NGINX as primary, preserving the current
application image/values. For development:

```bash
helm upgrade debug-visualizer-dev "$CHART" -n development \
  --reuse-values --set ingress.className=nginx \
  --set ingress.traefikPreview.enabled=true \
  --set-string "ingress.annotations.cert-manager\\.io/cluster-issuer=letsencrypt-prod" \
  --wait --atomic --timeout 3m
```

Verify the restored NGINX route and certificate by targeting the old IP, then
restore the previous DNS record. Keep the Traefik preview route until clients'
old DNS caches have expired; only then disable preview. Verify the restored
certificate issuer and renewal status. Apply the equivalent sequence for the
production release. A direct rollback to an old chart revision removes the
Traefik route immediately, so avoid doing that while clients still cache its IP.

Do not uninstall NGINX until every application, redirect, and certificate renewal
has completed its own migration. Keep the pre-migration Helm revision/image
available for recovery, accounting for the DNS overlap described above.

## Offline validation

Run from the chart directory:

```bash
helm lint . -f values-dev.yaml
helm lint . -f values-prod.yaml
helm lint . -f values-dev.yaml -f values-traefik-preview.yaml
helm lint . -f values-prod.yaml -f values-traefik.yaml
uv run --script tests/test_ingress.py
```

The render regression checks cover both environments, default route compatibility,
preview certificate/DNS ownership and TLS/backend handling, final issuer override,
unchanged application resources, annotation cleanup, invalid combinations, and
long release names.
They do not contact a cluster or prove live routing, DNS, or ACME issuance.
