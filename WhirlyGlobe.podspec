#
# Be sure to run `pod lib lint WhirlyGlobe.podspec' to ensure this is a
# valid spec and remove all comments before submitting the spec.
#
# Any lines starting with a # are optional, but encouraged
#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = "WhirlyGlobe"
  s.version          = "3.5"
  s.summary          = "WhirlyGlobe-Maply: Geospatial visualization for iOS and Android."
  s.description      = <<-DESC
                        WhirlyGlobe-Maply is a high performance geospatial display toolkit for iOS and Android.
                        The iOS version supports big, complex apps like Dark Sky and National Geographic World Atlas,
                        among others.  Even so, it's easy to get started on your own project.
                       DESC
  s.homepage         = "https://github.com/mousebird/WhirlyGlobe"
  s.license          = 'Apache 2.0'
  s.author           = { "Steve Gifford" => "contact@mousebirdconsulting.com" }
  s.social_media_url = 'https://twitter.com/@mousebirdc'
  s.platform         = :ios, '12.0'
  s.requires_arc     = true
  s.source           = { :git => 'https://github.com/mousebird/WhirlyGlobe.git', :branch => 'topic/xcframework' }
  s.module_name      = "WhirlyGlobe"
  s.default_subspec  = "WhirlyGlobe"
  s.compiler_flags   = ""

  s.pod_target_xcconfig = { 
    "DEFINES_MODULE" => "YES",
    "MTL_LANGUAGE_REVISION" => "Metal21",
    "GCC_PREPROCESSOR_DEFINITIONS" => %w(
      __USE_SDL_GLES__
      __IPHONEOS__
      SQLITE_OPEN_READONLY
      HAVE_PTHREAD=1
      LODEPNG_NO_COMPILE_ENCODER
      LODEPNG_NO_COMPILE_DISK
      LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
      LODEPNG_NO_COMPILE_ERROR_TEXT
      LODEPNG_NO_COMPILE_CRC
      LODEPNG_NO_COMPILE_CPP
      ).join(" "),
    "HEADER_SEARCH_PATHS" => %w(
      "$(SDKROOT)/usr/include/libxml2"
      "$(PODS_ROOT)/KissXML/KissXML/"
      "${PODS_TARGET_SRCROOT}/common/local_libs/eigen/"
      "${PODS_TARGET_SRCROOT}/common/local_libs/nanopb/"
      "${PODS_TARGET_SRCROOT}/common/local_libs/clipper"
      "${PODS_TARGET_SRCROOT}/common/local_libs/lodepng"
      "${PODS_TARGET_SRCROOT}/common/local_libs/glues/include/"
      "${PODS_TARGET_SRCROOT}/common/local_libs/GeographicLib/include/"
      "${PODS_TARGET_SRCROOT}/ios/library/WhirlyGlobe-MaplyComponent/include/private/"
      "${PODS_TARGET_SRCROOT}/ios/library/WhirlyGlobe-MaplyComponent/include/"
      "${PODS_TARGET_SRCROOT}/ios/library/WhirlyGlobe-MaplyComponent/include/vector_tiles/"
      ).join(" "),
    # For angle-bracket includes
    "SYSTEM_HEADER_SEARCH_PATHS" => %w(
      ).join(" "),
    "CLANG_WARN_DOCUMENTATION_COMMENTS" => "NO",
    "GCC_WARN_INHIBIT_ALL_WARNINGS" => "YES"
  }

  s.subspec 'locallibs' do |ll|
    ll.source_files =
        'common/local_libs/aaplus/**/*.{h,cpp}',
        'common/local_libs/clipper/cpp/*.{cpp,hpp}',
        'common/local_libs/shapefile/**/*.{c,h}',
        'common/local_libs/lodepng/*.{cpp,h}',
        'common/local_libs/nanopb/*.{c,h}',
        'common/local_libs/GeographicLib/src/*.cpp',
        'common/local_libs/GeographicLib/include/GeographicLib/*.{h,hpp}'
    ll.preserve_paths = 
        'common/local_libs/eigen/Eigen/**',
        'common/local_libs/lodepng/*.h',
        'common/local_libs/nanopb/*.h',
        'common/local_libs/GeographicLib/include/GeographicLib/*.{h,hpp}'
    ll.private_header_files =
        'common/local_libs/aaplus/**/*.h',
        'common/local_libs/clipper/cpp/*.hpp',
        'common/local_libs/shapefile/**/*.h',
        'common/local_libs/nanopb/*.h',
        'common/local_libs/GeographicLib/include/GeographicLib/*.{h,hpp}'
  end

  s.subspec 'glues' do |gl|
    gl.source_files = 'common/local_libs/glues/**/*.{cpp,h}'
    gl.private_header_files = 'common/local_libs/glues/**/*.h'
  end

  s.subspec 'WhirlyGlobe' do |mc|
    mc.source_files =
        'common/WhirlyGlobeLib/src/*.{c,cpp}',
        'common/WhirlyGlobeLib/include/*.h',
        'ios/library/WhirlyGlobeLib/src/*.{mm,m,cpp,metal}',
        'ios/library/WhirlyGlobeLib/include/*.h',
        'ios/library/WhirlyGlobe-MaplyComponent/include/**/*.h',
        'ios/library/WhirlyGlobe-MaplyComponent/src/**/*.{mm,m,cpp,metal}',
        'ios/library/WhirlyGlobe-MaplyComponent/WhirlyGlobeMaplyComponent/*.h'
    mc.exclude_files =
        'common/WhirlyGlobeLib/src/*GLES.{h,cpp}',
        'ios/library/WhirlyGlobeLib/src/Texture_iOS.mm',
        'ios/library/WhirlyGlobeLib/include/TextureGLES_iOS.h'
    mc.public_header_files =
        'ios/library/WhirlyGlobe-MaplyComponent/include/**/*.h',
        'ios/library/WhirlyGlobe-MaplyComponent/WhirlyGlobeMaplyComponent/*.h'
    mc.private_header_files =
        'common/WhirlyGlobeLib/include/**/*.h',
        'ios/library/WhirlyGlobeLib/include/*.h',
        'ios/library/WhirlyGlobe-MaplyComponent/include/private/*.h'
    mc.dependency 'WhirlyGlobe/locallibs'
    mc.dependency 'WhirlyGlobe/glues'
    mc.dependency 'SMCalloutView'
    mc.dependency 'FMDB'
    mc.dependency 'libjson'
    mc.dependency 'KissXML'
    mc.dependency 'proj4'
    mc.libraries = 'z', 'xml2', 'c++', 'sqlite3'
    mc.frameworks = 'CoreLocation', 'CoreServices', 'SystemConfiguration', 'CFNetwork', 'UIKit', 'Accelerate', 'MetalKit', 'MetalPerformanceShaders'
  end

end
