/*  ScreenSpaceManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2022 mousebird consulting.
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
 */

#import "ScreenSpaceBuilder.h"
#import "ScreenSpaceDrawableBuilder.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

ScreenSpaceBuilder::DrawableState::DrawableState(DrawableState&& other) noexcept :
        texIDs        (std::move(other.texIDs)),
        period        (other.period),
        progID        (other.progID),
        fadeUp        (other.fadeUp),
        fadeDown      (other.fadeDown),
        startEnable   (other.startEnable),
        endEnable     (other.endEnable),
        drawOrder     (other.drawOrder),
        renderTargetID(other.renderTargetID),
        minZoomVis    (other.minZoomVis),
        maxZoomVis    (other.maxZoomVis),
        minVis        (other.minVis),
        maxVis        (other.maxVis),
        zoomSlot      (other.zoomSlot),
        drawPriority  (other.drawPriority),
        enable        (other.enable),
        motion        (other.motion),
        rotation      (other.rotation),
        keepUpright   (other.keepUpright),
        hasMask       (other.hasMask),
        opacityExp    (std::move(other.opacityExp)),
        colorExp      (std::move(other.colorExp)),
        scaleExp      (std::move(other.scaleExp)),
        vertexAttrs   (std::move(other.vertexAttrs))
{
}

ScreenSpaceBuilder::DrawableState& ScreenSpaceBuilder::DrawableState::operator=(DrawableState&& other) noexcept
{
    if (this != &other)
    {
        texIDs         = std::move(other.texIDs);
        period         = other.period;
        progID         = other.progID;
        fadeUp         = other.fadeUp;
        fadeDown       = other.fadeDown;
        enable         = other.enable;
        startEnable    = other.startEnable;
        endEnable      = other.endEnable;
        drawOrder      = other.drawOrder;
        drawPriority   = other.drawPriority;
        renderTargetID = other.renderTargetID;
        minVis         = other.minVis;
        maxVis         = other.maxVis;
        zoomSlot       = other.zoomSlot;
        minZoomVis     = other.minZoomVis;
        maxZoomVis     = other.maxZoomVis;
        motion         = other.motion;
        rotation       = other.rotation;
        keepUpright    = other.keepUpright;
        hasMask        = other.hasMask;
        opacityExp     = std::move(other.opacityExp);
        colorExp       = std::move(other.colorExp);
        scaleExp       = std::move(other.scaleExp);
        vertexAttrs    = std::move(other.vertexAttrs);
    }
    return *this;
}

bool ScreenSpaceBuilder::DrawableState::operator <(const DrawableState &that) const
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
    const SimpleIdentity opacityExp0 = opacityExp ? opacityExp->getId() : EmptyIdentity;
    const SimpleIdentity opacityExp1 = that.opacityExp ? that.opacityExp->getId() : EmptyIdentity;
    if (opacityExp0 != opacityExp1)
        return opacityExp0 < opacityExp1;
    const SimpleIdentity colorExp0 = colorExp ? colorExp->getId() : EmptyIdentity;
    const SimpleIdentity colorExp1 = that.colorExp ? that.colorExp->getId() : EmptyIdentity;
    if (colorExp0 != colorExp1)
        return colorExp0 < colorExp1;
    const SimpleIdentity scaleExp0 = scaleExp ? scaleExp->getId() : EmptyIdentity;
    const SimpleIdentity scaleExp1 = that.scaleExp ? that.scaleExp->getId() : EmptyIdentity;
    if (scaleExp0 != scaleExp1)
        return scaleExp0 < scaleExp1;
    
    return false;
}
    
ScreenSpaceBuilder::DrawableWrap::DrawableWrap(SceneRenderer *render,const DrawableState &state) :
    state(state)
{
    if (!render)
    {
        throw std::invalid_argument("render");
    }

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
    const auto scene = render->getScene();
    if (scene && state.texIDs.size() > 1 && state.period != 0.0)
    {
        const TimeInterval now = scene->getCurrentTime();
        locDraw->addTweaker(std::make_shared<BasicDrawableTexTweaker>(state.texIDs,now,state.period));
    }
}

