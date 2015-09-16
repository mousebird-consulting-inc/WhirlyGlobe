rm app/libs/Maply.aar

#  Debug vs. Release
if [ -n "$1" ]; then
    vers=${1}
else
    vers="debug"
fi

# You can use several architectures here, like x86,mips64
if [ -n "$2" ]; then
    arch=${2}
else
    arch="x86,x86_64,armeabi,armeabi-v7a,arm64-v8a,mips"
fi

pushd ../Android/
./gradlew assemble$vers -Parchitecture=$arch
popd

# Note: This is the debug version
cp ../Android/build/outputs/aar/Android-$vers.aar app/libs/Maply.aar
