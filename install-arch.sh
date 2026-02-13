#!/bin/bash
# Arch Linux Installation Script for Inventory Management System
# Usage: ./install-arch.sh

set -e  # Exit on error

echo "=================================="
echo "Inventory System - Arch Linux Installer"
echo "=================================="
echo ""

# Check if running on Arch Linux
if [ ! -f /etc/arch-release ]; then
    echo "Warning: This script is designed for Arch Linux."
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

echo "Step 1: Checking dependencies..."

# Check for pacman
if ! command_exists pacman; then
    echo "Error: pacman not found. Are you on Arch Linux?"
    exit 1
fi

# Install dependencies
echo "Step 2: Installing dependencies..."
PACKAGES="gcc ncurses make"
MISSING_PACKAGES=""

for pkg in $PACKAGES; do
    if ! pacman -Q $pkg >/dev/null 2>&1; then
        MISSING_PACKAGES="$MISSING_PACKAGES $pkg"
    fi
done

if [ -n "$MISSING_PACKAGES" ]; then
    echo "Installing:$MISSING_PACKAGES"
    sudo pacman -S --needed --noconfirm $MISSING_PACKAGES
else
    echo "All dependencies already installed!"
fi

echo ""
echo "Step 3: Building application..."

# Clean previous builds
make clean 2>/dev/null || true

# Build
if make; then
    echo "✓ Build successful!"
else
    echo "✗ Build failed!"
    exit 1
fi

echo ""
echo "Step 4: Installation options"
echo ""
echo "Choose installation method:"
echo "  1) System-wide installation (/usr/local/bin) [requires sudo]"
echo "  2) User installation (~/.local/bin)"
echo "  3) Skip installation (use from current directory)"
echo ""
read -p "Enter choice [1-3]: " -n 1 -r
echo

case $REPLY in
    1)
        echo "Installing system-wide..."
        sudo make install
        echo "✓ Installed to /usr/local/bin/inventory"
        echo "  Run with: inventory"
        ;;
    2)
        echo "Installing to user directory..."
        mkdir -p ~/.local/bin
        cp inventory ~/.local/bin/
        chmod +x ~/.local/bin/inventory
        
        # Check if ~/.local/bin is in PATH
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            echo ""
            echo "⚠ Warning: ~/.local/bin is not in your PATH"
            echo "Add this line to your ~/.bashrc or ~/.zshrc:"
            echo ""
            echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
            echo ""
        fi
        
        echo "✓ Installed to ~/.local/bin/inventory"
        echo "  Run with: inventory (if ~/.local/bin is in PATH)"
        echo "  Or: ~/.local/bin/inventory"
        ;;
    3)
        echo "Skipping installation."
        echo "✓ You can run the application with: ./inventory"
        ;;
    *)
        echo "Invalid choice. Skipping installation."
        echo "✓ You can run the application with: ./inventory"
        ;;
esac

echo ""
echo "=================================="
echo "Installation Complete!"
echo "=================================="
echo ""
echo "Next steps:"
echo "  1. Run 'inventory' (or './inventory' if not installed)"
echo "  2. Check out QUICKSTART.md for a tutorial"
echo "  3. Import sample data from examples/sample_inventory.csv"
echo ""
echo "Documentation:"
echo "  - README.md - Full documentation"
echo "  - QUICKSTART.md - Quick tutorial"
echo "  - docs/TROUBLESHOOTING.md - Common issues"
echo ""
echo "Happy inventory managing! 🎉"
