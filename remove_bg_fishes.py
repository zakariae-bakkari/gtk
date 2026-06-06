import os
os.environ["NUMBA_NUM_THREADS"] = "1"
import sys
import shutil
from pathlib import Path

# Auto-install dependencies if missing
try:
    from rembg import remove
    from PIL import Image
    from tqdm import tqdm
except ImportError:
    print("Dependencies missing. Installing rembg[cpu], onnxruntime, pillow, and tqdm...")
    import subprocess
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "rembg[cpu]", "onnxruntime", "pillow", "tqdm"])
        from rembg import remove
        from PIL import Image
        from tqdm import tqdm
        print("Dependencies installed successfully!")
    except Exception as e:
        print(f"Failed to install dependencies automatically: {e}")
        print("Please run: pip install rembg pillow tqdm")
        sys.exit(1)

def main():
    base_dir = Path("resources/images/fishes")
    backup_dir = Path("resources/images/fishes_backup")
    
    if not base_dir.exists():
        print(f"Error: Directory {base_dir} does not exist.")
        return

    # 1. Create backup of original images
    if not backup_dir.exists():
        print(f"Creating backup of original images to: {backup_dir}")
        shutil.copytree(base_dir, backup_dir)
        print("Backup created successfully.")
    else:
        print(f"Backup directory already exists at: {backup_dir}. Skipping backup creation to avoid overwriting existing backup.")

    # 2. Gather files to process
    valid_extensions = {".png", ".jpg", ".jpeg", ".webp"}
    image_paths = [
        p for p in base_dir.rglob("*")
        if p.is_file() and p.suffix.lower() in valid_extensions
    ]
    
    if not image_paths:
        print("No images found to process.")
        return

    print(f"Found {len(image_paths)} images to process.")
    print("Starting background removal. (Note: The first image will download the ~176MB U2-Net model)...")

    # 3. Process images
    for img_path in tqdm(image_paths, desc="Removing backgrounds"):
        try:
            # Open image
            with Image.open(img_path) as img:
                # Convert to RGBA
                img_rgba = img.convert("RGBA")
                # Remove background
                output = remove(img_rgba)
                # Save back to original file (always PNG format to preserve alpha transparency)
                output.save(img_path, format="PNG")
        except Exception as e:
            print(f"\nError processing {img_path}: {e}")

    print("Background removal completed successfully!")

if __name__ == "__main__":
    main()
