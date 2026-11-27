#!/bin/sh
# Make a hi-res picture pack for SupremacyHD with an AI upscaler (run on your Mac).
#
#   sh tools/upscale_pictures.sh [DATA_DIR] [OUT_DIR]       # defaults: SupremacyData  HiRes
#
# 1. the game writes its nine pictures (320x200) to HiRes-original/
# 2. an upscaler enlarges each to 1280x800 into HiRes/
#
# Upscalers it can drive (install one):
#   realesrgan-ncnn-vulkan   https://github.com/xinntao/Real-ESRGAN-ncnn-vulkan/releases (macOS zip)
#   Upscayl (free app)       https://upscayl.org : no command line needed, do step 2 by hand:
#                            open HiRes-original/, pick an anime/illustration model, 4x scale,
#                            PNG, output folder HiRes/ (keep the file names: main.png ...)
# Then run SupremacyHD: it uses the HiRes folder next to it, inside the app, or in
# ~/Library/Application Support/SupremacyHD/HiRes.
set -eu
DATA=${1:-SupremacyData}
OUT=${2:-HiRes}
SRC=${OUT}-original
BIN=""
for c in build-hd/supremacy build/supremacy; do [ -x "$c" ] && { BIN=$c; break; }; done
[ -n "$BIN" ] || { echo "build the game first (cmake -S . -B build && cmake --build build)" >&2; exit 1; }
"$BIN" "$DATA" --export-pictures "$SRC"
UP=$(command -v realesrgan-ncnn-vulkan || true)
if [ -z "$UP" ]; then
  echo
  echo "The pictures are in $SRC/. No command-line upscaler found: use Upscayl (see the top of this file),"
  echo "save the results in $OUT/ with the same file names, then start SupremacyHD."
  exit 0
fi
mkdir -p "$OUT"
"$UP" -i "$SRC" -o "$OUT" -n realesrgan-x4plus-anime -s 4 -f png
echo "Done: $OUT/ holds the hi-res pack."
