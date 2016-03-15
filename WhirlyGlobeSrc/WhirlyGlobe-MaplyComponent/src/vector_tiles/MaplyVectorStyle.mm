/*
 *  MaplyVectorStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import "MaplyVectorStyle.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyVectorStyleSettings

- (instancetype)init
{
    self = [super init];
    _lineScale = 1.0;
    _textScale = 1.0;
    _markerScale = 1.0;
    _markerImportance = 2.0;
    _markerSize = 10.0;
    _mapScaleScale = 1.0;
    _dashPatternScale = 1.0;
    _useWideVectors = false;
    _wideVecCuttoff = 0.0;
    _oldVecWidthScale = 1.0;
    _selectable = false;
  
    return self;
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"%@: lineScale:%f textScale:%f markerScale:%f mapScaleScale:%f",
          [[self class] description], _lineScale, _textScale, _markerScale, _mapScaleScale];
}

@end
