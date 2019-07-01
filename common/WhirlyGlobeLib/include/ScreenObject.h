/*
 *  ScreenObject.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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

#import "WhirlyVector.h"
#import "Identifiable.h"

namespace WhirlyKit {
    
class SimplePoly {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SimplePoly();
    virtual ~SimplePoly();

    SimpleIdentity texID;
    RGBAColor color;
    Point2dVector pts;
    std::vector<TexCoord> texCoords;
};
    
typedef std::shared_ptr<SimplePoly> SimplePolyRef;

class StringWrapper
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    StringWrapper();
    virtual ~StringWrapper();
    
    Eigen::Matrix3d mat;
    Point2d size;
};
    
typedef std::shared_ptr<StringWrapper> StringWrapperRef;

// Screen objects are collections of polygons and strings we turn into geometry
class ScreenObject {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    ScreenObject();
    ~ScreenObject();

    std::vector<WhirlyKit::SimplePolyRef> polys;
    std::vector<WhirlyKit::StringWrapperRef> strings;
};

}
