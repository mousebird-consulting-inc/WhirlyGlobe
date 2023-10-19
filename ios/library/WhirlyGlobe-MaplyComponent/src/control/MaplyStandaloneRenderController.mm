/*
*  MaplyStandaloneRenderController.cpp
*  WhirlyGlobe
*
*  Created by Steve Gifford on 10/19/23.
*  Copyright 2011-2022 mousebird consulting
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

#import "MaplyGlobeRenderController.h"
#import "MaplyRenderController_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyStandaloneRenderController.h"
#import "SceneRendererMTL.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@implementation MaplyStandaloneRenderController
{
    Maply::FlatViewRef flatView;
    CoordSystemRef coordSys;
    CoordSystemDisplayAdapterRef coordAdapter;
}

- (instancetype __nullable)initWithSize:(CGSize)size
{
    // For now we just use Plate Carree and cover the whole extents
    const auto originLon = 0.0;
    const auto ll = GeoCoordD::CoordFromDegrees(-180.0,-90.0);
    const auto ur = GeoCoordD::CoordFromDegrees(180.0,90.0);

    coordSys = std::make_shared<PlateCarreeCoordSystem>();
    Point3d scale(1.0,1.0,1.0);
    Point3d center(0.0,0.0,0.0);
    coordAdapter = std::make_shared<GeneralCoordSystemDisplayAdapter>(coordSys.get(),
                                                                      Point3d(ll.x(),ll.y(),0.0),
                                                                      Point3d(ur.x(),ur.y(),0.0),
                                                                      center,
                                                                      scale);
    
    flatView = std::make_shared<FlatView>(coordAdapter.get());
    visualView = flatView;
    
    self = [super initWithSize:size mode:MaplyRenderMetal];
    [self resetLights];

    return self;
}

- (BOOL)renderTo:(id<MTLTexture>)texture period:(NSTimeInterval)howLong
{
    SceneRendererMTL::RenderInfoMTL renderInfo;
    
    MTLRenderPassDescriptor *desc = [[MTLRenderPassDescriptor alloc] init];
    desc.colorAttachments[0].texture = texture;
    desc.colorAttachments[0].loadAction = MTLLoadActionClear;
    desc.colorAttachments[0].clearColor = MTLClearColorMake(1.0,0.0,0.0,1.0);
    desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    if (!sceneRenderer)
        return false;

    SceneRendererMTLRef sceneRendererMTL = std::dynamic_pointer_cast<SceneRendererMTL>(sceneRenderer);
    
    sceneRendererMTL->forceDrawNextFrame();
    sceneRendererMTL->render(howLong,&renderInfo);
    
    return true;
}

@end

