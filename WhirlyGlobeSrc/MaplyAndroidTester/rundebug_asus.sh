# Source this script instead of executing it

export device=android_asus_bin
export arch=x86

# Kill off the old adb
kill -9 `ps aux | grep adb | grep darkskytest | awk '{print $2}'`

# Get the PID for the app on the device
export pid=`adb shell ps | grep darkskytest | grep -v gdbserver | awk '{print $2}'`
echo "Attaching to process $pid"

adb forward tcp:5039 localfilesystem:/data/data/com.mousebirdconsulting.darkskytest/debug-pipe
nohup adb shell run-as com.mousebirdconsulting.darkskytest /data/data/com.mousebirdconsulting.darkskytest/lib/gdbserver.so +debug-pipe --attach $pid &

~/Android/android-ndk-r10e/toolchains/x86-4.9/prebuilt/darwin-x86_64/bin/i686-linux-android-gdb ~/android_asus_bin/app_process -ex "target remote :5039;" -ex "set solib-search-path ~/$device/:~/$device/system_lib:~/$device/vendor_lib:~/$device/vendor_lib/egl:libs/WhirlyGlobe-Maply/WhirlyGlobeSrc/Android/obj/local/$arch/" -ex "c"

# Kill off the old adb
kill -9 `ps aux | grep adb | grep darkskytest | awk '{print $2}'`
