/*
 *  MaplyWideVectorInfo_iOS_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/18/19.
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

#import "MaplyBaseInfo_iOS_private.h"
#import "MaplyWideVectorInfo_iOS_private.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"

namespace WhirlyKit {
    
WideVectorInfo_iOS::WideVectorInfo_iOS(NSDictionary *desc)
{
    BaseInfoSetup(*this,desc);
    
    UIColor *theColor = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    if (theColor)
        color = [theColor asRGBAColor];

    width = [desc floatForKey:@"width" default:2.0];
    coordType = (WhirlyKit::WideVectorCoordsType)[desc enumForKey:@"wideveccoordtype" values:@[@"real",@"screen"] default:WideVecCoordScreen];
    joinType = (WhirlyKit::WideVectorLineJoinType)[desc enumForKey:@"wideveclinejointype" values:@[@"miter",@"round",@"bevel"] default:WideVecMiterJoin];
    capType = (WhirlyKit::WideVectorLineCapType)[desc enumForKey:@"wideveclinecaptype" values:@[@"butt",@"round",@"square"] default:WideVecButtCap];
    texID = [desc intForKey:@"texture" default:EmptyIdentity];
    repeatSize = [desc floatForKey:@"repeatSize" default:(coordType == WideVecCoordScreen ? 32 : 6371000.0 / 20)];
    edgeSize = [desc floatForKey:@"edgefalloff" default:1.0];
    miterLimit = [desc floatForKey:@"miterLimit" default:2.0];
}

}
