# Debug visualizer Helm chart

The chart exposes the visualizer through one native Kubernetes Ingress owned by
Traefik. Default, development, and production values use IngressClass `traefik`,
the HTTPS `websecure` entrypoint, and cert-manager ClusterIssuer
`letsencrypt-prod`. The existing Ingress names, hosts, Services, and TLS Secret
names are retained. The container's NGINX static-file server, SPA paths, cache
headers, and security headers are unchanged.

| Environment | Namespace | Release | Values | Host | Certificate/Secret |
| --- | --- | --- | --- | --- | --- |
| Development | `development` | `debug-visualizer-dev` | `values-dev.yaml` | `dev.visualizer.coregame.sh` | `dev-visualizer-coregame-sh-tls` |
| Production | `production` | `debug-visualizer` | `values-prod.yaml` | `visualizer.coregame.sh` | `visualizer-coregame-sh-tls` |

## Prerequisites

- Terraform must deploy Traefik with IngressClass `traefik`, HTTPS entrypoint
  `websecure` on port 443, and HTTP-to-HTTPS redirection on port 80 that permits
  cert-manager HTTP-01 challenges.
- The shared `letsencrypt-prod` ClusterIssuer must solve HTTP-01 through Traefik.
  Existing certificate Secrets must remain in their application namespaces.
- Update public DNS through the DNS management process to target Traefik's load
  balancer. DNS is managed outside Terraform after removal of Cloudflare DNS
  management. Every application subdomain currently CNAMEs to `coregame.sh`, so
  the apex A record affects the website, API, visualizer, and redirect hosts.

See [Traefik's Ingress documentation](https://doc.traefik.io/traefik/reference/routing-configuration/kubernetes/ingress/)
for the native routing annotations. There is no NGINX-controller annotation
compatibility requirement.

## Deployment

`.github/workflows/visualizer.yml` already supplies the selected environment
values, existing release name/namespace, and built image repository/tag. Deploy
this chart through that workflow. No workflow or application image changes are
required for the ingress configuration.

For an explicit development deployment from the monorepo root, supply the exact
image repository/tag intended for that release:

```bash
export KUBECONFIG=/Users/paul/.kube/kcoregame
CHART=visualizer/helm/debug-visualizer
: "${IMAGE_REPOSITORY:?set the visualizer image repository}"
: "${IMAGE_TAG:?set the intended built image tag}"
kubectl config current-context
helm upgrade debug-visualizer-dev "$CHART" -n development \
  -f "$CHART/values-dev.yaml" \
  --set-string image.repository="$IMAGE_REPOSITORY" \
  --set-string image.tag="$IMAGE_TAG" \
  --wait --atomic --timeout 3m
```

For production use release `debug-visualizer`, namespace `production`, and
`values-prod.yaml`. Supply the environment values explicitly; do not reuse old
release values that still contain NGINX annotations or ingress class settings.
The examples use Helm 3's `--atomic`, matching the workflow's pinned Helm 3.
With Helm 4, replace `--atomic` with `--rollback-on-failure`; retain `--wait`
and the timeout. See the [Helm upgrade reference](https://helm.sh/docs/helm/helm_upgrade/).

Verify the route, certificate, served HTTPS certificate, embedded version paths,
and replay loading after deployment. Helm's readiness checks do not prove public
DNS routing or ACME certificate issuance:

```bash
kubectl get ingress debug-visualizer-dev -n development
kubectl get certificate dev-visualizer-coregame-sh-tls -n development
kubectl get orders,challenges -n development
curl -I https://dev.visualizer.coregame.sh/
curl -I https://dev.visualizer.coregame.sh/dev/
```

## Offline validation

Run from the chart directory:

```bash
helm lint .
helm lint . -f values-dev.yaml
helm lint . -f values-prod.yaml
uv run --script tests/test_ingress.py
```

Render tests check the default and both environment routes, native annotations,
TLS and certificate ownership, unchanged application resources, image overrides,
and invalid ingress class/TLS settings. They do not contact a cluster or prove live routing,
DNS, or ACME issuance.
