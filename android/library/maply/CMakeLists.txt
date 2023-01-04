# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)
project(Maply)

set (WGTARGET "whirlyglobemaply")
set (LOCALLIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../common/local_libs/")
set (COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../common/")
set (WGLIBANDROID "${CMAKE_CURRENT_SOURCE_DIR}/WhirlyGlobeLib/")
set (JNIDIR "${CMAKE_CURRENT_SOURCE_DIR}/jni/")

if(NOT WG_LIB_TYPE)
    set(WG_LIB_TYPE SHARED)
endif()

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library(${WGTARGET} ${WG_LIB_TYPE} "")

set_target_properties(${WGTARGET} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${WGTARGET} PUBLIC "${LOCALLIBS_DIR}/eigen/")

include("${LOCALLIBS_DIR}/proj-4/src/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/aaplus/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/clipper/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/nanopb/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/shapefile/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/glues/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/libjson/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/lodepng/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/GeographicLib/wgmaplyCMakeLists.txt")
include("${COMMON_DIR}/WhirlyGlobeLib/src/CMakeLists.txt")

if (NOT WG_NO_ANDROID_DEPS)
    include("${WGLIBANDROID}/src/CMakeLists.txt")
    include("${JNIDIR}/CMakeLists.txt")
endif()

set_target_properties( ${WGTARGET} PROPERTIES
    COMPILE_FLAGS "-DHAVE_PTHREAD -DUSE_EIGEN_GEMM -DEIGEN_DONT_VECTORIZE -D_REENTRANT -D_THREAD_SAFE -DLASZIPDLL_EXPORTS -DHAVE_PTHREAD=1 -Wno-undefined-var-template")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

if (NOT WG_NO_ANDROID_DEPS)
    find_library(log-lib log)
endif()

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

if (WG_NO_ANDROID_DEPS)
    target_link_libraries(${WGTARGET})
else()
    target_link_libraries(${WGTARGET} ${log-lib} android jnigraphics EGL GLESv3 atomic m)
endif()

