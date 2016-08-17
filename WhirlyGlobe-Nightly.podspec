#
# This is the podspec to be used for the toolkit's nightly builds.
#
# IMPORTANT:
# This version is not uploaded to official Cocoapods repository, so you have to
# download this file to your local drive and then add this line in your Podfile:
#
#        pod 'WhirlyGlobe', podspec: 'path/to/your/local/WhirlyGlobe-Nightly.podspec'
#
# For more information about the nightly builds, go to:
#    http://mousebird.github.io/WhirlyGlobe/builds/builds.html
#
Pod::Spec.new do |s|
  s.name             = "WhirlyGlobe"
  s.version          = "2.4.1-nightly"
  s.summary          = "WhirlyGlobe-Maply: Geospatial visualization for iOS and Android."
  s.description      = <<-DESC
                        WhirlyGlobe-Maply is a high performance geospatial display toolkit for iOS and Android.
                        The iOS version supports big, complex apps like Dark Sky and National Geographic World Atlas,
                        among others.  Even so, it's easy to get started on your own project.
                       DESC
  s.homepage         = "https://github.com/mousebird/WhirlyGlobe"
  s.screenshots      = "http://mousebird.github.io/WhirlyGlobe/images/carousel/home/mapandglobe.jpg", "http://mousebird.github.io/WhirlyGlobe/images/carousel/home/mtrainier.jpg"
  s.license          = 'Apache 2.0'
  s.author           = { "Steve Gifford" => "contact@mousebirdconsulting.com" }
  s.social_media_url = 'https://twitter.com/@mousebirdc'

  s.platform     = :ios, '7.0'
  s.requires_arc = true

  s.source = { :http => "https://s3-us-west-1.amazonaws.com/whirlyglobemaplydistribution/iOS_daily_builds/WhirlyGlobe-Maply_Nightly_latest.zip" }

  s.frameworks = 'CoreLocation'
  s.vendored_frameworks = 'WhirlyGlobeMaplyComponent.framework'

  s.xcconfig = {
    'USER_HEADER_SEARCH_PATHS' => '$(inherited) ${PODS_ROOT}/WhirlyGlobe/WhirlyGlobeMaplyComponent.framework/Headers/',
    'LIBRARY_SEARCH_PATHS' => '$(SRCROOT)/Pods/**',
    'SWIFT_OBJC_BRIDGING_HEADER' => "${PODS_ROOT}/WhirlyGlobe/WhirlyGlobeMaplyComponent.framework/Headers/MaplyBridge.h"
  }
  s.library = 'z', 'c++', 'xml2', 'sqlite3'

end