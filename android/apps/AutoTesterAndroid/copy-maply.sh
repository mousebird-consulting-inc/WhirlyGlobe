# Usage: first run ./graddle assemble from Android directory,
#        then run this script to copy the aar file to the right 
#        directory for AutoTester app

#  Debug vs. Release
if [ -n "$1" ]; then
    vers=${1}
else
    vers="debug"
fi

if [[ $2 == "nocache" ]]; then
    echo "Copying Maply $vers (nocache)..."
    cp ../Android/build/outputs/aar/Android-$vers.aar Maply/Maply.aar
else
    if [ Maply/Maply.aar -ot ../Android/build/outputs/aar/Android-debug.aar ]; then
        echo "Copying Maply $vers..."
        cp ../Android/build/outputs/aar/Android-debug.aar Maply/Maply.aar
    else
        echo "Skipped"
    fi
fi

exit 0
