//
//  WhirlyGlobeMaplyComponent.h
//  WhirlyGlobeMaplyComponent
//
//  Created by Steve Gifford on 6/29/16.
//  Copyright © 2016-2019 mousebird consulting.
//

#import <UIKit/UIKit.h>

//! Project version number for WhirlyGlobeMaplyComponent.
FOUNDATION_EXPORT double WhirlyGlobeMaplyComponentVersionNumber;

//! Project version string for WhirlyGlobeMaplyComponent.
FOUNDATION_EXPORT const unsigned char WhirlyGlobeMaplyComponentVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <WhirlyGlobeMaplyComponent/PublicHeader.h>

#if !MAPLY_MINIMAL
# import <WhirlyGlobe/GeographicLib_ObjC.h>
# import <WhirlyGlobe/GeoJSONSource.h>
# import <WhirlyGlobe/GlobeDoubleTapDelegate.h>
# import <WhirlyGlobe/GlobeDoubleTapDragDelegate.h>
# import <WhirlyGlobe/GlobePanDelegate.h>
# import <WhirlyGlobe/GlobePinchDelegate.h>
# import <WhirlyGlobe/GlobeRotateDelegate.h>
# import <WhirlyGlobe/GlobeTapDelegate.h>
# import <WhirlyGlobe/GlobeTiltDelegate.h>
# import <WhirlyGlobe/GlobeTwoFingerTapDelegate.h>
# import <WhirlyGlobe/MapboxVectorInterpreter.h>
# import <WhirlyGlobe/MapboxVectorStyleSet.h>
# import <WhirlyGlobe/MapboxVectorTiles.h>
# import <WhirlyGlobe/Maply3DTouchPreviewDatasource.h>
# import <WhirlyGlobe/Maply3dTouchPreviewDelegate.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyActiveObject.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyAnnotation.h>
# import <WhirlyGlobe/MaplyAtmosphere.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyBaseViewController.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyBillboard.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyBridge.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyCluster.h>
# import <WhirlyGlobe/MaplyColorRampGenerator.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyComponent.h>
#import <WhirlyGlobe/MaplyComponentObject.h>
#import <WhirlyGlobe/MaplyControllerLayer.h>
#import <WhirlyGlobe/MaplyCoordinate.h>
#import <WhirlyGlobe/MaplyCoordinateSystem.h>
#import <WhirlyGlobe/MaplyDoubleTapDelegate.h>
#import <WhirlyGlobe/MaplyDoubleTapDragDelegate.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyGeomBuilder.h>
# import <WhirlyGlobe/MaplyGeomModel.h>
# import <WhirlyGlobe/MaplyGlobeRenderController.h>
# import <WhirlyGlobe/MaplyIconManager.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyImageTile.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyLabel.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyLight.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyLocationTracker.h>
# import <WhirlyGlobe/MaplyMarker.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyMatrix.h>
#import <WhirlyGlobe/MaplyMBTileFetcher.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyMoon.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyPanDelegate.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyParticleSystem.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyPinchDelegate.h>
#import <WhirlyGlobe/MaplyPoints.h>
#import <WhirlyGlobe/MaplyQuadImageFrameLoader.h>
#import <WhirlyGlobe/MaplyQuadImageLoader.h>
#import <WhirlyGlobe/MaplyQuadLoader.h>
#import <WhirlyGlobe/MaplyQuadPagingLoader.h>
#import <WhirlyGlobe/MaplyQuadSampler.h>
#import <WhirlyGlobe/MaplyRemoteTileFetcher.h>
#import <WhirlyGlobe/MaplyRenderController.h>
#import <WhirlyGlobe/MaplyRenderTarget.h>
#import <WhirlyGlobe/MaplyRotateDelegate.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyScreenLabel.h>
# import <WhirlyGlobe/MaplyScreenMarker.h>
# import <WhirlyGlobe/MaplyScreenObject.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyShader.h>
#import <WhirlyGlobe/MaplyShape.h>
#import <WhirlyGlobe/MaplySharedAttributes.h>
#import <WhirlyGlobe/MaplySimpleTileFetcher.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyStarsModel.h>
# import <WhirlyGlobe/MaplySticker.h>
# import <WhirlyGlobe/MaplySun.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyTapDelegate.h>
#import <WhirlyGlobe/MaplyTapMessage.h>
#import <WhirlyGlobe/MaplyTexture.h>
#import <WhirlyGlobe/MaplyTextureBuilder.h>
#import <WhirlyGlobe/MaplyTileSourceNew.h>
#import <WhirlyGlobe/MaplyTouchCancelAnimationDelegate.h>
#import <WhirlyGlobe/MaplyTwoFingerTapDelegate.h>
#import <WhirlyGlobe/MaplyUpdateLayer.h>
#import <WhirlyGlobe/MaplyVariableTarget.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyVectorObject.h>
# import <WhirlyGlobe/MaplyVectorStyle.h>
# import <WhirlyGlobe/MaplyVectorStyleSimple.h>
# import <WhirlyGlobe/MaplyVectorTileLineStyle.h>
# import <WhirlyGlobe/MaplyVectorTileMarkerStyle.h>
# import <WhirlyGlobe/MaplyVectorTilePolygonStyle.h>
# import <WhirlyGlobe/MaplyVectorTileStyle.h>
# import <WhirlyGlobe/MaplyVectorTileTextStyle.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyVertexAttribute.h>
#import <WhirlyGlobe/MaplyViewController.h>
#import <WhirlyGlobe/MaplyViewTracker.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MaplyWMSTileSource.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/MaplyZoomGestureDelegate.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/MapnikStyle.h>
# import <WhirlyGlobe/MapnikStyleRule.h>
# import <WhirlyGlobe/MapnikStyleSet.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/NSData+Zlib.h>
#import <WhirlyGlobe/NSDictionary+StyleRules.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/SLDExpressions.h>
# import <WhirlyGlobe/SLDOperators.h>
# import <WhirlyGlobe/SLDStyleSet.h>
# import <WhirlyGlobe/SLDSymbolizers.h>
# import <WhirlyGlobe/SLDWellKnownMarkers.h>
#endif //!MAPLY_MINIMAL
#import <WhirlyGlobe/WGCoordinate.h>
#if !MAPLY_MINIMAL
# import <WhirlyGlobe/WhirlyGlobeComponent.h>
# import <WhirlyGlobe/WhirlyGlobeComponent.h>
# import <WhirlyGlobe/WhirlyGlobeViewController.h>
# import <WhirlyGlobe/MaplyURLSessionManager.h>
#endif //!MAPLY_MINIMAL
