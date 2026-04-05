#!/usr/bin/env bash
set -euo pipefail

usage() {
	cat <<'EOF'
Usage: bots/sync_base_resources.sh [--check] [--verbose] [--lang <name> ...] [--mode softcore|hardcore ...]

Options:
  --check           Render and compare only; exit 1 if drift exists.
  --verbose         Print selection, source, and file outcome details.
  --lang <name>     Limit sync to a discovered language. Repeat to select more.
  --mode <mode>     Limit sync to softcore, hardcore, or both. Repeat to select more.
  --help            Show this help text.

Exit codes:
  0 success
  1 drift found in --check
  2 usage or structure error
EOF
}

die_usage() {
	echo "Error: $*" >&2
	usage >&2
	exit 2
}

die_structure() {
	echo "Error: $*" >&2
	exit 2
}

log_verbose() {
	if [[ "$VERBOSE" -eq 1 ]]; then
		echo "$*"
	fi
}

array_contains() {
	local needle="$1"
	shift
	local item
	for item in "$@"; do
		if [[ "$item" == "$needle" ]]; then
			return 0
		fi
	done
	return 1
}

append_unique_managed_path() {
	local candidate="$1"
	local existing
	for existing in "${MANAGED_REL_PATHS[@]-}"; do
		if [[ "$existing" == "$candidate" ]]; then
			return 0
		fi
	done
	MANAGED_REL_PATHS+=("$candidate")
}

discover_languages() {
	DISCOVERED_LANGS=()

	local dir
	while IFS= read -r dir; do
		[[ -z "$dir" ]] && continue
		local name
		name="$(basename "$dir")"
		if [[ "$name" == "base" || "$name" == .* ]]; then
			continue
		fi
		DISCOVERED_LANGS+=("$name")
	done < <(find "$BOTS_DIR" -mindepth 1 -maxdepth 1 -type d | LC_ALL=C sort)
}

collect_managed_paths_from_root() {
	local root="$1"

	[[ -d "$root" ]] || return 0

	local file rel
	while IFS= read -r -d '' file; do
		rel="${file#$root/}"
		append_unique_managed_path "$rel"
	done < <(find "$root" -type f -print0)
}

get_source_mode() {
	local source_path="$1"

	if stat -f '%Lp' "$source_path" >/dev/null 2>&1; then
		stat -f '%Lp' "$source_path"
		return 0
	fi

	if stat -c '%a' "$source_path" >/dev/null 2>&1; then
		stat -c '%a' "$source_path"
		return 0
	fi

	return 1
}

is_text_file() {
	local source_path="$1"

	if command -v file >/dev/null 2>&1; then
		local encoding
		encoding="$(file -b --mime-encoding "$source_path" 2>/dev/null || true)"
		if [[ -n "$encoding" && "$encoding" != "binary" ]]; then
			return 0
		fi
	fi

	if [[ ! -s "$source_path" ]]; then
		return 0
	fi

	LC_ALL=C grep -Iq . "$source_path"
}

render_text_file() {
	local source_path="$1"
	local output_path="$2"
	local lang="$3"

	LANG_VALUE="$lang" perl -0pe 's/\{\{LANG\}\}/$ENV{LANG_VALUE}/g' "$source_path" > "$output_path"
}

copy_with_source_mode() {
	local payload_path="$1"
	local destination_path="$2"
	local source_path="$3"
	local source_mode

	cp "$payload_path" "$destination_path"
	source_mode="$(get_source_mode "$source_path")" || die_structure "Unable to determine permissions for $source_path"
	chmod "$source_mode" "$destination_path"
}

CHECK=0
VERBOSE=0
REQUESTED_LANGS=()
REQUESTED_MODES=()

while [[ $# -gt 0 ]]; do
	case "$1" in
		--check)
			CHECK=1
			shift
			;;
		--verbose)
			VERBOSE=1
			shift
			;;
		--lang)
			[[ $# -ge 2 ]] || die_usage "--lang requires a value"
			REQUESTED_LANGS+=("$2")
			shift 2
			;;
		--mode)
			[[ $# -ge 2 ]] || die_usage "--mode requires a value"
			case "$2" in
				softcore|hardcore)
					REQUESTED_MODES+=("$2")
					;;
				*)
					die_usage "Unsupported mode: $2"
					;;
			esac
			shift 2
			;;
		--help|-h)
			usage
			exit 0
			;;
		*)
			die_usage "Unknown argument: $1"
			;;
	esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOTS_DIR="$SCRIPT_DIR"
GLOBAL_BASE_DIR="$BOTS_DIR/base"

[[ -d "$GLOBAL_BASE_DIR" ]] || die_structure "Missing global base directory: $GLOBAL_BASE_DIR"

discover_languages

if [[ "${#DISCOVERED_LANGS[@]}" -eq 0 ]]; then
	die_structure "No languages discovered under $BOTS_DIR"
fi

SELECTED_LANGS=()
if [[ "${#REQUESTED_LANGS[@]}" -gt 0 ]]; then
	local_lang=""
	for local_lang in "${REQUESTED_LANGS[@]}"; do
		if ! array_contains "$local_lang" "${DISCOVERED_LANGS[@]-}"; then
			die_usage "Unknown language: $local_lang"
		fi
		if ! array_contains "$local_lang" "${SELECTED_LANGS[@]-}"; then
			SELECTED_LANGS+=("$local_lang")
		fi
	done
else
	SELECTED_LANGS=("${DISCOVERED_LANGS[@]}")
fi

