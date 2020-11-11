/*
 *  MaplyRenderTarget.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/13/17.
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

#import "MaplyRenderTarget_private.h"

using namespace WhirlyKit;

@implementation MaplyRenderTarget

- (id)init
{
    self = [super init];
    _renderTargetID = Identifiable::genId();
    _clearEveryFrame = true;
    _blend = false;
    _clearColor = nil;
    _clearVal = 0.0;
    _mipmapType = MaplyMipmapNone;
    _calculateMinMax = false;
    
    return self;
}

- (NSData *)getValueAtX:(int)x y:(int)y
{
    if (const auto __strong rc = _renderControl) {
        if (const auto sceneRenderer = dynamic_cast<SceneRendererMTL*>(rc->sceneRenderer.get())) {
            const auto dataRef = sceneRenderer->getSnapshotAt(_renderTargetID, x, y);
            if (const auto rawData = dynamic_cast<RawNSDataReader*>(dataRef.get())) {
                return rawData->getData();
            }
        }
    }
    return nil;
}

- (NSData *)getSnapshot
{
    if (const auto __strong rc = _renderControl) {
        if (const auto sceneRenderer = dynamic_cast<SceneRendererMTL*>(rc->sceneRenderer.get())) {
            const auto dataRef = sceneRenderer->getSnapshot(_renderTargetID);
            if (const auto rawData = dynamic_cast<RawNSDataReader*>(dataRef.get())) {
                return rawData->getData();
            }
        }
    }
    return nil;
}

- (NSData *)getMinMaxValues
{
    if (const auto __strong rc = _renderControl) {
        if (const auto sceneRenderer = dynamic_cast<SceneRendererMTL*>(rc->sceneRenderer.get())) {
            const auto dataRef = sceneRenderer->getSnapshotMinMax(_renderTargetID);
            if (const auto rawData = dynamic_cast<RawNSDataReader*>(dataRef.get())) {
                return rawData->getData();
            }
        }
    }
    return nil;
}

@end
