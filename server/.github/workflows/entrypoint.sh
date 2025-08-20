#!/bin/bash
# Docker entrypoint script for game server
# Conditionally uploads replay and sends results to RabbitMQ based on environment variables

set -euo pipefail

# Configuration
UPLOAD_REPLAY="${UPLOAD_REPLAY:-false}"
SEND_RESULTS="${SEND_RESULTS:-false}"
REPLAY_FILE="${REPLAY_FILE:-/core/replays/replay_latest.json}"

# Script paths
UPLOAD_SCRIPT="/core/data/scripts/upload_replay_to_s3.sh"
RABBITMQ_SCRIPT="/core/data/scripts/send_misc_to_rabbitmq.sh"

echo "=== Game Server Entrypoint ==="
echo "UPLOAD_REPLAY: $UPLOAD_REPLAY"
echo "SEND_RESULTS: $SEND_RESULTS"
echo "REPLAY_FILE: $REPLAY_FILE"

# Function to run the main game server
run_game_server() {
    echo "Starting game server..."
    # Run the game server in the background and wait for it
    "$@" &
    local game_pid=$!

    # Wait for the game server to finish
    wait $game_pid
    local exit_code=$?

    echo "Game server finished with exit code: $exit_code"
    return $exit_code
}

# Function to handle post-game processing
post_game_processing() {
    echo "=== Post-game processing ==="

    # Check if replay file exists
    if [[ ! -f "$REPLAY_FILE" ]]; then
        echo "Warning: Replay file not found: $REPLAY_FILE"
        echo "Skipping post-game processing"
        return 0
    fi

    # Upload replay to S3 if enabled
    if [[ "$UPLOAD_REPLAY" == "true" ]]; then
        echo "Uploading replay to S3..."
        if [[ -x "$UPLOAD_SCRIPT" ]]; then
            if "$UPLOAD_SCRIPT" "$REPLAY_FILE"; then
                echo "✓ Replay upload completed successfully"
            else
                echo "✗ Replay upload failed"
                # Continue processing even if upload fails
            fi
        else
            echo "Error: Upload script not found or not executable: $UPLOAD_SCRIPT"
        fi
    else
        echo "Skipping replay upload (UPLOAD_REPLAY not set to true)"
    fi

    # Send results to RabbitMQ if enabled
    if [[ "$SEND_RESULTS" == "true" ]]; then
        echo "Sending results to RabbitMQ..."
        if [[ -x "$RABBITMQ_SCRIPT" ]]; then
            if "$RABBITMQ_SCRIPT" "$REPLAY_FILE"; then
                echo "✓ Results sent to RabbitMQ successfully"
            else
                echo "✗ Failed to send results to RabbitMQ"
                # Continue processing even if RabbitMQ fails
            fi
        else
            echo "Error: RabbitMQ script not found or not executable: $RABBITMQ_SCRIPT"
        fi
    else
        echo "Skipping RabbitMQ results (SEND_RESULTS not set to true)"
    fi

    echo "=== Post-game processing completed ==="
}

# Validate that required scripts exist if features are enabled
if [[ "$UPLOAD_REPLAY" == "true" && ! -f "$UPLOAD_SCRIPT" ]]; then
    echo "Error: UPLOAD_REPLAY is enabled but script not found: $UPLOAD_SCRIPT"
    exit 1
fi

if [[ "$SEND_RESULTS" == "true" && ! -f "$RABBITMQ_SCRIPT" ]]; then
    echo "Error: SEND_RESULTS is enabled but script not found: $RABBITMQ_SCRIPT"
    exit 1
fi

# Make scripts executable if they exist
[[ -f "$UPLOAD_SCRIPT" ]] && chmod +x "$UPLOAD_SCRIPT"
[[ -f "$RABBITMQ_SCRIPT" ]] && chmod +x "$RABBITMQ_SCRIPT"

# Run the main game server with all provided arguments
echo "Running game server with arguments: $*"
if run_game_server "$@"; then
    game_exit_code=0
else
    game_exit_code=$?
fi

echo "Game server exited with code: $game_exit_code"

# Only run post-game processing if either feature is enabled
if [[ "$UPLOAD_REPLAY" == "true" || "$SEND_RESULTS" == "true" ]]; then
    post_game_processing
else
    echo "No post-game processing enabled"
fi

echo "Entrypoint script completed"
exit $game_exit_code
