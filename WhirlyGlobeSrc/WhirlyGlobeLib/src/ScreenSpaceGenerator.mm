/*
 *  ScreenSpaceGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/12.
 *  Copyright 2011 mousebird consulting. All rights reserved.
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
#import "SceneRendererES1.h"
#import "GlobeView.h"

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
}

// Calculate its position and add this feature to the appropriate drawable
void ScreenSpaceGenerator::addToDrawables(ConvexShape *shape,WhirlyKitRendererFrameInfo *frameInfo,ScreenSpaceGenerator::DrawableMap &drawables)
{
    float visVal = [frameInfo.theView heightAboveSurface];
    if (!(shape->minVis == DrawVisibleInvalid || shape->maxVis == DrawVisibleInvalid ||
          ((shape->minVis <= visVal && visVal <= shape->maxVis) ||
           (shape->maxVis <= visVal && visVal <= shape->minVis))))
        return;
    
    // If it's pointed away from the user, don't bother
    if (shape->worldLoc.dot(frameInfo.eyeVec) < 0.0)
        return;
    
    // Set up the alpha scaling
    bool hasAlpha = false;
    float scale = 1.0;
    if (shape->fadeDown < shape->fadeUp)
    {
        // Heading to 1
        if (frameInfo.currentTime < shape->fadeDown)
            scale = 0.0;
        else
            if (frameInfo.currentTime > shape->fadeUp)
                scale = 1.0;
            else
            {
                scale = (frameInfo.currentTime - shape->fadeDown)/(shape->fadeUp - shape->fadeDown);
                hasAlpha = true;
            }
    } else
        if (shape->fadeUp < shape->fadeDown)
        {
            // Heading to 0
            if (frameInfo.currentTime < shape->fadeUp)
                scale = 1.0;
            else
                if (frameInfo.currentTime > shape->fadeDown)
                    scale = 0.0;
                else
                {
                    hasAlpha = true;
                    scale = 1.0-(frameInfo.currentTime - shape->fadeUp)/(shape->fadeDown - shape->fadeUp);
                }
        }
    
    // Look for an existing drawable or add one
    for (unsigned int si=0;si<shape->geom.size();si++)
    {
        SimpleGeometry &geom = shape->geom[si];

        DrawableMap::iterator it = drawables.find(geom.texID);
        BasicDrawable *draw = NULL;
        if (it == drawables.end())
        {
            draw = new BasicDrawable();
            draw->setType(GL_TRIANGLES);
            draw->setTexId(geom.texID);
            drawables[geom.texID] = draw;
        } else
            draw = it->second;

        // Project the world location to the screen
        CGPoint screenPt;
        // Note: Make this work for generic 3D views
        WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo.theView;
        if (![globeView isKindOfClass:[WhirlyGlobeView class]])
            return;
        
        Eigen::Affine3f modelTrans = frameInfo.modelTrans;
        screenPt = [globeView pointOnScreenFromSphere:shape->worldLoc transform:&modelTrans frameSize:Point2f(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight)];    
    
        // Now build the points, texture coordinates and colors
        Point2f center(screenPt.x,screenPt.y);
        int vOff = draw->getNumPoints();

        RGBAColor color(scale*geom.color.r,scale*geom.color.g,scale*geom.color.b,scale*geom.color.a);
    
        // Set up the point, including snap to make it look better
        float resScale = frameInfo.sceneRenderer.scale;
        std::vector<Point2f> pts;
        pts.resize(geom.coords.size());
        Point2f org(MAXFLOAT,MAXFLOAT);
        for (unsigned int ii=0;ii<geom.coords.size();ii++)
        {
            Point2f &coord = geom.coords[ii];
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
            draw->addTexCoord(geom.texCoords[ii]);
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
    
ScreenSpaceGenerator::ScreenSpaceGenerator()
    : Generator()
{    
}
    
ScreenSpaceGenerator::ScreenSpaceGenerator(const std::string &name)
    : Generator(name)
{
}
    
ScreenSpaceGenerator::~ScreenSpaceGenerator()
{
    for (ConvexShapeSet::iterator it = convexShapes.begin();
         it != convexShapes.end(); ++it)
    {
        delete *it;
    }
    convexShapes.clear();    
}
    
void ScreenSpaceGenerator::addConvexShapes(std::vector<ConvexShape *> inShapes)
{
    convexShapes.insert(inShapes.begin(),inShapes.end());
}

void ScreenSpaceGenerator::removeConvexShape(SimpleIdentity shapeID)
{
    ConvexShape dummyShape;
    dummyShape.setId(shapeID);
    ConvexShapeSet::iterator it = convexShapes.find(&dummyShape);
    if (it != convexShapes.end())
    {
        delete *it;
        convexShapes.erase(it);
    }
}

void ScreenSpaceGenerator::removeConvexShapes(std::vector<SimpleIdentity> &shapeIDs)
{
    for (unsigned int ii=0;ii<shapeIDs.size();ii++)
        removeConvexShape(shapeIDs[ii]);
}
    
void ScreenSpaceGenerator::generateDrawables(WhirlyKitRendererFrameInfo *frameInfo, std::vector<Drawable *> &outDrawables, std::vector<Drawable *> &screenDrawables)
{
    if (convexShapes.empty())
        return;
    
    // Keep drawables sorted by destination texture ID
    DrawableMap drawables;
    
    // Work through the markers, asking each to generate its content
    for (ConvexShapeSet::iterator it = convexShapes.begin();
         it != convexShapes.end(); ++it)
    {
        ConvexShape *shape = *it;
        addToDrawables(shape,frameInfo,drawables);
    }
    
    // Copy the drawables out
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
        screenDrawables.push_back(it->second);
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
    
void ScreenSpaceGeneratorAddRequest::execute2(Scene *scene,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
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
    
void ScreenSpaceGeneratorRemRequest::execute2(Scene *scene,Generator *gen)
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
    
void ScreenSpaceGeneratorFadeRequest::execute2(Scene *scene,Generator *gen)
{
    ScreenSpaceGenerator *screenGen = (ScreenSpaceGenerator *)gen;
    
    for (unsigned int ii=0;ii<shapeIDs.size();ii++)
    {
        ScreenSpaceGenerator::ConvexShape *shape = screenGen->getConvexShape(shapeIDs[ii]);
        if (shape)
        {
            shape->fadeUp = fadeUp;
            shape->fadeDown = fadeDown;
        }
    }    
}
    
}
