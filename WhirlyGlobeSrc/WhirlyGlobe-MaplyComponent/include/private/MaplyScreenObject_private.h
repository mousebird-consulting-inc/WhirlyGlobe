/*
 *  MaplyScreenObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 3/2/15
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

#import "MaplyScreenObject.h"
#import <vector>

namespace WhirlyKit
{
// Extremely simple polygon class
class SimplePoly
{
public:
    // Texture or UIImage
    id texture;
    UIColor *color;
    std::vector<Point2d> pts;
    std::vector<TexCoord> texCoords;
};

// Wraps strings with size and translation
class StringWrapper
{
public:
    StringWrapper() : mat(mat.Identity()), str(nil) { }
    
    Eigen::Matrix3d mat;
    CGSize size;
    NSAttributedString *str;
};
}

@interface MaplyScreenObject()
{
@public
    __weak MaplyBaseViewController *viewC;
    // 2D polygons
    std::vector<WhirlyKit::SimplePoly> polys;
    std::vector<WhirlyKit::StringWrapper> strings;
}
@end
