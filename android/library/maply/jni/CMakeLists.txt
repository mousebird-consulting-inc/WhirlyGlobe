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
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/BaseInfo_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/BillboardManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/CoordSystem_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Formats_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Geometry_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/GeometryManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/LabelManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Maply_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/MarkerManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/ParticleManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Scene_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/Selection_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/ShapeManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/SphericalChunk_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/VectorManager_jni.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/classInfo/View_jni.h"

        "${CMAKE_CURRENT_LIST_DIR}/src/jni_util/Maply_jni.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/src/util/AttrDictionary_jni.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/util/Identifiable_jni.cpp"
)
