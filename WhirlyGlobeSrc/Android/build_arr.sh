rm -rf maply_aar
mkdir maply_aar
mkdir maply_aar/jni
mkdir maply_aar/jni/x86
mkdir maply_aar/jni/armeabi

ant build-project
cp bin/R.txt maply_aar/
cp bin/maply.jar maply_aar/

# ndk-build clean
ndk-build NDK_DEBUG=0
cp libs/armeabi/libMaply.so maply_aar/jni/armeabi/

ndk-build NDK_DEBUG=0 APP_ABI=x86
cp libs/x86/libMaply.so maply_aar/jni/x86/

zip -r maply_aar.zip maply_aar/
mv maply_aar.zip maply.aar
