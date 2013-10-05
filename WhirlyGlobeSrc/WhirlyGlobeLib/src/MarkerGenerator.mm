/*
 *  MarkerGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/14/11.
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

#import "MarkerGenerator.h"
#import "SceneRendererES.h"

namespace WhirlyKit
{

// Add this marker to the appropriate drawable
void MarkerGenerator::Marker::addToDrawables(WhirlyKit::RendererFrameInfo *frameInfo,DrawableMap &drawables,float minZres)
{
    if (!enable)
        return;
    float visVal = [frameInfo->theView heightAboveSurface];
    if (!(minVis == DrawVisibleInvalid || maxVis == DrawVisibleInvalid ||
         ((minVis <= visVal && visVal <= maxVis) ||
          (maxVis <= visVal && visVal <= minVis))))
        return;
    
    // If it's pointed away from the user, don't bother
    if (norm.dot(frameInfo->eyeVec) < 0.0)
        return;
    
    float where = fmod(frameInfo->currentTime - start,period);
    int which = where/(float)period * texIDs.size();

    std::vector<TexCoord> &theTexCoords = texCoords[which];
    SimpleIdentity texID = texIDs[which];
    
    // Look for an existing Drawable or set one up
    DrawableMap::iterator it = drawables.find(texID);
    BasicDrawable *draw = NULL;
    if (it == drawables.end())
    {
        draw = new BasicDrawable("Marker Generator");
        draw->setType(GL_TRIANGLES);
        draw->setTexId(0,texID);
        drawables[texID] = draw;
    } else
        draw = it->second;
    
    // Deal with a draw offset
    Point3f thePts[4];
    for (unsigned int ii=0;ii<4;ii++)
        thePts[ii] = pts[ii];
    if (drawOffset != 0)
    {        
		float scale = minZres*drawOffset;
		for (unsigned int ii=0;ii<4;ii++)
		{
            Eigen::Vector3f pt = thePts[ii];
			thePts[ii] = norm * scale + pt;
		}
    }
    
    // Add the geometry to the drawable
    bool hasAlpha = false;
    int vOff = draw->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        draw->addPoint(thePts[ii]);
        draw->addNormal(norm);
        draw->addTexCoord(0,theTexCoords[ii]);
        float scale = 1.0;
        if (fadeDown < fadeUp)
        {
            // Heading to 1
            if (frameInfo->currentTime < fadeDown)
                scale = 0.0;
            else
                if (frameInfo->currentTime > fadeUp)
                    scale = 1.0;
                else
                {
                    scale = (frameInfo->currentTime - fadeDown)/(fadeUp - fadeDown);
                    hasAlpha = true;
                }
        } else
            if (fadeUp < fadeDown)
            {
                // Heading to 0
                if (frameInfo->currentTime < fadeUp)
                    scale = 1.0;
                else
                    if (frameInfo->currentTime > fadeDown)
                        scale = 0.0;
                    else
                    {
                        hasAlpha = true;
                        scale = 1.0-(frameInfo->currentTime - fadeUp)/(fadeDown - fadeUp);
                    }
            }
        draw->addColor(RGBAColor(scale*color.r,scale*color.g,scale*color.b,scale*color.a));
    }
    draw->setAlpha(hasAlpha);
    draw->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
    draw->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));

    Mbr localMbr = draw->getLocalMbr();
    localMbr.addPoint(loc);
    draw->setLocalMbr(localMbr);
    // Note: Should really be sorting by this, but that's kinda nuts
    int oldDrawPriority = draw->getDrawPriority();
    draw->setDrawPriority((drawPriority > oldDrawPriority) ? drawPriority : oldDrawPriority);}

MarkerGenerator::MarkerGenerator()
{    
}
    
MarkerGenerator::~MarkerGenerator()
{
    for (MarkerSet::iterator it = markers.begin();
         it != markers.end(); ++it)
    {
        delete *it;
    }
    markers.clear();
}
    
void MarkerGenerator::addMarker(Marker *marker)
{
    markers.insert(marker);
}
    
void MarkerGenerator::addMarkers(std::vector<Marker *> inMarkers)
{
    markers.insert(inMarkers.begin(),inMarkers.end());
}
    
void MarkerGenerator::enableMarkers(std::vector<SimpleIdentity> &markerIDs,bool enable)
{
    for (unsigned int ii=0;ii<markerIDs.size();ii++)
    {
        Marker dummyMarker;
        dummyMarker.setId(markerIDs[ii]);
        MarkerSet::iterator it = markers.find(&dummyMarker);
        if (it != markers.end())
            (*it)->enable = enable;
    }
}
    
void MarkerGenerator::removeMarker(SimpleIdentity markerId)
{
    Marker dummyMarker;
    dummyMarker.setId(markerId);
    MarkerSet::iterator it = markers.find(&dummyMarker);
    if (it != markers.end())
    {
        delete *it;
        markers.erase(it);
    }
}
    
void MarkerGenerator::removeMarkers(std::vector<SimpleIdentity> &markerIDs)
{
    for (unsigned int ii=0;ii<markerIDs.size();ii++)
        removeMarker(markerIDs[ii]);
}
    
MarkerGenerator::Marker *MarkerGenerator::getMarker(SimpleIdentity markerId)
{
    Marker dummyMarker;
    dummyMarker.setId(markerId);
    MarkerSet::iterator it = markers.find(&dummyMarker);
    if (it != markers.end())
        return *it;
    
    return NULL;
}
    
void MarkerGenerator::generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo, std::vector<DrawableRef> &outDrawables, std::vector<DrawableRef> &screenDrawables)
{
    if (markers.empty())
        return;

    float minZres = [frameInfo->theView calcZbufferRes];
    
    // Keep drawables sorted by destination texture ID
    DrawableMap drawables;
    
    // Work through the markers, asking each to generate its content
    for (MarkerSet::iterator it = markers.begin();
         it != markers.end(); ++it)
    {
        Marker *marker = *it;
        marker->addToDrawables(frameInfo,drawables,minZres);
    }

    // Copy the drawables out
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
        outDrawables.push_back(DrawableRef(it->second));
}

void MarkerGenerator::dumpStats()
{
    NSLog(@"Marker Generator: %ld markers",markers.size());
}

MarkerGeneratorAddRequest::MarkerGeneratorAddRequest(SimpleIdentity genId,MarkerGenerator::Marker *marker)
    : GeneratorChangeRequest(genId)
{   
    markers.push_back(marker);
}

MarkerGeneratorAddRequest::MarkerGeneratorAddRequest(SimpleIdentity genId,const std::vector<MarkerGenerator::Marker *> &markers)
    : GeneratorChangeRequest(genId), markers(markers)
{    
}

MarkerGeneratorAddRequest::~MarkerGeneratorAddRequest()
{
    for (unsigned int ii=0;ii<markers.size();ii++)
        delete markers[ii];
    markers.clear();
}
    
void MarkerGeneratorAddRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    MarkerGenerator *markerGen = (MarkerGenerator *)gen;
    markerGen->addMarkers(markers);
    markers.clear();
}
    
MarkerGeneratorRemRequest::MarkerGeneratorRemRequest(SimpleIdentity genID,SimpleIdentity markerID)
    : GeneratorChangeRequest(genID)
{    
    markerIDs.push_back(markerID);
}
    
MarkerGeneratorRemRequest::MarkerGeneratorRemRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &markerIDs)
    : GeneratorChangeRequest(genID), markerIDs(markerIDs)
{
    
}
    
MarkerGeneratorRemRequest::~MarkerGeneratorRemRequest()
{    
}
    
void MarkerGeneratorRemRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    MarkerGenerator *markerGen = (MarkerGenerator *)gen;
    markerGen->removeMarkers(markerIDs);
}
    
MarkerGeneratorEnableRequest::MarkerGeneratorEnableRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &markerIDs,bool enable) : GeneratorChangeRequest(genID), enable(enable), markerIDs(markerIDs)
{
    
}
    
MarkerGeneratorEnableRequest::~MarkerGeneratorEnableRequest()
{
}
    
void MarkerGeneratorEnableRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    MarkerGenerator *markerGen = (MarkerGenerator *)gen;
    markerGen->enableMarkers(markerIDs,enable);
}
    
MarkerGeneratorFadeRequest::MarkerGeneratorFadeRequest(SimpleIdentity genID,SimpleIdentity markerID,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : GeneratorChangeRequest(genID), fadeUp(fadeUp), fadeDown(fadeDown)
{    
    markerIDs.push_back(markerID);
}

MarkerGeneratorFadeRequest::MarkerGeneratorFadeRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> markerIDs,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : GeneratorChangeRequest(genID), fadeUp(fadeUp), fadeDown(fadeDown), markerIDs(markerIDs)
{    
}
    
void MarkerGeneratorFadeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{    
    MarkerGenerator *markerGen = (MarkerGenerator *)gen;
    
    for (unsigned int ii=0;ii<markerIDs.size();ii++)
    {
        MarkerGenerator::Marker *marker = markerGen->getMarker(markerIDs[ii]);
        if (marker)
        {
            marker->fadeUp = fadeUp;
            marker->fadeDown = fadeDown;
        }
    }
}


}
