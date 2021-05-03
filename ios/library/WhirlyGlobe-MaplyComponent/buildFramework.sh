# Most of this borrowed from http://www.cocoanetics.com/2010/04/making-your-own-iphone-frameworks/

#!/bin/bash

# Locations for build products
BUILT_PRODUCTS_SIMULATOR=`xcodebuild -target WhirlyGlobeMaplyComponent -scheme WhirlyGlobeMaplyComponent -configuration Release -sdk iphonesimulator -showBuildSettings OTHER_CFLAGS='-fembed-bitcode' | grep -m 1 "BUILT_PRODUCTS_DIR" | grep -oEi "\/.*"`
echo Simulator products: $BUILT_PRODUCTS_SIMULATOR

BUILT_PRODUCTS_IPHONEOS=`xcodebuild -target WhirlyGlobeMaplyComponent -scheme WhirlyGlobeMaplyComponent -configuration Release -sdk iphoneos -showBuildSettings OTHER_CFLAGS='-fembed-bitcode' | grep -m 1 "BUILT_PRODUCTS_DIR" | grep -oEi "\/.*"`
echo iPhoneOS products: $BUILT_PRODUCTS_IPHONEOS

# simulator build disabled for now to stay under CI time limit
echo Building for simulator...
DEST="platform=iOS Simulator,name=iPad (8th generation)"
xcodebuild -target WhirlyGlobeMaplyComponent -scheme WhirlyGlobeMaplyComponent -configuration Archive -sdk iphonesimulator -destination "$DEST" OTHER_CFLAGS='-fembed-bitcode' build
echo Building for iPhoneOS
xcodebuild -target WhirlyGlobeMaplyComponent -scheme WhirlyGlobeMaplyComponent -configuration Archive -sdk iphoneos -DONLY_ACTIVE_ARCH=NO OTHER_CFLAGS='-fembed-bitcode' build

echo Constructing Framework...

# name and build location
PROJECT_NAME=WhirlyGlobeMaplyComponent
FRAMEWORK_NAME=WhirlyGlobeMaplyComponent
FRAMEWORK_BUILD_PATH="./build/Framework"

# these never change
FRAMEWORK_VERSION=A
FRAMEWORK_CURRENT_VERSION=1
FRAMEWORK_COMPATIBILITY_VERSION=1

# Clean any existing framework that might be there
if [ -d "$FRAMEWORK_BUILD_PATH" ]; then
    echo "Framework: Cleaning $FRAMEWORK_BUILD_PATH"
    rm -rf "$FRAMEWORK_BUILD_PATH"
fi

# Build the canonical Framework bundle directory structure
FRAMEWORK_DIR=$FRAMEWORK_BUILD_PATH/$FRAMEWORK_NAME.framework
echo "Framework: Setting up directories in $FRAMEWORK_DIR"
mkdir -p $FRAMEWORK_DIR/Headers
mkdir -p $FRAMEWORK_DIR/Modules

# combine lib files for various platforms into one
echo "Framework: Creating library..."
# lipo -create build/Debug-iphoneos/libWhirlyGlobeLib.a build/Debug-iphonesimulator/libWhirlyGlobeLib.a -output "$FRAMEWORK_DIR/Versions/Current/$FRAMEWORK_NAME"
echo "  Linking libraries in $BUILT_PRODUCTS_IPHONEOS and $BUILT_PRODUCTS_SIMULATOR"
lipo -create $BUILT_PRODUCTS_IPHONEOS/WhirlyGlobeMaplyComponent.framework/WhirlyGlobeMaplyComponent $BUILT_PRODUCTS_SIMULATOR/WhirlyGlobeMaplyComponent.framework/WhirlyGlobeMaplyComponent -output "$FRAMEWORK_DIR/${FRAMEWORK_NAME}"

if [ $? -ne 0 ]; then
    # Lipo failed, assume that's because the outputs are the same architecture, and just use one as-is
    cp "$BUILT_PRODUCTS_IPHONEOS/${FRAMEWORK_NAME}.framework/${FRAMEWORK_NAME}" "$FRAMEWORK_DIR/${FRAMEWORK_NAME}"
fi

# lipo -create "${PROJECT_DIR}/build/${BUILD_STYLE}-iphoneos/lib${PROJECT_NAME}.a" "${PROJECT_DIR}/build/${BUILD_STYLE}-iphonesimulator/lib${PROJECT_NAME}.a" -o "$FRAMEWORK_DIR/Versions/Current/$FRAMEWORK_NAME"

echo "Framework: Copying assets into current version..."
cp -r include/ $FRAMEWORK_DIR/Headers/

#replace placeholder in plist with project name
cp $BUILT_PRODUCTS_IPHONEOS/WhirlyGlobeMaplyComponent.framework/Modules/module.modulemap $FRAMEWORK_DIR/Modules/
cp $BUILT_PRODUCTS_IPHONEOS/WhirlyGlobeMaplyComponent.framework/default.metallib $FRAMEWORK_DIR/
cp $BUILT_PRODUCTS_IPHONEOS/WhirlyGlobeMaplyComponent.framework/Info.plist $FRAMEWORK_DIR/Info.plist

mv $FRAMEWORK_DIR WhirlyGlobeMaplyComponent.framework

