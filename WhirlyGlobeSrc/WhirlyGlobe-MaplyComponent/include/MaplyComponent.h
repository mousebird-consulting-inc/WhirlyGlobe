/*
 *  MaplyComponent.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
 *  Copyright 2012-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "MaplyCoordinate.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyMatrix.h"
#import "MaplyVertexAttribute.h"
#import "MaplyTexture.h"
#import "MaplyLabel.h"
#import "MaplyScreenLabel.h"
#import "MaplyMarker.h"
#import "MaplyScreenMarker.h"
#import "MaplyShape.h"
#import "MaplySticker.h"
#import "MaplyBillboard.h"
#import "MaplyParticleSystem.h"
#import "MaplyVectorObject.h"
#import "MaplyViewTracker.h"
#import "MaplyViewController.h"
#import "MaplyQuadPagingLayer.h"
#import "MaplyUpdateLayer.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyQuadImageOfflineLayer.h"
#import "MaplyBlankTileSource.h"
#import "MaplyTileSource.h"
#import "MaplyWMSTileSource.h"
#import "MaplyMBTileSource.h"
#import "MaplyGDALRetileSource.h"
#import "MaplyRemoteTileSource.h"
#import "MaplyMultiplexTileSource.h"
#import "MaplyAnimationTestTileSource.h"
#import "MaplyPagingVectorTestTileSource.h"
#import "MaplyElevationSource.h"
#import "MaplyElevationDatabase.h"
#import "MaplyIconManager.h"
#import "MaplyTextureBuilder.h"
#import "MaplyGeomModel.h"
#import "MaplyVertexAttribute.h"
#import "MaplyQuadTracker.h"
#import "MaplyStarsModel.h"
#import "MaplySun.h"
#import "MaplyAtmosphere.h"
#import "MaplyMoon.h"
#import "MaplyRemoteTileElevationSource.h"

// These are not pulled in by the podspec
#ifdef NOTPODSPECWG
#import "vector_tiles/MaplyVectorTiles.h"
#import "vector_tiles/MaplyMapnikVectorTiles.h"
#import "vector_tiles/MapnikStyleSet.h"
#import "vector_tiles/MapboxMultiSourceTileInfo.h"
#import "vector_tiles/MapboxVectorStyleSet.h"
#import "vector_tiles/MapboxVectorStyleBackground.h"
#endif
