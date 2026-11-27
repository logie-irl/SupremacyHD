#!/bin/sh
# Build SupremacyHD.app: a separate app (its own name, settings and saved games, in
# ~/Library/Application Support/SupremacyHD/) that draws the game's pictures from a
# hi-res pack. Run this on your Mac:
#
#   brew install cmake sdl3
#   sh tools/make_hd_app.sh                      # SupremacyHD.app, game data and HiRes folder
#                                                # taken from ./SupremacyData and ./HiRes if present
#   sh tools/make_hd_app.sh --data DIR --hires DIR
#   sh tools/make_hd_app.sh --no-data            # "Supremacy HD.app" with no game files inside:
#                                                # the launcher asks for them (BROWSE / EXTRACT),
#                                                # so this one can be shared
#   --name "NAME"                                # another app name
#
# Making the pack: see README.md, "SupremacyHD". The app contains the game's
# copyrighted files and pictures: keep it for your own use.
set -eu
DATA=SupremacyData
HIRES=HiRes
NAME=""
NODATA=0
while [ $# -gt 0 ]; do
  case "$1" in
    --data) DATA=$2; shift 2 ;;
    --hires) HIRES=$2; shift 2 ;;
    --no-data) NODATA=1; shift ;;
    --name) NAME=$2; shift 2 ;;
    \#*) break ;;                      # a pasted comment
    *) echo "unknown option $1" >&2; exit 1 ;;
  esac
done
cd "$(dirname "$0")/.."
# a build folder made before the repository was moved or renamed: start it again
if [ -f build-hd/CMakeCache.txt ] && ! grep -q "CMAKE_HOME_DIRECTORY:INTERNAL=$(pwd)\$" build-hd/CMakeCache.txt; then
  echo "build-hd was made in another folder: starting it again"
  rm -rf build-hd
fi
cmake -S . -B build-hd -DCMAKE_BUILD_TYPE=Release -DSUP_HD=ON
cmake --build build-hd
if [ "$NODATA" = 1 ]; then
  # no game files and no pictures inside: safe to share
  APP="${NAME:-Supremacy HD}.app"
  sh tools/make_macos_app.sh build-hd "$APP"
  echo "No game files inside: the launcher asks for them (BROWSE or EXTRACT) on first start."
  exit 0
fi
APP="${NAME:-SupremacyHD}.app"
EXTRA=""
if [ -d "$HIRES" ]; then EXTRA="--with-hires $HIRES"; else echo "HD pictures: built in (made from your game files). A $HIRES pack, if you make one, replaces them."; fi
sh tools/make_macos_app.sh --with-data "$DATA" $EXTRA build-hd "$APP"
