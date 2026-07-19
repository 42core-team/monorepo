#!/usr/bin/env bash
set -euo pipefail

CHECK=0
if [[ "${1:-}" == "--check" ]]; then
	CHECK=1
	shift
fi

if [[ $# -ne 0 ]]; then
	echo "Usage: bots/sync_base_resources.sh [--check]" >&2
	exit 2
fi

BOTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="$BOTS_DIR/base"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bots-sync.XXXXXX")"
trap 'rm -rf "$TMP_DIR"' EXIT

DRIFT=0
for BOT_DIR in "$BOTS_DIR"/*/bot; do
	[[ -d "$BOT_DIR" ]] || continue
	LANG="$(basename "$(dirname "$BOT_DIR")")"

	while IFS= read -r -d '' SOURCE; do
		RELATIVE_PATH="${SOURCE#$BASE_DIR/}"
		TARGET="$BOT_DIR/$RELATIVE_PATH"
		LANG_VALUE="$LANG" perl -0pe 's/\{\{LANG\}\}/$ENV{LANG_VALUE}/g' "$SOURCE" > "$TMP_DIR/rendered"

		if [[ "$CHECK" -eq 1 ]]; then
			if [[ ! -f "$TARGET" ]] || ! cmp -s "$TMP_DIR/rendered" "$TARGET"; then
				echo "DRIFT bots/$LANG/bot/$RELATIVE_PATH"
				DRIFT=1
			fi
			continue
		fi

		mkdir -p "$(dirname "$TARGET")"
		cp -p "$SOURCE" "$TARGET"
		LANG_VALUE="$LANG" perl -0pi -e 's/\{\{LANG\}\}/$ENV{LANG_VALUE}/g' "$TARGET"
		echo "SYNCED bots/$LANG/bot/$RELATIVE_PATH"
	done < <(find "$BASE_DIR" -type f -print0)
done

exit "$DRIFT"
