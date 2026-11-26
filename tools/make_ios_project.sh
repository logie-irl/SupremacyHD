#!/bin/sh
# Make the Xcode project for the iPhone / iPad app (run this on your Mac).
#
#   sh tools/make_ios_project.sh --with-data SupremacyData
#
# then open build-ios/Supremacy.xcodeproj in Xcode, pick your iPhone, and in the
# "Signing & Capabilities" tab choose your Team (a free Apple ID works), then
# press Run. See README.md, "iPhone and iPad".
#
# Options:
#   --with-data DIR   your extracted SupremacyData folder, copied into the app
#   --name NAME       the app's name on the home screen (default SuperMac)
#   --id ID           the bundle identifier (default com.example.supermac; with a
#                     free Apple ID it must be one no other developer uses, for
#                     example com.yourname.supermac)
#   --team TEAMID     your Apple Developer team ID (optional; you can also pick it in Xcode)
#
# The app will contain the game's copyrighted files: keep it for your own use.
set -eu

DATA=""
NAME="SuperMac"
ID="com.example.supermac"
TEAM=""
while [ $# -gt 0 ]; do
  case "$1" in
    --with-data) DATA=$2; shift 2 ;;
    --name) NAME=$2; shift 2 ;;
    --id) ID=$2; shift 2 ;;
    --team) TEAM=$2; shift 2 ;;
    *) echo "unknown option $1" >&2; exit 1 ;;
  esac
done

ROOT=$(cd "$(dirname "$0")/.." && pwd)
command -v cmake >/dev/null || { echo "cmake is needed: brew install cmake" >&2; exit 1; }
xcodebuild -version >/dev/null 2>&1 || { echo "Xcode is needed (install it from the App Store, then run it once)" >&2; exit 1; }
if [ -n "$DATA" ]; then
  DATA=$(cd "$DATA" && pwd)
  for f in GAM1 GAM2 INT1 INT2 Main DIEM WINM; do
    [ -f "$DATA/$f" ] || { echo "$DATA does not look like the game data folder (no $f)" >&2; exit 1; }
  done
fi

cmake -S "$ROOT" -B "$ROOT/build-ios" -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
  -DSUP_IOS_DATA="$DATA" -DSUP_IOS_NAME="$NAME" -DSUP_IOS_BUNDLE_ID="$ID" -DSUP_IOS_TEAM="$TEAM"

echo
echo "Made $ROOT/build-ios/Supremacy.xcodeproj"
echo "Open it in Xcode, choose your iPhone at the top, set your Team under"
echo "Signing & Capabilities for the 'supremacy' target, and press Run."
