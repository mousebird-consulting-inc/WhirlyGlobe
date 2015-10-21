/*
 *  MaplyPoints_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/21/15
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

#import "MaplyPoints.h"
#import "MaplyParticleSystem_private.h"
#import <vector>

namespace WhirlyKit
{
    
class PointAttrData
{
public:
    std::string name;
    virtual ~PointAttrData() { }
};

class PointAttrDataInt : public PointAttrData
{
public:
    virtual ~PointAttrDataInt() { }
    std::vector<int> vals;
};
    
class PointAttrDataFloat : public PointAttrData
{
public:
    virtual ~PointAttrDataFloat() { }
    std::vector<float> vals;
};

class PointAttrDataPoint2f : public PointAttrData
{
public:
    virtual ~PointAttrDataPoint2f() { }
    std::vector<Point2f> vals;
};

class PointAttrDataPoint2d : public PointAttrData
{
public:
    virtual ~PointAttrDataPoint2d() { }
    std::vector<Point2d> vals;
};

class PointAttrDataPoint3f : public PointAttrData
{
public:
    virtual ~PointAttrDataPoint3f() { }
    std::vector<Point3f> vals;
};
    
class PointAttrDataPoint3d : public PointAttrData
{
public:
    virtual ~PointAttrDataPoint3d() { }
    std::vector<Point3d> vals;
};

class PointAttrDataPoint4f : public PointAttrData
{
public:
    virtual ~PointAttrDataPoint4f() { }
    std::vector<Eigen::Vector4f> vals;
};

}

@interface MaplyPoints()
{
@public
    bool coordsAreGeo;
    int coordIdx;
    int colorIdx;
    std::vector<WhirlyKit::PointAttrData *> attrData;
}

@end
