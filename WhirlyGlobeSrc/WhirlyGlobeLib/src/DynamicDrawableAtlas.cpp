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
    
DynamicDrawableAtlas::DynamicDrawableAtlas(const std::string &name,int singleElementSize,int numVertexBytes,int numElementBytes,OpenGLMemManager *memManager,BigDrawable *(*newBigDrawable)(BasicDrawable *draw,int singleElementSize,int numVertexBytes,int numElementBytes),
                                           SimpleIdentity shaderId)
    : name(name), singleVertexSize(0), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), memManager(memManager), newBigDrawable(newBigDrawable), shaderId(shaderId), enable(true), hasChanges(false)
{
}
    
DynamicDrawableAtlas::~DynamicDrawableAtlas()
{
    for (unsigned int ii=0;ii<swapChanges.size();ii++)
        delete swapChanges[ii];
    swapChanges.clear();
}
    
bool DynamicDrawableAtlas::addDrawable(BasicDrawable *draw,ChangeSet &changes,bool enabled,std::vector<SimpleIdentity> *destTexIDs,bool *addedNewBigDrawable,const Point3d *center,double objSize)
{
    hasChanges = true;

    if (addedNewBigDrawable)
        *addedNewBigDrawable = false;

    // See if we're already representing it
    {
        DrawRepresent represent(draw->getId());
        if (drawables.find(represent) != drawables.end())
        {
            return true;
        }
    }
    
    // Only doing triangles at the moment
    if (draw->getType() != GL_TRIANGLES)
        return false;
    
    // If this is the first one we've seen, we'll configure ourselves to match it
    const std::vector<VertexAttribute *> &drawVertexAttributes = draw->getVertexAttributes();
    if (singleVertexSize == 0)
    {
        singleVertexSize = draw->singleVertexSize();
        vertexAttributes.clear();
        for (unsigned int ii=0;ii<drawVertexAttributes.size();ii++)
        {
            vertexAttributes.push_back(drawVertexAttributes[ii]->templateCopy());
        }
    }

    if (singleVertexSize != draw->singleVertexSize() || vertexAttributes.size() != drawVertexAttributes.size() || shaderId != draw->getProgram())
    {
//        NSLog(@"DynamicDrawableAtlas::addDrawable(): Drawable mismatch.  Punting drawable.");
        return false;
    }
    for (unsigned int ii=0;ii<drawVertexAttributes.size();ii++)
        // Note: Comparison could be more comprehensive
        if (vertexAttributes[ii].getDataType() != drawVertexAttributes[ii]->getDataType())
        {
//            NSLog(@"DynamicDrawableAtlas::addDrawable(): Drawable mismatch.  Punting drawable.");
            return false;
        }

    // Turn the vertex and element data into data objects
    MutableRawDataRef vertData, elementData;
    if (!center)
    {
        draw->asVertexAndElementData(vertData,elementData,singleElementSize,NULL);
        if (!vertData || !elementData)
            return false;
    }
    
    // Look for a big drawable that uses the same texture
    BigDrawable *foundBigDraw = NULL;
    DrawRepresent represent(draw->getId());
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawableInfo bigDrawInfo = *it;
        if (bigDrawInfo.baseTexId == draw->getTexId(0)
            && bigDrawInfo.bigDraw->isCompatible(draw,center,objSize))
        {
            // If there is a center, it's dependent on the big drawable's center
            if (!vertData)
                draw->asVertexAndElementData(vertData,elementData,singleElementSize,bigDrawInfo.bigDraw->getCenter());
            if (!vertData || !elementData)
                return false;

            if ((represent.elementChunkId = bigDrawInfo.bigDraw->addRegion(vertData, represent.vertexPos, elementData,enabled)) != EmptyIdentity)
            {
                represent.vertexSize = (int)vertData->getLen();
                represent.bigDrawId = bigDrawInfo.bigDraw->getId();
                foundBigDraw = bigDrawInfo.bigDraw;
                break;
            } else {
                vertData.reset();
                elementData.reset();
            }
        }
    }
    
    // Didn't find one, so create one
    if (!foundBigDraw)
    {
        BigDrawable *newBigDraw = NULL;
        if (newBigDrawable)
            newBigDraw = (*newBigDrawable)(draw,singleElementSize,numVertexBytes,numElementBytes);
        else
            newBigDraw = new BigDrawable(name,singleVertexSize,vertexAttributes,singleElementSize,numVertexBytes,numElementBytes);
        if (addedNewBigDrawable)
            *addedNewBigDrawable = true;
        if (center)
            newBigDraw->setCenter(*center);
        newBigDraw->setOnOff(this->enable);
        newBigDraw->setProgram(shaderId);

        newBigDraw->setModes(draw);
        newBigDraw->setupGL(NULL, memManager);
        changes.push_back(new AddDrawableReq(newBigDraw));
        bigDrawables.insert(BigDrawableInfo(draw->getTexId(0),newBigDraw));
        if (destTexIDs && newBigDraw->texInfo.size() > 0)
        {
            for (unsigned int ti=0;ti<destTexIDs->size();ti++)
                if (ti<newBigDraw->texInfo.size())
                    newBigDraw->texInfo[ti].texId = destTexIDs->at(ti);
        }
        represent.bigDrawId = newBigDraw->getId();
        // If there's a center, the data is dependent on that center.
        // Note: We're creating this twice in some rare cases, which is annoying
        if (!vertData)
              draw->asVertexAndElementData(vertData,elementData,singleElementSize,newBigDraw->getCenter());
        if (!vertData || !elementData)
            return false;
        if ((represent.elementChunkId = newBigDraw->addRegion(vertData, represent.vertexPos, elementData,enabled)) != EmptyIdentity)
        {
            represent.vertexSize = (int)vertData->getLen();
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

void DynamicDrawableAtlas::setEnableDrawable(SimpleIdentity drawId,bool enabled)
{
    // Look for the representation
    DrawRepresent represent(drawId);

    // Find it
    DrawRepresentSet::iterator it = drawables.find(represent);
    if (it == drawables.end())
        return;
    represent = *it;
    
    // Find the big drawable enable/disable it in there
    BigDrawable *bigDraw = NULL;
    BigDrawableSet::iterator bit;
    for (bit = bigDrawables.begin(); bit != bigDrawables.end(); ++bit)
    {
        if (bit->bigDraw->getId() == represent.bigDrawId)
        {
            bigDraw = bit->bigDraw;
            break;
        }
    }

    if (bigDraw)
        bigDraw->setEnableRegion(represent.elementChunkId,enabled);
}
    
void DynamicDrawableAtlas::setEnableAllDrawables(bool newEnable,ChangeSet &changes)
{
    if (newEnable == enable)
        return;
    
    enable = newEnable;
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new BigDrawableOnOffChangeRequest(it->bigDraw->getId(),enable));
}

void DynamicDrawableAtlas::setDrawPriorityAllDrawables(int drawPriority,ChangeSet &changes)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new BigDrawableDrawPriorityChangeRequest(it->bigDraw->getId(),drawPriority));
}
    
