/*
 *  VectorObject.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/17/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "VectorObject.h"

namespace WhirlyKit
{
    
VectorObject::VectorObject()
{
}
    
bool VectorObject::fromGeoJSON(const std::string &json)
{
    return VectorParseGeoJSON(shapes,json);
}
    
bool VectorObject::FromGeoJSONAssembly(const std::string &json,std::map<std::string,VectorObject *> &vecData)
{
    std::map<std::string, ShapeSet> newShapes;
    if (!VectorParseGeoJSONAssembly(json, newShapes))
        return false;
    
    for (std::map<std::string, ShapeSet>::iterator it = newShapes.begin();
         it != newShapes.end(); ++it)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.insert(it->second.begin(),it->second.end());
        vecData[it->first] = vecObj;
    }
    
    return true;
}
    
Dictionary *VectorObject::getAttributes()
{
    if (shapes.empty())
        return NULL;
    
    return (*shapes.begin())->getAttrDict();
}
 
void VectorObject::splitVectors(std::vector<VectorObject *> &vecs)
{    
    for (WhirlyKit::ShapeSet::iterator it = shapes.begin();
         it != shapes.end(); ++it)
    {
        VectorObject *vecObj = new VectorObject();
        vecObj->shapes.insert(*it);
        vecs.push_back(vecObj);
    }
    
}
    
bool VectorObject::fromFile(const std::string &fileName)
{
    return VectorReadFile(fileName, shapes);
}

bool VectorObject::toFile(const std::string &file)
{
    return VectorWriteFile(file, shapes);
}


}
