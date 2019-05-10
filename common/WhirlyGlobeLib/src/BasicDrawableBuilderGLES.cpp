/*
 *  BasicDrawableBuilderGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
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

#import "BasicDrawableBuilderGLES.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableBuilderGLES::BasicDrawableBuilderGLES(const std::string &name)
    : BasicDrawableBuilder(name)
{
    basicDraw = new BasicDrawableGLES(name);
}

BasicDrawableBuilderGLES::BasicDrawableBuilderGLES(const std::string &name, unsigned int numVert,unsigned int numTri)
    : BasicDrawableBuilder(name,numVert,numTri)
{
    basicDraw = new BasicDrawableGLES(name);
}

int BasicDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings)
{
    VertexAttribute *attr = new VertexAttributeGLES(dataType,nameID);
    if (numThings > 0)
        attr->reserve(numThings);
    basicDraw->vertexAttributes.push_back(attr);
    
    return (unsigned int)(basicDraw->vertexAttributes.size()-1);
}

BasicDrawable *BasicDrawableBuilderGLES::getDrawable()
{
    if (!basicDraw)
        return NULL;
    
    BasicDrawableGLES *draw = (BasicDrawableGLES *)basicDraw;
    
    draw->vertexSize = draw->singleVertexSize();
    draw->points = points;
    draw->tris = tris;
    
    basicDraw = NULL;
    
    return draw;
}
    
}
