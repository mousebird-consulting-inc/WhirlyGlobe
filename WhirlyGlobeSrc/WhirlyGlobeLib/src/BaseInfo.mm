/*
 *  BaseInfo.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "BaseInfo.h"
#import "NSDictionary+Stuff.h"
#import "Drawable.h"
#import "BasicDrawable.h"
#import "BasicDrawableInstance.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation WhirlyKitBaseInfo

- (id)initWithDesc:(NSDictionary *)desc
{
    self = [super init];
    
    _minVis = [desc doubleForKey:@"minVis" default:DrawVisibleInvalid];
    _maxVis = [desc doubleForKey:@"maxVis" default:DrawVisibleInvalid];
    _minViewerDist = [desc doubleForKey:@"minviewerdist" default:DrawVisibleInvalid];
    _maxViewerDist = [desc doubleForKey:@"maxviewerdist" default:DrawVisibleInvalid];
    _viewerCenter.x() = [desc doubleForKey:@"viewablecenterx" default:DrawVisibleInvalid];
    _viewerCenter.y() = [desc doubleForKey:@"viewablecentery" default:DrawVisibleInvalid];
    _viewerCenter.z() = [desc doubleForKey:@"viewablecenterz" default:DrawVisibleInvalid];
    _fade = [desc doubleForKey:@"fade" default:0.0];
    _fadeIn = _fade;
    _fadeOut = _fade;
    _fadeIn = [desc doubleForKey:@"fadein" default:_fadeIn];
    _fadeOut = [desc doubleForKey:@"fadeout" default:_fadeOut];
    _fadeOutTime = [desc doubleForKey:@"fadeouttime" default:0.0];
    _drawPriority = [desc intForKey:@"priority" default:0];
    _drawPriority = [desc intForKey:@"drawPriority" default:_drawPriority];
    _drawOffset = [desc doubleForKey:@"drawOffset" default:0.0];
    _enable = [desc boolForKey:@"enable" default:true];
    _startEnable = [desc doubleForKey:@"enablestart" default:0.0];
    _endEnable = [desc doubleForKey:@"enableend" default:0.0];
    SimpleIdentity shaderID = [desc intForKey:@"shader" default:EmptyIdentity];
    _programID = [desc intForKey:@"program" default:shaderID];
    
    return self;
}

- (void)setupBasicDrawable:(BasicDrawable *)drawable
{
    drawable->setOnOff(_enable);
    drawable->setEnableTimeRange(_startEnable, _endEnable);
    drawable->setDrawPriority(_drawPriority);
    drawable->setVisibleRange(_minVis,_maxVis);
    drawable->setViewerVisibility(_minViewerDist,_maxViewerDist,_viewerCenter);
}

- (void)setupBasicDrawableInstance:(WhirlyKit::BasicDrawableInstance *)drawInst
{
    drawInst->setEnable(_enable);
    drawInst->setEnableTimeRange(_startEnable, _endEnable);
    drawInst->setDrawPriority(_drawPriority);
    drawInst->setVisibleRange(_minVis,_maxVis);
    drawInst->setViewerVisibility(_minViewerDist,_maxViewerDist,_viewerCenter);
}

@end
