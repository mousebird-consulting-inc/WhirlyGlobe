rm app/libs/Maply.aar

pushd ../Android/
./gradlew assembleRelease
popd

# Note: This is the debug version
cp ../Android/build/outputs/aar/Android-debug.aar app/libs/Maply.aar
