/*
 *  GeoJSONSource.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 4/4/17.
 *  Copyright 2011-2017 mousebird consulting
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

#import "GeoJSONSource.h"

namespace WhirlyKit
{

GeoJSONSource::GeoJSONSource()
{
}

GeoJSONSource::~GeoJSONSource()
{
}

bool GeoJSONSource::parseData(std::string json, std::vector<VectorObject *> &vecObjs)
{
    ShapeSet shapes;
    std::string crs;
    bool parsed = VectorParseGeoJSON(shapes, json, crs);
    if (!parsed)
        return false;

    vecObjs = std::vector<VectorObject *>(shapes.size());

    for (ShapeSet::iterator it = shapes.begin(); it != shapes.end(); ++it) {

        Dictionary *attributes = (*it)->getAttrDict();

        VectorPointsRef points = std::dynamic_pointer_cast<VectorPoints>(*it);
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);

        if (points) {
            attributes->setString("geometry_type", "POINT");
            processPoints(points, vecObjs);
        } else if (lin) {
            attributes->setString("geometry_type", "LINESTRING");
            processLinear(lin, vecObjs);
        } else if (ar) {
            attributes->setString("geometry_type", "POLYGON");
            processAreal(ar, vecObjs);
        }
    }
    return true;
}

void GeoJSONSource::processPoints(const VectorPointsRef &points, std::vector<VectorObject *> &vecObjs)
{
    VectorObject *vecObj = new VectorObject();
    vecObj->shapes.insert(points);
    vecObjs.push_back(vecObj);
}

void GeoJSONSource::processLinear(const VectorLinearRef &linear, std::vector<VectorObject *> &vecObjs)
{
    VectorObject *vecObj = new VectorObject();
    vecObj->shapes.insert(linear);
    vecObjs.push_back(vecObj);
}

void GeoJSONSource::processAreal(const VectorArealRef &areal, std::vector<VectorObject *> &vecObjs)
{
    VectorObject *vecObj = new VectorObject();
    vecObj->shapes.insert(areal);
    vecObjs.push_back(vecObj);

    vecObj = new VectorObject();
    for (unsigned int li=0;li<areal->loops.size();li++)
    {
        const VectorRing &ring = areal->loops[li];
        VectorRing newRing(ring);
        VectorLinearRef linear = VectorLinear::createLinear();
        linear->pts = newRing;
        linear->initGeoMbr();
        vecObj->shapes.insert(linear);
    }
    vecObjs.push_back(vecObj);
}


}
