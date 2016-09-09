LOCAL_PROPS=/Users/jenkins/Dev/local.properties
ANDROID_SDK_HOME=/Users/jenkins/Dev/android-sdk-macosx

cd ..

pushd WhirlyGlobe-MaplyComponent
git submodule init
git submodule update
popd

pushd Android
find ./src -name "*.class" -type f -delete
./cleanheaders.sh
./buildheaders.sh $ANDROID_SDK_HOME nocache

cp $LOCAL_PROPS .
./gradlew clean assembleRelease
popd

pushd AutoTesterAndroid
./copy-maply.sh release nocache
cp $LOCAL_PROPS .
./gradlew clean assembleRelease
popd
