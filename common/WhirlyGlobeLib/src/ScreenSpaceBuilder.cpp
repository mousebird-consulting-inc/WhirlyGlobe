/*
 *  ScreenSpaceManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2019 mousebird consulting.
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
#import "ScreenSpaceDrawableBuilder.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

ScreenSpaceBuilder::DrawableState::DrawableState()
    : period(0.0), progID(EmptyIdentity), fadeUp(0.0), fadeDown(0.0),
    enable(true), startEnable(0.0), endEnable(0.0),
    drawPriority(0), renderTargetID(EmptyIdentity), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid),
    zoomSlot(-1), minZoomVis(DrawVisibleInvalid), maxZoomVis(DrawVisibleInvalid),
    motion(false), rotation(false), keepUpright(false), hasMask(false)
{
}
    
bool ScreenSpaceBuilder::DrawableState::operator < (const DrawableState &that) const
{
    if (texIDs != that.texIDs)
        return texIDs < that.texIDs;
    if (period != that.period)
        return period < that.period;
    if (progID != that.progID)
        return progID < that.progID;
    if (drawPriority != that.drawPriority)
        return drawPriority < that.drawPriority;
    if (renderTargetID != that.renderTargetID)
        return renderTargetID < that.renderTargetID;
    if (minVis != that.minVis)
        return  minVis < that.minVis;
    if (maxVis != that.maxVis)
        return  maxVis < that.maxVis;
    if (zoomSlot != that.zoomSlot)
        return zoomSlot < that.zoomSlot;
    if (minZoomVis != that.minZoomVis)
        return minZoomVis < that.minZoomVis;
    if (maxZoomVis != that.maxZoomVis)
        return maxZoomVis < that.maxZoomVis;
    if (fadeUp != that.fadeUp)
        return fadeUp < that.fadeUp;
    if (fadeDown != that.fadeDown)
        return fadeDown < that.fadeDown;
    if (enable != that.enable)
        return enable < that.enable;
    if (startEnable != that.startEnable)
        return startEnable < that.startEnable;
    if (endEnable != that.endEnable)
        return endEnable < that.endEnable;
    if (motion != that.motion)
        return motion < that.motion;
    if (rotation != that.rotation)
        return rotation < that.rotation;
    if (keepUpright != that.keepUpright)
        return keepUpright < that.keepUpright;
    if (hasMask != that.hasMask)
        return hasMask < that.hasMask;
    if (vertexAttrs != that.vertexAttrs)
        return vertexAttrs < that.vertexAttrs;
    SimpleIdentity opacityExp0 = opacityExp ? opacityExp->getId() : EmptyIdentity,
                    opacityExp1 = that.opacityExp ? that.opacityExp->getId() : EmptyIdentity;
    if (opacityExp0 != opacityExp1)
        return opacityExp0 < opacityExp1;
    SimpleIdentity colorExp0 = colorExp ? colorExp->getId() : EmptyIdentity,
                    colorExp1 = that.colorExp ? that.colorExp->getId() : EmptyIdentity;
    if (colorExp0 != colorExp1)
        return colorExp0 < colorExp1;
    SimpleIdentity scaleExp0 = scaleExp ? scaleExp->getId() : EmptyIdentity,
                    scaleExp1 = that.scaleExp ? that.scaleExp->getId() : EmptyIdentity;
    if (scaleExp0 != scaleExp1)
        return scaleExp0 < scaleExp1;
    
    return false;
}
    
ScreenSpaceBuilder::DrawableWrap::~DrawableWrap()
{
}
    
ScreenSpaceBuilder::DrawableWrap::DrawableWrap(SceneRenderer *render,const DrawableState &state)
    : state(state), center(0,0,0)
{
    locDraw = render->makeScreenSpaceDrawableBuilder("ScreenSpace Builder");
    locDraw->ScreenSpaceInit(state.motion,state.rotation,state.hasMask);
    locDraw->setType(Triangles);
    // A max of two textures per
    for (unsigned int ii=0;ii<state.texIDs.size() && ii<2;ii++)
        locDraw->setTexId(ii, state.texIDs[ii]);
    locDraw->setProgram(state.progID);
    locDraw->setDrawOrder(state.drawOrder);
    locDraw->setDrawPriority(state.drawPriority);
    if (state.renderTargetID != EmptyIdentity)
        locDraw->setRenderTarget(state.renderTargetID);
    locDraw->setFade(state.fadeDown, state.fadeUp);
    locDraw->setVisibleRange(state.minVis, state.maxVis);
    locDraw->setZoomInfo(state.zoomSlot, state.minZoomVis, state.maxZoomVis);
    locDraw->setOpacityExpression(state.opacityExp);
    locDraw->setColorExpression(state.colorExp);
    locDraw->setScaleExpression(state.scaleExp);
    // TODO: Set the opacity/color/scale from the drawable state
    locDraw->setRequestZBuffer(false);
    locDraw->setWriteZBuffer(false);
    locDraw->setVertexAttributes(state.vertexAttrs);
    locDraw->setOnOff(state.enable);
    if (state.startEnable != state.endEnable)
        locDraw->setEnableTimeRange(state.startEnable, state.endEnable);
    
    // If we've got more than one texture ID and a period, we need a tweaker
    if (state.texIDs.size() > 1 && state.period != 0.0)
    {
        TimeInterval now = render->getScene()->getCurrentTime();
        BasicDrawableTexTweaker *tweak = new BasicDrawableTexTweaker(state.texIDs,now,state.period);
        locDraw->addTweaker(DrawableTweakerRef(tweak));
    }
}

Point3d ScreenSpaceBuilder::CalcRotationVec(CoordSystemDisplayAdapter *coordAdapter,const Point3d &worldLoc,float rot)
{
    // Switch from counter-clockwise to clockwise
    rot = 2*M_PI-rot;
    
    Point3d upVec,northVec,eastVec;
    if (coordAdapter->isFlat())
    {
        upVec = Point3d(0,0,1);
        northVec = Point3d(0,1,0);
        eastVec = Point3d(1,0,0);
    } else {
        upVec = worldLoc.normalized();
        // Vector pointing north
        northVec = Point3d(-worldLoc.x(),-worldLoc.y(),1.0-worldLoc.z());
        eastVec = northVec.cross(upVec);
        northVec = upVec.cross(eastVec);
    }
    
    Point3d rotVec = eastVec * sin(rot) + northVec * cos(rot);
    
    return rotVec;
}

void ScreenSpaceBuilder::DrawableWrap::addVertex(CoordSystemDisplayAdapter *coordAdapter,float scale,const Point3d &worldLoc,const Point3f *dir,float rot,const Point2d &inVert,const TexCoord *texCoord,const RGBAColor *color,const SingleVertexAttributeSet *vertAttrs)
{
    locDraw->addPoint(Point3d(worldLoc.x()-center.x(),worldLoc.y()-center.y(),worldLoc.z()-center.z()));
    Point3d norm = coordAdapter->isFlat() ? Point3d(0,0,1) : worldLoc.normalized();
    locDraw->addNormal(norm);
    Point2d vert = inVert * scale;
    locDraw->addOffset(vert);
    if (texCoord)
        locDraw->addTexCoord(0, *texCoord);
    if (color)
        locDraw->addColor(*color);
    if (dir)
        locDraw->addDir(*dir);
    if (vertAttrs && !vertAttrs->empty())
        locDraw->addVertexAttributes(*vertAttrs);
    if (state.rotation)
        locDraw->addRot(ScreenSpaceBuilder::CalcRotationVec(coordAdapter,worldLoc,rot));
}

void ScreenSpaceBuilder::DrawableWrap::addTri(int v0, int v1, int v2)
{
    if (!locDraw)
        return;
    
    locDraw->addTriangle(BasicDrawable::Triangle(v0,v1,v2));
}
    
ScreenSpaceBuilder::ScreenSpaceBuilder(SceneRenderer *sceneRender,CoordSystemDisplayAdapter *coordAdapter,float scale,float centerDist)
    : sceneRender(sceneRender), coordAdapter(coordAdapter), scale(scale), drawPriorityOffset(0), centerDist(centerDist)
{
}

ScreenSpaceBuilder::~ScreenSpaceBuilder()
{
}
    
void ScreenSpaceBuilder::setTexID(SimpleIdentity texID)
{
    curState.texIDs.clear();
    curState.texIDs.push_back(texID);
}
    
void ScreenSpaceBuilder::setTexIDs(const std::vector<SimpleIdentity> &texIDs,double period)
{
    curState.texIDs = texIDs;
    curState.period = period;
}

void ScreenSpaceBuilder::setProgramID(SimpleIdentity progID)
{
    curState.progID = progID;
}

void ScreenSpaceBuilder::setFade(TimeInterval fadeUp,TimeInterval fadeDown)
{
    curState.fadeUp = fadeUp;
    curState.fadeDown = fadeDown;
}

void ScreenSpaceBuilder::setDrawOrder(int64_t drawOrder)
{
    curState.drawOrder = drawOrder;
}

void ScreenSpaceBuilder::setDrawPriority(int drawPriority)
{
    curState.drawPriority = drawPriority;
}

void ScreenSpaceBuilder::setRenderTarget(SimpleIdentity renderTargetID)
{
    curState.renderTargetID = renderTargetID;
}

void ScreenSpaceBuilder::setVisibility(float minVis,float maxVis)
{
    curState.minVis = minVis;
    curState.maxVis = maxVis;
}

void ScreenSpaceBuilder::setZoomInfo(int zoomSlot,double minZoomVis,double maxZoomVis)
{
    curState.zoomSlot = zoomSlot;
    curState.minZoomVis = minZoomVis;
    curState.maxZoomVis = maxZoomVis;
}

void ScreenSpaceBuilder::setOpacityExp(FloatExpressionInfoRef opacityExp)
{
    curState.opacityExp = opacityExp;
}

void ScreenSpaceBuilder::setColorExp(ColorExpressionInfoRef colorExp)
{
    curState.colorExp = colorExp;
}

void ScreenSpaceBuilder::setScaleExp(FloatExpressionInfoRef scaleExp)
{
    curState.scaleExp = scaleExp;
}

void ScreenSpaceBuilder::setEnable(bool inEnable)
{
    curState.enable = inEnable;
}

void ScreenSpaceBuilder::setEnableRange(TimeInterval inStartEnable,TimeInterval inEndEnable)
{
    curState.startEnable = inStartEnable;
    curState.endEnable = inEndEnable;
}

ScreenSpaceBuilder::DrawableWrapRef ScreenSpaceBuilder::findOrAddDrawWrap(const DrawableState &state,int numVerts,int numTri,const Point3d &center)
{
    // Look for an existing drawable
    DrawableWrapRef drawWrap;
    auto it = drawables.find(state);
    if (it == drawables.end())
    {
        // Nope, create one
        drawWrap = DrawableWrapRef(new DrawableWrap(sceneRender,state));
        drawWrap->center = center;
        Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
        Eigen::Matrix4d transMat = trans.matrix();
        drawWrap->locDraw->setMatrix(&transMat);
        if (state.motion)
            drawWrap->locDraw->setStartTime(sceneRender->getScene()->getCurrentTime());
        drawables[state] = (drawWrap);
    } else {
        drawWrap = it->second;
        
        // Make sure this one isn't too large
        if (drawWrap->locDraw->getNumPoints() + numVerts >= MaxDrawablePoints || drawWrap->locDraw->getNumTris() >= MaxDrawableTriangles)
        {
            // It is, so we need to flush it and create a new one
            fullDrawables.push_back(drawWrap);
            drawables.erase(it);
            drawWrap = DrawableWrapRef(new DrawableWrap(sceneRender,state));
            drawables[state] = drawWrap;
        }
    }
    
    return drawWrap;
}
    
void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,const Point2d *coords,const TexCoord *texCoords,const RGBAColor &color)
{
    DrawableWrapRef drawWrap = findOrAddDrawWrap(curState,4,2,worldLoc);
    
    int baseVert = drawWrap->getDrawableBuilder()->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point2d coord(coords[ii].x(),coords[ii].y());
        const TexCoord *texCoord = (texCoords ? &texCoords[ii] : NULL);
        drawWrap->addVertex(coordAdapter,scale,worldLoc, NULL, 0.0, coord, texCoord, &color, NULL);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,double rotation,bool keepUpright,const Point2d *coords,const TexCoord *texCoords,const RGBAColor &color)
{
    DrawableWrapRef drawWrap = findOrAddDrawWrap(curState,4,2,worldLoc);
    
    // Note: Do something with keepUpright
    int baseVert = drawWrap->getDrawableBuilder()->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point2d coord(coords[ii].x(),coords[ii].y());
        const TexCoord *texCoord = (texCoords ? &texCoords[ii] : NULL);
        drawWrap->addVertex(coordAdapter,scale,worldLoc, NULL, rotation, coord, texCoord, &color, NULL);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObject> &screenObjects)
{
    std::sort(screenObjects.begin(),screenObjects.end(),
                  [](const ScreenSpaceObject &a, const ScreenSpaceObject &b) {return a.orderBy < b.orderBy; });
    
    for (unsigned int ii=0;ii<screenObjects.size();ii++)
    {
        ScreenSpaceObject &ssObj = screenObjects[ii];
        
        addScreenObject(ssObj,ssObj.worldLoc,&ssObj.geometry);
    }
}
    
void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObject *> &screenObjects)
{
    std::sort(screenObjects.begin(),screenObjects.end(),
                  [](const ScreenSpaceObject *a, const ScreenSpaceObject *b) {return a->orderBy < b->orderBy; });

    for (unsigned int ii=0;ii<screenObjects.size();ii++)
    {
        ScreenSpaceObject *ssObj = screenObjects[ii];
        
        addScreenObject(*ssObj, ssObj->worldLoc, &ssObj->geometry);
    }
}

void ScreenSpaceBuilder::addScreenObject(const ScreenSpaceObject &ssObj,
                                         const Point3d &worldLoc,
                                         const std::vector<ScreenSpaceConvexGeometry> *geoms,
                                         const std::vector<Eigen::Matrix3d> *places)
{
    for (unsigned int ii=0;ii<geoms->size();ii++)
    {
        ScreenSpaceConvexGeometry geom = geoms->at(ii);
        // Apply a matrix to the geometry for a given version of the placement
        if (places) {
            Eigen::Matrix3d placeMat = places->at(ii);
            for (auto &pt: geom.coords) {
                Point3d pt2d = placeMat * Point3d(pt.x(),pt.y(),1.0);
                pt = Point2d(pt2d.x(),pt2d.y());
            }
        }
        DrawableState state = ssObj.state;
        state.texIDs = geom.texIDs;
        if (geom.progID != EmptyIdentity)
            state.progID = geom.progID;
        if (geom.drawPriority > -1)
            state.drawPriority = geom.drawPriority;
        if (geom.renderTargetID != EmptyIdentity)
            state.renderTargetID = geom.renderTargetID;
        state.enable = ssObj.enable;
        state.startEnable = ssObj.startEnable;
        state.endEnable = ssObj.endEnable;
        VertexAttributeSetConvert(geom.vertexAttrs,state.vertexAttrs);
        DrawableWrapRef drawWrap = findOrAddDrawWrap(state,(int)geom.coords.size(),(int)(geom.coords.size()-2),worldLoc);
        
        // May need to adjust things based on time
        Point3d startLoc3d = worldLoc;
        Point3f dir(0,0,0);
        if (state.motion)
        {
            double dur = ssObj.endTime - ssObj.startTime;
            Point3d dir3d = (ssObj.endWorldLoc - ssObj.worldLoc)/dur;
            // May need to knock the start back a bit
            double dt = drawWrap->locDraw->getStartTime() - ssObj.startTime;
            startLoc3d = dir3d * dt + startLoc3d;
            dir = Point3f(dir3d.x(),dir3d.y(),dir3d.z());
        }
        Point3d startLoc(startLoc3d.x(),startLoc3d.y(),startLoc3d.z());

        int baseVert = drawWrap->locDraw->getNumPoints();
        for (unsigned int jj=0;jj<geom.coords.size();jj++)
        {
            Point2d coord = geom.coords[jj] + ssObj.offset;
            const TexCoord *texCoord = geom.texCoords.size() > jj ? &geom.texCoords[jj] : NULL;
            if (state.motion)
            {
                Point3f dir3f(dir.x(),dir.y(),dir.z());
                drawWrap->addVertex(coordAdapter,scale,startLoc, &dir3f, ssObj.rotation, Point2d(coord.x(),coord.y()), texCoord, &geom.color, &geom.vertexAttrs);
            } else {
                drawWrap->addVertex(coordAdapter,scale,startLoc, NULL, ssObj.rotation, Point2d(coord.x(),coord.y()), texCoord, &geom.color, &geom.vertexAttrs);
            }
        }
        for (unsigned int jj=0;jj<geom.coords.size()-2;jj++)
            drawWrap->addTri(0+baseVert, jj+1+baseVert, jj+2+baseVert);
    }
}
    
void ScreenSpaceBuilder::buildDrawables(std::vector<BasicDrawableRef> &draws)
{
    for (auto it : fullDrawables)
    {
        draws.push_back(it->getDrawableBuilder()->getDrawable());
    }
    fullDrawables.clear();

    for (auto it : drawables)
    {
        draws.push_back(it.second->getDrawableBuilder()->getDrawable());
    }
    drawables.clear();
}
    
void ScreenSpaceBuilder::flushChanges(ChangeSet &changes,SimpleIDSet &drawIDs)
{
    std::vector<BasicDrawableRef> draws;
    buildDrawables(draws);
    for (unsigned int ii=0;ii<draws.size();ii++)
    {
        BasicDrawableRef draw = draws[ii];
        drawIDs.insert(draw->getId());
        changes.push_back(new AddDrawableReq(draw));
    }
    draws.clear();
}
        
ScreenSpaceObject::ScreenSpaceObject()
    : enable(true), startEnable(0.0), endEnable(0.0), worldLoc(0,0,0), endWorldLoc(0,0,0), startTime(0.0), endTime(0.0), offset(0,0), rotation(0), keepUpright(false), orderBy(-1)
{
}

ScreenSpaceObject::ScreenSpaceObject(SimpleIdentity theID)
: Identifiable(theID), enable(true), startEnable(0.0), endEnable(0.0), worldLoc(0,0,0), endWorldLoc(0,0,0), startTime(0), endTime(0), offset(0,0), rotation(0), keepUpright(false)
{
}

ScreenSpaceObject::~ScreenSpaceObject()
{
}
    
void ScreenSpaceObject::setEnable(bool inEnable)
{
    enable = inEnable;
}
    
void ScreenSpaceObject::setEnableTime(TimeInterval inStartEnable,TimeInterval inEndEnable)
{
    startEnable = inStartEnable;
    endEnable = inEndEnable;
}
    
void ScreenSpaceObject::setWorldLoc(const Point3d &inWorldLoc)
{
    worldLoc = inWorldLoc;
}
    
void ScreenSpaceObject::setMovingLoc(const Point3d &worldLoc,TimeInterval inStartTime,TimeInterval inEndTime)
{
    state.motion = true;
    endWorldLoc = worldLoc;
    startTime = inStartTime;
    endTime = inEndTime;
}

Point3d ScreenSpaceObject::getEndWorldLoc()
{
    return endWorldLoc;
}

TimeInterval ScreenSpaceObject::getStartTime()
{
    return startTime;
}

TimeInterval ScreenSpaceObject::getEndTime()
{
    return endTime;
}
    
Point3d ScreenSpaceObject::getWorldLoc()
{
    return worldLoc;
}
    
void ScreenSpaceObject::setVisibility(float minVis,float maxVis)
{
    state.minVis = minVis;
    state.maxVis = maxVis;
}

void ScreenSpaceObject::setZoomInfo(int zoomSlot,double minZoomVis,double maxZoomVis)
{
    state.zoomSlot = zoomSlot;
    state.minZoomVis = minZoomVis;
    state.maxZoomVis = maxZoomVis;
}

void ScreenSpaceObject::setOpacityExp(FloatExpressionInfoRef opacityExp)
{
    state.opacityExp = opacityExp;
}

void ScreenSpaceObject::setColorExp(ColorExpressionInfoRef colorExp)
{
    state.colorExp = colorExp;
}

void ScreenSpaceObject::setScaleExp(FloatExpressionInfoRef scaleExp)
{
    state.scaleExp = scaleExp;
}

void ScreenSpaceObject::setDrawOrder(int64_t drawOrder)
{
    state.drawOrder = drawOrder;
}

void ScreenSpaceObject::setDrawPriority(int drawPriority)
{
    state.drawPriority = drawPriority;
}

void ScreenSpaceObject::setRenderTarget(SimpleIdentity renderTargetID)
{
    state.renderTargetID = renderTargetID;
}

void ScreenSpaceObject::setKeepUpright(bool inKeepUpright)
{
    keepUpright = inKeepUpright;
}

void ScreenSpaceObject::setRotation(double inRot)
{
    state.rotation = true;
    rotation = inRot;
}

void ScreenSpaceObject::setFade(TimeInterval fadeUp,TimeInterval fadeDown)
{
    state.fadeUp = fadeUp;
    state.fadeDown = fadeDown;
}
    
void ScreenSpaceObject::setOffset(const Point2d &inOffset)
{
    offset = inOffset;
}
    
void ScreenSpaceObject::setPeriod(TimeInterval period)
{
    state.period = period;
}

void ScreenSpaceObject::setOrderBy(long inOrderBy)
{
    orderBy = inOrderBy;
}

void ScreenSpaceObject::addGeometry(const ScreenSpaceConvexGeometry &geom)
{
    geometry.push_back(geom);
}

void ScreenSpaceObject::addGeometry(const std::vector<ScreenSpaceConvexGeometry> &geom)
{
    geometry.insert(geometry.end(), geom.begin(), geom.end());
}

SimpleIdentity ScreenSpaceObject::getTypicalProgramID()
{
    for (auto geom : geometry)
    {
        if (geom.progID != EmptyIdentity)
            return geom.progID;
    }
    
    return state.progID;
}

int64_t ScreenSpaceObject::getDrawOrder() const
{
    return state.drawPriority;
}

int ScreenSpaceObject::getDrawPriority() const
{
    return state.drawPriority;
}

ScreenSpaceObjectLocation::ScreenSpaceObjectLocation()
: isCluster(false), dispLoc(0,0,0), offset(0,0), keepUpright(false), rotation(0.0)
{
    
}

}
