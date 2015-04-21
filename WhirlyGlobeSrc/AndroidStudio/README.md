WhirlyGlobe-Maply Android for Android Studio
============================================

This is the Android version of WhirlyGlobe-Maply restructured to be compiled in Android Studio.
The native libs have to be built on the command line, they are not included in the gradle build
script.

Build instructions:
- Navigate to the AndroidStudio folder and execute $ANDROID_NDK/ndk-build
This results in the native library being compiled for all supported platforms and put into the libs/
folder.
- Open the project in Android Studio
- The project should automatically be compiled, the result is a .aar (see app/build/outputs/aar/)

It also adds an additional tile source which supports