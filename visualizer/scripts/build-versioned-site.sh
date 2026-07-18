#!/usr/bin/env sh

set -eu

SOURCE_ROOT=${1:?source root is required}
OUTPUT_ROOT=${2:?output root is required}
INCLUDE_DEV_ALIAS=${3:-false}
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

build_visualizer() {
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

CURRENT_OUTPUT="${OUTPUT_ROOT}.current"
rm -rf -- "${CURRENT_OUTPUT}"
trap 'rm -rf -- "${CURRENT_OUTPUT}"' EXIT
build_visualizer "${SOURCE_ROOT}/current" "${CURRENT_OUTPUT}"
cp -a "${CURRENT_OUTPUT}/." "${OUTPUT_ROOT}"

if [ "${INCLUDE_DEV_ALIAS}" = "true" ]; then
	mkdir -p "${OUTPUT_ROOT}/dev"
	cp -a "${CURRENT_OUTPUT}/." "${OUTPUT_ROOT}/dev"
	node "${SCRIPT_DIR}/rewrite-base-path.mjs" "${OUTPUT_ROOT}/dev" /dev
fi

while IFS="$(printf '\t')" read -r route tag; do
	[ -n "${route}" ] || continue
	echo "Building ${tag} for /${route}/"
	build_visualizer "${SOURCE_ROOT}/releases/${route}" "${OUTPUT_ROOT}/${route}"
	node "${SCRIPT_DIR}/rewrite-base-path.mjs" "${OUTPUT_ROOT}/${route}" "/${route}"
done < "${SOURCE_ROOT}/releases.tsv"