Point3d ScreenSpaceBuilder::CalcRotationVec(CoordSystemDisplayAdapter *coordAdapter,const Point3d &worldLoc,float rot)
{
    // Switch from counter-clockwise to clockwise
    rot = (float)(2*M_PI-rot);

    Point3d northVec,eastVec;
    if (coordAdapter->isFlat())
    {
        //upVec = Point3d(0,0,1);
        northVec = Point3d(0,1,0);
        eastVec = Point3d(1,0,0);
    } else {
        const Point3d upVec = worldLoc.normalized();
        // Vector pointing north
        northVec = Point3d(-worldLoc.x(),-worldLoc.y(),1.0-worldLoc.z());
        eastVec = northVec.cross(upVec);
        northVec = upVec.cross(eastVec);
    }

    Point3d rotVec = eastVec * sin(rot) + northVec * cos(rot);

    return rotVec;
}

void ScreenSpaceBuilder::DrawableWrap::addVertex(CoordSystemDisplayAdapter *inCoordAdapter, float inScale,
                                                 const Point3d &worldLoc, const Point3f *dir, float rot,
                                                 const Point2d &inVert, const TexCoord *texCoord,
                                                 const RGBAColor *color, const SingleVertexAttributeSet *vertAttrs)
{
    locDraw->addPoint(Point3d(worldLoc.x()-center.x(),worldLoc.y()-center.y(),worldLoc.z()-center.z()));
    Point3d norm = inCoordAdapter->isFlat() ? Point3d(0, 0, 1) : worldLoc.normalized();
    locDraw->addNormal(norm);
    Point2d vert = inVert * inScale;
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
        locDraw->addRot(ScreenSpaceBuilder::CalcRotationVec(inCoordAdapter, worldLoc, rot));
}

void ScreenSpaceBuilder::DrawableWrap::addTri(int v0, int v1, int v2)
{
    if (!locDraw)
        return;
    
    locDraw->addTriangle(BasicDrawable::Triangle(v0,v1,v2));
}
    
ScreenSpaceBuilder::ScreenSpaceBuilder(SceneRenderer *sceneRender,
                                       CoordSystemDisplayAdapter *coordAdapter,
                                       float scale,float centerDist) :
    centerDist(centerDist),
    scale(scale),
    sceneRender(sceneRender),
    coordAdapter(coordAdapter)
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
    curState.opacityExp = std::move(opacityExp);
}

void ScreenSpaceBuilder::setColorExp(ColorExpressionInfoRef colorExp)
{
    curState.colorExp = std::move(colorExp);
}

void ScreenSpaceBuilder::setScaleExp(FloatExpressionInfoRef scaleExp)
{
    curState.scaleExp = std::move(scaleExp);
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

ScreenSpaceBuilder::DrawableWrapRef ScreenSpaceBuilder::findOrAddDrawWrap(
    const DrawableState &state,int numVerts,int numTris,const Point3d &center)
{
    // Look for an existing drawable
    DrawableWrapRef drawWrap;
    const auto it = drawables.find(state);
    if (it == drawables.end())
    {
        // Nope, create one
        try
        {
            drawWrap = std::make_shared<DrawableWrap>(sceneRender, state);
        }
        catch (const std::exception &ex)
        {
            wkLogLevel(Error, "Failed to create drawable wrapper: %s", ex.what());
        }

        if (drawWrap)
        {
            drawWrap->center = center;
            const Eigen::Affine3d trans(Eigen::Translation3d(center.x(), center.y(), center.z()));
            drawWrap->locDraw->setMatrix(trans.matrix());
            if (state.motion)
                drawWrap->locDraw->setStartTime(sceneRender->getScene()->getCurrentTime());
            drawables[state] = (drawWrap);
        }
    }
    else
    {
        drawWrap = it->second;
        
        // Make sure this one isn't too large
        if (drawWrap && (drawWrap->locDraw->getNumPoints() + numVerts >= MaxDrawablePoints ||
                         drawWrap->locDraw->getNumTris()   + numTris  >= MaxDrawableTriangles))
        {
            // It is, so we need to flush it and create a new one
            fullDrawables.push_back(drawWrap);
            drawWrap = std::make_shared<DrawableWrap>(sceneRender,state);
            it->second = drawWrap;
        }
    }
    
    return drawWrap;
}

void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,const Point2d *coords,
                                      const TexCoord *texCoords,const RGBAColor &color,
                                      SimpleIDUnorderedSet *drawIDs)
{
    const DrawableWrapRef drawWrap = findOrAddDrawWrap(curState,4,2,worldLoc);
    auto &builder = drawWrap->getDrawableBuilder();

    if (drawIDs)
    {
        drawIDs->insert(builder->getDrawableID());
    }

    const unsigned int baseVert = builder->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        const Point2d &coord = coords[ii];
        const TexCoord *texCoord = (texCoords ? &texCoords[ii] : nullptr);
        drawWrap->addVertex(coordAdapter,scale,worldLoc, nullptr, 0.0, coord, texCoord, &color, nullptr);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,double rotation,
                                      __unused bool keepUpright,
                                      const Point2d *coords,
                                      const TexCoord *texCoords,const RGBAColor &color,
                                      SimpleIDUnorderedSet *drawIDs)
{
    const DrawableWrapRef drawWrap = findOrAddDrawWrap(curState,4,2,worldLoc);
    auto &builder = drawWrap->getDrawableBuilder();

    if (drawIDs)
    {
        drawIDs->insert(builder->getDrawableID());
    }

    // Note: Do something with keepUpright
    const unsigned int baseVert = drawWrap->getDrawableBuilder()->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        const Point2d &coord = coords[ii];
        const TexCoord *texCoord = (texCoords ? &texCoords[ii] : nullptr);
        drawWrap->addVertex(coordAdapter,scale,worldLoc, nullptr,
                            (float)rotation, coord, texCoord, &color, nullptr);
    }
    drawWrap->addTri(0+baseVert,1+baseVert,2+baseVert);
    drawWrap->addTri(0+baseVert,2+baseVert,3+baseVert);
}

