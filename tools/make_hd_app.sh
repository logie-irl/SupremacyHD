#!/bin/sh
# Build SupremacyHD.app: a separate app (its own name, settings and saved games, in
# ~/Library/Application Support/SupremacyHD/) that draws the game's pictures from a
# hi-res pack. Run this on your Mac:
#
#   brew install cmake sdl3
#   sh tools/make_hd_app.sh                      # SupremacyHD.app, game data and HiRes folder
#                                                # taken from ./SupremacyData and ./HiRes if present
#   sh tools/make_hd_app.sh --data DIR --hires DIR
#
# Making the pack: see README.md, "SupremacyHD". The app contains the game's
# copyrighted files and pictures: keep it for your own use.
set -eu
DATA=SupremacyData
HIRES=HiRes
while [ $# -gt 0 ]; do
  case "$1" in
    --data) DATA=$2; shift 2 ;;
    --hires) HIRES=$2; shift 2 ;;
    \#*) break ;;                      # a pasted comment
    *) echo "unknown option $1" >&2; exit 1 ;;
  esac
done
cd "$(dirname "$0")/.."
cmake -S . -B build-hd -DCMAKE_BUILD_TYPE=Release -DSUP_HD=ON
cmake --build build-hd
EXTRA=""
if [ -d "$HIRES" ]; then EXTRA="--with-hires $HIRES"; else echo "No $HIRES folder yet: the app will look in ~/Library/Application Support/SupremacyHD/HiRes"; fi
sh tools/make_macos_app.sh --with-data "$DATA" $EXTRA build-hd SupremacyHD.app
