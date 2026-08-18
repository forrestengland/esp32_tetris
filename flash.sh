#!/bin/bash

# --- Configuration ---
SKETCH_DIR="./" # Folder name must match the .ino file name
FQBN="esp32:esp32:esp32"            # Fully Qualified Board Name (adjust if using a different ESP32 model)
PORT="/dev/ttyUSB0"                 # Replace with your ESP32's port (e.g., /dev/ttyUSB0 on Linux, COM4 on Windows)

# --- Commands ---

echo "Compiling sketch: $SKETCH_DIR"
# The compile command generates the necessary build files
../arduino_cli compile --fqbn $FQBN $SKETCH_DIR

if [ $? -eq 0 ]; then
    echo "Compilation successful. Uploading to port $PORT"
    # The upload command flashes the compiled code to the board
    ../arduino_cli upload -p $PORT --fqbn $FQBN $SKETCH_DIR
    
    if [ $? -eq 0 ]; then
        echo "Upload successful."
    else
        echo "Upload failed. Check your port and ensure the board is in flashing mode (you might need to press the BOOT button)."
    fi
else
    echo "Compilation failed."
fi
