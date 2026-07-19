#!/usr/bin/env bash

set -euo pipefail

# Prepare self-contained source snapshots for the Docker build. Git archives are
# used so every version has exactly the files committed at that revision.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
VISUALIZER_DIR=$(cd -- "${SCRIPT_DIR}/.." && pwd)
REPOSITORY_ROOT=$(git -C "${VISUALIZER_DIR}" rev-parse --show-toplevel)
OUTPUT_DIR=${1:-"${VISUALIZER_DIR}/.version-builds"}

# This script replaces its output, so only allow the dedicated build directory.
case "${OUTPUT_DIR}" in
	"${VISUALIZER_DIR}"/.version-builds|"${VISUALIZER_DIR}"/.version-builds/*) ;;
	*)
		echo "Refusing to replace unexpected output directory: ${OUTPUT_DIR}" >&2
		exit 1
		;;
esac

rm -rf -- "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}/current" "${OUTPUT_DIR}/releases"

git -C "${REPOSITORY_ROOT}" archive HEAD visualizer \
	| tar -x -C "${OUTPUT_DIR}/current" --strip-components=1

# Tags use vMAJOR.MINOR.PATCH.REVISION. Public routes omit REVISION, so collect
# all candidates and select the highest numeric revision for each route below.
CANDIDATE_TAGS=$(mktemp)
trap 'rm -f -- "${CANDIDATE_TAGS}"' EXIT
while IFS= read -r tag; do
	if ! [[ "${tag}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
		continue
	fi

	route=${tag%.*}
	revision=${tag##*.}
	printf '%s\t%s\t%s\n' "${route}" "${revision}" "${tag}" >> "${CANDIDATE_TAGS}"
done < <(git -C "${REPOSITORY_ROOT}" tag --list 'v*')

if [[ ! -s "${CANDIDATE_TAGS}" ]]; then
	echo "No stable visualizer tags matching vN.N.N.N were found" >&2
	exit 1
fi

# Sorting by route and numeric revision makes the last tag in each route the
# release to publish. The output is consumed by build-versioned-site.sh.
LC_ALL=C sort -t $'\t' -k1,1 -k2,2n "${CANDIDATE_TAGS}" \
	| awk -F '\t' '
		NR > 1 && $1 != route { print route "\t" tag }
		{ route = $1; tag = $3 }
		END { if (NR > 0) print route "\t" tag }
	' > "${OUTPUT_DIR}/releases.tsv"

# Export each selected tag and fail early if it lacks its locked dependencies.
while IFS=$'\t' read -r route tag; do
	if ! git -C "${REPOSITORY_ROOT}" cat-file -e "${tag}:visualizer/package-lock.json"; then
		echo "Tag ${tag} does not contain a buildable visualizer" >&2
		exit 1
	fi

	mkdir -p "${OUTPUT_DIR}/releases/${route}"
	git -C "${REPOSITORY_ROOT}" archive "${tag}" visualizer \
		| tar -x -C "${OUTPUT_DIR}/releases/${route}" --strip-components=1
	echo "Prepared ${route} from ${tag}"
done < "${OUTPUT_DIR}/releases.tsv"
