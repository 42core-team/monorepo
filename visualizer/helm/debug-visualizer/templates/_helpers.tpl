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

