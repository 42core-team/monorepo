#!/usr/bin/env sh

set -eu

# Build the prepared visualizer sources into one site:
#   /          current checkout
#   /dev/      optional alias of the current checkout
#   /vN.N.N/   newest tagged revision for that release
SOURCE_ROOT=${1:?source root is required}
OUTPUT_ROOT=${2:?output root is required}
INCLUDE_DEV_ALIAS=${3:-false}
SCRIPT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)

build() {
	source_dir=$1
	output_dir=$2

	(
		cd "${source_dir}"
		HUSKY=0 npm ci
		npm run build -- --outDir "${output_dir}"
	)
}

rm -rf -- "${OUTPUT_ROOT}"
mkdir -p "${OUTPUT_ROOT}"

# Build the current version outside the site so it can be copied to both /
# and /dev/ without accidentally copying /dev/ into itself.
CURRENT_BUILD="${OUTPUT_ROOT}.current"
rm -rf -- "${CURRENT_BUILD}"
trap 'rm -rf -- "${CURRENT_BUILD}"' EXIT
build "${SOURCE_ROOT}/current" "${CURRENT_BUILD}"
cp -a "${CURRENT_BUILD}/." "${OUTPUT_ROOT}"

if [ "${INCLUDE_DEV_ALIAS}" = "true" ]; then
	mkdir -p "${OUTPUT_ROOT}/dev"
	cp -a "${CURRENT_BUILD}/." "${OUTPUT_ROOT}/dev"
	node "${SCRIPT_DIR}/rewrite-base-path.mjs" "${OUTPUT_ROOT}/dev" /dev
fi

# releases.tsv maps each public route to the tag selected by the prepare step.
while IFS="$(printf '\t')" read -r route tag; do
	[ -n "${route}" ] || continue
	echo "Building ${tag} for /${route}/"
	build "${SOURCE_ROOT}/releases/${route}" "${OUTPUT_ROOT}/${route}"
	node "${SCRIPT_DIR}/rewrite-base-path.mjs" "${OUTPUT_ROOT}/${route}" "/${route}"
done < "${SOURCE_ROOT}/releases.tsv"
