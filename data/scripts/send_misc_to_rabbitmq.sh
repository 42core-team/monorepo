#!/bin/bash
# Script to extract misc section from replay file and send to RabbitMQ with pattern wrapper

set -euo pipefail

# Configuration
REPLAY_FILE="${1:-/workspaces/sandbox/debug-visualizer/src/public/misc/replay_latest.json}"
RABBITMQ_URL="${RABBITMQ_URL:-}"
MAX_RETRIES=20
RETRY_DELAY=2
EXCHANGE="${RABBITMQ_EXCHANGE:-game_results}"

# Check if RabbitMQ URL is set
if [[ -z "$RABBITMQ_URL" ]]; then
    echo "Error: RABBITMQ_URL environment variable is not set"
    echo "Example: export RABBITMQ_URL='http://guest:guest@localhost:15672/api/exchanges/%2f/amq.direct/publish'"
    exit 1
fi

# Check if replay file exists
if [[ ! -f "$REPLAY_FILE" ]]; then
    echo "Error: Replay file not found: $REPLAY_FILE"
    exit 1
fi

# Extract misc section from JSON
echo "Extracting misc section from replay file..."
MISC_DATA=$(jq '.misc' "$REPLAY_FILE")

if [[ "$MISC_DATA" == "null" || -z "$MISC_DATA" ]]; then
    echo "Error: No misc section found in replay file"
    exit 1
fi

# Wrap misc data in the required pattern structure
WRAPPED_DATA=$(jq -n \
  --argjson misc_data "$MISC_DATA" \
  '{
    "pattern": "game_server",
    "data": $misc_data
  }')

# Prepare payload for RabbitMQ
PAYLOAD=$(jq -n \
  --argjson properties '{}' \
  --argjson payload "$WRAPPED_DATA" \
  '{
    "properties": $properties,
    "routing_key": "game_results",
    "payload": ($payload | tostring),
    "payload_encoding": "string"
  }')

# Function to send data to RabbitMQ
send_to_rabbitmq() {
    RESPONSE=$(curl -s -X POST \
        -H "Content-Type: application/json" \
        -d "$PAYLOAD" \
        "$RABBITMQ_URL" \
        -w "\n%{http_code}")
    
    # Extract HTTP code (last line) and response body
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    RESPONSE_BODY=$(echo "$RESPONSE" | head -n -1)
    
    echo "$HTTP_CODE:$RESPONSE_BODY"
}

# Retry logic
echo "Sending wrapped misc data to RabbitMQ..."
for attempt in $(seq 1 $MAX_RETRIES); do
    echo "Attempt $attempt/$MAX_RETRIES..."
    
    RESULT=$(send_to_rabbitmq)
    HTTP_CODE=$(echo "$RESULT" | cut -d: -f1)
    RESPONSE_BODY=$(echo "$RESULT" | cut -d: -f2-)
    
    if [[ "$HTTP_CODE" == "200" ]]; then
        echo "✓ Successfully sent wrapped misc data to RabbitMQ"
        echo "Response: $RESPONSE_BODY"
        
        # Check if message was routed
        if echo "$RESPONSE_BODY" | jq -e '.routed == true' >/dev/null 2>&1; then
            echo "✓ Message was successfully routed to a queue"
        elif echo "$RESPONSE_BODY" | jq -e '.routed == false' >/dev/null 2>&1; then
            echo "⚠ Warning: Message was accepted but not routed to any queue"
            echo "  This might mean no queue is bound to the exchange with routing key 'game_results'"
        fi
        
        echo "Data sent:"
        echo "$WRAPPED_DATA" | jq .
        exit 0
    else
        echo "✗ Failed with HTTP code: $HTTP_CODE"
        echo "Response: $RESPONSE_BODY"
        
        if [[ $attempt -lt $MAX_RETRIES ]]; then
            echo "Retrying in $RETRY_DELAY seconds..."
            sleep $RETRY_DELAY
            RETRY_DELAY=$((RETRY_DELAY * 2))  # Exponential backoff
        fi
    fi
done

echo "Error: Failed to send data to RabbitMQ after $MAX_RETRIES attempts"
exit 1
