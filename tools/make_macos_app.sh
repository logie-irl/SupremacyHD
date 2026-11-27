#!/bin/sh
# Package the built game as a double-clickable Supremacy.app (macOS).
#
#   brew install cmake sdl3
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
#   sh tools/make_macos_app.sh            # makes ./Supremacy.app
#   sh tools/make_macos_app.sh --with-data SupremacyData build SuperMac.app
#                                         # one self-contained SuperMac.app with your game
#                                         # files inside: drag it to Applications
#   sh tools/make_macos_app.sh --with-hires HiRes ...   # also bundle a hi-res picture pack
#   (SupremacyHD: see tools/make_hd_app.sh)
#
# The app carries its own copy of SDL3 and is ad-hoc signed, which is enough to run
# it on the Mac that built it. Settings, saved games and the game data live in
# ~/Library/Application Support/Supremacy/ (the first launch opens the launcher:
# EXTRACT your IPF disks or WHDLoad archive there). The app contains none of the
# game's data unless you pass --with-data DIR (your extracted SupremacyData
# folder, 12 files). That copy is for you only: an app with the game's files
# inside must never be shared or published.
set -eu

DATA=""
HIRES=""
BUILD=""
APP=""
while [ $# -gt 0 ]; do
  case "$1" in
    --with-data) [ $# -ge 2 ] || { echo "--with-data needs a folder" >&2; exit 1; }; DATA=$2; shift 2 ;;
    --with-hires) [ $# -ge 2 ] || { echo "--with-hires needs a folder" >&2; exit 1; }; HIRES=$2; shift 2 ;;
    *) if [ -z "$BUILD" ]; then BUILD=$1; else APP=$1; fi; shift ;;   # (names may contain spaces)
  esac
done
BUILD=${BUILD:-build}
APP=${APP:-Supremacy.app}
BIN="$BUILD/supremacy"
ROOT=$(cd "$(dirname "$0")/.." && pwd)

[ -x "$BIN" ] || { echo "$BIN not found: build the game first (see the top of this file)" >&2; exit 1; }

if [ -n "$DATA" ]; then
  for f in GAM1 GAM2 INT1 INT2 Main DIEM WINM; do
    [ -f "$DATA/$f" ] || { echo "$DATA does not look like the game data folder (no $f)" >&2; exit 1; }
  done
fi

rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources" "$APP/Contents/Frameworks"
cp "$BIN" "$APP/Contents/MacOS/supremacy"

# --- bundle SDL3 ---
REF=$(otool -L "$BIN" | awk '/libSDL3.*\.dylib/ {print $1; exit}')
[ -n "$REF" ] || { echo "the binary does not link libSDL3" >&2; exit 1; }
case "$REF" in
  @rpath/*|@loader_path/*|@executable_path/*)
    NAME=$(basename "$REF")
    LIB=""
    for d in "$(brew --prefix sdl3 2>/dev/null)/lib" /opt/homebrew/lib /usr/local/lib; do
      [ -f "$d/$NAME" ] && { LIB="$d/$NAME"; break; }
    done
    [ -n "$LIB" ] || { echo "cannot find $NAME (is SDL3 installed with Homebrew?)" >&2; exit 1; }
    ;;
  *) LIB="$REF"; NAME=$(basename "$REF") ;;
esac
cp -L "$LIB" "$APP/Contents/Frameworks/$NAME"
chmod u+w "$APP/Contents/Frameworks/$NAME"
install_name_tool -id "@executable_path/../Frameworks/$NAME" "$APP/Contents/Frameworks/$NAME"
install_name_tool -change "$REF" "@executable_path/../Frameworks/$NAME" "$APP/Contents/MacOS/supremacy"

# --- icon (from assets/icons/supremacy.png) ---
ICONSET=$(mktemp -d)/Supremacy.iconset
mkdir -p "$ICONSET"
SRC="$ROOT/assets/icons/supremacy.png"
for s in 16 32 128 256 512; do
  sips -z $s $s "$SRC" --out "$ICONSET/icon_${s}x${s}.png" >/dev/null
  sips -z $((s * 2)) $((s * 2)) "$SRC" --out "$ICONSET/icon_${s}x${s}@2x.png" >/dev/null
done
iconutil -c icns "$ICONSET" -o "$APP/Contents/Resources/Supremacy.icns" || echo "icon skipped"

if [ -n "$DATA" ]; then
  mkdir -p "$APP/Contents/Resources/SupremacyData"
  cp "$DATA"/* "$APP/Contents/Resources/SupremacyData/"
  echo "Game data bundled: this app is for your own use only, do not share it."
fi

if [ -n "$HIRES" ]; then
  [ -d "$HIRES" ] || { echo "$HIRES is not a folder" >&2; exit 1; }
  mkdir -p "$APP/Contents/Resources/HiRes"
  cp "$HIRES"/* "$APP/Contents/Resources/HiRes/"
  echo "Hi-res pictures bundled: for your own use only, do not share this app."
fi

cat > "$APP/Contents/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>Supremacy</string>
  <key>CFBundleDisplayName</key><string>Supremacy</string>
  <key>CFBundleIdentifier</key><string>io.github.supremacy-mac</string>
  <key>CFBundleExecutable</key><string>supremacy</string>
  <key>CFBundleIconFile</key><string>Supremacy</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleVersion</key><string>1.0</string>
  <key>CFBundleShortVersionString</key><string>1.0</string>
  <key>LSMinimumSystemVersion</key><string>11.0</string>
  <key>NSHighResolutionCapable</key><true/>
  <key>LSApplicationCategoryType</key><string>public.app-category.strategy-games</string>
</dict>
</plist>
EOF

# The app's name follows its file name (SuperMac.app is called "SuperMac").
NAME=$(basename "$APP" .app)
ID=$(echo "$NAME" | tr " " "-" | tr -cd "A-Za-z0-9-" | tr "A-Z" "a-z")
sed -e "s|io.github.supremacy-mac|io.github.supremacy-mac.$ID|" \
    -e "s|<key>CFBundleName</key><string>Supremacy</string>|<key>CFBundleName</key><string>$NAME</string>|" \
    -e "s|<key>CFBundleDisplayName</key><string>Supremacy</string>|<key>CFBundleDisplayName</key><string>$NAME</string>|" \
    "$APP/Contents/Info.plist" > "$APP/Contents/Info.plist.new"
mv "$APP/Contents/Info.plist.new" "$APP/Contents/Info.plist"

# Ad-hoc signature: required for arm64 code to run.
codesign --force --deep --sign - "$APP"
echo "Built $APP"