void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObject> &screenObjects,
                                          const std::vector<Eigen::Matrix3d> *places,
                                          SimpleIDUnorderedSet *drawIDs)
{
    std::sort(screenObjects.begin(),screenObjects.end(),
          [](const auto &a, const auto &b) { return a.orderBy < b.orderBy; });

    for (const auto &ssObj : screenObjects)
    {
        addScreenObject(ssObj,ssObj.worldLoc,&ssObj.geometry,places,drawIDs);
    }
}

void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObject *> &screenObjects,
                                          const std::vector<Eigen::Matrix3d> *places,
                                          SimpleIDUnorderedSet *drawIDs)
{
    std::sort(screenObjects.begin(),screenObjects.end(),
                  [](const auto *a, const auto *b) {return a->orderBy < b->orderBy; });

    for (const auto *ssObj : screenObjects)
    {
        addScreenObject(*ssObj, ssObj->worldLoc, &ssObj->geometry,places,drawIDs);
    }
}

void ScreenSpaceBuilder::addScreenObjects(std::vector<ScreenSpaceObjectRef> &screenObjects,
                                          const std::vector<Eigen::Matrix3d> *places,
                                          SimpleIDUnorderedSet *drawIDs)
{
    std::sort(screenObjects.begin(),screenObjects.end(),
              [](const auto &a, const auto &b) {return a->orderBy < b->orderBy; });

    for (const auto &ssObj : screenObjects)
    {
        addScreenObject(*ssObj, ssObj->worldLoc, &ssObj->geometry,places,drawIDs);
    }
}

