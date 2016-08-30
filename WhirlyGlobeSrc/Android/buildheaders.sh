compile() {
    echo "Compiling java for $2"
    javac -classpath "${1}"/platforms/android-21/android.jar:../libs/okhttp-2.0.0-RC1.jar:.  com/mousebird/maply/$2.java
}

header() {
    echo "Building header for $2"

    javah -classpath "${1}"/platforms/android-21/android.jar:../libs/okhttp-2.0.0-RC1.jar:. -jni com.mousebird.maply.$2
    mv com_mousebird_maply_$2*.h ../jni/
}

cd src

for mod in AngleAxis AttrDictionary BaseInfo Billboard BillboardInfo BillboardManager ChangeSet CoordSystem CoordSystemDisplayAdapter DirectionalLight FakeGeocentricDisplayAdapter GeneralDisplayAdapter GeoCoordSystem GlobeView MapboxVectorTileParser GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Material Matrix3d Matrix4d Moon PlateCarreeCoordSystem ParticleSystemManager ParticleSystem ParticleBatch Proj4CoordSystem Point2d Point3d Point4d ScreenObject Shape ShapeInfo ShapeManager ShapeSphere SimplePoly StringWrapper QuadPagingLayer QuadImageTileLayer QuadImageOfflineLayer QuadTracker Quaternion Scene SelectionManager SelectedObject Shader SphericalMercatorCoordSystem Sticker StickerInfo StickerManager Sun Texture  VectorIterator VectorInfo VectorManager VectorObject View ViewState VertexAttribute
do
    if [[ $2 == "nocache" ]]; then
        compile "$1" "$mod"
        header "$1" "$mod"
    else
        if [ com/mousebird/maply/$mod.class -ot com/mousebird/maply/$mod.java ]; then
            compile "$1" "$mod"
        else
            echo "Skipped compile for $mod"
        fi

        if [ ../jni/com_mousebird_maply_$mod.h -ot com/mousebird/maply/$mod.java ]; then
            header "$1" "$mod"
        else
            echo "Skipped header for $mod"
        fi
    fi
done

exit 0
