rm app/libs/Maply.aar

# You can use several architectures here, like x86,mips64
if [ -n "$1" ]; then
    arch=${1}
else
    arch="x86,x86_64,armeabi,armeabi-v7a,arm64-v8a,mips"
fi

pushd ../Android/
./gradlew assembleDebug -Parchitecture=$arch
popd

# Note: This is the debug version
cp ../Android/build/outputs/aar/Android-debug.aar app/libs/Maply.aar
