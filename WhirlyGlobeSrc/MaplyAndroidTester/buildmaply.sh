rm app/libs/Maply.aar

# You can use several architectures here, like x86,mips64
if [ -n "$1" ]; then
    arch=${1}
else
    arch="x86"
fi

pushd ../Android/
./gradlew assembleRelease -Parchitecture=$arch
popd

# Note: This is the debug version
cp ../Android/build/outputs/aar/Android-debug.aar app/libs/Maply.aar
