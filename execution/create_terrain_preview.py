"""Assemble the five native TERCOM terrain captures into a listing GIF."""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageChops


ROOT = Path(__file__).resolve().parents[1]
STORE = ROOT / "store"
OUTPUT = STORE / "listing" / "emery_00_preview.gif"
FRAMES = [
    STORE / "emery_tercom.png",
    STORE / "emery_canyon.png",
    STORE / "emery_dunes.png",
    STORE / "emery_caldera.png",
    STORE / "emery_alpine.png",
]


def main() -> None:
    images = []
    for path in FRAMES:
        with Image.open(path) as source:
            if source.size != (200, 228):
                raise ValueError(f"{path.name}: expected 200×228, got {source.size}")
            images.append(source.convert("RGB"))
    for index, image in enumerate(images[1:], start=1):
        if ImageChops.difference(images[0], image).getbbox() is None:
            raise ValueError(f"{FRAMES[index].name}: duplicate of first frame")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    images[0].save(
        OUTPUT,
        save_all=True,
        append_images=images[1:],
        duration=[900, 800, 750, 800, 850],
        loop=0,
        optimize=True,
        disposal=2,
    )
    with Image.open(OUTPUT) as result:
        if result.size != (200, 228) or result.n_frames != 5:
            raise ValueError("Output GIF is not five 200×228 frames")
    if OUTPUT.stat().st_size > 4_400_000:
        raise ValueError("Output GIF exceeds RePebble's 4.4 MB limit")
    print(f"PASS {OUTPUT} ({OUTPUT.stat().st_size} bytes, 5 frames)")


if __name__ == "__main__":
    main()
