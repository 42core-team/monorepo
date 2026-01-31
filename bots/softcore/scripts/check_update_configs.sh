#!/usr/bin/env bash
set -euo pipefail

check_config() {
	local config_name="$1"
	local local_path="configs/${config_name}.config.json"
	local url="[[event_url]]/${config_name}-config"

	local remote_content
	remote_content=$(curl -sf --connect-timeout 10 "$url") || {
		echo "Failed to fetch: $url - this might mean you are playing with an out of date config. You should address this." >&2
		return 1
	}

	local local_content
	local_content=$(<"$local_path")

	if [[ "$remote_content" == "$local_content" ]]; then
		return 0
	fi

	echo ""
	echo "=================================================="
	echo "⚠  CONFIG CHANGED: ${config_name}.config.json"
	echo "=================================================="
	echo ""

	local backup_path="${local_path%.*}_old.${local_path##*.}"

	cp "$local_path" "$backup_path"
	echo "$remote_content" > "$local_path"

	echo "The following changes have occurred:"
	diff --color=auto -u "$backup_path" "$local_path" || true
}

check_config "game"
check_config "server"
