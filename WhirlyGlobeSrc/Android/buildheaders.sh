cd src

for mod in AngleAxis AttrDictionary BaseInfo ChangeSet CoordSystem CoordSystemDisplayAdapter FakeGeocentricDisplayAdapter GeoCoordSystem GlobeView GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Matrix4d PlateCarreeCoordSystem Point2d Point3d Point4d QuadPagingLayer QuadImageTileLayer Quaternion Scene SelectionManager Shader SphericalMercatorCoordSystem Sticker StickerInfo StickerManager Texture VectorIterator VectorInfo VectorManager VectorObject View ViewState
do
    if [ com/mousebird/maply/$mod.class -ot com/mousebird/maply/$mod.java ]; then
        echo "Compiling java for $mod"

        javac -classpath "${1}"/platforms/android-18/android.jar:.  com/mousebird/maply/$mod.java
    else
        echo "Skipped compile for $mod"
    fi

    if [ ../jni/com_mousebird_maply_$mod.h -ot com/mousebird/maply/$mod.java ]; then
        echo "Building header for $mod"

        javah -classpath "${1}"/platforms/android-18/android.jar:. -jni com.mousebird.maply.$mod
        mv com_mousebird_maply_$mod.h ../jni/
        mv com_mousebird_maply_$mod_*.h ../jni/
    else
        echo "Skipped header for $mod"
    fi
done

exit 0
