import sys
import shutil
from pathlib import Path

# Auto-install dependencies if missing
try:
    from PIL import Image
except ImportError:
    print("Pillow is missing. Installing pillow...")
    import subprocess
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "pillow"])
        from PIL import Image
        print("Pillow installed successfully!")
    except Exception as e:
        print(f"Failed to install pillow automatically: {e}")
        print("Please run: pip install pillow")
        sys.exit(1)

def flip_image(img_path: Path):
    try:
        # Create a backup if it doesn't exist yet
        backup_path = img_path.with_name(img_path.name + ".bak")
        if not backup_path.exists():
            shutil.copy(img_path, backup_path)
            print(f"Backup created: {backup_path.name}")
        
        with Image.open(img_path) as img:
            flipped_img = img.transpose(Image.FLIP_LEFT_RIGHT)
            # Save using the original format
            flipped_img.save(img_path)
            print(f"Flipped: {img_path}")
    except Exception as e:
        print(f"Error flipping {img_path}: {e}")

def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python reverse_images.py [image_path1] [image_path2] ...")
        print("  python reverse_images.py [folder_path]")
        sys.exit(1)

    valid_extensions = {".png", ".jpg", ".jpeg", ".webp"}
    images_to_process = []

    for arg in sys.argv[1:]:
        path = Path(arg)
        if not path.exists():
            print(f"Warning: Path does not exist: {arg}")
            continue

        if path.is_file():
            if path.suffix.lower() in valid_extensions:
                images_to_process.append(path)
            else:
                print(f"Warning: Unsupported file format: {arg}")
        elif path.is_dir():
            # Find all images recursively
            dir_images = [
                p for p in path.rglob("*")
                if p.is_file() and p.suffix.lower() in valid_extensions
            ]
            images_to_process.extend(dir_images)

    if not images_to_process:
        print("No valid images found to process.")
        return

    print(f"Found {len(images_to_process)} images to flip.")
    for img_path in images_to_process:
        flip_image(img_path)

    print("All image flipping operations completed!")

if __name__ == "__main__":
    main()
