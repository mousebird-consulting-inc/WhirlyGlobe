# Usage: first run ./graddle assemble from Android directory,
#        then run this script to copy the aar file to the right 
#        directory for AutoTester app

if [ Maply/Maply.aar -ot ../Android/build/outputs/aar/Android-debug.aar ]; then
    echo "Copying Maply..."
    cp ../Android/build/outputs/aar/Android-debug.aar Maply/Maply.aar
else
    echo "Skipped"
fi

exit 0
