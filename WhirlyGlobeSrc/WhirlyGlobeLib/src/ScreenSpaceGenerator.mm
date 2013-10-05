/*
 *  ScreenSpaceGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/12.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "ScreenSpaceGenerator.h"
#import "SceneRendererES.h"
#import "GlobeView.h"
#import "MaplyView.h"
#import "GlobeMath.h"

using namespace Eigen;

namespace WhirlyKit
{
    
ScreenSpaceGenerator::SimpleGeometry::SimpleGeometry()
    : texID(EmptyIdentity), color(255,255,255,255)
{
}

ScreenSpaceGenerator::SimpleGeometry::SimpleGeometry(SimpleIdentity texID,RGBAColor color,const std::vector<Point2f> &coords,const std::vector<TexCoord> &texCoords)
    : texID(texID), color(color), coords(coords), texCoords(texCoords)
{    
}

ScreenSpaceGenerator::ConvexShape::ConvexShape()
{
    minVis = maxVis = DrawVisibleInvalid;
    fadeUp = fadeDown = 0.0;
    drawPriority = 0;
    useRotation = false;
    rotation = 0.0;
    offset.x() = 0.0;
    offset.y() = 0.0;
    enable = true;
}

// Calculate its position and add this feature to the appropriate drawable
void ScreenSpaceGenerator::addToDrawables(ConvexShape *shape,WhirlyKit::RendererFrameInfo *frameInfo,ScreenSpaceGenerator::DrawableMap &drawables,Mbr &frameMbr,std::vector<ProjectedPoint> &projPts)
{
    float visVal = [frameInfo->theView heightAboveSurface];
    if (!shape->enable || !(shape->minVis == DrawVisibleInvalid || shape->maxVis == DrawVisibleInvalid ||
          ((shape->minVis <= visVal && visVal <= shape->maxVis) ||
           (shape->maxVis <= visVal && visVal <= shape->minVis))))
        return;
        
    // Project the world location to the screen
    CGPoint screenPt;
    Eigen::Matrix4d modelTrans = Matrix4fToMatrix4d(frameInfo->viewAndModelMat);

    WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo->theView;
    MaplyView *mapView = (MaplyView *)frameInfo->theView;
    if ([globeView isKindOfClass:[WhirlyGlobeView class]])
    {
        mapView = nil;
        // Make sure this one is facing toward the viewer
        if (CheckPointAndNormFacing(shape->worldLoc,shape->worldLoc.normalized(),frameInfo->viewAndModelMat,frameInfo->viewModelNormalMat) < 0.0)
            return;

        // Note: Need to move to the view frustum logic
        screenPt = [globeView pointOnScreenFromSphere:Vector3fToVector3d(shape->worldLoc) transform:&modelTrans frameSize:Point2f(frameInfo->sceneRenderer.framebufferWidth,frameInfo->sceneRenderer.framebufferHeight)];
    } else {
        globeView = nil;
        if ([mapView isKindOfClass:[MaplyView class]])
            screenPt = [mapView pointOnScreenFromPlane:Vector3fToVector3d(shape->worldLoc) transform:&modelTrans frameSize:Point2f(frameInfo->sceneRenderer.framebufferWidth,frameInfo->sceneRenderer.framebufferHeight)];
        else
            // No idea what this could be
            return;
    }
            
    // Note: This check is too simple
    if (screenPt.x < frameMbr.ll().x() || screenPt.y < frameMbr.ll().y() || 
        screenPt.x > frameMbr.ur().x() || screenPt.y > frameMbr.ur().y())
        return;
    
    float resScale = frameInfo->sceneRenderer.scale;

    screenPt.x += shape->offset.x()*resScale;
    screenPt.y += shape->offset.y()*resScale;
    
    // It survived, so add it to the list if someone else needs to know where they wound up
    ProjectedPoint projPt;
    projPt.shapeID = shape->getId();
    projPt.screenLoc = Point2f(screenPt.x,screenPt.y);
    projPts.push_back(projPt);

    // If we need to do a rotation, throw out a point along the vector and see where it goes
    float screenRot = 0.0;
    Matrix2f screenRotMat;
    if (shape->useRotation)
    {
        Point3f norm,right,up;
        
        if (globeView)
        {
            Point3f simpleUp(0,0,1);
            norm = shape->worldLoc;
            norm.normalize();
            right = simpleUp.cross(norm);
            up = norm.cross(right);
            right.normalize();
            up.normalize();
        } else {
            right = Point3f(1,0,0);
            norm = Point3f(0,0,1);
            up = Point3f(0,1,0);
        }
        // Note: Check if the axes made any sense.  We might be at a pole.
        Point3f rightDir = right * sinf(shape->rotation);
        Point3f upDir = up * cosf(shape->rotation);
        
        Point3f outPt = rightDir * 1.0 + upDir * 1.0 + shape->worldLoc;
        CGPoint outScreenPt;
        if (globeView)
            outScreenPt = [globeView pointOnScreenFromSphere:Vector3fToVector3d(outPt) transform:&modelTrans frameSize:Point2f(frameInfo->sceneRenderer.framebufferWidth,frameInfo->sceneRenderer.framebufferHeight)];
        else
            outScreenPt = [mapView pointOnScreenFromPlane:Vector3fToVector3d(outPt) transform:&modelTrans frameSize:Point2f(frameInfo->sceneRenderer.framebufferWidth,frameInfo->sceneRenderer.framebufferHeight)];
        screenRot = M_PI/2.0-atan2f(screenPt.y-outScreenPt.y,outScreenPt.x-screenPt.x);
        screenRotMat = Eigen::Rotation2Df(screenRot);
    }

    // Set up the alpha scaling
    bool hasAlpha = false;
    float scale = 1.0;
    if (shape->fadeDown < shape->fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < shape->fadeDown)
            scale = 0.0;
        else
            if (frameInfo->currentTime > shape->fadeUp)
                scale = 1.0;
            else
            {
                scale = (frameInfo->currentTime - shape->fadeDown)/(shape->fadeUp - shape->fadeDown);
                hasAlpha = true;
            }
    } else
        if (shape->fadeUp < shape->fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < shape->fadeUp)
                scale = 1.0;
            else
                if (frameInfo->currentTime > shape->fadeDown)
                    scale = 0.0;
                else
                {
                    hasAlpha = true;
                    scale = 1.0-(frameInfo->currentTime - shape->fadeUp)/(shape->fadeDown - shape->fadeUp);
                }
        }

    // Work through the individual pieces of geometry
    for (unsigned int si=0;si<shape->geom.size();si++)
    {
        SimpleGeometry &geom = shape->geom[si];

        DrawableMap::iterator it = drawables.find(geom.texID);
        BasicDrawable *draw = NULL;
        if (it == drawables.end())
        {
            draw = new BasicDrawable("Screen Space Generator");
            draw->setType(GL_TRIANGLES);
            draw->setTexId(0,geom.texID);
            drawables[geom.texID] = draw;
        } else
            draw = it->second;

        // Now build the points, texture coordinates and colors
        Point2f center(screenPt.x,screenPt.y);
        int vOff = draw->getNumPoints();

        RGBAColor color(scale*geom.color.r,scale*geom.color.g,scale*geom.color.b,scale*geom.color.a);
    
        // Set up the point, including snap to make it look better
        float resScale = frameInfo->sceneRenderer.scale;
        std::vector<Point2f> pts;
        pts.resize(geom.coords.size());
        Point2f org(MAXFLOAT,MAXFLOAT);
        for (unsigned int ii=0;ii<geom.coords.size();ii++)
        {
            Point2f coord = geom.coords[ii];
            if (screenRot != 0.0)
                coord = screenRotMat * coord;
            pts[ii] = Point2f(coord.x()*resScale,coord.y()*resScale)+center;
            org.x() = std::min(pts[ii].x(),org.x());
            org.y() = std::min(pts[ii].y(),org.y());
        }
    
    // Snapping doesn't look that good
#if 0
        float orgX = org.x() - floorf(org.x());
        float orgY = org.y() - floorf(org.y());
        if (orgX < 0.5)
            orgX = -orgX;
        else
            orgX = 1.0 - orgX;
        if (orgY < 0.5)
            orgY = -orgY;
        else
            orgY = 1.0 - orgY;
#else
        float orgX = 0.0;
        float orgY = 0.0;
#endif
    
        for (unsigned int ii=0;ii<geom.coords.size();ii++)
        {
            Point2f coord = pts[ii];
            draw->addPoint(Point3f(coord.x()+orgX,coord.y()+orgY,0.0));
            draw->addTexCoord(0,geom.texCoords[ii]);
            draw->addColor(color);
            draw->addNormal(Point3f(0,0,1));
        }
        draw->setAlpha(hasAlpha);

        // Build the triangles
        for (unsigned int ii=2;ii<geom.coords.size();ii++)
        {
            draw->addTriangle(BasicDrawable::Triangle(0+vOff,ii+vOff,ii-1+vOff));
        }
        
        int oldDrawPriority = draw->getDrawPriority();
        draw->setDrawPriority((shape->drawPriority > oldDrawPriority) ? shape->drawPriority : oldDrawPriority);
    }    
}
    
ScreenSpaceGenerator::ScreenSpaceGenerator(const std::string &name,Point2f margin)
    : Generator(name), margin(margin)
{
    pthread_mutex_init(&projectedPtsLock,NULL);    
}
    
ScreenSpaceGenerator::~ScreenSpaceGenerator()
{
    pthread_mutex_destroy(&projectedPtsLock);
    
    for (ConvexShapeSet::iterator it = convexShapes.begin();
         it != convexShapes.end(); ++it)
    {
        delete *it;
    }
    convexShapes.clear();    
    activeShapes.clear();
}
    
void ScreenSpaceGenerator::addConvexShapes(std::vector<ConvexShape *> inShapes)
{
    convexShapes.insert(inShapes.begin(),inShapes.end());
    for (unsigned int ii=0;ii<inShapes.size();ii++)
    {
        ConvexShape *shape = inShapes[ii];
        if (shape->enable && shape->offset.x() != MAXFLOAT)
            activeShapes.insert(shape);
    }
}

void ScreenSpaceGenerator::removeConvexShape(SimpleIdentity shapeID)
{
    ConvexShape dummyShape;
    dummyShape.setId(shapeID);
    ConvexShapeSet::iterator it = convexShapes.find(&dummyShape);
    ConvexShape *theShape = NULL;
    if (it != convexShapes.end())
    {
        theShape = *it;
        convexShapes.erase(it);
    }
    if (theShape)
    {
        it = activeShapes.find(theShape);
        if (it != activeShapes.end())
        {
            activeShapes.erase(it);
        }
        delete theShape;
    }
}

void ScreenSpaceGenerator::removeConvexShapes(std::vector<SimpleIdentity> &shapeIDs)
{
    for (unsigned int ii=0;ii<shapeIDs.size();ii++)
        removeConvexShape(shapeIDs[ii]);
}
    
void ScreenSpaceGenerator::generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo, std::vector<DrawableRef> &outDrawables, std::vector<DrawableRef> &screenDrawables)
{
    if (activeShapes.empty())
        return;
    
    // Keep drawables sorted by destination texture ID
    DrawableMap drawables;
    
    // Overall extents we'll look at.  Everything else is tossed.
    Mbr frameMbr;
    float marginX = frameInfo->sceneRenderer.framebufferWidth * margin.x();
    float marginY = frameInfo->sceneRenderer.framebufferHeight * margin.y();
    frameMbr.ll() = Point2f(0 - marginX,0 - marginY);
    frameMbr.ur() = Point2f(frameInfo->sceneRenderer.framebufferWidth + marginX,frameInfo->sceneRenderer.framebufferHeight + marginY);
    
    // Keep track of where the shapes wound up
    std::vector<ProjectedPoint> newProjPts;
    
    // Work through the markers, asking each to generate its content
    for (ConvexShapeSet::iterator it = activeShapes.begin();
         it != activeShapes.end(); ++it)
    {
        ConvexShape *shape = *it;
        addToDrawables(shape,frameInfo,drawables,frameMbr,newProjPts);
    }
    
    // Copy the drawables out
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
        screenDrawables.push_back(DrawableRef(it->second));
    
    // Now put the projected points in place
    pthread_mutex_lock(&projectedPtsLock);
    projectedPoints = newProjPts;
    pthread_mutex_unlock(&projectedPtsLock);
}
    
void ScreenSpaceGenerator::getProjectedPoints(std::vector<ProjectedPoint> &projPoints)
{
    pthread_mutex_lock(&projectedPtsLock);
    projPoints = projectedPoints;
    pthread_mutex_unlock(&projectedPtsLock);    
}
    
ScreenSpaceGenerator::ConvexShape *ScreenSpaceGenerator::getConvexShape(SimpleIdentity markerId)
{
    ConvexShape dummyShape;
    dummyShape.setId(markerId);
    ConvexShapeSet::iterator it = convexShapes.find(&dummyShape);
    if (it != convexShapes.end())
        return *it;
    
    return NULL;
}

void ScreenSpaceGenerator::changeEnable(ConvexShape *shape,bool enable)
{
    if (shape->enable)
        activeShapes.erase(shape);
    if (enable && shape->offset.x() != MAXFLOAT)
        activeShapes.insert(shape);

    shape->enable = enable;
}

void ScreenSpaceGenerator::dumpStats()
{
    pthread_mutex_lock(&projectedPtsLock);
    NSLog(@"ScreenSpace Generator: %ld shapes, %ld active",convexShapes.size(),activeShapes.size());
    NSLog(@"ScreenSpace Generator: %ld projected points",projectedPoints.size());
    pthread_mutex_unlock(&projectedPtsLock);
}


ScreenSpaceGeneratorAddRequest::ScreenSpaceGeneratorAddRequest(SimpleIdentity genID,ScreenSpaceGenerator::ConvexShape *shape)
    : GeneratorChangeRequest(genID)
{
    shapes.push_back(shape);
}
    
ScreenSpaceGeneratorAddRequest::ScreenSpaceGeneratorAddRequest(SimpleIdentity genID,const std::vector<ScreenSpaceGenerator::ConvexShape *> &inShapes)
    : GeneratorChangeRequest(genID)
{
    shapes = inShapes;
}
    
ScreenSpaceGeneratorAddRequest::~ScreenSpaceGeneratorAddRequest()
{
    for (unsigned int ii=0;ii<shapes.size();ii++)
        delete shapes[ii];
    shapes.clear();
}
    
void ScreenSpaceGeneratorAddRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    for (unsigned int ii=0;ii<shapes.size();ii++)
    {
        [renderer setRenderUntil:shapes[ii]->fadeUp];
        [renderer setRenderUntil:shapes[ii]->fadeDown];
    }
    screenGen->addConvexShapes(shapes);
    shapes.clear();
}
    
ScreenSpaceGeneratorRemRequest::ScreenSpaceGeneratorRemRequest(SimpleIdentity genID,SimpleIdentity shapeID)
    : GeneratorChangeRequest(genID)
{
    shapeIDs.push_back(shapeID);
}
    
ScreenSpaceGeneratorRemRequest::ScreenSpaceGeneratorRemRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &inShapeIDs)
    : GeneratorChangeRequest(genID), shapeIDs(inShapeIDs)
{
}
   
ScreenSpaceGeneratorRemRequest::~ScreenSpaceGeneratorRemRequest()
{
}
    
void ScreenSpaceGeneratorRemRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    screenGen->removeConvexShapes(shapeIDs);
}
    
ScreenSpaceGeneratorFadeRequest::ScreenSpaceGeneratorFadeRequest(SimpleIdentity genID,SimpleIdentity shapeID,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : GeneratorChangeRequest(genID), fadeUp(fadeUp), fadeDown(fadeDown)
{
    shapeIDs.push_back(shapeID);
}

ScreenSpaceGeneratorFadeRequest::ScreenSpaceGeneratorFadeRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> shapeIDs,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : GeneratorChangeRequest(genID), fadeUp(fadeUp), fadeDown(fadeDown), shapeIDs(shapeIDs)
{
}

ScreenSpaceGeneratorFadeRequest::~ScreenSpaceGeneratorFadeRequest()
{        
}
    
void ScreenSpaceGeneratorFadeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    
    for (unsigned int ii=0;ii<shapeIDs.size();ii++)
    {
        ScreenSpaceGenerator::ConvexShape *shape = screenGen->getConvexShape(shapeIDs[ii]);
        if (shape)
        {
            shape->fadeUp = fadeUp;
            shape->fadeDown = fadeDown;
            [renderer setRenderUntil:fadeUp];
            [renderer setRenderUntil:fadeDown];
        }
    }    
}
    
ScreenSpaceGeneratorEnableRequest::ScreenSpaceGeneratorEnableRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &shapeIDs,bool enable)
    : GeneratorChangeRequest(genID), enable(enable), shapeIDs(shapeIDs)
{
}
    
void ScreenSpaceGeneratorEnableRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    
    for (unsigned int ii=0;ii<shapeIDs.size();ii++)
    {
        ScreenSpaceGenerator::ConvexShape *shape = screenGen->getConvexShape(shapeIDs[ii]);
        if (shape)
        {
            screenGen->changeEnable(shape,enable);
        }
    }
}
    
ScreenSpaceGeneratorGangChangeRequest::ShapeChange::ShapeChange()
 : shapeID(EmptyIdentity), fadeUp(0.0), fadeDown(0.0), offset(0.0,0.0)
{
}
    
ScreenSpaceGeneratorGangChangeRequest::ScreenSpaceGeneratorGangChangeRequest(SimpleIdentity genID,const std::vector<ShapeChange> &changes)
    : GeneratorChangeRequest(genID), changes(changes)
{
}
    
void ScreenSpaceGeneratorGangChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    
    for (unsigned int ii=0;ii<changes.size();ii++)
    {
        ShapeChange &change = changes[ii];
        ScreenSpaceGenerator::ConvexShape *shape = screenGen->getConvexShape(change.shapeID);
        if (shape)
        {
            shape->fadeUp = change.fadeUp;
            shape->fadeDown = change.fadeDown;
            shape->offset = change.offset;
            screenGen->changeEnable(shape,shape->enable);
            [renderer setRenderUntil:change.fadeUp];
            [renderer setRenderUntil:change.fadeDown];
        }
    }
}
    
}
