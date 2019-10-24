/*
*  MaplyGlobeRenderController.mm
*  WhirlyGlobeComponent
*
*  Created by Steve Gifford on 10/23/10.
*  Copyright 2011-2019 mousebird consulting
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

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeRenderController
{
    WhirlyGlobe::GlobeView_iOSRef globeView;
}

- (instancetype __nullable)initWithSize:(CGSize)size
{
    return [self initWithSize:size mode:MaplyRenderMetal];
}

- (instancetype) initWithSize:(CGSize)screenSize mode:(MaplyRenderType)renderType
{
    globeView = GlobeView_iOSRef(new GlobeView_iOS());
    globeView->continuousZoom = true;
    visualView = globeView;
    coordAdapter = globeView->coordAdapter;
    
    self = [super initWithSize:screenSize mode:renderType];
    [self resetLights];

    return self;
}

- (NSTimeInterval)currentTime
{
    return scene->getCurrentTime();
}

- (void)setCurrentTime:(NSTimeInterval)currentTime
{
    scene->setCurrentTime(currentTime);
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *__nonnull)viewState
{
    // TODO: Fill this in
}

- (WhirlyGlobeViewControllerAnimationState *)getViewState
{
    // TODO: Fill this in
    return nil;
}

- (UIImage *__nullable)snapshot
{
    // TODO: Fill this in
    return nil;
}

/// Return a tile fetcher we may share between loaders
- (MaplyRemoteTileFetcher * __nonnull)addTileFetcher:(NSString * __nonnull)name
{
    // TODO: Fill this in
    return nil;
}

@end
