#!/bin/bash
# Script to upload replay file to S3 using pre-signed URL, naming the file with game_id

set -euo pipefail

# Configuration
REPLAY_FILE="${1:-/workspaces/sandbox/debug-visualizer/src/public/misc/replay_latest.json}"
PRESIGNED_URL="${S3_PRESIGNED_URL:-}"
MAX_RETRIES=10
RETRY_DELAY=2

# Check if pre-signed URL is set
if [[ -z "$PRESIGNED_URL" ]]; then
    echo "Error: S3_PRESIGNED_URL environment variable is not set"
    echo "Example: export S3_PRESIGNED_URL='https://your-bucket.s3.amazonaws.com/path?X-Amz-Algorithm=...'"
    exit 1
fi

# Check if replay file exists
if [[ ! -f "$REPLAY_FILE" ]]; then
    echo "Error: Replay file not found: $REPLAY_FILE"
    exit 1
fi

# Extract game_id from the replay file
echo "Extracting game_id from replay file..."
GAME_ID=$(jq -r '.misc.game_id // .game_id // empty' "$REPLAY_FILE")

if [[ -z "$GAME_ID" || "$GAME_ID" == "null" ]]; then
    echo "Error: No game_id found in replay file"
    echo "Checked paths: .misc.game_id and .game_id"
    exit 1
fi

echo "Found game_id: $GAME_ID"

# Generate filename with game_id
FILENAME="${GAME_ID}.json"
echo "Will upload as: $FILENAME"

# Modify the pre-signed URL to include the filename
# This assumes the pre-signed URL was generated for a specific key pattern
# You might need to adjust this based on how your pre-signed URLs are structured
if [[ "$PRESIGNED_URL" == *"?" ]]; then
    # URL already has query parameters
    UPLOAD_URL="$PRESIGNED_URL"
else
    # URL doesn't have query parameters (shouldn't happen with pre-signed URLs)
    UPLOAD_URL="$PRESIGNED_URL"
fi

# Function to upload file to S3
upload_to_s3() {
    local temp_file="/tmp/${FILENAME}"
    
    # Copy the replay file with the new name
    cp "$REPLAY_FILE" "$temp_file"
    
    # Upload using curl with PUT method (typical for pre-signed URLs)
    curl -s -X PUT \
        -H "Content-Type: application/json" \
        --data-binary "@$temp_file" \
        "$UPLOAD_URL" \
        -w "%{http_code}" \
        -o /dev/null
    
    # Clean up temp file
    rm -f "$temp_file"
}

# Retry logic
echo "Uploading replay file to S3..."
echo "File size: $(du -h "$REPLAY_FILE" | cut -f1)"

for attempt in $(seq 1 $MAX_RETRIES); do
    echo "Attempt $attempt/$MAX_RETRIES..."
    
    HTTP_CODE=$(upload_to_s3)
    
    if [[ "$HTTP_CODE" == "200" || "$HTTP_CODE" == "204" ]]; then
        echo "✓ Successfully uploaded replay file to S3"
        echo "Filename: $FILENAME"
        echo "Game ID: $GAME_ID"
        exit 0
    else
        echo "✗ Failed with HTTP code: $HTTP_CODE"
        
        if [[ $attempt -lt $MAX_RETRIES ]]; then
            echo "Retrying in $RETRY_DELAY seconds..."
            sleep $RETRY_DELAY
            RETRY_DELAY=$((RETRY_DELAY * 2))  # Exponential backoff
        fi
    fi
done

echo "Error: Failed to upload file to S3 after $MAX_RETRIES attempts"
exit 1
