cmake_minimum_required(VERSION 3.4.1)

target_include_directories(
        ${WGTARGET}

        PUBLIC

        "${CMAKE_CURRENT_LIST_DIR}/../include/"
)

target_sources(
        ${WGTARGET}

        PUBLIC

        "${CMAKE_CURRENT_LIST_DIR}/ComponentManager_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Dictionary_Android.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/platform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/WhirlyKitLog.cpp"
)
