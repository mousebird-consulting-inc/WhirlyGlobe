cmake_minimum_required(VERSION 3.4.1)

target_include_directories(
        ${WGTARGET}

        PUBLIC

        "${CMAKE_CURRENT_LIST_DIR}/../include/"
)

set (WGLIBANDROIDINC "${CMAKE_CURRENT_LIST_DIR}/../include/")

target_sources(
        ${WGTARGET}

        PUBLIC

        "${WGLIBANDROIDINC}/ComponentManager_Android.h"
        "${WGLIBANDROIDINC}/Dictionary_Android.h"
        "${WGLIBANDROIDINC}/FontTextureManager_Android.h"
        "${WGLIBANDROIDINC}/GreatCircle_Android.h"
        "${WGLIBANDROIDINC}/ImageTile_Android.h"
        "${WGLIBANDROIDINC}/LabelInfo_Android.h"
        "${WGLIBANDROIDINC}/MapboxVectorTileParser_Android.h"
        "${WGLIBANDROIDINC}/MapboxVectorStyleSet_Android.h"
        "${WGLIBANDROIDINC}/ParticleBatch_Android.h"
        "${WGLIBANDROIDINC}/QuadImageFrameLoader_Android.h"
        "${WGLIBANDROIDINC}/QuadSamplingController_Android.h"
        "${WGLIBANDROIDINC}/SceneRenderer_Android.h"
        "${WGLIBANDROIDINC}/Shader_Android.h"
        "${WGLIBANDROIDINC}/SingleLabel_Android.h"
        "${WGLIBANDROIDINC}/VectorTileData_Android.h"

        "${CMAKE_CURRENT_LIST_DIR}/ComponentManager_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Dictionary_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FontTextureManager_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/GreatCircle_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ImageTile_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/LabelInfo_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MapboxVectorTileParser_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MapboxVectorStyleSet_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ParticleBatch_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/platform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/QuadImageFrameLoader_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/QuadSamplingController_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SceneRenderer_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Shader_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SingleLabel_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/VectorTileData_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/WhirlyKitLog.cpp"
)
