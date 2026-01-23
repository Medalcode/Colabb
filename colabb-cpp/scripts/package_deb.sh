#!/bin/bash
set -e

# Configuration
APP_NAME="colabb"
VERSION="1.0.0"
BUILD_DIR="build"
PACKAGE_DIR="pkg/deb"
DEB_NAME="${APP_NAME}_${VERSION}_amd64.deb"

echo "📦 Packaging $APP_NAME v$VERSION..."

# Ensure build exists
if [ ! -f "$BUILD_DIR/$APP_NAME" ]; then
    echo "❌ Build not found! Please run 'make' first."
    exit 1
fi

# Create directory structure
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/DEBIAN"
mkdir -p "$PACKAGE_DIR/usr/bin"
mkdir -p "$PACKAGE_DIR/usr/share/applications"
mkdir -p "$PACKAGE_DIR/usr/share/icons/hicolor/scalable/apps"

# Copy binary
cp "$BUILD_DIR/$APP_NAME" "$PACKAGE_DIR/usr/bin/"
chmod +x "$PACKAGE_DIR/usr/bin/$APP_NAME"

# Create control file
cat > "$PACKAGE_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: amd64
Depends: libgtk-3-0, libvte-2.91-0, libcurl4, libjsoncpp25
Maintainer: Medalcode <medalcode@example.com>
Description: Colabb Terminal
 A modern terminal emulator with AI assistance.
EOF

# Create desktop entry
cat > "$PACKAGE_DIR/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Name=Colabb Terminal
Comment=Modern terminal with AI assistance
Exec=$APP_NAME
Icon=utilities-terminal
Terminal=false
Type=Application
Categories=Utility;TerminalEmulator;
EOF

# Build .deb
dpkg-deb --build "$PACKAGE_DIR" "$DEB_NAME"

echo "✅ Package created: $DEB_NAME"
echo "   Install with: sudo apt install ./$DEB_NAME"
