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

// Calculate its position and add this feature to the appropriate drawable
void ScreenSpaceGenerator::ConvexShape::addToDrawables(WhirlyKitRendererFrameInfo *frameInfo,ScreenSpaceGenerator::DrawableMap &drawables)
{
    float visVal = [frameInfo.theView heightAboveSurface];
    if (!(minVis == DrawVisibleInvalid || maxVis == DrawVisibleInvalid ||
          ((minVis <= visVal && visVal <= maxVis) ||
           (maxVis <= visVal && visVal <= minVis))))
        return;
    
    // If it's pointed away from the user, don't bother
    if (worldLoc.dot(frameInfo.eyeVec) < 0.0)
        return;
    
    // Look for an existing drawable or add one
    DrawableMap::iterator it = drawables.find(texID);
    BasicDrawable *draw = NULL;
    if (it == drawables.end())
    {
        draw = new BasicDrawable();
        draw->setType(GL_TRIANGLES);
        draw->setTexId(texID);
        drawables[texID] = draw;
    } else
        draw = it->second;

    // Project the world location to the screen
    CGPoint screenPt;
    // Note: Make this work for generic 3D views
    WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo.theView;
    if (![globeView isKindOfClass:[WhirlyGlobeView class]])
        return;
    
    Eigen::Affine3f modelTrans = frameInfo.modelTrans;
    screenPt = [globeView pointOnScreenFromSphere:worldLoc transform:&modelTrans frameSize:Point2f(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight)];    
    
    // Now build the points, texture coordinates and colors
    Point3f center(screenPt.x,screenPt.y,0.0);
    bool hasAlpha = false;
    int vOff = draw->getNumPoints();
    for (unsigned int ii=0;ii<coords.size();ii++)
    {
        Point2f coord = coords[ii];
        draw->addPoint(Point3f(coord.x(),coord.y(),0.0)+center);
        draw->addTexCoord(texCoords[ii]);
        float scale = 1.0;
        if (fadeDown < fadeUp)
        {
            // Heading to 1
            if (frameInfo.currentTime < fadeDown)
                scale = 0.0;
            else
                if (frameInfo.currentTime > fadeUp)
                    scale = 1.0;
                else
                {
                    scale = (frameInfo.currentTime - fadeDown)/(fadeUp - fadeDown);
                    hasAlpha = true;
                }
        } else
            if (fadeUp < fadeDown)
            {
                // Heading to 0
                if (frameInfo.currentTime < fadeUp)
                    scale = 1.0;
                else
                    if (frameInfo.currentTime > fadeDown)
                        scale = 0.0;
                    else
                    {
                        hasAlpha = true;
                        scale = 1.0-(frameInfo.currentTime - fadeUp)/(fadeDown - fadeUp);
                    }
            }
        draw->addColor(RGBAColor(scale*color.r,scale*color.g,scale*color.b,scale*color.a));
    }
    draw->setAlpha(hasAlpha);
    
    // Build the triangles
    for (unsigned int ii=2;ii<coords.size();ii++)
    {
        draw->addTriangle(BasicDrawable::Triangle(0+vOff,ii-1+vOff,ii+vOff));
    }
    
    int oldDrawPriority = draw->getDrawPriority();
    draw->setDrawPriority((drawPriority > oldDrawPriority) ? drawPriority : oldDrawPriority);
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
        shape->addToDrawables(frameInfo,drawables);
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
