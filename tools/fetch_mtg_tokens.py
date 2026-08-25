#!/usr/bin/env python3
"""Fetch the most common MTG token cards from Scryfall and convert them to
1-bit BMPs for the CrossPoint /tokens viewer.

"Most common" = token names ranked by number of distinct printings on
Scryfall, a good proxy for how often decks actually make the token.

Usage:
    pip install requests pillow
    python3 tools/fetch_mtg_tokens.py --count 100 --out tokens

Copy the resulting tokens/ folder to the root of the device SD card.
"""

import argparse
import io
import re
import time

import requests
from PIL import Image, ImageOps

SEARCH_URL = "https://api.scryfall.com/cards/search"
HEADERS = {"User-Agent": "crosspoint-token-fetcher/1.0", "Accept": "application/json"}
REQUEST_DELAY = 0.15  # Scryfall asks for 50-100ms between requests


def fetch_all_token_prints():
    """Every token printing (paper, non-digital), paginated."""
    cards = []
    params = {"q": "is:token game:paper", "unique": "prints"}
    url = SEARCH_URL
    while url:
        resp = requests.get(url, params=params, headers=HEADERS, timeout=30)
        params = None  # next_page URLs already carry the query
        resp.raise_for_status()
        data = resp.json()
        cards.extend(data.get("data", []))
        url = data.get("next_page")
        time.sleep(REQUEST_DELAY)
        print(f"  fetched {len(cards)} printings...", end="\r")
    print()
    return cards


def pick_top_tokens(cards, count):
    """Group printings by display name; return [(name, best_card)] of the top `count`."""
    groups = {}
    for card in cards:
        name = card.get("name", "")
        if not name or "//" in name:  # skip double-faced token sheets
            continue
        groups.setdefault(name, []).append(card)

    ranked = sorted(groups.items(), key=lambda kv: (-len(kv[1]), kv[0]))[:count]

    picks = []
    for name, prints in ranked:
        # newest printing with an image
        prints = [c for c in prints if c.get("image_uris")]
        if not prints:
            continue
        prints.sort(key=lambda c: c.get("released_at", ""), reverse=True)
        picks.append((name, prints[0]))
    return picks


def safe_filename(name):
    name = re.sub(r"[^A-Za-z0-9 '\-]", "", name).strip()
    return name or "Token"


def convert(img_bytes, width, height):
    """Fit the card onto a white width x height canvas, dithered to 1-bit."""
    img = Image.open(io.BytesIO(img_bytes)).convert("L")
    img = ImageOps.autocontrast(img, cutoff=1)
    img.thumbnail((width, height), Image.LANCZOS)
    canvas = Image.new("L", (width, height), 255)
    canvas.paste(img, ((width - img.width) // 2, (height - img.height) // 2))
    return canvas.convert("1")  # PIL default = Floyd-Steinberg dithering


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--count", type=int, default=100)
    ap.add_argument("--width", type=int, default=528, help="X3: 528, X4: 480")
    ap.add_argument("--height", type=int, default=792, help="X3: 792, X4: 800")
    ap.add_argument("--out", default="tokens")
    args = ap.parse_args()

    import os

    os.makedirs(args.out, exist_ok=True)

    print("Fetching token list from Scryfall...")
    cards = fetch_all_token_prints()
    picks = pick_top_tokens(cards, args.count)
    print(f"Top {len(picks)} tokens selected.")

    for i, (name, card) in enumerate(picks, 1):
        uris = card["image_uris"]
        image_url = uris.get("border_crop") or uris.get("large") or uris.get("normal")
        if not image_url:
            continue
        resp = requests.get(image_url, headers=HEADERS, timeout=60)
        resp.raise_for_status()
        bmp = convert(resp.content, args.width, args.height)
        path = os.path.join(args.out, safe_filename(name) + ".bmp")
        bmp.save(path, "BMP")
        print(f"[{i}/{len(picks)}] {name} ({card.get('set', '?').upper()})")
        time.sleep(REQUEST_DELAY)

    print(f"Done. Copy '{args.out}/' to the SD card root.")


if __name__ == "__main__":
    main()
