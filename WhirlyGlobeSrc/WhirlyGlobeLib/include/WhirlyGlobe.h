/*
 *  WhirlyGlobe.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/12/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "Platform.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "Quadtree.h"
#import "WhirlyKitView.h"
#import "GlobeView.h"
//#import "AnimateRotation.h"
//#import "AnimateViewMomentum.h"
#import "GlobeScene.h"
//#import "SphericalEarthLayer.h"
//#import "GridLines.h"
//#import "TextureGroup.h"
#import "SceneRendererES.h"
#import "SceneRendererES2.h"
#import "SceneRendererES3.h"
//#import "EAGLView.h"
//#import "PinchDelegate.h"
//#import "SwipeDelegate.h"
//#import "PanDelegate.h"
//#import "TapDelegate.h"
//#import "LongPressDelegate.h"
//#import "RotateDelegate.h"
//#import "LayerThread.h"
#import "VectorData.h"
#import "FontTextureManager.h"
#import "LabelManager.h"
#import "SharedAttributes.h"
//#import "VectorDatabase.h"
#import "ShapeReader.h"
#import "VectorManager.h"
//#import "VectorLayer.h"
//#import "LabelLayer.h"
//#import "ParticleSystemLayer.h"
#import "MarkerManager.h"
//#import "LoftLayer.h"
#import "SelectionManager.h"
#import "TextureAtlas.h"
//#import "LayerThread.h"
//#import "BigDrawable.h"
#import "FlatMath.h"
#import "SphericalMercator.h"
#import "MaplyView.h"
#import "MaplyFlatView.h"
#import "ViewState.h"
#import "GlobeViewState.h"
#import "MaplyViewState.h"
//#import "MaplyAnimateFlat.h"
//#import "MaplyPinchDelegate.h"
//#import "MaplyPanDelegate.h"
//#import "MaplyAnimateTranslation.h"
//#import "MaplyTapDelegate.h"
//#import "MaplyRotateDelegate.h"
#import "QuadDisplayController.h"
#import "TileQuadLoader.h"
//#import "MBTileQuadSource.h"
#import "TileQuadOfflineRenderer.h"
//#import "NetworkTileQuadSource.h"
#import "ScreenSpaceGenerator.h"
//#import "SceneGraphManager.h"
//#import "SphericalEarthChunkLayer.h"
#import "SphericalEarthChunkManager.h"
//#import "SphericalEarthQuadLayer.h"
//#import "UpdateDisplayLayer.h"
//#import "GeometryLayer.h"
//#import "ViewPlacementGenerator.h"
//#import "ActiveModel.h"
#import "MaplyScene.h"
//#import "ShapeDrawableBuilder.h"
//#import "ShapeLayer.h"
//#import "LayoutLayer.h"
//#import "BillboardLayer.h"
#import "OpenGLES2Program.h"
#import "DefaultShaderPrograms.h"
#import "Tesselator.h"
#import "GridClipper.h"
#import "GLUtils.h"
#import "VectorObject.h"
#import "ParticleSystemManager.h"
#import "ParticleSystemDrawable.h"
#import "WhirlyKitLogs.h"