void ScreenSpaceBuilder::addScreenObject(const ScreenSpaceObject &ssObj,
                                         const Point3d &worldLoc,
                                         const std::vector<ScreenSpaceConvexGeometry> *geoms,
                                         const std::vector<Eigen::Matrix3d> *places,
                                         SimpleIDUnorderedSet *drawIDs)
{
    for (unsigned int ii=0;ii<geoms->size();ii++)
    {
        ScreenSpaceConvexGeometry geom = geoms->at(ii); // make a copy

        // Apply a matrix to the geometry for a given version of the placement
        if (places)
        {
            const Eigen::Matrix3d &placeMat = places->at(ii);
            for (auto &pt: geom.coords)
            {
                const Point3d pt3d = placeMat * Point3d(pt.x(),pt.y(),1.0);
                pt = Point2d(pt3d.x(),pt3d.y());
            }
        }

        DrawableState state = ssObj.state;  // make a copy
        state.texIDs = geom.texIDs;
        if (geom.progID != EmptyIdentity)
            state.progID = geom.progID;
        if (geom.drawPriority > -1)
            state.drawPriority = geom.drawPriority;
        if (geom.renderTargetID != EmptyIdentity)
            state.renderTargetID = geom.renderTargetID;
        if (ssObj.startTime < ssObj.endTime)
            state.motion = true;
        state.enable = ssObj.enable;
        state.startEnable = ssObj.startEnable;
        state.endEnable = ssObj.endEnable;
        VertexAttributeSetConvert(geom.vertexAttrs,state.vertexAttrs);

        DrawableWrapRef drawWrap = findOrAddDrawWrap(state,(int)geom.coords.size(),(int)(geom.coords.size()-2),worldLoc);
        auto &builder = drawWrap->getDrawableBuilder();

        if (drawIDs)
        {
            drawIDs->insert(builder->getDrawableID());
        }

        // May need to adjust things based on time
        Point3d startLoc3d = worldLoc;
        Point3f dir(0,0,0);
        if (state.motion && ssObj.startTime < ssObj.endTime)
        {
            const double dur = ssObj.endTime - ssObj.startTime;
            const Point3d dir3d = (ssObj.endWorldLoc - ssObj.worldLoc)/dur;
            // May need to knock the start back a bit
            const double dt = drawWrap->locDraw->getStartTime() - ssObj.startTime;
            startLoc3d = dir3d * dt + startLoc3d;
            dir = dir3d.cast<float>();
        }

        const unsigned int baseVert = drawWrap->locDraw->getNumPoints();
        for (unsigned int jj=0;jj<geom.coords.size();jj++)
        {
            const Point2d coord = geom.coords[jj] + ssObj.offset;
            const TexCoord *texCoord = (jj < geom.texCoords.size()) ? &geom.texCoords[jj] : nullptr;
            const auto rot = (float)ssObj.getRotation();
            if (state.motion)
            {
                const Point3f dir3f = dir.cast<float>();
                drawWrap->addVertex(coordAdapter,scale,startLoc3d, &dir3f, rot, coord,
                                    texCoord, &geom.color, &geom.vertexAttrs);
            }
            else
            {
                drawWrap->addVertex(coordAdapter,scale,startLoc3d, nullptr, rot, coord,
                                    texCoord, &geom.color, &geom.vertexAttrs);
            }
        }
        for (unsigned int jj=0;jj<geom.coords.size()-2;jj++)
        {
            drawWrap->addTri(0+baseVert, jj+1+baseVert, jj+2+baseVert);
        }
    }
}
    
void ScreenSpaceBuilder::buildDrawables(std::vector<BasicDrawableRef> &draws)
{
    draws.reserve(fullDrawables.size() + drawables.size());
    for (const auto &it : fullDrawables)
    {
        draws.push_back(it->getDrawableBuilder()->getDrawable());
    }
    fullDrawables.clear();

    for (const auto &it : drawables)
    {
        draws.push_back(it.second->getDrawableBuilder()->getDrawable());
    }
    drawables.clear();
}
    
std::vector<BasicDrawableRef> ScreenSpaceBuilder::flushChanges(ChangeSet &changes,SimpleIDSet &drawIDs)
{
    return flushChanges(changes, &drawIDs);
}

std::vector<BasicDrawableRef> ScreenSpaceBuilder::flushChanges(ChangeSet &changes,SimpleIDSet *drawIDs)
{
    std::vector<BasicDrawableRef> draws;
    buildDrawables(draws);

    for (const auto &draw : draws)
    {
        if (drawIDs)
        {
            drawIDs->insert(draw->getId());
        }
        changes.push_back(new AddDrawableReq(draw));
    }

    return draws;
}

ScreenSpaceConvexGeometry::ScreenSpaceConvexGeometry(ScreenSpaceConvexGeometry &&other) noexcept :
    texIDs         (std::move(other.texIDs)),
    progID         (other.progID),
    color          (other.color),
    drawOrder      (other.drawOrder),
    drawPriority   (other.drawPriority),
    renderTargetID (other.renderTargetID),
    vertexAttrs    (std::move(other.vertexAttrs)),
    coords         (std::move(other.coords)),
    texCoords      (std::move(other.texCoords))
{
}

ScreenSpaceConvexGeometry& ScreenSpaceConvexGeometry::operator=(ScreenSpaceConvexGeometry &&other) noexcept
{
    if (this != &other)
    {
        texIDs         = std::move(other.texIDs);
        progID         = other.progID;
        color          = other.color;
        drawOrder      = other.drawOrder;
        drawPriority   = other.drawPriority;
        renderTargetID = other.renderTargetID;
        vertexAttrs    = std::move(other.vertexAttrs);
        coords         = std::move(other.coords);
        texCoords      = std::move(other.texCoords);
    }
    return *this;
}

ScreenSpaceObject::ScreenSpaceObject(SimpleIdentity theID) :
        Identifiable(theID)
{
}

