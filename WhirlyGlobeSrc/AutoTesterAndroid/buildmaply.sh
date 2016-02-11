rm Maply/Maply.aar

#  Debug vs. Release
if [ -n "$1" ]; then
    vers=${1}
else
    vers="Release"
fi

# You can use several architectures here, like x86,mips64
if [ -n "$2" ]; then
    arch=${2}
else
    arch="x86,armeabi,armeabi-v7a"
fi

rm -rf app/src/main/jniLibs/
mkdir app/src/main/jniLibs

oldIFS=$IFS
IFS=","
if [ "$vers" = "Debug" ]
then
    for v in $arch
        do
            cp -r gdb_jniLibs/$v app/src/main/jniLibs
        done
fi
IFS=$oldIFS

WGTREE=../Android

pushd $WGTREE
./gradlew assemble$vers -Parchitecture=$arch
popd

# Note: This is the debug version
cp $WGTREE/build/outputs/aar/Android-$vers.aar Maply/Maply.aar
