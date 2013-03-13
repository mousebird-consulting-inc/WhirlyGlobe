/*
 *  DynamicDrawableAtlas.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
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

#import "DynamicDrawableAtlas.h"
#import "Scene.h"

namespace WhirlyKit
{
    
DynamicDrawableAtlas::DynamicDrawableAtlas(const std::string &name,int singleVertexSize,int singleElementSize,int numVertexBytes,int numElementBytes,OpenGLMemManager *memManager)
    : name(name), singleVertexSize(singleVertexSize), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), memManager(memManager), drawPriority(0)
{
}
    
DynamicDrawableAtlas::~DynamicDrawableAtlas()
{    
}
    
bool DynamicDrawableAtlas::addDrawable(BasicDrawable *draw,std::vector<ChangeRequest *> &changes)
{
    // Turn the vertex and element data in to NSData objects
    NSMutableData *vertData = nil, *elementData = nil;
    draw->asVertexAndElementData(&vertData,&elementData,singleElementSize);
    if (!vertData || !elementData)
        return false;
    
    // Look for a big drawable that uses the same texture
    BigDrawable *foundBigDraw = nil;
    DrawRepresent represent(draw->getId());
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = *it;
        if (bigDraw->getTexId() == draw->getTexId() && bigDraw->getForceZBufferOn() == draw->getForceZBufferOn())
        {
            if (bigDraw->addRegion(vertData, represent.vertexPos, elementData, represent.elementPos))
            {
                represent.vertexSize = [vertData length];
                represent.elementSize = [elementData length];
                represent.bigDrawId = bigDraw->getId();
                foundBigDraw = bigDraw;
                break;
            }
        }
    }
    
    // Didn't find one, so create one
    if (!foundBigDraw)
    {
        BigDrawable *newBigDraw = new BigDrawable(name,singleVertexSize,singleElementSize,numVertexBytes,numElementBytes);
        newBigDraw->setDrawPriority(drawPriority);
        newBigDraw->setTexId(draw->getTexId());
        newBigDraw->setForceZBufferOn(draw->getForceZBufferOn());
        newBigDraw->setupGL(NULL, memManager);
        changes.push_back(new AddDrawableReq(newBigDraw));
        bigDrawables.insert(newBigDraw);
        represent.bigDrawId = newBigDraw->getId();
        if (newBigDraw->addRegion(vertData, represent.vertexPos, elementData, represent.elementPos))
        {
            represent.vertexSize = [vertData length];
            represent.elementSize = [elementData length];
            represent.bigDrawId = newBigDraw->getId();
            foundBigDraw = newBigDraw;
        }
    }
    
    // This drawable is probably too big.  Punt
    if (!foundBigDraw)
        return false;
    
    // This tracks the region by drawable ID so we can get to it later
    drawables.insert(represent);

    return true;
}

bool DynamicDrawableAtlas::removeDrawable(SimpleIdentity drawId,std::vector<ChangeRequest *> &changes)
{
    // Look for the representation
    DrawRepresent represent(drawId);
    
    // Find it and remove it
    DrawRepresentSet::iterator it = drawables.find(represent);
    if (it == drawables.end())
        return false;
    represent = *it;
    drawables.erase(it);
    
    // Find the big drawable and remove it from that
    BigDrawable *bigDraw = NULL;
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        if ((*it)->getId() == represent.bigDrawId)
        {
            bigDraw = *it;
            break;
        }
    }
    
    if (!bigDraw)
        return false;
    
    // Set up the requests to clear the region
    bigDraw->clearRegion(represent.vertexPos, represent.vertexSize, represent.elementPos, represent.elementSize);
    
    return true;
}
    
void DynamicDrawableAtlas::flush(std::vector<ChangeRequest *> &changes)
{
    // Note: We could keep a list of changes ones if these get to be more than a few
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = *it;
        bigDraw->flush(changes);
    }
}
    
void DynamicDrawableAtlas::shutdown(std::vector<ChangeRequest *> &changes)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new RemDrawableReq((*it)->getId()));
    
    bigDrawables.clear();
    drawables.clear();
}
    
}
