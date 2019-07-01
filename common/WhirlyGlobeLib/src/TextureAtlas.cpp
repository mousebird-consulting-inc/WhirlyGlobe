/*
 *  TextureAtlas.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
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

#import "TextureAtlas.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"

using namespace Eigen;
using namespace WhirlyKit;

// Set up the texture mapping matrix from the destination texture coords
void SubTexture::setFromTex(const TexCoord &texOrg,const TexCoord &texDest)
{
    trans = trans.Identity();
    trans.translate(texOrg);
    trans.scale(Point2f(texDest.x()-texOrg.x(),texDest.y()-texOrg.y()));
}

// Calculate a destination texture coordinate
TexCoord SubTexture::processTexCoord(const TexCoord &inCoord) const
{
    Vector3f res = trans * Vector3f(inCoord.x(),inCoord.y(),1.0);
    return TexCoord(res.x(),res.y());
}

// Calculate destination texture coords for a while group
void SubTexture::processTexCoords(std::vector<TexCoord> &coords) const
{
    for (unsigned int ii=0;ii<coords.size();ii++)
    {
        TexCoord &coord = coords[ii];
        Vector3f res = trans * Vector3f(coord.x(),coord.y(),1.0);
        coord.x() = res.x();  coord.y() = res.y();
    }
}