ScreenSpaceObject::ScreenSpaceObject(ScreenSpaceObject &&other) noexcept :
    Identifiable(other.myId),
    enable      (other.enable),
    startEnable (other.startEnable),
    endEnable   (other.endEnable),
    worldLoc    (std::move(other.worldLoc)),
    endWorldLoc (std::move(other.endWorldLoc)),
    startTime   (other.startTime),
    endTime     (other.endTime),
    offset      (std::move(other.offset)),
    rotation    (other.rotation),
    orderBy     (other.orderBy),
    keepUpright (other.keepUpright),
    state       (std::move(other.state)),
    geometry    (std::move(other.geometry))
{
}

ScreenSpaceObject &ScreenSpaceObject::operator=(ScreenSpaceObject &&other) noexcept
{
    if (this != &other)
    {
        this->Identifiable::operator=(other);
        enable      = other.enable;
        startEnable = other.startEnable;
        endEnable   = other.endEnable;
        worldLoc    = other.worldLoc;
        endWorldLoc = other.endWorldLoc;
        startTime   = other.startTime;
        endTime     = other.endTime;
        offset      = other.offset;
        rotation    = other.rotation;
        orderBy     = other.orderBy;
        keepUpright = other.keepUpright;
        state       = std::move(other.state);
        geometry    = std::move(other.geometry);
    }
    return *this;
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
    
void ScreenSpaceObject::setMovingLoc(const Point3d &inWorldLoc,TimeInterval inStartTime,TimeInterval inEndTime)
{
    state.motion = true;
    endWorldLoc = inWorldLoc;
    startTime = inStartTime;
    endTime = inEndTime;
}

Point3d ScreenSpaceObject::getEndWorldLoc() const
{
    return endWorldLoc;
}

TimeInterval ScreenSpaceObject::getStartTime() const
{
    return startTime;
}

TimeInterval ScreenSpaceObject::getEndTime() const
{
    return endTime;
}
    
Point3d ScreenSpaceObject::getWorldLoc() const
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
    state.opacityExp = std::move(opacityExp);
}

void ScreenSpaceObject::setColorExp(ColorExpressionInfoRef colorExp)
{
    state.colorExp = std::move(colorExp);
}

void ScreenSpaceObject::setScaleExp(FloatExpressionInfoRef scaleExp)
{
    state.scaleExp = std::move(scaleExp);
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

void ScreenSpaceObject::addGeometry(ScreenSpaceConvexGeometry &&geom)
{
    geometry.emplace_back(std::move(geom));
}

void ScreenSpaceObject::addGeometry(const std::vector<ScreenSpaceConvexGeometry> &geom)
{
    if (geometry.empty())
    {
        geometry.reserve(geom.size());
    }
    geometry.insert(geometry.end(), geom.begin(), geom.end());
}

void ScreenSpaceObject::addGeometry(std::vector<ScreenSpaceConvexGeometry> &&geom)
{
    if (geometry.empty())
    {
        geometry.reserve(geom.size());
    }
    geometry.insert(geometry.end(),
                    std::make_move_iterator(geom.begin()),
                    std::make_move_iterator(geom.end()));
}

SimpleIdentity ScreenSpaceObject::getTypicalProgramID()
{
    for (const auto& geom : geometry)
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

ScreenSpaceObjectLocation::ScreenSpaceObjectLocation(ScreenSpaceObjectLocation &&other) noexcept :
    shapeIDs     (std::move(other.shapeIDs)),
    dispLoc      (std::move(other.dispLoc)),
    offset       (std::move(other.offset)),
    keepUpright  (other.keepUpright),
    rotation     (other.rotation),
    pts          (std::move(other.pts)),
    mbr          (std::move(other.mbr)),
    clusterGroup (other.clusterGroup),
    clusterId    (other.clusterId)
{
}

ScreenSpaceObjectLocation& ScreenSpaceObjectLocation::operator=(ScreenSpaceObjectLocation &&other) noexcept
{
    if (this != &other)
    {
        shapeIDs     = std::move(other.shapeIDs);
        dispLoc      = std::move(other.dispLoc);
        offset       = std::move(other.offset);
        keepUpright  = other.keepUpright;
        rotation     = other.rotation;
        pts          = std::move(other.pts);
        mbr          = other.mbr;
        clusterGroup = other.clusterGroup;
        clusterId    = other.clusterId;
    }
    return *this;
}

}
