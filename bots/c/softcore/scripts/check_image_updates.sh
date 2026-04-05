#!/bin/bash

# Configuration
URL="[[event_url]]/version"
MAX_TIME=0.5
RED='\033[0;31m'
NC='\033[0m' # No Color

# Determine paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
COMPOSE_FILE="$ROOT_DIR/.devcontainer/docker-compose.yml"

# Check if compose file exists
if [ ! -f "$COMPOSE_FILE" ]; then
    # If not found, just skip without error
    exit 0
fi

# Fetch versions from API with timeout
# Using -s for silent, --max-time for timeout
RESPONSE=$(curl -s --max-time "$MAX_TIME" "$URL" 2>/dev/null)

# Check if response is empty (timeout or error)
if [ -z "$RESPONSE" ]; then
    exit 0
fi

get_json_value() {
    local key=$1
    local json=$2

    echo "$json" | yq eval ".$key" -
}

NEW_BOT_VER=$(get_json_value "myCoreBotVersion" "$RESPONSE")
NEW_VIS_VER=$(get_json_value "visualizerVersion" "$RESPONSE")

# Validate versions to prevent injection
validate_version() {
    local version=$1
    if [[ ! "$version" =~ ^[a-zA-Z0-9._:/-]+$ ]]; then
        return 1
    fi
    return 0
}

if [ -n "$NEW_BOT_VER" ] && [ "$NEW_BOT_VER" != "null" ]; then
    if ! validate_version "$NEW_BOT_VER"; then
        echo "Error: Invalid characters detected in new bot version: '$NEW_BOT_VER'. Update aborted." >&2
        exit 1
    fi
fi

if [ -n "$NEW_VIS_VER" ] && [ "$NEW_VIS_VER" != "null" ]; then
    if ! validate_version "$NEW_VIS_VER"; then
        echo "Error: Invalid characters detected in new visualizer version: '$NEW_VIS_VER'. Update aborted." >&2
        exit 1
    fi
fi

get_current_image() {
    local service_name=$1
    yq eval ".services[\"$service_name\"].image" "$COMPOSE_FILE"
}

CURRENT_BOT_VER=$(get_current_image "my-core-bot")
CURRENT_VIS_VER=$(get_current_image "visualizer")

UPDATES_FOUND=false
BOT_NEEDS_UPDATE=false
VIS_NEEDS_UPDATE=false

# Helper to check if value is valid (non-empty and not "null")
is_valid() {
    [ -n "$1" ] && [ "$1" != "null" ]
}

if is_valid "$NEW_BOT_VER" && is_valid "$CURRENT_BOT_VER" && [ "$CURRENT_BOT_VER" != "$NEW_BOT_VER" ]; then
    UPDATES_FOUND=true
    BOT_NEEDS_UPDATE=true
fi

if is_valid "$NEW_VIS_VER" && is_valid "$CURRENT_VIS_VER" && [ "$CURRENT_VIS_VER" != "$NEW_VIS_VER" ]; then
    UPDATES_FOUND=true
    VIS_NEEDS_UPDATE=true
fi

if [ "$UPDATES_FOUND" = false ]; then
    exit 0
fi

# Prompt user
echo ""
echo "New Docker image versions available!"
if [ "$BOT_NEEDS_UPDATE" = true ]; then
    echo "  - My Core Bot: $CURRENT_BOT_VER -> $NEW_BOT_VER"
fi
if [ "$VIS_NEEDS_UPDATE" = true ]; then
    echo "  - Visualizer:  $CURRENT_VIS_VER -> $NEW_VIS_VER"
fi
echo ""
echo -e "${RED}⚠️  WARNING: You are running an outdated version. To ensure your bot remains functional on the website and in tournaments, please update to the latest version immediately.${NC}"
printf "Do you want to update? (yes/no): "
read -r USER_INPUT

if [ "$USER_INPUT" = "yes" ]; then
    # Perform updates

    if [ "$BOT_NEEDS_UPDATE" = true ]; then
        echo "Updating 'my-core-bot' image to: $NEW_BOT_VER"
        yq eval ".services[\"my-core-bot\"].image = \"$NEW_BOT_VER\"" -i "$COMPOSE_FILE"
    fi

    if [ "$VIS_NEEDS_UPDATE" = true ]; then
        echo "Updating 'visualizer' image to: $NEW_VIS_VER"
        yq eval ".services[\"visualizer\"].image = \"$NEW_VIS_VER\"" -i "$COMPOSE_FILE"
    fi

    echo ""
    echo "Updated docker-compose.yml"
    echo -e "${RED}Please close the current devcontainer. Then, from your local terminal in the bot folder, run this command to update and restart:${NC}"
    echo "make update && make devcontainer"
    echo ""
    echo "Note: Please also commit the changes of the docker-compose.yml file!"
    echo ""

    exit 1
else
    exit 0
fi
