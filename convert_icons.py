from pathlib import Path
from PIL import Image
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPM
import io

def svg_to_bitmap_header(svg_dir, svg_path, output_path):
    # Convert SVG to PNG using pure Python implementation
    drawing = svg2rlg(str(svg_path))

    # Use memory buffer instead of temporary file
    png_data = io.BytesIO()
    renderPM.drawToFile(drawing, png_data, fmt="PNG", dpi=72)
    png_data.seek(0)

    # Open PNG and convert to 1-bit bitmap
    img = Image.open(png_data).convert('1')
    width, height = img.size

    # Convert to bitmap array
    bitmap_data = []
    for y in range(height):
        byte = 0
        bit_count = 0
        for x in range(width):
            pixel = img.getpixel((x, y))
            byte = (byte << 1) | (1 if pixel == 0 else 0)
            bit_count += 1
            if bit_count == 8:
                bitmap_data.append(byte)
                byte = 0
                bit_count = 0
        if bit_count > 0:
            byte = byte << (8 - bit_count)
            bitmap_data.append(byte)

    # Create directory if it doesn't exist
    output_path.parent.mkdir(parents=True, exist_ok=True)

    path_parts = svg_path.relative_to(svg_dir).with_suffix('').parts
    sanitized_name = '_'.join(part.replace('-', '_') for part in path_parts)

    # Write header file
    with open(output_path, 'w') as f:
        f.write(f"""#pragma once
#include <EPDIcon.h>
// Auto-generated from {svg_path.name}
inline EPD::Icon &{sanitized_name}_icon() {{
    static const unsigned char PROGMEM bitmap[] = {{
        """)
        f.write(",\n        ".join([", ".join(f"0x{b:02X}" for b in bitmap_data[i:i+12])
                                for i in range(0, len(bitmap_data), 12)]))
        f.write(f"""
    }};
    static const auto icon = new EPD::Icon({{{width}, {height}}}, bitmap);
    return *icon;
}}

//#define {sanitized_name}_icon {sanitized_name}_icon()
""")

    print(f"Converted: {svg_path} -> {output_path}")


def generate_index_header(output_dir: Path, icon_files: list):
    index_path = output_dir / "icons.h"
    with open(index_path, 'w') as f:
        f.write("#pragma once\n\n")
        f.write("// Auto-generated index of all icons\n\n")
        
        # Include all icon headers
        for icon_file in icon_files:
            relative_path = icon_file.relative_to(output_dir)
            f.write(f'#include "{relative_path}"\n')
        
        print(f"Generated index header: {index_path}")

def process_svg_directory(svg_dir: Path | str | None = None, output_dir: Path | str | None = None):
    # Fallbacks to original defaults if not provided
    svg_dir = Path(svg_dir) if svg_dir is not None else Path("icons")
    output_dir = Path(output_dir) if output_dir is not None else Path("include/icons")

    if not svg_dir.exists():
        print(f"Error: Source directory {svg_dir} does not exist!")
        return

    # Create output directory if it doesn't exist
    output_dir.mkdir(parents=True, exist_ok=True)

    generated_files = []

    # Process all SVG files recursively
    for svg_path in svg_dir.rglob("*.svg"):
        # Maintain the same directory structure in output
        relative_path = svg_path.relative_to(svg_dir)
        output_path = output_dir / relative_path.with_suffix('.h')

        try:
            svg_to_bitmap_header(svg_dir, svg_path, output_path)
            generated_files.append(output_path)
        except Exception as e:
            print(f"Error processing {svg_path}: {e}")

    # Generate index header
    if generated_files:
        generate_index_header(output_dir, generated_files)

    print(f"\nProcessed {len(generated_files)} icons")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Convert SVG icons into C++ header bitmaps and generate an index header.")
    parser.add_argument(
        "--svg-dir",
        dest="svg_dir",
        type=str,
        default=None,
        help="Path to the source directory containing SVG files (default: ./icons)",
    )
    parser.add_argument(
        "--output-dir",
        dest="output_dir",
        type=str,
        default=None,
        help="Directory where generated headers will be written (default: ./include/icons)",
    )

    args = parser.parse_args()

    print("=" * 50)
    print("convert_icons.py is being executed!")
    process_svg_directory(args.svg_dir, args.output_dir)
    print("convert_icons.py is DONE!")
    print("=" * 50)