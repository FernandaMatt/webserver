#!/bin/bash

URL="http://localhost:8042/"
DURATION="60S"
CONCURRENT_USERS=10
MEMORY_THRESHOLD_PERCENT=5  # Adjust as needed (represents 5% of total memory)
CHECK_INTERVAL=10  # in seconds
PROCESS_NAME="./webserv"  # Replace with the actual server process name

# Function to get total system memory in KB
get_total_memory() {
    TOTAL_MEM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    echo $TOTAL_MEM
}

# Function to monitor memory usage
monitor_memory() {
    TOTAL_MEM=$(get_total_memory)
    end_time=$((SECONDS + $(echo $DURATION | sed 's/S//')))
    while [ $SECONDS -lt $end_time ]; do
        # Get memory usage in KB for the process
        MEM_USAGE=$(ps aux | grep "$PROCESS_NAME" | grep -v grep | awk '{sum += $6} END {print sum}')
        MEM_USAGE_MB=$(echo "scale=2; $MEM_USAGE / 1024" | bc)  # Convert to MB with two decimal places

        if [[ -z "$MEM_USAGE" ]]; then
            echo "Process not found: $PROCESS_NAME"
        else
            # Calculate memory usage percentage
            MEM_PERCENT=$(echo "scale=2; $MEM_USAGE * 10 / ($TOTAL_MEM / 1024)" | bc)
            if (( $(echo "$MEM_PERCENT > $MEMORY_THRESHOLD_PERCENT" | bc -l) )); then
                echo "Memory usage is too high: ${MEM_PERCENT}% ($MEM_USAGE_MB MB out of $(echo "$TOTAL_MEM / 1024" | bc) MB)"
            else
                echo "Memory usage is normal: ${MEM_PERCENT}% ($MEM_USAGE_MB MB out of $(echo "$TOTAL_MEM / 1024" | bc) MB)"
            fi
        fi
        sleep $CHECK_INTERVAL
    done
}

# Run Siege in the background
siege -b -c $CONCURRENT_USERS -t $DURATION $URL &

# Start monitoring memory usage
monitor_memory