SELECTED_MODES=()
if [[ "${#REQUESTED_MODES[@]}" -gt 0 ]]; then
	local_mode=""
	for local_mode in "${REQUESTED_MODES[@]}"; do
		if ! array_contains "$local_mode" "${SELECTED_MODES[@]-}"; then
			SELECTED_MODES+=("$local_mode")
		fi
	done
else
	SELECTED_MODES=("softcore" "hardcore")
fi

TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bots-sync.XXXXXX")"
trap 'rm -rf "$TMP_DIR"' EXIT

echo "Sync mode: $([[ "$CHECK" -eq 1 ]] && echo check || echo apply)"
echo "Languages: ${SELECTED_LANGS[*]}"
echo "Modes: ${SELECTED_MODES[*]}"

TOTAL_LANGS=0
TOTAL_TARGETS=0
TOTAL_COMPARISONS=0
TOTAL_UPDATED=0
TOTAL_DRIFT=0

LANG=""
for LANG in "${SELECTED_LANGS[@]}"; do
	TOTAL_LANGS=$((TOTAL_LANGS + 1))

	LANG_BASE_DIR="$BOTS_DIR/$LANG/base"
	MANAGED_REL_PATHS=()
	EXISTING_TARGET_MODES=()

	collect_managed_paths_from_root "$GLOBAL_BASE_DIR"
	collect_managed_paths_from_root "$LANG_BASE_DIR"

	MODE=""
	for MODE in "${SELECTED_MODES[@]}"; do
		if [[ -d "$BOTS_DIR/$LANG/$MODE" ]]; then
			EXISTING_TARGET_MODES+=("$MODE")
		else
			log_verbose "Skipping missing target: bots/$LANG/$MODE"
		fi
	done

	if [[ "${#EXISTING_TARGET_MODES[@]}" -eq 0 ]]; then
		echo "[$LANG] skipped: no selected targets exist"
		continue
	fi

	LANG_COMPARISONS=0
	LANG_UPDATED=0
	LANG_DRIFT=0

	log_verbose "[$LANG] managed relative paths: ${#MANAGED_REL_PATHS[@]}"

	REL_PATH=""
	for REL_PATH in "${MANAGED_REL_PATHS[@]}"; do
		EFFECTIVE_SOURCE=""
		SOURCE_LAYER=""

		if [[ -f "$LANG_BASE_DIR/$REL_PATH" ]]; then
			EFFECTIVE_SOURCE="$LANG_BASE_DIR/$REL_PATH"
			SOURCE_LAYER="language"
		elif [[ -f "$GLOBAL_BASE_DIR/$REL_PATH" ]]; then
			EFFECTIVE_SOURCE="$GLOBAL_BASE_DIR/$REL_PATH"
			SOURCE_LAYER="global"
		else
			die_structure "Managed path has no effective source for $LANG: $REL_PATH"
		fi

		RENDERED_PATH="$TMP_DIR/rendered"
		COMPARE_PATH="$EFFECTIVE_SOURCE"
		RENDER_KIND="binary"

		if is_text_file "$EFFECTIVE_SOURCE"; then
			render_text_file "$EFFECTIVE_SOURCE" "$RENDERED_PATH" "$LANG"
			COMPARE_PATH="$RENDERED_PATH"
			RENDER_KIND="text"
		fi

		log_verbose "[$LANG] $REL_PATH -> ${SOURCE_LAYER} base (${RENDER_KIND})"

		for MODE in "${EXISTING_TARGET_MODES[@]}"; do
			TARGET_PATH="$BOTS_DIR/$LANG/$MODE/$REL_PATH"
			LANG_COMPARISONS=$((LANG_COMPARISONS + 1))
			TOTAL_COMPARISONS=$((TOTAL_COMPARISONS + 1))

			if [[ -f "$TARGET_PATH" ]] && cmp -s "$COMPARE_PATH" "$TARGET_PATH"; then
				log_verbose "  in sync: bots/$LANG/$MODE/$REL_PATH"
				continue
			fi

			if [[ "$CHECK" -eq 1 ]]; then
				echo "DRIFT bots/$LANG/$MODE/$REL_PATH"
				LANG_DRIFT=$((LANG_DRIFT + 1))
				TOTAL_DRIFT=$((TOTAL_DRIFT + 1))
				continue
			fi

			mkdir -p "$(dirname "$TARGET_PATH")"
			copy_with_source_mode "$COMPARE_PATH" "$TARGET_PATH" "$EFFECTIVE_SOURCE"
			echo "UPDATED bots/$LANG/$MODE/$REL_PATH"
			LANG_UPDATED=$((LANG_UPDATED + 1))
			TOTAL_UPDATED=$((TOTAL_UPDATED + 1))
		done
	done

	TOTAL_TARGETS=$((TOTAL_TARGETS + ${#EXISTING_TARGET_MODES[@]}))
	echo "[$LANG] targets=${#EXISTING_TARGET_MODES[@]} managed=${#MANAGED_REL_PATHS[@]} compared=$LANG_COMPARISONS updated=$LANG_UPDATED drift=$LANG_DRIFT"
done

echo "Summary: languages=$TOTAL_LANGS targets=$TOTAL_TARGETS compared=$TOTAL_COMPARISONS updated=$TOTAL_UPDATED drift=$TOTAL_DRIFT"

if [[ "$CHECK" -eq 1 && "$TOTAL_DRIFT" -gt 0 ]]; then
	echo "Remediation: run 'make -C bots sync-base' and commit the regenerated files."
	exit 1
fi

exit 0
