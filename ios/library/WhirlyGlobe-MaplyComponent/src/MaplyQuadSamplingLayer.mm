/*
 *  MaplyQuadSamplingLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/27/18.
 *  Copyright 2011-2018 Saildrone Inc
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

#import "MaplyQuadSamplingLayer.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyBaseViewController_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@interface MaplyQuadSamplingLayer() <WhirlyKitQuadDataStructure>
@end

@implementation MaplyQuadSamplingLayer
{
    MaplyCoordinateSystem *coordSys;
    MaplyBaseViewController * __weak viewC;
    WhirlyKit::Scene *scene;
    WhirlyKitQuadDisplayLayerNew *quadLayer;
    WhirlyKitSceneRendererES * __weak renderer;
    int minZoom,maxZoom;
    double import;
}

- (nullable instancetype)initWithCoordSystem:(MaplyCoordinateSystem *__nonnull)inCoordSys
{
    self = [super init];
    
    coordSys = inCoordSys;
    
    return self;
}

- (void)setMinZoom:(int)inMinZoom maxZoom:(int)inMaxZoom importance:(float)inImport
{
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
    import = inImport;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)inRenderer viewC:(MaplyBaseViewController *)inViewC
{
    viewC = inViewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    renderer = inRenderer;
    
    quadLayer = [[WhirlyKitQuadDisplayLayerNew alloc] initWithDataSource:self renderer:renderer];
    quadLayer.minImportance = import;
    [super.layerThread addLayer:quadLayer];
    
    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
}

- (WhirlyKit::CoordSystem *)coordSystem
{
    return [coordSys getCoordSystem];
}

- (WhirlyKit::Mbr)totalExtents
{
    MaplyCoordinate ll,ur;
    [coordSys getBoundsLL:&ll ur:&ur];
    
    Mbr mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y));
    return mbr;
}

- (WhirlyKit::Mbr)validExtents
{
    return [self totalExtents];
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

- (void)newViewState:(WhirlyKitViewState *)viewState
{
}

- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return MAXFLOAT;
    
    double import = ScreenImportance(viewState, frameSize, viewState.eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
    return import;
}

- (void)teardown
{
    quadLayer = nil;
}

@end
