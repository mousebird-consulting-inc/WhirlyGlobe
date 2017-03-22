keep_headers=(QuadImageTileLayer QuadImageOfflineLayer Texture)

for mod in AngleAxis AttrDictionary BaseInfo Billboard BillboardInfo BillboardManager ChangeSet CoordSystem CoordSystemDisplayAdapter DirectionalLight FakeGeocentricDisplayAdapter GeneralDisplayAdapter GeoCoordSystem GlobeView MapboxVectorTileParser GlobeViewState GlobeScene Identifiable InternalLabel InternalMarker LabelInfo LabelManager LayoutManager MapScene MapView MapViewState MarkerInfo MarkerManager MaplyRenderer Material Matrix3d Matrix4d Moon PlateCarreeCoordSystem ParticleSystemManager ParticleSystem ParticleBatch Proj4CoordSystem Point2d Point3d Point4d ScreenObject Shape ShapeInfo ShapeManager ShapeSphere SimplePoly SingleBillboardPoly StringWrapper QuadPagingLayer QuadImageTileLayer QuadImageOfflineLayer QuadTracker Quaternion Scene SelectionManager SelectedObject Shader SphericalMercatorCoordSystem Sticker StickerInfo StickerManager Sun Texture  VectorIterator VectorInfo VectorManager VectorObject View ViewState VertexAttribute
do
    finder=`echo ${keep_headers[*]} | grep "$mod"`

    if [ "${finder}" != "" ]; then
      echo Skip $mod
    else
      rm -rf ./jni/com_mousebird_maply_$mod*.h
    fi
done

exit 0
