cd src


for mod in ChangeSet CoordSystemDisplayAdapter InternalMarker MapScene MapView MarkerInfo MarkerManager MaplyRenderer Matrix4d Point2d Point3d Texture VectorInfo VectorManager VectorObject
do

echo "Building header for $mod"
javac -classpath /Users/sjg/android/adt-bundle-mac-x86_64-20130917/sdk/platforms/android-18/android.jar:.  com/mousebirdconsulting/maply/$mod.java
javah -jni com.mousebirdconsulting.maply.$mod
mv com_mousebirdconsulting_maply_$mod.h ../jni/

done
