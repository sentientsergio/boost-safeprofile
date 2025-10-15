#!/usr/bin/env bash
# Install build dependencies on macOS via Homebrew

set -e  # Exit on error

echo "🔧 Installing Boost.SafeProfile build dependencies..."
echo ""

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "❌ Homebrew not found. Please install from https://brew.sh"
    exit 1
fi

echo "📦 Installing CMake..."
brew install cmake

echo "📦 Installing Boost..."
brew install boost

echo ""
echo "✅ Dependencies installed successfully!"
echo ""
echo "Verifying versions:"
cmake --version | head -n1
brew list --versions boost | head -n1

echo ""
echo "🚀 You can now build the project:"
echo "   cmake -S . -B build"
echo "   cmake --build build"
