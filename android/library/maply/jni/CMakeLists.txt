cmake_minimum_required(VERSION 3.4.1)

target_include_directories(
        ${WGTARGET}

        PUBLIC

        "${CMAKE_CURRENT_LIST_DIR}/include/"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/"
        "${CMAKE_CURRENT_LIST_DIR}/include/generated/"
)

target_sources(
        ${WGTARGET}

        PUBLIC

        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Astronomy_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Base_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Billboard_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Components_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/CoordSystem_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Formats_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Geometry_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/GeometryManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/LabelsAndMarkers_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Maply_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Particles_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/QuadLoading_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Scene_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Selection_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Shapes_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Stickers_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Vectors_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/View_jni.h"

        "${CMAKE_CURRENT_LIST_DIR}/src/jni_util/Maply_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/math/AngleAxis_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Matrix3d_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Matrix4d_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Point2d_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Point3d_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Point4d_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/math/Quaternion_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/CoordSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/CoordSystemDisplayAdapter_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/FakeGeocentricDisplayAdapter_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/GeneralDisplayAdapter_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/GeoCoordSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/PassThroughCoordSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/PlateCarreeCoordSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/Proj4CoordSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/coordSystem/SphericalMercatorCoordSystem_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/scene/ChangeSet_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/scene/DirectionalLight_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/scene/Material_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/scene/Scene_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/scene/Shader_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/scene/Texture_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/view/FlatView_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/GlobeView_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/GlobeViewState_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/MapView_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/MapViewState_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/View_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/view/ViewState_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/base/Identifiable_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/base/BaseInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/base/VertexAttribute_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/Billboard_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/BillboardInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/BillboardManager_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/ScreenObject_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/StringWrapper_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/billboard/SimplePoly_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/geometryManager/GeometryInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/geometryManager/GeometryInstance_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/geometryManager/GeometryManager_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/geometryManager/GeometryRaw_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/geometryManager/GeometryRawPoints_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/MarkerInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/InternalMarker_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/MarkerManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/LabelInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/InternalLabel_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/labelsMarkers/LabelManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/particles/ParticleBatch_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/particles/ParticleSystem_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/particles/ParticleSystemManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/layoutSelection/LayoutManager_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/layoutSelection/SelectedObject_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/layoutSelection/SelectionManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/Shape_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeCircle_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeCylinder_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeExtruded_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeGreatCircle_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeLinear_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeRectangle_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeSphere_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/shapes/ShapeManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/stickers/Sticker_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/stickers/StickerInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/stickers/StickerManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/AttrDictionary_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/AttrDictionaryEntry_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/VectorObject_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/VectorIterator_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/VectorInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/VectorManager_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/WideVectorInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/WideVectorManager_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/LoftedPolyInfo_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/vectors/LoftedPolyManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/astronomy/Moon_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/astronomy/Sun_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/components/ComponentObject_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/components/ComponentManager_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/SamplingParams_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/LoaderReturn_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/ImageTile_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/ImageLoaderReturn_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/ObjectLoaderReturn_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QIFBatchOps_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QIFFrameAsset_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QuadLoaderBase_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QuadImageLoaderBase_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QuadImageFrameLoader_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/quadLoading/QuadSamplingLayer_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/renderer/RenderController_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/formats/MapboxVectorTileParser_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/formats/MapboxVectorStyleSet_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/formats/VectorStyleSettings_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/formats/VectorTileData_jni.cpp"
)
