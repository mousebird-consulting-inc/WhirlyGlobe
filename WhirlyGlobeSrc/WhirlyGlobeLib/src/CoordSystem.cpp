/*
 *  CoordSystem.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/11.
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

#import "CoordSystem.h"

using namespace Eigen;

namespace WhirlyKit
{

Point3f CoordSystemConvert(CoordSystem *inSystem,CoordSystem *outSystem,Point3f inCoord)
{
    // Easy if the coordinate systems are the same
    if (inSystem->isSameAs(outSystem))
        return inCoord;
    
    // We'll go through geocentric which isn't horrible, but obviously we're assuming the same datum
    Point3f geoCPt = inSystem->localToGeocentric(inCoord);
    Point3f outPt = outSystem->geocentricToLocal(geoCPt);
    return outPt;
}

Point3d CoordSystemConvert3d(CoordSystem *inSystem,CoordSystem *outSystem,Point3d inCoord)
{
    // Easy if the coordinate systems are the same
    if (inSystem->isSameAs(outSystem))
        return inCoord;
    
    // We'll go through geocentric which isn't horrible, but obviously we're assuming the same datum
    Point3d geoCPt = inSystem->localToGeocentric(inCoord);
    Point3d outPt = outSystem->geocentricToLocal(geoCPt);
    return outPt;
}
    
GeneralCoordSystemDisplayAdapter::GeneralCoordSystemDisplayAdapter(CoordSystem *coordSys,const Point3d &ll,const Point3d &ur,const Point3d &inCenter,const Point3d &inScale)
    : CoordSystemDisplayAdapter(coordSys,inCenter), ll(ll), ur(ur), coordSys(coordSys)
{
    scale = inScale;
    center = inCenter;
    dispLL = localToDisplay(ll);
    dispUR = localToDisplay(ur);
    geoLL = coordSys->localToGeographicD(ll);
    geoUR = coordSys->localToGeographicD(ur);
}

GeneralCoordSystemDisplayAdapter::~GeneralCoordSystemDisplayAdapter()
{
}
    
bool GeneralCoordSystemDisplayAdapter::getBounds(Point3f &outLL,Point3f &outUR)
{
    outLL = Point3f(ll.x(),ll.y(),ll.z());
    outUR = Point3f(ur.x(),ur.y(),ur.z());
    return true;
}
    
bool GeneralCoordSystemDisplayAdapter::getDisplayBounds(Point3d &ll,Point3d &ur)
{
    ll = dispLL;
    ur = dispUR;
    return true;
}
    
bool GeneralCoordSystemDisplayAdapter::getGeoBounds(Point2d &ll,Point2d &ur)
{
    ll = geoLL;
    ur = geoUR;
    return true;
}
    
WhirlyKit::Point3f GeneralCoordSystemDisplayAdapter::localToDisplay(WhirlyKit::Point3f localPt)
{
    Point3f dispPt = Point3f(localPt.x()*scale.x(),localPt.y()*scale.y(),localPt.z()*scale.z())-Point3f(center.x(),center.y(),center.z());
    return dispPt;
}

WhirlyKit::Point3d GeneralCoordSystemDisplayAdapter::localToDisplay(WhirlyKit::Point3d localPt)
{
    Point3d dispPt = Point3d(localPt.x()*scale.x(),localPt.y()*scale.y(),localPt.z()*scale.z())-center;
    return dispPt;
}
    
WhirlyKit::Point3f GeneralCoordSystemDisplayAdapter::displayToLocal(WhirlyKit::Point3f dispPt)
{
    Point3f localPt = Point3f(dispPt.x()/scale.x(),dispPt.y()/scale.y(),dispPt.z()/scale.z())+Point3f(center.x(),center.y(),center.z());
    return localPt;
}

WhirlyKit::Point3d GeneralCoordSystemDisplayAdapter::displayToLocal(WhirlyKit::Point3d dispPt)
{
    Point3d localPt = Point3d(dispPt.x()/scale.x(),dispPt.y()/scale.y(),dispPt.z()/scale.z())+center;
    return localPt;
}
    
}
