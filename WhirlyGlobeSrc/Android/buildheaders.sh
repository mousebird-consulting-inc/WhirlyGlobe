cd src

for mod in AngleAxis AttrDictionary BaseInfo ChangeSet CoordSystem CoordSystemDisplayAdapter DirectionalLight FakeGeocentricDisplayAdapter GeneralDisplayAdapter GeoCoordSystem GlobeView GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Material Matrix4d PlateCarreeCoordSystem ParticleSystemManager ParticleSystem ParticleBatch Proj4CoordSystem Point2d Point3d Point4d Shape ShapeInfo ShapeManager ShapeSphere QuadPagingLayer QuadImageTileLayer QuadImageOfflineLayer QuadTracker Quaternion Scene SelectionManager Shader SphericalMercatorCoordSystem Sticker StickerInfo StickerManager Sun Texture VectorIterator VectorInfo VectorManager VectorObject View ViewState VertexAttribute
do
    if [ com/mousebird/maply/$mod.class -ot com/mousebird/maply/$mod.java ]; then
        echo "Compiling java for $mod"

        javac -classpath "${1}"/platforms/android-21/android.jar:../libs/okhttp-2.0.0-RC1.jar:.  com/mousebird/maply/$mod.java
    else
        echo "Skipped compile for $mod"
    fi

    if [ ../jni/com_mousebird_maply_$mod.h -ot com/mousebird/maply/$mod.java ]; then
        echo "Building header for $mod"

        javah -classpath "${1}"/platforms/android-21/android.jar:../libs/okhttp-2.0.0-RC1.jar:. -jni com.mousebird.maply.$mod
        mv com_mousebird_maply_$mod.h ../jni/
        mv com_mousebird_maply_$mod_*.h ../jni/
    else
        echo "Skipped header for $mod"
    fi
done

exit 0
