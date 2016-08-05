---
title: Local Image Layer
layout: android-tutorial
---

*Tutorial by Nicholas Hallahan.*

Now we're going to add a local image layer to your map. You can have map image tiles stored on your device using the SQLite based MBTiles format. We're going to show you how to load an MBTiles file from your device and display these tiles.

### Copy Geography Class MBTiles to Device

For the sake of keeping this tutorial simple, we're going to manually copy an MBTiles file to your Android device's `ExternalStorage`. For a production app, you'll want to [explore various means](https://github.com/mousebird/WhirlyGlobe/blob/e9dec4068156191861d40d5dded1c079449c26f2/WhirlyGlobeSrc/AutoTesterAndroid/app/src/main/java/com/mousebirdconsulting/autotester/TestCases/MBTilesImageTestCase.java#L138-L162) of getting an MBTiles file onto the device automatically, but that is out of scope from this basic tutorial.

First, download our sample [Geography Class MBTiles file](http://whirlyglobedocs.s3-website-us-east-1.amazonaws.com/tutorialsupport/geography-class_medres.mbtiles) to your computer. Then, using [Android File Transfer](https://www.android.com/filetransfer/), create an `mbtiles` directory in `Phone` tab of the file explorer.

<img src="resources/android-file-transfer.png" alt="Android File Transfer" style="max-width: 500px;display:block;margin:auto;">

Drag over your `geography-class_medres.mbtiles` file into your `mbtiles` directory.

### Setup Fragment

