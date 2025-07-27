#!/bin/bash

# Documentation Generation Script for Connection Library
# This script generates both HTML and Markdown documentation

echo "Connection Library Documentation Generator"
echo "========================================"

# Check if doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed. Please install it first."
    echo "On macOS: brew install doxygen"
    echo "On Ubuntu/Debian: sudo apt-get install doxygen"
    exit 1
fi

# Check if we're in the right directory
if [ ! -f "../Doxyfile" ]; then
    echo "Error: Doxyfile not found. Please run this script from the docs directory."
    exit 1
fi

echo "Step 1: Generating Doxygen documentation..."
cd ..
doxygen Doxyfile
cd docs

if [ $? -ne 0 ]; then
    echo "Error: Doxygen generation failed!"
    exit 1
fi

echo "Step 2: Converting XML to Markdown..."
if [ -f "generate_md_docs.py" ]; then
    python3 generate_md_docs.py
    if [ $? -ne 0 ]; then
        echo "Warning: Markdown generation failed, but HTML documentation is available."
    else
        echo "Markdown documentation generated successfully!"
    fi
else
    echo "Warning: generate_md_docs.py not found. Skipping Markdown generation."
fi

echo ""
echo "Documentation Generation Complete!"
echo "================================="
echo "HTML Documentation: html/index.html"
echo "Markdown Documentation: docs.md"
echo "XML Documentation: xml/"
echo ""
echo "To view HTML documentation, open html/index.html in a web browser."
echo "To view Markdown documentation, open docs.md in any text editor or markdown viewer."
