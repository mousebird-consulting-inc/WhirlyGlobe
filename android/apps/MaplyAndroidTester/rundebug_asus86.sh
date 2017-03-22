# Source this script instead of executing it

export device=android_asus_bin
export arch=x86
export app=darkskytest
export apploc=darkskytest-1
export vers=-1
export adb=/Users/sjg/Android_sdk/platform-tools//adb
export port=5039

# Kill off the old adb
kill -9 `ps aux | grep adb | grep $app | awk '{print $2}'`

# Get the PID for the app on the device
export pid=`$adb shell ps | grep $app | grep -v gdbserver | awk '{print $2}'`
echo "Attaching to process $pid"

$adb forward tcp:$port localfilesystem:/data/data/com.mousebirdconsulting.$apploc/debug-pipe
rm nohup.out
nohup $adb shell run-as com.mousebirdconsulting.$apploc /data/data/com.mousebirdconsulting.$apploc/lib/gdbserver.so +debug-pipe --attach $pid &
sleep 1

echo "~/Android/android-ndk-r10e/toolchains/x86-4.9/prebuilt/darwin-x86_64/bin/i686-linux-android-gdb ~/$device/app_process -i=mi -ex \"target remote :$port\" -ex \"set solib-search-path ~/$device/:~/$device/system_lib:~/$device/vendor_lib:~/$device/vendor_lib/egl:~/iPhone/DarkSkyAndroid/libs/WhirlyGlobe-Maply/WhirlyGlobeSrc/Android/obj/local/$arch/\""

# Kill off the old adb
# kill -9 `ps aux | grep adb | grep $app | awk '{print $2}'`
