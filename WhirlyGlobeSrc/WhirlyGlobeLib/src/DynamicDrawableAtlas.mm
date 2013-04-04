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
    : name(name), singleVertexSize(singleVertexSize), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), memManager(memManager)
{
}
    
DynamicDrawableAtlas::~DynamicDrawableAtlas()
{
    for (unsigned int ii=0;ii<swapChanges.size();ii++)
        delete swapChanges[ii];
    swapChanges.clear();
}
    
bool DynamicDrawableAtlas::addDrawable(BasicDrawable *draw,std::vector<ChangeRequest *> &changes)
{
    // See if we're already representing it
    {
        DrawRepresent represent(draw->getId());
        if (drawables.find(represent) != drawables.end())
        {
            return true;
        }
    }
    
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
        if (bigDraw->isCompatible(draw))
        {
            if ((represent.elementChunkId = bigDraw->addRegion(vertData, represent.vertexPos, elementData)) != EmptyIdentity)
            {
                represent.vertexSize = [vertData length];
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
        newBigDraw->setModes(draw);
        newBigDraw->setupGL(NULL, memManager);
        changes.push_back(new AddDrawableReq(newBigDraw));
        bigDrawables.insert(newBigDraw);
        represent.bigDrawId = newBigDraw->getId();
        if ((represent.elementChunkId = newBigDraw->addRegion(vertData, represent.vertexPos, elementData)) != EmptyIdentity)
        {
            represent.vertexSize = [vertData length];
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
    bigDraw->clearRegion(represent.vertexPos, represent.vertexSize, represent.elementChunkId);
    
    return true;
}
    
    
bool DynamicDrawableAtlas::hasUpdates()
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = *it;
        if (bigDraw->hasChanges())
            return true;
    }
    
    return false;
}
    
void DynamicDrawableAtlas::swap(std::vector<ChangeRequest *> &changes,NSObject * __weak target,SEL sel)
{
    BigDrawableSwap *swapRequest = new BigDrawableSwap(target,sel);
    changes.push_back(swapRequest);

    // Toss in the other changes we've been waiting on
    changes.insert(changes.end(), swapChanges.begin(), swapChanges.end());
    swapChanges.clear();
    
    // Note: We could keep a list of changes ones if these get to be more than a few
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = *it;
        bigDraw->swap(changes,swapRequest);
    }
}
    
bool DynamicDrawableAtlas::waitingOnSwap()
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        if ((*it)->isWaitingOnSwap())
            return true;
    
    return false;
}
    
void DynamicDrawableAtlas::addSwapChanges(const std::vector<ChangeRequest *> &inSwapChanges)
{
    swapChanges.insert(swapChanges.end(), inSwapChanges.begin(), inSwapChanges.end());
}
    
void DynamicDrawableAtlas::shutdown(std::vector<ChangeRequest *> &changes)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new RemDrawableReq((*it)->getId()));
    
    bigDrawables.clear();
    drawables.clear();
}
    
}
