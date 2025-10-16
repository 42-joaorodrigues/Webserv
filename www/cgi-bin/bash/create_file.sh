#!/bin/bash

# CGI script to create a file of specified size
# Expects POST body with: size=1024 (bytes) or size=10K or size=5M

# Read the POST body
read -n $CONTENT_LENGTH POST_DATA

# Extract the size parameter
# Expected format: size=<number>[K|M] or just <number>[K|M]
SIZE_RAW=$(echo "$POST_DATA" | grep -oP '(?<=size=)[0-9]+[KMkm]?' || echo "$POST_DATA" | grep -oP '^[0-9]+[KMkm]?$')

# Parse size with K/M suffix and convert to bytes
parse_size() {
    local size_str=$1
    local size_bytes
    
    # Check if size is empty
    if [ -z "$size_str" ]; then
        echo "0"
        return
    fi
    
    # Extract number and suffix
    local number=$(echo "$size_str" | grep -oP '^\d+')
    local suffix=$(echo "$size_str" | grep -oP '[KMkm]$' || echo "")
    
    # Convert to uppercase for easier comparison
    suffix=$(echo "$suffix" | tr '[:lower:]' '[:upper:]')
    
    # Calculate bytes based on suffix
    case "$suffix" in
        K)
            size_bytes=$((number * 1024))
            ;;
        M)
            size_bytes=$((number * 1024 * 1024))
            ;;
        *)
            # No suffix, assume bytes
            size_bytes=$number
            ;;
    esac
    
    echo "$size_bytes"
}

# Convert size to bytes
SIZE=$(parse_size "$SIZE_RAW")

# Validation function
validate_size() {
    local size=$1
    
    # Check if size is empty or zero
    if [ -z "$size" ] || [ "$size" -eq 0 ]; then
        return 1
    fi
    
    # Check if size is a valid positive integer
    if ! [[ "$size" =~ ^[0-9]+$ ]]; then
        return 1
    fi
    
    # Check if size is not too large (max 100MB = 104857600 bytes)
    if [ "$size" -gt 2 * 1024 * 1024 ]; then
        return 1
    fi
    
    return 0
}

# Validate the size
if validate_size "$SIZE"; then
    # Generate a unique filename with timestamp
    FILENAME="$POST_DATA.dat"
    FILEPATH="./$FILENAME"
    
    # Create the file with the specified size
    # Use truncate for instant file creation (sparse file) or dd with better block size
    # Option 1: truncate (instant, creates sparse file)
    # truncate -s "$SIZE" "$FILEPATH" 2>/dev/null
    
    # Option 2: dd with adaptive block size for real data
    # For sizes >= 1MB, use 1MB blocks; otherwise use the size itself as block size
    if [ "$SIZE" -ge 1024 * 1024 ]; then
        # Use 1MB blocks for large files
        dd if=/dev/zero of="$FILEPATH" bs=1024 * 1024 count=$((SIZE / 1024 * 1024)) 2>/dev/null
        # Append remaining bytes if needed
        local remainder=$((SIZE % 1024 * 1024))
        if [ $remainder -gt 0 ]; then
            dd if=/dev/zero of="$FILEPATH" bs=1 count=$remainder oflag=append conv=notrunc 2>/dev/null
        fi
    else
        # For small files, write all at once
        dd if=/dev/zero of="$FILEPATH" bs="$SIZE" count=1 2>/dev/null
    fi
    
    if [ $? -eq 0 ]; then
        # Success response
        echo "Content-Type: text/html"
        echo ""
        echo "<!DOCTYPE html>"
        echo "<html>"
        echo "<head><title>File Created</title></head>"
        echo "<body>"
        echo "<h1>Success!</h1>"
        echo "<p>File created successfully:</p>"
        echo "<ul>"
        echo "<li><strong>Input:</strong> $SIZE_RAW</li>"
        echo "<li><strong>Filename:</strong> $FILENAME</li>"
        echo "<li><strong>Size:</strong> $SIZE bytes</li>"
        echo "<li><strong>Path:</strong> $FILEPATH</li>"
        echo "</ul>"
        echo "<p><a href='/upload/$FILENAME'>Download file</a></p>"
        echo "<p><a href='/'>Back to home</a></p>"
        echo "</body>"
        echo "</html>"
    else
        # File creation failed
        echo "Content-Type: text/html"
        echo "Status: 500 Internal Server Error"
        echo ""
        echo "<!DOCTYPE html>"
        echo "<html>"
        echo "<head><title>Error</title></head>"
        echo "<body>"
        echo "<h1>Error</h1>"
        echo "<p>Failed to create file.</p>"
        echo "<p><a href='/'>Back to home</a></p>"
        echo "</body>"
        echo "</html>"
    fi
else
    # Invalid size - send error response but don't create file
    echo "Content-Type: text/html"
    echo "Status: 400 Bad Request"
    echo ""
    echo "<!DOCTYPE html>"
    echo "<html>"
    echo "<head><title>Invalid Input</title></head>"
    echo "<body>"
    echo "<h1>Invalid Input</h1>"
    echo "<p>The size parameter is invalid. Please provide:</p>"
    echo "<ul>"
    echo "<li>A positive integer (with optional K or M suffix)</li>"
    echo "<li>Greater than 0 bytes</li>"
    echo "<li>Less than or equal to 100MB (104857600 bytes)</li>"
    echo "<li><strong>Examples:</strong> 1024, 10K, 5M</li>"
    echo "</ul>"
    echo "<p><strong>Received:</strong> '$POST_DATA'</p>"
    echo "<p><strong>Parsed input:</strong> '$SIZE_RAW'</p>"
    echo "<p><strong>Converted to bytes:</strong> '$SIZE'</p>"
    echo "<p><a href='/'>Back to home</a></p>"
    echo "</body>"
    echo "</html>"
fi
