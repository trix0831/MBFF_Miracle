import sys
import os
import re
from PIL import Image

def extract_key(filename):
    if filename == "init.png":
        return (0, 0)
    m = re.match(r"merged_(\d+)\.png$", filename)
    if m:
        num = int(m.group(1))
        if num == 0:
            return (2, 0)  # last
        else:
            return (1, num)  # in order
    return (3, filename)  # fallback for unknowns

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 make_gif.py <directory_path>")
        sys.exit(1)

    dir_path = sys.argv[1].rstrip("/")

    if not os.path.isdir(dir_path):
        print(f"Error: '{dir_path}' is not a valid directory.")
        sys.exit(1)

    png_files = [f for f in os.listdir(dir_path) if f.lower().endswith(".png")]
    if not png_files:
        print("No PNG files found in the directory.")
        sys.exit(1)

    sorted_files = sorted(png_files, key=extract_key)
    image_paths = [os.path.join(dir_path, f) for f in sorted_files]
    images = [Image.open(p) for p in image_paths]

    # Extract case name from the final directory part
    case_name = os.path.basename(dir_path)
    gif_path = os.path.join(dir_path, f"{case_name}.gif")

    images[0].save(
        gif_path,
        save_all=True,
        append_images=images[1:],
        duration=120,
        loop=0
    )

    print(f"GIF saved to: {gif_path}")

if __name__ == "__main__":
    main()
