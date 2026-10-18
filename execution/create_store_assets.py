"""Generate Pebble App Store icons for TERCOM (icon-small.png and icon-large.png)."""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
STORE = ROOT / "store"


def draw_icon(size: int) -> Image.Image:
    img = Image.new("RGBA", (size, size), (0, 0, 0, 255))
    draw = ImageDraw.Draw(img)

    cx = size // 2
    cy = size // 2

    # Draw wireframe mountain lines (Phosphor green: #00FF55)
    green = (0, 255, 85, 255)
    dark_green = (0, 170, 0, 255)
    yellow = (255, 170, 0, 255)

    # Mountain ridges
    base_y = int(size * 0.75)
    peak1 = (int(size * 0.25), int(size * 0.45))
    valley = (int(size * 0.5), int(size * 0.65))
    peak2 = (int(size * 0.78), int(size * 0.40))

    ridge_pts = [
        (0, base_y),
        (int(size * 0.15), int(size * 0.60)),
        peak1,
        (int(size * 0.38), int(size * 0.58)),
        valley,
        (int(size * 0.65), int(size * 0.52)),
        peak2,
        (size - 1, base_y),
    ]

    width = max(1, size // 36)
    draw.line(ridge_pts, fill=green, width=width)

    # Secondary lower ridge
    ridge2_pts = [
        (0, int(size * 0.88)),
        (int(size * 0.3), int(size * 0.72)),
        (int(size * 0.55), int(size * 0.78)),
        (int(size * 0.8), int(size * 0.70)),
        (size - 1, int(size * 0.88)),
    ]
    draw.line(ridge2_pts, fill=dark_green, width=width)

    # Perspective grid lines running down
    draw.line([(cx, int(size * 0.65)), (int(size * 0.35), size - 1)], fill=dark_green, width=width)
    draw.line([(cx, int(size * 0.65)), (int(size * 0.65), size - 1)], fill=dark_green, width=width)

    # Flight path reticle (Yellow: #FFAA00)
    reticle_r = int(size * 0.26)
    reticle_box = [cx - reticle_r, cy - int(size * 0.12) - reticle_r, cx + reticle_r, cy - int(size * 0.12) + reticle_r]
    draw.ellipse(reticle_box, outline=yellow, width=max(1, size // 40))

    # Crosshair ticks
    rcy = cy - int(size * 0.12)
    draw.line([(cx - reticle_r - int(size * 0.08), rcy), (cx - reticle_r, rcy)], fill=yellow, width=width)
    draw.line([(cx + reticle_r, rcy), (cx + reticle_r + int(size * 0.08), rcy)], fill=yellow, width=width)
    draw.line([(cx, rcy - reticle_r - int(size * 0.08)), (cx, rcy - reticle_r)], fill=yellow, width=width)
    draw.line([(cx, rcy + reticle_r), (cx, rcy + reticle_r + int(size * 0.08))], fill=yellow, width=width)

    return img


def main():
    STORE.mkdir(parents=True, exist_ok=True)
    icon_small = draw_icon(28)
    icon_small.save(STORE / "icon-small.png")
    print(f"Saved {STORE / 'icon-small.png'}")

    icon_large = draw_icon(144)
    icon_large.save(STORE / "icon-large.png")
    print(f"Saved {STORE / 'icon-large.png'}")


if __name__ == "__main__":
    main()
