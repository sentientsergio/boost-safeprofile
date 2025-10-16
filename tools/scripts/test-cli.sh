#!/usr/bin/env bash
# Test script for CLI functionality

set -e

CLI="./build/boost-safeprofile"

echo "=== Testing CLI ==="
echo ""

echo "1. Testing --help:"
$CLI --help
echo ""

echo "2. Testing --version:"
$CLI --version
echo ""

echo "3. Testing basic analysis:"
$CLI ./src
echo ""

echo "4. Testing with all options:"
$CLI --profile memory-safety --sarif out.sarif --report report.html --evidence ./evidence ./src
echo ""

echo "✅ All CLI tests passed!"
