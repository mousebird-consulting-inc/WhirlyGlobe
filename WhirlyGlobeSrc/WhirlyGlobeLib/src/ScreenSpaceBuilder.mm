/*
 *  ScreenSpaceManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2014 mousebird consulting. All rights reserved.
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

#import "ScreenSpaceBuilder.h"
#import "ScreenSpaceDrawable.h"

static int ScreenSpaceDrawPriorityOffset = 1000000;

namespace WhirlyKit
{

ScreenSpaceBuilder::DrawableState::DrawableState()
    : texID(EmptyIdentity), progID(EmptyIdentity), fadeUp(0.0), fadeDown(0.0),
    drawPriority(ScreenSpaceDrawPriorityOffset), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid)
{
}
    
bool ScreenSpaceBuilder::DrawableState::operator < (const DrawableState &that) const
{
    if (texID != that.texID)
        return texID < that.texID;
    if (progID != that.progID)
        return progID < that.progID;
    if (drawPriority != that.drawPriority)
        return drawPriority < that.drawPriority;
    if (minVis != that.minVis)
        return  minVis < that.minVis;
    if (maxVis != that.maxVis)
        return  maxVis < that.maxVis;
    if (fadeUp != that.fadeUp)
        return fadeUp < that.fadeUp;
    if (fadeDown != that.fadeDown)
        return fadeDown < that.fadeDown;
    
    return false;
}
    
ScreenSpaceBuilder::DrawableWrap::DrawableWrap()
    : draw(NULL)
{
}
    
ScreenSpaceBuilder::DrawableWrap::DrawableWrap(const DrawableState &state)
    : state(state)
{
    draw = new ScreenSpaceDrawable();
    draw->setType(GL_TRIANGLES);
    draw->setTexId(0, state.texID);
    draw->setProgram(state.progID);
    draw->setDrawPriority(state.drawPriority);
    draw->setFade(state.fadeDown, state.fadeUp);
    draw->setVisibleRange(state.minVis, state.maxVis);
    draw->setRequestZBuffer(false);
    draw->setWriteZBuffer(false);
}
    
bool ScreenSpaceBuilder::DrawableWrap::operator < (const DrawableWrap &that) const
{
    return state < that.state;
}
    
void ScreenSpaceBuilder::DrawableWrap::addVertex(CoordSystemDisplayAdapter *coordAdapter,float scale,const Point3f &worldLoc,float rot,const Point2f &inVert,const TexCoord &texCoord,const RGBAColor &color)
{
    draw->addPoint(worldLoc);
    Point3f norm = coordAdapter->isFlat() ? Point3f(0,0,1) : worldLoc.normalized();
    draw->addNormal(norm);
    // Note: Rotation
    Point2f vert = inVert * scale;
    draw->addOffset(vert);
    draw->addTexCoord(0, texCoord);
    draw->addColor(color);
}
    
void ScreenSpaceBuilder::DrawableWrap::addTri(int v0, int v1, int v2)
{
    if (!draw)
        return;
    
    draw->addTriangle(BasicDrawable::Triangle(v0,v1,v2));
}
    
ScreenSpaceBuilder::ScreenSpaceBuilder(CoordSystemDisplayAdapter *coordAdapter,float scale)
    : coordAdapter(coordAdapter), scale(scale), drawPriorityOffset(ScreenSpaceDrawPriorityOffset)
{
}

ScreenSpaceBuilder::~ScreenSpaceBuilder()
{
    for (DrawableWrapSet::iterator it = drawables.begin(); it != drawables.end(); ++it)
        delete *it;
}
    
void ScreenSpaceBuilder::setTexID(SimpleIdentity texID)
{
    curState.texID = texID;
}

void ScreenSpaceBuilder::setProgramID(SimpleIdentity progID)
{
    curState.progID = progID;
}

void ScreenSpaceBuilder::setFade(NSTimeInterval fadeUp,NSTimeInterval fadeDown)
{
    curState.fadeUp = fadeUp;
    curState.fadeDown = fadeDown;
}

void ScreenSpaceBuilder::setDrawPriority(int drawPriority)
{
    curState.drawPriority = ScreenSpaceDrawPriorityOffset+drawPriority;
}

void ScreenSpaceBuilder::setVisibility(float minVis,float maxVis)
{
    curState.minVis = minVis;
    curState.maxVis = maxVis;
}
    
ScreenSpaceBuilder::DrawableWrap *ScreenSpaceBuilder::findOrAddDrawWrap(const DrawableState &state,int numVerts,int numTri)
{
    // Look for an existing drawable
    DrawableWrap dummy(state);
    DrawableWrap *drawWrap = NULL;
    DrawableWrapSet::iterator it = drawables.find(&dummy);
    if (it == drawables.end())
    {
        // Nope, create one
        drawWrap = new DrawableWrap(state);
        drawables.insert(drawWrap);
    } else {
        drawWrap = *it;
        
        // Make sure this one isn't too large
        if (drawWrap->draw->getNumPoints() + numVerts >= MaxDrawablePoints || drawWrap->draw->getNumTris() >= MaxDrawableTriangles)
        {
            // It is, so we need to flush it and create a new one
            fullDrawables.push_back(drawWrap);
            drawables.erase(it);
            drawWrap = new DrawableWrap(state);
            drawables.insert(drawWrap);
        }
    }
    
    return drawWrap;
}
    
void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,const Point2d *coords,const TexCoord *texCoords,const RGBAColor &color)
{
    DrawableWrap *drawWrap = findOrAddDrawWrap(curState,4,2);
    
    int baseVert = drawWrap->draw->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point2f coord(coords[ii].x(),coords[ii].y());
        drawWrap->addVertex(coordAdapter,scale,Point3f(worldLoc.x(),worldLoc.y(),worldLoc.z()), 0.0, coord, texCoords[ii], color);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,double rotation,bool keepUpright,const Point2d *coords,const TexCoord *texCoords,const RGBAColor &color)
{
    DrawableWrap *drawWrap = findOrAddDrawWrap(curState,4,2);
    
    // Note: Do something with keepUpright
    int baseVert = drawWrap->draw->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point2f coord(coords[ii].x(),coords[ii].y());
        drawWrap->addVertex(coordAdapter,scale,Point3f(worldLoc.x(),worldLoc.y(),worldLoc.z()), rotation, coord, texCoords[ii], color);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObject> &screenObjects)
{
    for (unsigned int ii=0;ii<screenObjects.size();ii++)
    {
        ScreenSpaceObject &ssObj = screenObjects[ii];
        
        addScreenObject(ssObj);
    }
}
    
void ScreenSpaceBuilder::addScreenObject(const ScreenSpaceObject &ssObj)
{
    for (unsigned int ii=0;ii<ssObj.geometry.size();ii++)
    {
        const ScreenSpaceObject::ConvexGeometry &geom = ssObj.geometry[ii];
        DrawableState state = ssObj.state;
        state.texID = geom.texID;
        state.progID = geom.progID;
        DrawableWrap *drawWrap = findOrAddDrawWrap(state,geom.coords.size(),geom.coords.size()-2);
        
        int baseVert = drawWrap->draw->getNumPoints();
        for (unsigned int jj=0;jj<geom.coords.size();jj++)
        {
            Point2d coord = geom.coords[jj] + ssObj.offset;
            drawWrap->addVertex(coordAdapter,scale,Point3f(ssObj.worldLoc.x(),ssObj.worldLoc.y(),ssObj.worldLoc.z()), ssObj.rotation, Point2f(coord.x(),coord.y()), geom.texCoords[jj], geom.color);
        }
        for (unsigned int jj=0;jj<geom.coords.size()-2;jj++)
            drawWrap->addTri(0+baseVert, jj+1+baseVert, jj+2+baseVert);
    }
}
    
void ScreenSpaceBuilder::buildDrawables(std::vector<ScreenSpaceDrawable *> &draws)
{
    for (unsigned int ii=0;ii<fullDrawables.size();ii++)
    {
        draws.push_back(fullDrawables[ii]->draw);
        delete fullDrawables[ii];
    }
    fullDrawables.clear();
    
    for (DrawableWrapSet::iterator it = drawables.begin(); it != drawables.end(); ++it)
    {
        draws.push_back((*it)->draw);
        delete *it;
    }
    drawables.clear();
}
    
void ScreenSpaceBuilder::flushChanges(ChangeSet &changes,SimpleIDSet &drawIDs)
{
    std::vector<ScreenSpaceDrawable *> draws;
    buildDrawables(draws);
    for (unsigned int ii=0;ii<draws.size();ii++)
    {
        ScreenSpaceDrawable *draw = draws[ii];
        drawIDs.insert(draw->getId());
        changes.push_back(new AddDrawableReq(draw));
    }
}
    
ScreenSpaceObject::ScreenSpaceObject::ConvexGeometry::ConvexGeometry()
    : texID(EmptyIdentity), progID(EmptyIdentity), color(255,255,255,255)
{
}
    
ScreenSpaceObject::ScreenSpaceObject()
    : enable(true), worldLoc(0,0,0), offset(0,0), rotation(0), useRotation(false), keepUpright(false)
{
}

ScreenSpaceObject::ScreenSpaceObject(SimpleIdentity theID)
: Identifiable(theID), enable(true), worldLoc(0,0,0), offset(0,0), rotation(0), useRotation(false), keepUpright(false)
{
}

ScreenSpaceObject::~ScreenSpaceObject()
{
}
    
void ScreenSpaceObject::setWorldLoc(const Point3d &inWorldLoc)
{
    worldLoc = inWorldLoc;
}
    
Point3d ScreenSpaceObject::getWorldLoc()
{
    return worldLoc;
}

void ScreenSpaceObject::setEnable(bool inEnable)
{
    enable = inEnable;
}
    
void ScreenSpaceObject::setVisibility(float minVis,float maxVis)
{
    state.minVis = minVis;
    state.maxVis = maxVis;
}

void ScreenSpaceObject::setDrawPriority(int drawPriority)
{
    state.drawPriority = ScreenSpaceDrawPriorityOffset+drawPriority;
}

void ScreenSpaceObject::setKeepUpright(bool inKeepUpright)
{
    keepUpright = inKeepUpright;
}

void ScreenSpaceObject::setRotation(double inRot)
{
    useRotation = true;
    rotation = inRot;
}

void ScreenSpaceObject::setFade(NSTimeInterval fadeUp,NSTimeInterval fadeDown)
{
    state.fadeUp = fadeUp;
    state.fadeDown = fadeDown;
}
    
void ScreenSpaceObject::setOffset(const Point2d &inOffset)
{
    offset = inOffset;
}

void ScreenSpaceObject::addGeometry(const ConvexGeometry &geom)
{
    geometry.push_back(geom);
}
    
}
