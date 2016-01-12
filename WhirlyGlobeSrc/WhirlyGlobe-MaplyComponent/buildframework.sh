# Most of this borrowed from http://www.cocoanetics.com/2010/04/making-your-own-iphone-frameworks/

#!/bin/bash
# xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphonesimulator9.0 clean
# xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphoneos9.0 clean
# xcodebuild -target WhirlyGlobe-MaplyComponent -configuration Debug -sdk iphonesimulator

# Locations for build products
BUILT_PRODUCTS_SIMULATOR=`xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphonesimulator -showBuildSettings | grep -m 1 "BUILT_PRODUCTS_DIR" | grep -oEi "\/.*"`
BUILT_PRODUCTS_IPHONEOS=`xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphoneos -showBuildSettings | grep -m 1 "BUILT_PRODUCTS_DIR" | grep -oEi "\/.*"`

xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphonesimulator -destination 'platform=iOS Simulator,name=iPad 2' clean build
# xcodebuild -target WhirlyGlobe-MaplyComponent -configuration Debug -sdk iphoneos
xcodebuild -target WhirlyGlobe-MaplyComponent -scheme WhirlyGlobe-MaplyComponent -configuration Release -sdk iphoneos -DONLY_ACTIVE_ARCH=NO

# name and build location
PROJECT_NAME=WhirlyGlobeMaplyComponent
FRAMEWORK_NAME=WhirlyGlobeMaplyComponent
FRAMEWORK_BUILD_PATH="./build/Framework"

# these never change
FRAMEWORK_VERSION=A
FRAMEWORK_CURRENT_VERSION=1
FRAMEWORK_COMPATIBILITY_VERSION=1

# Clean any existing framework that might be there
if [ -d "$FRAMEWORK_BUILD_PATH" ]
then
echo "Framework: Cleaning framework..."
rm -rf "$FRAMEWORK_BUILD_PATH"
fi

# Build the canonical Framework bundle directory structure
echo "Framework: Setting up directories..."
FRAMEWORK_DIR=$FRAMEWORK_BUILD_PATH/$FRAMEWORK_NAME.framework
mkdir -p $FRAMEWORK_DIR
mkdir -p $FRAMEWORK_DIR/Versions
mkdir -p $FRAMEWORK_DIR/Versions/$FRAMEWORK_VERSION
mkdir -p $FRAMEWORK_DIR/Versions/$FRAMEWORK_VERSION/Resources
mkdir -p $FRAMEWORK_DIR/Versions/$FRAMEWORK_VERSION/Headers

echo "Framework: Creating symlinks..."
ln -s $FRAMEWORK_VERSION $FRAMEWORK_DIR/Versions/Current
ln -s Versions/Current/Headers $FRAMEWORK_DIR/Headers
ln -s Versions/Current/Resources $FRAMEWORK_DIR/Resources
ln -s Versions/Current/lib${FRAMEWORK_NAME}.a $FRAMEWORK_DIR/${FRAMEWORK_NAME}

# combine lib files for various platforms into one
echo "Framework: Creating library..."
# lipo -create build/Debug-iphoneos/libWhirlyGlobeLib.a build/Debug-iphonesimulator/libWhirlyGlobeLib.a -output "$FRAMEWORK_DIR/Versions/Current/$FRAMEWORK_NAME"
lipo -create $BUILT_PRODUCTS_IPHONEOS/libWhirlyGlobe-MaplyComponent.a $BUILT_PRODUCTS_SIMULATOR/libWhirlyGlobe-MaplyComponent.a -output "$FRAMEWORK_DIR/Versions/Current/lib${FRAMEWORK_NAME}.a"

# lipo -create "${PROJECT_DIR}/build/${BUILD_STYLE}-iphoneos/lib${PROJECT_NAME}.a" "${PROJECT_DIR}/build/${BUILD_STYLE}-iphonesimulator/lib${PROJECT_NAME}.a" -o "$FRAMEWORK_DIR/Versions/Current/$FRAMEWORK_NAME"

echo "Framework: Copying assets into current version..."
cp include/*.h $FRAMEWORK_DIR/Headers/

#replace placeholder in plist with project name
cp framework_info.plist $FRAMEWORK_DIR/Resources/Info.plist

rm -rf WhirlyGlobeMaplyComponent.Framework
mv $FRAMEWORK_DIR WhirlyGlobeMaplyComponent.framework

