# builds iOS slice
xcodebuild archive -scheme WhirlyGlobeMaplyComponent -configuration Release -destination 'generic/platform=iOS' -archivePath './archives/WhirlyGlobeMaplyComponent.framework-iphoneos.xcarchive' SKIP_INSTALL=NO
# builds iOS simulator slice
xcodebuild archive -scheme WhirlyGlobeMaplyComponent -configuration Release -destination 'generic/platform=iOS Simulator' -archivePath './archives/WhirlyGlobeMaplyComponent.framework-iphonesimulator.xcarchive' SKIP_INSTALL=NO
# merges both into .xcframework
xcodebuild -create-xcframework -framework './archives/WhirlyGlobeMaplyComponent.framework-iphonesimulator.xcarchive/Products/Library/Frameworks/WhirlyGlobeMaplyComponent.framework' -framework './archives/WhirlyGlobeMaplyComponent.framework-iphoneos.xcarchive/Products/Library/Frameworks/WhirlyGlobeMaplyComponent.framework' -output 'WhirlyGlobeMaplyComponent.xcframework'