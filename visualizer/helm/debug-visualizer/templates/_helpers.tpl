{{- define "debug-visualizer.name" -}}
{{- default .Chart.Name .Values.nameOverride | trunc 63 | trimSuffix "-" -}}
{{- end -}}

{{- define "debug-visualizer.fullname" -}}
{{- if .Values.fullnameOverride -}}
{{- .Values.fullnameOverride | trunc 63 | trimSuffix "-" -}}
{{- else -}}
{{- .Release.Name | trunc 63 | trimSuffix "-" -}}
{{- end -}}
{{- end -}}

{{- define "debug-visualizer.labels" -}}
app.kubernetes.io/name: {{ include "debug-visualizer.name" . }}
helm.sh/chart: {{ .Chart.Name }}-{{ .Chart.Version | replace "+" "_" }}
app.kubernetes.io/instance: {{ .Release.Name }}
app.kubernetes.io/managed-by: {{ .Release.Service }}
{{- end -}}

{{- define "debug-visualizer.selectorLabels" -}}
app.kubernetes.io/name: {{ include "debug-visualizer.name" . }}
app.kubernetes.io/instance: {{ include "debug-visualizer.fullname" . }}
{{- end -}}


{{/* Render both routes from the same hosts, backend and TLS configuration. */}}
{{- define "debug-visualizer.ingress" -}}
{{- $root := .root -}}
{{- $preview := .preview -}}
{{- $traefik := or $preview (eq $root.Values.ingress.className "traefik") -}}
{{- $name := include "debug-visualizer.fullname" $root -}}
{{- if $preview -}}
{{- if gt (len $name) 47 -}}
{{- $name = printf "%s-%s-traefik-preview" ($name | trunc 38 | trimSuffix "-") ($name | sha256sum | trunc 8) -}}
{{- else -}}
{{- $name = printf "%s-traefik-preview" $name -}}
{{- end -}}
{{- end -}}
{{- if $traefik -}}
{{- if not $root.Values.ingress.tls -}}
{{- fail "Traefik routes require ingress.tls with existing certificate Secrets" -}}
{{- end -}}
{{- range $root.Values.ingress.tls -}}
{{- if or (not .secretName) (not .hosts) -}}
{{- fail "each Traefik ingress.tls entry requires secretName and hosts" -}}
{{- end -}}
{{- end -}}
{{- end -}}
{{- $annotations := dict -}}
{{- range $key, $value := $root.Values.ingress.annotations -}}
{{- $nginx := or (eq $key "kubernetes.io/ingress.class") (hasPrefix "nginx.ingress.kubernetes.io/" $key) -}}
{{- $certificateOwner := or (hasPrefix "cert-manager.io/" $key) (hasPrefix "acme.cert-manager.io/" $key) (eq $key "kubernetes.io/tls-acme") -}}
{{- $dnsOwner := or (hasPrefix "external-dns.alpha.kubernetes.io/" $key) (hasPrefix "external-dns.kubernetes.io/" $key) -}}
{{- if not (or (and $traefik $nginx) (and $preview (or $certificateOwner $dnsOwner))) -}}
{{- $_ := set $annotations $key $value -}}
{{- end -}}
{{- end -}}
{{- if $traefik -}}
{{- $_ := set $annotations "traefik.ingress.kubernetes.io/router.entrypoints" "websecure" -}}
{{- $_ := set $annotations "traefik.ingress.kubernetes.io/router.tls" "true" -}}
{{- end -}}
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: {{ $name }}
  labels:
    {{- include "debug-visualizer.labels" $root | nindent 4 }}
  {{- with $annotations }}
  annotations:
    {{- toYaml . | nindent 4 }}
  {{- end }}
spec:
  {{- if $traefik }}
  ingressClassName: traefik
  {{- else if $root.Values.ingress.className }}
  ingressClassName: {{ $root.Values.ingress.className }}
  {{- end }}
  {{- with $root.Values.ingress.tls }}
  tls:
    {{- toYaml . | nindent 4 }}
  {{- end }}
  rules:
    {{- range $root.Values.ingress.hosts }}
    - host: {{ .host | quote }}
      http:
        paths:
          {{- range .paths }}
          - path: {{ .path }}
            pathType: {{ .pathType }}
            backend:
              service:
                name: {{ include "debug-visualizer.fullname" $root }}
                port:
                  number: {{ $root.Values.service.port }}
          {{- end }}
    {{- end }}
{{- end -}}
