#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path


def convert_mp3(input_file, output_file):
    command = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel", "error",
        "-y",
        "-i", str(input_file),
        "-f", "s16le",       # Signed 16-bit little-endian PCM
        "-acodec", "pcm_s16le",
        "-ar", "44100",      # 44.1 kHz sample rate
        "-ac", "2",          # Stereo
        str(output_file)
    ]

    try:
        subprocess.run(command, check=True)
        print(f"Converted: {input_file.name} -> {output_file.name}")
    except subprocess.CalledProcessError:
        print(f"Error converting: {input_file.name}")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} DIRECTORY")
        sys.exit(1)

    directory = Path(sys.argv[1])

    if not directory.is_dir():
        print(f"Error: {directory} is not a directory")
        sys.exit(1)

    mp3_files = sorted(
        file for file in directory.iterdir()
        if file.is_file() and file.suffix.lower() == ".mp3"
    )

    if not mp3_files:
        print("No MP3 files found.")
        return

    for input_file in mp3_files:
        output_file = input_file.with_suffix(".raw")
        convert_mp3(input_file, output_file)


if __name__ == "__main__":
    main()
    
