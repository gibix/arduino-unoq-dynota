"""
Arduino Uno Q Dynamic Sketch Loader - Upload CLI

This CLI uploads ELF binaries to Arduino Uno Q for dynamic loading via LLEXT.
"""

import argparse
import sys
from bridge import Bridge


def upload_elf(file_path: str):
    """Upload an ELF binary to the Zephyr device

    Args:
        file_path: Path to the ELF file to upload
    """
    try:
        with open(file_path, "rb") as f:
            file_data = f.read()

        file_size_kb = len(file_data) / 1024
        file_name = file_path.split("/")[-1].split(".")[0]

        print(f"\nUploading: {file_path}")
        print(f"Size: {file_size_kb:.2f} KB")

        # Convert file to hex and send in chunks to avoid Bridge message size limits
        hex_data = file_data.hex()
        chunk_size = 200
        total_chunks = (len(hex_data) + chunk_size - 1) // chunk_size

        print(f"Starting upload ({total_chunks} chunks)...")
        Bridge.call("start_llext_xfer", file_name)

        # Stream ELF binary to device in manageable chunks
        for i in range(0, len(hex_data), chunk_size):
            chunk = hex_data[i : i + chunk_size]
            Bridge.call("load_elf_chunk", chunk, timeout=20)

            chunk_num = i // chunk_size + 1
            progress = chunk_num / total_chunks
            print(f"Progress: {progress * 100:.0f}% ({chunk_num}/{total_chunks} chunks)", end="\r")

        # Finalize upload and trigger dynamic loading on device
        result = Bridge.call("stop_llext_xfer")
        print()  # New line after progress

        if result == "OK":
            print(f"Successfully uploaded {file_path}!")
            return True
        else:
            print(f"Upload failed: {result}", file=sys.stderr)
            return False

    except FileNotFoundError:
        print(f"Error: File not found: {file_path}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Error uploading file: {e}", file=sys.stderr)
        return False


def main():
    """Main CLI entry point"""
    parser = argparse.ArgumentParser(
        description="Arduino Uno Q Dynamic Sketch Loader - Upload CLI",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example:
  python main.py sketch.elf
        """
    )

    parser.add_argument(
        "file",
        help="Path to the ELF file to upload"
    )

    args = parser.parse_args()

    # Upload file and exit
    success = upload_elf(args.file)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
