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
  s.version          = "3.0"
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

  s.platform     = :ios, '9.0'
  s.requires_arc = true

  s.source = { :git => 'https://github.com/mousebird/WhirlyGlobe.git', :branch => 'develop' }

  s.compiler_flags = '-D__USE_SDL_GLES__ -D__IPHONEOS__ -DSQLITE_OPEN_READONLY -DHAVE_PTHREAD=1 -DUNORDERED=1 -DLASZIPDLL_EXPORTS=1'
  s.xcconfig = { "HEADER_SEARCH_PATHS" => " \"$(PODS_ROOT)/eigen/\" \"$(PODS_ROOT)/KissXML/KissXML/\" \"${PODS_ROOT}/WhirlyGlobe/common/local_libs/protobuf/src/\" \"${PODS_ROOT}/WhirlyGlobe/common/local_libs/clipper\" \"$(SDKROOT)/usr/include/libxml2\" \"${PODS_ROOT}/WhirlyGlobe/common/local_libs/glues/include/\" \"$(PODS_ROOT)/WhirlyGlobe/ios/library/WhirlyGlobe-MaplyComponent/include/private/\" \"$(PODS_ROOT)/WhirlyGlobe/ios/library/WhirlyGlobe-MaplyComponent/include/\" \"$(PODS_ROOT)/WhirlyGlobe/ios/library/WhirlyGlobe-MaplyComponent/include/vector_tiles/\" " }

  s.default_subspec = 'MaplyComponent'

  s.subspec 'locallibs' do |ll|
    ll.source_files = 'common/local_libs/aaplus/**/*.{h,cpp}',
        'common/local_libs/clipper/cpp/*.{cpp,hpp}',
        'common/local_libs/laszip/include/laszip/*.h', 'common/local_libs/laszip/src/*.{cpp,hpp}',
        'common/local_libs/protobuf/src/google/protobuf/*.{cc,h}', 'common/local_libs/protobuf/src/google/protobuf/stubs/*.{cc,h}', 'common/local_libs/protobuf/src/google/protobuf/io/*.{cc,h}', 'common/local_libs/protobuf/src/google/protobuf/testing/*.h',
        'common/local_libs/shapefile/**/*.{c,h}'
    ll.preserve_paths = 'common/local_libs/protobuf/src/google/protobuf/*.h', 'common/local_libs/protobuf/src/google/protobuf/stubs/*.h', 'common/local_libs/protobuf/src/google/protobuf/io/*.h',
        'common/local_libs/laszip/include/laszip/*.h', 'common/local_libs/laszip/src/*.hpp'
    ll.private_header_files = 'common/local_libs/aaplus/**/*.h',
        'common/local_libs/clipper/cpp/*.hpp',
        'common/local_libs/laszip/include/laszip/*.h','common/local_libs/laszip/src/*.hpp',
        'common/local_libs/protobuf/src/google/protobuf/*.h', 'common/local_libs/protobuf/src/google/protobuf/stubs/*.h', 'common/local_libs/protobuf/src/google/protobuf/io/*.h', 'common/local_libs/protobuf/src/google/protobuf/testing/*.h',
        'common/local_libs/shapefile/**/*.h'
  end

  s.subspec 'glues' do |gl|
    gl.source_files = 'common/local_libs/glues/**/*.{c,h}'
    gl.preserve_paths = 'common/local_libs/glues/**/*.i'
    gl.private_header_files = 'common/local_libs/glues/**/*.h'
  end

  s.subspec 'MaplyComponent' do |mc|
    mc.source_files = 'common/WhirlyGlobeLib/include/*.h', 'common/WhirlyGlobeLib/src/*.cpp', 'ios/library/WhirlyGlobeLib/src/*.{mm,m}', 'ios/library/WhirlyGlobeLib/include/*.h', 'ios/library/WhirlyGlobe-MaplyComponent/include/**/*.h', 'ios/library/WhirlyGlobe-MaplyComponent/src/**/*.{mm,m,cpp}'
    mc.public_header_files = 'ios/library/WhirlyGlobe-MaplyComponent/include/*.h', "ios/library/WhirlyGlobe-MaplyComponent/include/vector_tiles/*.h"
    mc.private_header_files = 'ios/library/WhirlyGlobeLib/include/*.h', 'ios/**/vector_tile.pb.h', 'ios/**/MaplyBridge.h'
    mc.dependency 'WhirlyGlobe/locallibs'
    mc.dependency 'WhirlyGlobe/glues'
    mc.dependency 'SMCalloutView'
    mc.dependency 'FMDB'
    mc.dependency 'libjson'
    mc.dependency 'KissXML'
    mc.dependency 'eigen'
    mc.dependency 'proj4'
    mc.libraries = 'z', 'xml2', 'c++', 'sqlite3'
    mc.frameworks = 'CoreLocation', 'MobileCoreServices', 'SystemConfiguration', 'CFNetwork', 'UIKit', 'OpenGLES', 'Accelerate', 'MetalKit'
  end

end
