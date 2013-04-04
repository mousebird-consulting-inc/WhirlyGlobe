/*
 *  UpdateDisplayLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/19/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "GlobeScene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES.h"
#import "GlobeLayerViewWatcher.h"

/// Fill in this protocol to be called 
@protocol WhirlyGlobeUpdateDataSource <NSObject>

/// The update display layer will call this when the viewer has moved sufficiently
- (void)viewerDidUpdate:(WhirlyGlobeViewState *)viewState scene:(WhirlyKit::Scene *)scene;

/// Called when the layer needs to shutdown.  Clean up your objects.
- (void)shutdown;

@end

/** The update display layer takes a data source which it will notify when
    the user has moved sufficiently.  Presumably the data source is going to
    add and remove viewable objects as a result.
  */
@interface WhirlyGlobeUpdateDisplayLayer : NSObject<WhirlyKitLayer>
{
    /// Layer thread we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene, just for the data source
    WhirlyKit::Scene *scene;
    
    /// Distance we can move before triggering an update.
    /// The units are geocentric-like with a radius of 1.0
    float moveDist;
    
    /// Minimum time between updates (we don't want to trigger too often)
    float minTime;
    
    /// Last view state we were given
    WhirlyGlobeViewState *viewState;
    
    /// The data source that will be called when the viewer moves sufficiently.
    NSObject<WhirlyGlobeUpdateDataSource> *dataSource;
}

@property (nonatomic,assign) float moveDist;
@property (nonatomic,assign) float minTime;
@property (nonatomic,retain) NSObject<WhirlyGlobeUpdateDataSource> *dataSource;

/// Create with the input data source, the distance to move before triggering and the min trigger time
- (id)initWithDataSource:(NSObject<WhirlyGlobeUpdateDataSource> *)dataSource moveDist:(float)moveDist minTime:(float)minTime;

@end
