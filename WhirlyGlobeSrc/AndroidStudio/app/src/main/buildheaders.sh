#!/usr/bin/env bash
[ -z "$ANDROID_SDK" ] && { echo "Please set the environment variable ANDROID_SDK to the location of the Android SDK"; exit 1; }

cd java

for mod in AngleAxis AttrDictionary ChangeSet CoordSystem CoordSystemDisplayAdapter FakeGeocentricDisplayAdapter GeoCoordSystem GlobeView GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Matrix4d PlateCarreeCoordSystem Point2d Point3d Point4d QuadPagingLayer QuadImageTileLayer Quaternion Scene SelectionManager SphericalMercatorCoordSystem Texture VectorIterator VectorInfo VectorManager VectorObject View ViewState
do
    echo "Building header for $mod"
    javac -classpath $ANDROID_SDK/platforms/android-18/android.jar:. com/mousebird/maply/$mod.java
    javah -classpath $ANDROID_SDK/platforms/android-18/android.jar:. -jni com.mousebird.maply.$mod
    mv com_mousebird_maply_$mod.h ../jni/
    mv com_mousebird_maply_$mod_*.h ../jni/
done

cd ..