# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set (WGTARGET "whirlyglobemaply")
set (LOCALLIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../common/local_libs/")
set (COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../common/")
set (WGLIBANDROID "${CMAKE_CURRENT_SOURCE_DIR}/WhirlyGlobeLib/")
set (JNIDIR "${CMAKE_CURRENT_SOURCE_DIR}/jni/")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ${WGTARGET}

        # Sets the library as a shared library.
        SHARED

        ""
        )

set_target_properties(
        ${WGTARGET}

        PROPERTIES LINKER_LANGUAGE CXX
)

target_include_directories(
        ${WGTARGET}

        PUBLIC

        "${LOCALLIBS_DIR}/eigen/"
)

include("${LOCALLIBS_DIR}/proj-4/src/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/aaplus/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/protobuf/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/clipper/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/shapefile/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/glues/wgmaplyCMakeLists.txt")
include("${LOCALLIBS_DIR}/libjson/wgmaplyCMakeLists.txt")
include("${COMMON_DIR}/WhirlyGlobeLib/src/CMakeLists.txt")
include("${WGLIBANDROID}/src/CMakeLists.txt")
include("${JNIDIR}/CMakeLists.txt")

set_target_properties(
        ${WGTARGET}

        PROPERTIES COMPILE_FLAGS "-DHAVE_PTHREAD -DUSE_EIGEN_GEMM -DEIGEN_DONT_VECTORIZE -D__USE_SDL_GLES__ -D_REENTRANT -D_THREAD_SAFE -DUNORDERED -DLASZIPDLL_EXPORTS -DHAVE_PTHREAD=1 -Wno-undefined-var-template"
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${WGTARGET}

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}

        GLESv3 android EGL jnigraphics atomic m
        )
