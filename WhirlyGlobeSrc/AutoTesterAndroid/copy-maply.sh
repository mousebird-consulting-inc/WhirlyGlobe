# Usage: first run ./graddle assemble from Android directory,
#        then run this script to copy the aar files to the right 
#        directory for AutoTester app

if [ app/Maply/Maply-debug.aar -ot ../Android/build/outputs/aar/Android-debug.aar ]; then
    echo "Copying Maply-debug"
    cp ../Android/build/outputs/aar/Android-debug.aar app/Maply/Maply-debug.aar
else
    echo "Skipped Maply-debug"
fi

if [ Maply-release/Maply-release.aar -ot ../Android/build/outputs/aar/Android-release.aar ]; then
    echo "Copying Maply-release"
    cp ../Android/build/outputs/aar/Android-release.aar Maply-release/Maply-release.aar
else
    echo "Skipped Maply-release"
fi

exit 0
