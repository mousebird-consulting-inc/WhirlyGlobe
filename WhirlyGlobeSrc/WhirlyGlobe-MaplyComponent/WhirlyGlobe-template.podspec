# ******************************************************************************
#
# IMPORTANT:
#
# This podspec is intended to be used for the toolkit's nightly build 
# distribution. It isn't uploaded to official Cocoapods repository, so you have 
# to use as follows:
#
#        pod 'WhirlyGlobe', :http => 'URL_TO_NIGHTLY_BUILD.zip'
#
# For more information about the nightly builds, go to:
#    http://mousebird.github.io/WhirlyGlobe/builds/builds.html
#
# ******************************************************************************
Pod::Spec.new do |s|
  s.name             = "WhirlyGlobe"
  s.version          = "2.4.1-{{BUILD_TAG}}"
  s.summary          = "WhirlyGlobe-Maply: Geospatial visualization for iOS and Android."
  s.description      = <<-DESC
                        IMPORTANT: This is an early adopter version. Use it at your own risk.
                       DESC
  s.homepage         = "http://mousebird.github.io/WhirlyGlobe/builds/builds.html"
  s.screenshots      = "http://mousebird.github.io/WhirlyGlobe/images/carousel/home/mapandglobe.jpg", "http://mousebird.github.io/WhirlyGlobe/images/carousel/home/mtrainier.jpg"
  s.license          = 'Apache 2.0'
  s.author           = { "Steve Gifford" => "contact@mousebirdconsulting.com" }
  s.social_media_url = 'https://twitter.com/@mousebirdc'

  s.platform     = :ios, '7.0'
  s.requires_arc = true
  
  s.source = {
    :path => "./"
  }

  s.frameworks = 'CoreLocation'
  s.vendored_frameworks = 'WhirlyGlobeMaplyComponent.framework'

  s.xcconfig = {
    'USER_HEADER_SEARCH_PATHS' => '$(inherited) ${PODS_ROOT}/WhirlyGlobe/WhirlyGlobeMaplyComponent.framework/Headers/',
    'LIBRARY_SEARCH_PATHS' => '$(SRCROOT)/Pods/**',
    'SWIFT_OBJC_BRIDGING_HEADER' => "${PODS_ROOT}/WhirlyGlobe/WhirlyGlobeMaplyComponent.framework/Headers/MaplyBridge.h"
  }
  s.library = 'z', 'c++', 'xml2', 'sqlite3'

end

