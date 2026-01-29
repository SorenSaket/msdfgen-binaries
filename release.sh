#!/bin/bash

# Release script for msdfgen-binaries
# This script creates a release by building all platforms and uploading artifacts

set -e

VERSION="1.13.0"
RELEASE_NAME="msdfgen-${VERSION}"

echo "Creating release ${RELEASE_NAME}"

# Clean previous builds
rm -rf dist
mkdir -p dist

# Function to build for a specific platform
build_platform() {
    local platform=$1
    local triplet=$2
    local generator=$3
    
    echo "Building for ${platform}..."
    
    mkdir -p build-${platform}
    cd build-${platform}
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET=${triplet} \
        -G "${generator}" \
        -DMSDFGEN_USE_SKIA=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DMSDFGEN_BUILD_STANDALONE=ON
    
    cmake --build . --config Release --parallel
    cmake --build . --target package-all --config Release
    
    # Copy artifacts to dist
    cp packages/${platform}/* ../dist/
    
    cd ..
    rm -rf build-${platform}
    
    echo "✓ ${platform} build complete"
}

# Check if vcpkg exists, if not clone it
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git
    ./vcpkg/bootstrap-vcpkg.sh
fi

# Check if msdfgen submodule exists, if not clone it
if [ ! -d "msdfgen" ]; then
    echo "Cloning msdfgen..."
    git clone --recurse-submodules https://github.com/Chlumsky/msdfgen.git
fi

# Build for each platform (only works on the current platform)
# This is a simplified script - for full cross-platform builds, use GitHub Actions
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    build_platform "linux-x64" "x64-linux" "Ninja"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    build_platform "macos-x64" "x64-osx" "Ninja"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    build_platform "windows-x64" "x64-windows" "Visual Studio 17 2022"
fi

# Create checksums
cd dist
sha256sum * > checksums.txt
cd ..

echo "Release artifacts created in 'dist' directory:"
ls -la dist/

echo "To create a GitHub release:"
echo "1. Create a new release on GitHub"
echo "2. Upload all files from the 'dist' directory"
echo "3. Include the contents of checksums.txt in the release notes"