void DynamicDrawableAtlas::setProgramIDAllDrawables(SimpleIdentity programID,ChangeSet &changes)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new BigDrawableProgramIDChangeRequest(it->bigDraw->getId(),programID));
}
    
bool DynamicDrawableAtlas::removeDrawable(SimpleIdentity drawId,ChangeSet &changes)
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
    BigDrawableSet::iterator bit;
    for (bit = bigDrawables.begin(); bit != bigDrawables.end(); ++bit)
    {
        if (bit->bigDraw->getId() == represent.bigDrawId)
        {
            bigDraw = bit->bigDraw;
            break;
        }
    }
    
    if (!bigDraw)
        return false;
    
    // Set up the requests to clear the region
    bigDraw->clearRegion(represent.vertexPos, represent.vertexSize, represent.elementChunkId);
    
    return true;
}
    
void DynamicDrawableAtlas::getDrawableTextures(std::vector<DrawTexInfo> &remaps)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin();
         it != bigDrawables.end(); ++it)
    {
        remaps.push_back(DrawTexInfo(it->bigDraw->getId(),it->baseTexId));
    }
}
    
void DynamicDrawableAtlas::getDrawableIDs(SimpleIDSet &drawIDs)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin();
         it != bigDrawables.end(); ++it)
        drawIDs.insert(it->bigDraw->getId());
}
    
bool DynamicDrawableAtlas::hasUpdates()
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = it->bigDraw;
        if (bigDraw->hasChanges())
            return true;
    }
    
    return false;
}
    
void DynamicDrawableAtlas::swap(ChangeSet &changes,BigDrawableSwap::SwapCallback *swapCB, void *swapData)
{
    BigDrawableSwap *swapRequest = new BigDrawableSwap(swapCB,swapData);
    changes.push_back(swapRequest);

    // Toss in the other changes we've been waiting on
    changes.insert(changes.end(), swapChanges.begin(), swapChanges.end());
    swapChanges.clear();
    
    // Note: We could keep a list of changed ones if these get to be more than a few
    std::vector<BigDrawableSet::iterator> toErase;
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
    {
        BigDrawable *bigDraw = it->bigDraw;
        bigDraw->swap(changes,swapRequest);
        
        if (bigDraw->empty())
        {
            changes.push_back(new RemDrawableReq(bigDraw->getId()));
            toErase.push_back(it);
        }
    }
    
    // Now clear out the ones that are empty
    for (unsigned int ii=0;ii<toErase.size();ii++)
        bigDrawables.erase(toErase[ii]);
}
    
bool DynamicDrawableAtlas::waitingOnSwap()
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        if (it->bigDraw->isWaitingOnSwap())
            return true;
    
    return false;
}
    
void DynamicDrawableAtlas::addSwapChanges(const ChangeSet &inSwapChanges)
{
    swapChanges.insert(swapChanges.end(), inSwapChanges.begin(), inSwapChanges.end());
}
    
void DynamicDrawableAtlas::shutdown(ChangeSet &changes)
{
    for (BigDrawableSet::iterator it = bigDrawables.begin(); it != bigDrawables.end(); ++it)
        changes.push_back(new RemDrawableReq(it->bigDraw->getId()));
    
    bigDrawables.clear();
    drawables.clear();
}
    
void DynamicDrawableAtlas::log()
{
//    NSLog(@"Drawable Atlas: Big Drawables: %ld (%.2f MB + %.2f MB)\tRepresented Drawables:%ld",bigDrawables.size(),bigDrawables.size()*(numVertexBytes)/(float)(1024*1024),bigDrawables.size()*(numElementBytes)/(float)(1024*1024),
//          drawables.size());
//    int vertTotal = 0, elTotal = 0;
//    for (BigDrawableSet::iterator it = bigDrawables.begin();
//         it != bigDrawables.end(); ++it)
//    {
//        int thisVertSize,thisElSize;
//        it->bigDraw->getUtilization(thisVertSize,thisElSize);
//        vertTotal += thisVertSize;
//        elTotal += thisElSize;
//    }
//    NSLog(@"Drawable Atlas: using (%.2f MB) for vertices, (%.2f MB) for elements.",vertTotal/(float)(1024*1024),elTotal/(float)(1024*1024));
}
    
}
