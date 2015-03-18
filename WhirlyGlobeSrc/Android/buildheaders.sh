cd src

for mod in AttrDictionary ChangeSet CoordSystem CoordSystemDisplayAdapter FakeGeocentricDisplayAdapter GeoCoordSystem GlobeView GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Matrix4d PlateCarreeCoordSystem Point2d Point3d QuadPagingLayer QuadImageTileLayer Quaternion Scene SelectionManager SphericalMercatorCoordSystem Texture VectorIterator VectorInfo VectorManager VectorObject View ViewState
do

echo "Building header for $mod"
javac -classpath /Users/sjg/android/adt-bundle-mac-x86_64-20130917/sdk/platforms/android-18/android.jar:.  com/mousebird/maply/$mod.java
javah -classpath /Users/sjg/android/adt-bundle-mac-x86_64-20130917/sdk/platforms/android-18/android.jar:. -jni com.mousebird.maply.$mod
mv com_mousebird_maply_$mod.h ../jni/
mv com_mousebird_maply_$mod_*.h ../jni/

done
