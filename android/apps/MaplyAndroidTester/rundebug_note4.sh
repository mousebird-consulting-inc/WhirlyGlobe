# Source this script instead of executing it

export device=android_note4_bin
export arch=armeabi-v7a
export app=darkskytest
export adb=/Users/sjg/Android_sdk/platform-tools//adb

# Kill off the old adb
kill -9 `ps aux | grep adb | grep $app | awk '{print $2}'`

# Get the PID for the app on the device
export pid=`$adb shell ps | grep $app | grep -v gdbserver | awk '{print $2}'`
echo "Attaching to process $pid"

$adb forward tcp:5039 localfilesystem:/data/data/com.mousebirdconsulting.$app/debug-pipe
rm nohup.out
nohup $adb shell run-as com.mousebirdconsulting.$app /data/data/com.mousebirdconsulting.$app/lib/gdbserver.so +debug-pipe --attach $pid &
sleep 1

echo "~/Android/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gdb  ~/$device/app_process -i=mi -ex \"target remote :5039;\" -ex \"set solib-search-path ~/$device/:~/$device/system_lib:~/$device/vendor_lib:~/$device/vendor_lib/egl:~/iPhone/DarkSkyAndroid/libs/WhirlyGlobe-Maply/WhirlyGlobeSrc/Android/obj/local/$arch/\""

