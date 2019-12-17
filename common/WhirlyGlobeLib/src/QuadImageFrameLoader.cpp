/*
 *  QuadImageFrameLoader.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "QuadImageFrameLoader.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
QIFBatchOps::QIFBatchOps()
{
}

QIFBatchOps::~QIFBatchOps()
{
}
    
QIFFrameAsset::QIFFrameAsset()
: state(Empty), priority(0), importance(0.0), loadReturnSet(false)
{ }
    
QIFFrameAsset::~QIFFrameAsset()
{
}

QIFFrameAsset::State QIFFrameAsset::getState()
{
    return state;
}

int QIFFrameAsset::getPriority()
{
    return priority;
}

const std::vector<SimpleIdentity> &QIFFrameAsset::getTexIDs()
{
    return texIDs;
}
    
void QIFFrameAsset::setupFetch(QuadImageFrameLoader *loader)
{
    state = Loading;
}

void QIFFrameAsset::clear(QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes) {
    state = Empty;
    for (auto texID : texIDs)
        changes.push_back(new RemTextureReq(texID));
    texIDs.clear();
}

bool QIFFrameAsset::updateFetching(QuadImageFrameLoader *loader,int newPriority,double newImportance)
{
    if (priority == newPriority && importance == newImportance)
        return false;
    priority = newPriority;
    importance = newImportance;
    
    return true;
}

void QIFFrameAsset::cancelFetch(QuadImageFrameLoader *loader,QIFBatchOps *batchOps)
{
    state = Empty;
}

void QIFFrameAsset::loadSuccess(QuadImageFrameLoader *loader,const std::vector<Texture *> &texs)
{
    state = Loaded;
    texIDs.clear();
    for (auto tex : texs)
        texIDs.push_back(tex->getId());
}

void QIFFrameAsset::loadFailed(QuadImageFrameLoader *loader)
{
    state = Empty;
}
    
void QIFFrameAsset::loadSkipped()
{
    loadReturnSet = true;
    state = Loaded;
}

void QIFFrameAsset::setLoadReturn(const RawDataRef &data)
{
    loadReturn = data;
    loadReturnSet = true;
}

void QIFFrameAsset::clearLoadReturn()
{
    loadReturn.reset();
    loadReturnSet = false;
}
    
RawDataRef QIFFrameAsset::getLoadReturn()
{
    return loadReturn;
}

bool QIFFrameAsset::hasLoadReturn()
{
    return loadReturnSet;
}

QIFTileAsset::QIFTileAsset(const QuadTreeNew::ImportantNode &ident) : state(Waiting), shouldEnable(false), ident(ident), drawPriority(0)
{
}
    
void QIFTileAsset::setupFrames(QuadImageFrameLoader *loader,int numFrames)
{
    frames.reserve(numFrames);
    for (unsigned int ii = 0; ii < numFrames; ii++) {
        frames.push_back(makeFrameAsset(loader));
    }
}
    
QIFTileAsset::~QIFTileAsset()
{
}

QIFTileAsset::State QIFTileAsset::getState()
{
    return state;
}

bool QIFTileAsset::getShouldEnable()
{
    return shouldEnable;
}

void QIFTileAsset::setShouldEnable(bool newVal)
{
    shouldEnable = newVal;
}

QuadTreeNew::ImportantNode QIFTileAsset::getIdent()
{
    return ident;
}

const std::vector<SimpleIdentity> &QIFTileAsset::getInstanceDrawIDs(int focusID)
{
    return instanceDrawIDs[focusID];
}

QIFFrameAssetRef QIFTileAsset::getFrame(int frameID)
{
    if (frameID < 0 || frameID >= frames.size())
        return QIFFrameAssetRef(NULL);
    
    return frames[frameID];
}

// True if any of the frames are in the process of loading
bool QIFTileAsset::anyFramesLoading(QuadImageFrameLoader *loader)
{
    // In single frame mode, just care about the first
    if (loader->getMode() == QuadImageFrameLoader::SingleFrame) {
        if (frames.empty())
            return false;
        return frames[0]->getState() == QIFFrameAsset::Loading;
    }
    
    for (auto frame : frames)
        if (frame->getState() == QIFFrameAsset::Loading)
            return true;
    
    return false;
}

// True if any frames have loaded
bool QIFTileAsset::anyFramesLoaded(QuadImageFrameLoader *loader)
{
    // In single frame mode, just care about the first
    if (loader->getMode() == QuadImageFrameLoader::SingleFrame) {
        if (frames.empty())
            return false;
        return frames[0]->getState() == QIFFrameAsset::Loaded;
    }

    for (auto frame : frames)
        if (frame->getState() == QIFFrameAsset::Loaded)
            return true;
    
    return false;
}

// True if the given frame is loading
bool QIFTileAsset::isFrameLoading(int which)
{
    if (which < 0 || which >= frames.size())
        return false;
    
    auto frame = frames[which];
    return frame->getState() == QIFFrameAsset::Loading;
}

void QIFTileAsset::setImportance(QuadImageFrameLoader *loader,double import)
{
    for (auto frame : frames) {
        frame->updateFetching(loader, frame->getPriority(), import);
    }
    ident.importance = import;
}

// Clear out the individual frames, loads and all
void QIFTileAsset::clearFrames(QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes)
{
    for (auto frame : frames)
        frame->clear(loader,batchOps, changes);
}

// Clear out geometry and all the frame info
void QIFTileAsset::clear(QuadImageFrameLoader *loader,QIFBatchOps *batchOps, ChangeSet &changes)
{
    clearFrames(loader,batchOps, changes);
    
    state = Waiting;
    for (auto drawIDs : instanceDrawIDs) {
        for (auto drawID : drawIDs) {
            changes.push_back(new RemDrawableReq(drawID));
        }
    }
    instanceDrawIDs.clear();
    
    if (!compObjs.empty()) {
        loader->compManager->removeComponentObjects(compObjs,changes);
        compObjs.clear();
    }
    if (!ovlCompObjs.empty()) {
        loader->compManager->removeComponentObjects(ovlCompObjs, changes);
        ovlCompObjs.clear();
    }

    shouldEnable = false;
}
    
void QIFTileAsset::startFetching(QuadImageFrameLoader *inLoader,int frameToLoad,QIFBatchOps *inBatchOps)
{
    state = Active;
}

// Set up the geometry for this tile
void QIFTileAsset::setupContents(QuadImageFrameLoader *loader,LoadedTileNewRef loadedTile,int defaultDrawPriority,const std::vector<SimpleIdentity> &shaderIDs,ChangeSet &changes)
{
    drawPriority = defaultDrawPriority;
    
    // One set of instances per focus
    for (unsigned int focusID = 0; focusID < loader->getNumFocus(); focusID++) {
        std::vector<SimpleIdentity> drawIDs;
        for (auto di : loadedTile->drawInfo) {
            int newDrawPriority = defaultDrawPriority;
            bool zBufferRead = false;
            bool zBufferWrite = true;
            switch (di.kind) {
                case WhirlyKit::LoadedTileNew::DrawableGeom:
                    newDrawPriority = defaultDrawPriority;
                    break;
                case WhirlyKit::LoadedTileNew::DrawableSkirt:
                    zBufferWrite = false;
                    zBufferRead = true;
                    newDrawPriority = 11;
                    break;
                case WhirlyKit::LoadedTileNew::DrawablePole:
                    newDrawPriority = defaultDrawPriority;
                    zBufferWrite = false;
                    zBufferRead = false;
                    break;
            }
            
            // Make a drawable instance to shadow the geometry
            auto drawInst = loader->getController()->getRenderer()->makeBasicDrawableInstanceBuilder("MaplyQuadImageFrameLoader");
            drawInst->setMasterID(di.drawID, BasicDrawableInstance::ReuseStyle);
            drawInst->setTexId(0, EmptyIdentity);
            if (frames.size() > 1)
                drawInst->setTexId(1, EmptyIdentity);
            drawInst->setDrawPriority(newDrawPriority);
            drawInst->setOnOff(false);
            drawInst->setProgram(shaderIDs[focusID]);
            drawInst->setColor(loader->getColor());
            drawInst->setRequestZBuffer(zBufferRead);
            drawInst->setWriteZBuffer(zBufferWrite);
            SimpleIdentity renderTargetID = loader->getRenderTarget(focusID);
            if (renderTargetID != EmptyIdentity)
                drawInst->setRenderTarget(renderTargetID);
            changes.push_back(new AddDrawableReq(drawInst->getDrawable()));
            drawIDs.push_back(drawInst->getDrawableID());
        }
        instanceDrawIDs.push_back(drawIDs);
    }
}

void QIFTileAsset::setColor(QuadImageFrameLoader *loader,const RGBAColor &newColor,ChangeSet &changes)
{
    for (auto drawIDs : instanceDrawIDs) {
        for (auto drawID : drawIDs) {
            changes.push_back(new ColorChangeRequest(drawID,newColor));
        }
    }
}

// Cancel any outstanding fetches
void QIFTileAsset::cancelFetches(QuadImageFrameLoader *loader,int frameToCancel,QIFBatchOps *batchOps)
{
    if (frameToCancel == -1) {
        for (auto frame : frames) {
            frame->cancelFetch(loader,batchOps);
        }
    } else {
        frames[frameToCancel]->cancelFetch(loader, batchOps);
    }
}

// A single frame loaded successfully
bool QIFTileAsset::frameLoaded(QuadImageFrameLoader *loader,QuadLoaderReturn *loadReturn,std::vector<Texture *> &texs,ChangeSet &changes) {
    if (frames.size() > 0 && (loadReturn->frame < 0 || loadReturn->frame >= frames.size()))
    {
        if (!loadReturn->compObjs.empty())
            loader->compManager->removeComponentObjects(loadReturn->compObjs, changes);
        if (!loadReturn->ovlCompObjs.empty())
            loader->compManager->removeComponentObjects(loadReturn->ovlCompObjs, changes);
        wkLogLevel(Warn,"QuadImageFrameLoader: Got frame back outside of range");
        return false;
    }
    
    // Check the generation.  This is how we catch old data that was in transit.
    if (loadReturn->generation < loader->getGeneration()) {
        if (!loadReturn->compObjs.empty())
            loader->compManager->removeComponentObjects(loadReturn->compObjs, changes);
        if (!loadReturn->ovlCompObjs.empty())
            loader->compManager->removeComponentObjects(loadReturn->ovlCompObjs, changes);
//        wkLogLevel(Debug, "QuadImageFrameLoader: Dropped an old loadReturn after a reload.");
        return true;
    }
    
    // We may be replacing data that's already there
    if (!compObjs.empty()) {
        loader->compManager->removeComponentObjects(compObjs, changes);
        compObjs.clear();
    }
    if (!ovlCompObjs.empty()) {
        loader->compManager->removeComponentObjects(ovlCompObjs, changes);
        ovlCompObjs.clear();
    }
    
    // Component objects (if there)
    for (ComponentObjectRef compObj : loadReturn->compObjs)
        compObjs.insert(compObj->getId());
    for (ComponentObjectRef ovlCompObj : loadReturn->ovlCompObjs)
        ovlCompObjs.insert(ovlCompObj->getId());
    
    if (loadReturn->frame >= 0 && frames.size() > 0) {
        auto frame = frames[loadReturn->frame];
        
        // Clear out the old texture if it's there
        // Happens in the reload case
        if (!frame->getTexIDs().empty()) {
            for (auto texID : frame->getTexIDs())
                changes.push_back(new RemTextureReq(texID));
        }
        
        frame->loadSuccess(loader,texs);
    }
    
    if (!texs.empty()) {
        for (auto tex : texs)
            changes.push_back(new AddTextureReq(tex));
    } else
        changes.push_back(NULL);
    
    return true;
}
    
void QIFTileAsset::mergeLoadedFrame(QuadImageFrameLoader *loader,int frameID,const RawDataRef &data)
{
    if (frameID >= 0 && frameID < frames.size()) {
        auto frame = frames[frameID];
        frame->setLoadReturn(data);
    }
}
    
void QIFTileAsset::getLoadedData(std::vector<RawDataRef> &allData)
{
    for (auto frame : frames) {
        if (frame->hasLoadReturn()) {
            auto loadReturn = frame->getLoadReturn();
            if (loadReturn.get() != NULL)
                allData.push_back(loadReturn);
        }
        frame->clearLoadReturn();
    }
}
    
bool QIFTileAsset::allFramesLoaded()
{
    for (auto frame : frames) {
        if (!frame->hasLoadReturn())
            return false;
    }
    
    return true;
}

// A single frame failed to load
void QIFTileAsset::frameFailed(QuadImageFrameLoader *loader,QuadLoaderReturn *loadReturn,ChangeSet &changes) {
    if (frames.size() > 0 && (loadReturn->frame < 0 || loadReturn->frame >= frames.size()))
    {
        wkLogLevel(Warn,"MaplyQuadImageFrameLoader: Got frame back outside of range.");
        return;
    }
    
    if (loadReturn->frame >= 0 && loadReturn->frame < frames.size()) {
        auto frame = frames[loadReturn->frame];
        frame->loadFailed(loader);
    }
}

QIFTileState::QIFTileState(int numFrames,const QuadTreeNew::Node &node)
: node(node), enable(false)
{
    frames.resize(numFrames);
    for (auto &frame : frames)
        frame.texNode = node;
}

QIFTileState::FrameInfo::FrameInfo()
: texNode(0,0,-1)
{ }

QIFRenderState::QIFRenderState()
: lastUpdate(0.0), lastRenderTime(0.0)
{ }

QIFRenderState::QIFRenderState(int numFocus,int numFrames)
{
    lastCurFrames.resize(numFocus,-1.0);
    lastUpdate = 0.0;
    tilesLoaded.resize(numFrames,0);
    topTilesLoaded.resize(numFrames,false);
}

bool QIFRenderState::hasUpdate(const std::vector<double> &curFrames)
{
    // Current frame moved
    if (curFrames != lastCurFrames)
        return true;
    
    // We got an update from the layer thread
    if (lastUpdate > lastRenderTime)
        return true;
    
    return false;
}

// Update what the scene is looking at.  Ideally not every frame.
void QIFRenderState::updateScene(Scene *scene,
                                 const std::vector<double> &curFrames,
                                 TimeInterval now,
                                 bool flipY,
                                 const RGBAColor &color,
                                 ChangeSet &changes)
{
    if (tiles.empty())
        return;
    unsigned char color4[4];
    color.asUChar4(color4);
    
    lastRenderTime = now;
    lastCurFrames = curFrames;
    
    // We allow one or more points in the time slices where we're rendering
    // Useful if we're doing multi-stage rendering
    for (unsigned int focusID=0;focusID<curFrames.size();focusID++) {
        double curFrame = curFrames[focusID];
        int activeFrames[2];
        activeFrames[0] = floor(curFrame);
        activeFrames[1] = ceil(curFrame);
        
        // Figure out how many valid frames we've got to look at
        int numFrames = 2;
        if (activeFrames[0] == activeFrames[1]) {
            numFrames = 1;
        }
        // Make sure we've got full coverage on those frames
        if (numFrames > 1 && topTilesLoaded[activeFrames[0]] && topTilesLoaded[activeFrames[1]]) {
            // We're good
        } else if (topTilesLoaded[activeFrames[1]]) {
            // Just one valid frame
            activeFrames[0] = activeFrames[1];
            numFrames = 1;
        } else {
            // Hunt for a good frame
            numFrames = 1;
            bool foundOne = false;
            for (int ii=0;ii<tilesLoaded.size();ii++) {
                int testFrame[2];
                testFrame[0] = activeFrames[0]-ii;
                testFrame[1] = activeFrames[0]+ii+1;
                for (int jj=0;jj<2;jj++) {
                    int theFrame = testFrame[jj];
                    if (theFrame >= 0 && theFrame < tilesLoaded.size()) {
                        if (topTilesLoaded[theFrame]) {
                            activeFrames[0] = theFrame;
                            foundOne = true;
                            break;
                        }
                    }
                }
                if (foundOne)
                    break;
            }
            
            if (!foundOne)
                numFrames = 0;
        }
        
        bool bigEnable = numFrames > 0;
        
        //        NSLog(@"numFrames = %d, activeFrames[0] = %d, activeFrames[1] = %d",numFrames,activeFrames[0],activeFrames[1]);
        
        // Work through the tiles, figure out what's to be on and off
        for (auto tileIt : tiles) {
            auto tileID = tileIt.first;
            auto tile = tileIt.second;
            
            bool enable = bigEnable && tile->enable;
            if (enable) {
                // Assign as many active textures as we've got
                for (unsigned int ii=0;ii<numFrames;ii++) {
                    auto frame = tile->frames[activeFrames[ii]];
                    if (!frame.texIDs.empty()) {
                        int relLevel = tileID.level - frame.texNode.level;
                        int relX = tileID.x - frame.texNode.x * (1<<relLevel);
                        int tileIDY = tileID.y;
                        int frameIdentY = frame.texNode.y;
                        // Note: Confused why this works for both modes
                        //       Might be how the textures are laid out.  Still.  Wah?
    //                    if (flipY) {
                            tileIDY = (1<<tileID.level)-tileIDY-1;
                            frameIdentY = (1<<frame.texNode.level)-frameIdentY-1;
    //                    }
                        int relY = tileIDY - frameIdentY * (1<<relLevel);
                        
                        for (auto drawID : tile->instanceDrawIDs[focusID]) {
                            // Note: In this case we just use the first texture
                            //       We're assuming that each frame has only one texture
                            if (frame.texIDs.empty())
                                changes.push_back(new DrawTexChangeRequest(drawID,ii,EmptyIdentity,0,0,relLevel,relX,relY));
                            else
                                changes.push_back(new DrawTexChangeRequest(drawID,ii,frame.texIDs[0],0,0,relLevel,relX,relY));
                        //                        NSLog(@"tile %d: (%d,%d), frame = %d getting texNode %d: (%d,%d texID = %d)",tileID.level,tileID.x,tileID.y,ii,frame.texNode.level,frame.texNode.x,frame.texNode.y,frame.texID);
                        }
                    } else {
                        enable = false;
                        break;
                    }
                }
                // Clear out the other texture if there is one
                if (numFrames == 1) {
                    for (auto drawID : tile->instanceDrawIDs[focusID]) {
                        changes.push_back(new DrawTexChangeRequest(drawID,1,EmptyIdentity));
                    }
                }
                
                // Interpolate between two frames or snap to one
                double t = 0.0;
                if (numFrames > 1) {
                    t = curFrame-activeFrames[0];
                }
                
                // We set the interpolation value per drawable
                SingleVertexAttributeSet attrs;
                attrs.insert(SingleVertexAttribute(u_interpNameID,(float)t));
                attrs.insert(SingleVertexAttribute(u_colorNameID,color4));
                
                // Turn it all on
                for (auto drawID : tile->instanceDrawIDs[focusID]) {
                    changes.push_back(new OnOffChangeRequest(drawID,true));
                    changes.push_back(new DrawUniformsChangeRequest(drawID,attrs));
                }
            }
            
            // Just turn the geometry off if we've got nothing
            if (!enable) {
                for (auto drawID : tile->instanceDrawIDs[focusID]) {
                    changes.push_back(new OnOffChangeRequest(drawID,false));
                    changes.push_back(new DrawTexChangeRequest(drawID,0,EmptyIdentity));
                    changes.push_back(new DrawTexChangeRequest(drawID,1,EmptyIdentity));
                }
            }
        }
    }
}
    
QuadImageFrameLoader::QuadImageFrameLoader(const SamplingParams &params,Mode mode)
: mode(mode), loadMode(Narrow), debugMode(false),
    params(params),
    requiringTopTilesLoaded(true),
    texType(TexTypeUnsignedByte), flipY(true),
    baseDrawPriority(100), drawPriorityPerLevel(1),
    colorChanged(false),
    color(RGBAColor(255,255,255,255)),
    control(NULL), builder(NULL),
    changesSinceLastFlush(true),
    compManager(NULL),
    generation(0),
    targetLevel(-1), curOvlLevel(-1),
    lastRunReqFlag(NULL), loadingStatus(true)
{
    lastRunReqFlag = new bool();
    *lastRunReqFlag = true;
    numFocus = 1;
    renderTargetIDs.push_back(EmptyIdentity);
    shaderIDs.push_back(EmptyIdentity);
    curFrames.push_back(0.0);
    
    updatePriorityDefaults();
}
    
void QuadImageFrameLoader::addFocus()
{
    numFocus++;
    renderTargetIDs.push_back(EmptyIdentity);
    shaderIDs.push_back(EmptyIdentity);
    curFrames.push_back(0.0);
}
    
int QuadImageFrameLoader::getNumFocus()
{
    return numFocus;
}
    
QuadImageFrameLoader::Mode QuadImageFrameLoader::getMode()
{
    return mode;
}
    
QuadImageFrameLoader::~QuadImageFrameLoader()
{
}
    
void QuadImageFrameLoader::setSamplingParams(const SamplingParams &inParams)
{
    params = inParams;
}
    
void QuadImageFrameLoader::setRequireTopTilesLoaded(bool newVal)
{
    requiringTopTilesLoaded = newVal;
}

QuadDisplayControllerNew *QuadImageFrameLoader::getController()
{
    return control;
}
    
void QuadImageFrameLoader::setDebugMode(bool newMode)
{
    debugMode = newMode;
}
    
bool QuadImageFrameLoader::getDebugMode()
{
    return debugMode;
}
    
void QuadImageFrameLoader::setLoadMode(LoadMode newMode)
{
    loadMode = newMode;
    updatePriorityDefaults();
}

bool QuadImageFrameLoader::getLoadingStatus()
{
    return loadingStatus;
}
    
void QuadImageFrameLoader::updatePriorityDefaults()
{
    if (loadMode == Broad) {
        topPriority = 0;
        nearFramePriority = -1;
        restPriority = 1;
    } else {
        // Focus on the current frame
        topPriority = -1;
        nearFramePriority = 1;
        restPriority = 2;
    }
}
    
int QuadImageFrameLoader::calcLoadPriority(const QuadTreeNew::ImportantNode &ident,int frame)
{
    if (getNumFrames() == 1)
        return 0;
    
    // Min zoom level has priority
    if (topPriority > -1) {
        if (ident.level == params.minZoom)
            return topPriority;
    }
    
    // Frames next to the one we're loading have priority
    if (nearFramePriority > -1) {
        for (auto focusFrame : curFrames) {
            // We want a frame before and after the current position
            int minFrame = floor(focusFrame);
            int maxFrame = ceil(focusFrame);
            if (minFrame == maxFrame) {
                maxFrame = minFrame + 1;
            }
            
            if (std::abs(frame-minFrame) <= 1 || std::abs(frame-maxFrame) <= 1)
                return nearFramePriority;
        }
    }
    
    return restPriority;
}
    
void QuadImageFrameLoader::setColor(RGBAColor &inColor,ChangeSet *changes)
{
    color = inColor;

    if (changes) {
        // Have all the tiles change their base color
        // For multi-frame tiles, they'll get a new color on the next frame as well
        for (auto it : tiles) {
            auto tile = it.second;
            tile->setColor(this,color,*changes);
        }
    }
}

const RGBAColor &QuadImageFrameLoader::getColor()
{
    return color;
}
    
void QuadImageFrameLoader::setRenderTarget(int focusID,SimpleIdentity inRenderTargetID)
{
    renderTargetIDs[focusID] = inRenderTargetID;
}

SimpleIdentity QuadImageFrameLoader::getRenderTarget(int focusID)
{
    return renderTargetIDs[focusID];
}
    
void QuadImageFrameLoader::setShaderID(int focusID,SimpleIdentity inShaderID)
{
    shaderIDs[focusID] = inShaderID;
}

SimpleIdentity QuadImageFrameLoader::getShaderID(int focusID)
{
    return shaderIDs[focusID];
}
    
void QuadImageFrameLoader::setTexType(TextureType inTexType)
{
    texType = inTexType;
}
    
void QuadImageFrameLoader::setBaseDrawPriority(int newPrior)
{
    baseDrawPriority = newPrior;
}

void QuadImageFrameLoader::setDrawPriorityPerLevel(int newPrior)
{
    drawPriorityPerLevel = newPrior;
}
    
void QuadImageFrameLoader::setCurFrame(int focusID,double inCurFrame)
{
    curFrames[focusID] = inCurFrame;
}
    
double QuadImageFrameLoader::getCurFrame(int focusID)
{
    return curFrames[focusID];
}
    
void QuadImageFrameLoader::setFlipY(bool newFlip)
{
    flipY = newFlip;
}
    
bool QuadImageFrameLoader::getFlipY()
{
    return flipY;
}
    
int QuadImageFrameLoader::getGeneration()
{
    return generation;
}
    
void QuadImageFrameLoader::reload(int frame)
{
    if (debugMode)
        wkLogLevel(Debug, "QuadImageFrameLoader: Starting reload of frame %d",frame);
    
    loadingStatus = true;
    
    QIFBatchOps *batchOps = makeBatchOps();

    generation++;
    
    // Note: Deal with a load coming in that we might already have
    
    // Look through the tiles and:
    //  Cancel outstanding fetches (that match our frame)
    //  Start new requests
    for (auto it: tiles) {
        QIFTileAssetRef tile = it.second;
        
        tile->cancelFetches(this, frame, batchOps);
        tile->startFetching(this, frame, batchOps);
    }
    
    // Process all the fetches and cancels at once
    // We're not making any visual changes here, just messing with loading so no ChangeSet
    processBatchOps(batchOps);
    delete batchOps;
}
    
QIFTileAssetRef QuadImageFrameLoader::addNewTile(const QuadTreeNew::ImportantNode &ident,QIFBatchOps *batchOps,ChangeSet &changes)
{
    // Set up a new tile
    auto newTile = makeTileAsset(ident);
    int defaultDrawPriority = baseDrawPriority + drawPriorityPerLevel * ident.level;
    tiles[ident] = newTile;
    
    auto loadedTile = builder->getLoadedTile(ident);
    
    // Make the instance drawables we'll use to mirror the geometry
    if (loadedTile) {
        if (mode != Object)
            newTile->setupContents(this,loadedTile,defaultDrawPriority,shaderIDs,changes);
        newTile->setShouldEnable(loadedTile->enabled);
    }
    
    if (debugMode)
        wkLogLevel(Debug,"Starting fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
    
    // Normal remote data fetching
    newTile->startFetching(this, -1, batchOps);
        
    return newTile;
}

void QuadImageFrameLoader::removeTile(const QuadTreeNew::Node &ident, QIFBatchOps *batchOps, ChangeSet &changes)
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        if (debugMode)
            wkLogLevel(Debug,"Unloading tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        
        it->second->clear(this, batchOps, changes);
        
        batchOps->deletes.push_back(QuadTreeIdentifier(ident.x,ident.y,ident.level));
        
        tiles.erase(it);
    }
}
    
    
void QuadImageFrameLoader::mergeLoadedTile(QuadLoaderReturn *loadReturn,ChangeSet &changes)
{
    changesSinceLastFlush = true;

    bool failed = false;
    
    if (debugMode)
        wkLogLevel(Debug, "MaplyQuadImageLoader: Merging data from tile %d: (%d,%d)",loadReturn->ident.level,loadReturn->ident.x,loadReturn->ident.y);

    QuadTreeNew::Node ident(loadReturn->ident);
    auto it = tiles.find(ident);
    // Tile disappeared in the mean time, so drop it
    if (it == tiles.end() || loadReturn->hasError) {
        if (debugMode)
            wkLogLevel(Debug,"MaplyQuadImageLoader: Failed to load tile before it was erased %d: (%d,%d)",loadReturn->ident.level,loadReturn->ident.x,loadReturn->ident.y);
        failed = true;
    }
    
    ImageTileRef image;
    std::vector<Texture *> texs;

    if (!failed) {
        // Build the texture(s)
        for (auto image : loadReturn->images) {
            LoadedTileNewRef loadedTile = builder->getLoadedTile(ident);
            if (image) {
                Texture *tex = image->buildTexture();
                image->clearTexture();
                if (tex) {
                    tex->setFormat(texType);
                    texs.push_back(tex);
                }
            }
        }
    }

    if (!failed) {
        // Failure depends on what mode we're in
        if (mode == Object) {
            // In object mode, we might not get anything, but it's not a failure
            failed = loadReturn->hasError;
        } else {
            // In the images modes we need, ya know, and image
            failed = texs.empty();
        }
    }

    // If there is a tile, then notify it
    if (it != tiles.end()) {
        auto tile = it->second;
        if (failed) {
            tile->frameFailed(this, loadReturn, changes);
        } else {
            if (!tile->frameLoaded(this, loadReturn, texs, changes))
                failed = true;
        }
    }

    // For whatever reason, didn't correctly integrate the tile
    // so now delete everything
    if (failed) {
        for (auto tex: texs)
            delete tex;
        texs.clear();
        SimpleIDSet compObjs;
        for (auto compObj : loadReturn->compObjs)
            compObjs.insert(compObj->getId());
        for (auto compObj : loadReturn->ovlCompObjs)
            compObjs.insert(compObj->getId());
        compManager->removeComponentObjects(compObjs, changes);
        loadReturn->compObjs.clear();
        loadReturn->ovlCompObjs.clear();
    }
}
    
// Figure out what needs to be on/off for the non-frame cases
void QuadImageFrameLoader::updateRenderState(ChangeSet &changes)
{
    // See if there's any loading happening
    bool allLoaded = true;
    for (auto it : tiles) {
        auto tileID = it.first;
        auto tile = it.second;
        if (tileID.level == targetLevel && tile->anyFramesLoading(this)) {
            allLoaded = false;
            break;
        }
    }
    loadingStatus = !allLoaded;

    // Figure out the overlay level
    if (curOvlLevel == -1) {
        curOvlLevel = targetLevel;
        if (debugMode)
            wkLogLevel(Debug, "Picking new overlay level %d, targetLevel = %d",curOvlLevel,targetLevel);
    } else {
        if (allLoaded) {
            curOvlLevel = targetLevel;
            if (debugMode)
                wkLogLevel(Debug, "Picking new overlay level %d, targetLevel = %d",curOvlLevel,targetLevel);
        }
    }
    
    // Work through the tiles, figuring out textures and objects
    for (auto tileIt : tiles) {
        auto tileID = tileIt.first;
        auto tile = tileIt.second;
        
        // Enable/disable the various visual objects
        bool enable = tile->getShouldEnable() || !params.singleLevel;
        auto compObjIDs = tile->getCompObjs();
        if (!compObjIDs.empty())
            compManager->enableComponentObjects(compObjIDs, enable, changes);

        // Only turn on the overlays if the Tile ID is one of the overlay level
        bool ovlEnable = enable && (tileID.level == curOvlLevel);
        auto ovlCompObjIDs = tile->getOvlCompObjs();
        if (!ovlCompObjIDs.empty())
            compManager->enableComponentObjects(ovlCompObjIDs, ovlEnable, changes);

        // In object mode we just turn things on
        if (mode != Object) {
            // For the image modes, we try to refer to parent textures as needed
            std::vector<SimpleIdentity> texIDs;
            QuadTreeNew::Node texNode = tile->getIdent();

            // Look for a tile or parent tile that has a texture ID
            do {
                auto it = tiles.find(texNode);
                if (it == tiles.end())
                    break;
                auto parentTile = it->second;
                auto parentFrame = parentTile->getFrame(0);
                if (parentFrame && !parentFrame->getTexIDs().empty()) {
                    // Got one, so stop
                    texIDs = parentFrame->getTexIDs();
                    texNode = parentTile->getIdent();
                    break;
                }
                
                // Work our way up the hierarchy
                if (texNode.level <= 0)
                    break;
                texNode.level -= 1;
                texNode.x /= 2;
                texNode.y /= 2;
            } while (texIDs.empty());

            // Turn on the node and adjust the texture
            // Note: Should cache this so we're not changing it every frame
            if (!texIDs.empty()) {
                int relLevel = tileID.level - texNode.level;
                int relX = tileID.x - texNode.x * (1<<relLevel);
                int tileIDY = tileID.y;
                int frameIdentY = texNode.y;
                if (flipY) {
                    tileIDY = (1<<tileID.level)-tileIDY-1;
                    frameIdentY = (1<<texNode.level)-frameIdentY-1;
                }
                int relY = tileIDY - frameIdentY * (1<<relLevel);
                
                for (unsigned int focusID = 0;focusID<getNumFocus();focusID++) {
                    for (auto drawID : tile->getInstanceDrawIDs(focusID)) {
                        changes.push_back(new OnOffChangeRequest(drawID,true));
                        int texIDCount = 0;
                        for (auto texID : texIDs) {
                            changes.push_back(new DrawTexChangeRequest(drawID,texIDCount,texID,0,0,relLevel,relX,relY));
                            texIDCount++;
                        }
                    }
                }
            } else {
                for (unsigned int focusID = 0;focusID<getNumFocus();focusID++) {
                    for (auto drawID : tile->getInstanceDrawIDs(focusID)) {
                        changes.push_back(new OnOffChangeRequest(drawID,false));
                    }
                }
            }
        }
    }
}

// Build up the drawing state for use on the main thread
// All the texture are assigned there
void QuadImageFrameLoader::buildRenderState(ChangeSet &changes)
{
    int numFrames = getNumFrames();
    QIFRenderState newRenderState(numFocus,numFrames);
    for (int frameID=0;frameID<numFrames;frameID++)
        newRenderState.topTilesLoaded[frameID] = true;
        
    // Work through the tiles, figure out their textures as we go
    for (auto tileIt : tiles) {
        auto tileID = tileIt.first;
        auto tile = tileIt.second;
        
        QIFTileStateRef tileState(new QIFTileState(numFrames,tileID));
        tileState->instanceDrawIDs = tile->instanceDrawIDs;
        tileState->enable = tile->getShouldEnable();
        tileState->compObjs = tile->getCompObjs();
        tileState->ovlCompObjs = tile->getOvlCompObjs();
        
        // Work through the frames
        for (int frameID=0;frameID<numFrames;frameID++) {
            auto inFrame = tile->getFrame(frameID);
            auto &outFrame = tileState->frames[frameID];
            
            // Shouldn't happen
            if (!inFrame)
                continue;
            
            // Look for a tile or parent tile that has a texture ID
            QuadTreeNew::Node texNode = tile->getIdent();
            do {
                auto it = tiles.find(texNode);
                if (it == tiles.end())
                    break;
                auto parentTile = it->second;
                auto parentFrame = parentTile->getFrame(frameID);
                if (parentFrame && !parentFrame->getTexIDs().empty()) {
                    // Got one, so stop
                    outFrame.texIDs = parentFrame->getTexIDs();
                    outFrame.texNode = parentTile->getIdent();
                    break;
                }
                
                // Work our way up the hierarchy
                if (texNode.level <= 0)
                    break;
                texNode.level -= 1;
                texNode.x /= 2;
                texNode.y /= 2;
            } while (outFrame.texIDs.empty());
            
            // Metrics for overall loading used by the display side
            if (outFrame.texIDs.empty() && inFrame->getState() != QIFFrameAsset::Loaded) {
                if (tile->getIdent().level == params.minZoom && requiringTopTilesLoaded)
                    newRenderState.topTilesLoaded[frameID] = false;
            } else {
                newRenderState.tilesLoaded[frameID]++;
            }
        }
        
        newRenderState.tiles[tileID] = tileState;
    }
    
    bool *theLastRunReqFlag = lastRunReqFlag;
    auto mergeReq = new RunBlockReq([this,newRenderState,theLastRunReqFlag](Scene *scene,SceneRenderer *renderer,View *view)
    {
        if (*theLastRunReqFlag) {
            if (builder)
                renderState = newRenderState;
        }
    });
    
    changes.push_back(mergeReq);
}

// MARK: Quad Build Delegate

/// Called when the builder first starts up.  Keep this around if you need it.
void QuadImageFrameLoader::setBuilder(QuadTileBuilder *inBuilder,QuadDisplayControllerNew *inControl)
{
    builder = inBuilder;
    control = inControl;
    compManager = (ComponentManager *)control->getScene()->getManager(kWKComponentManager);
}

/// Before we tell the delegate to unload tiles, see if they want to keep them around
/// Returns the tiles we want to preserve after all
QuadTreeNew::NodeSet QuadImageFrameLoader::builderUnloadCheck(QuadTileBuilder *builder,
        const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
        const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
        int targetLevel)
{
    QuadTreeNew::NodeSet toKeep;

    // Not initialized yet
    if (!this->builder)
        return toKeep;
    
    // List all the tiles that we're going to load or are loading
    QuadTreeNew::NodeSet allLoads;
    for (auto node : loadTiles)
        allLoads.insert(node);
    for (auto node : tiles)
        if (node.second->anyFramesLoading(this))
            allLoads.insert(node.first);
    
    // For all those loading or will be loading nodes, nail down their parents
    for (auto node : allLoads) {
        auto parent = node;
        while (parent.level > 0) {
            parent.level -= 1; parent.x /= 2;  parent.y /= 2;
            if (unloadTiles.find(parent) != unloadTiles.end())
            {
                auto it = tiles.find(parent);
                // Nail down the parent that's loaded, but don't care otherwise
                if (it != tiles.end() && it->second->anyFramesLoaded(this)) {
                    toKeep.insert(parent);
                    break;
                }
            }
        }
    }
    
    // Now check all the unloads to see if their parents are loading
    for (auto node : unloadTiles) {
        auto it = tiles.find(node);
        if (it == tiles.end())
            continue;
        // If this tile (to be unloaded) isn't loaded, then we don't care about it
        if (!it->second->anyFramesLoaded(this))
            continue;
        
        // Check that it's not already being kept around
        if (toKeep.find(node) == toKeep.end()) {
            auto parent = node;
            while (parent.level > 0) {
                parent.level -= 1; parent.x /= 2;  parent.y /= 2;
                if (allLoads.find(parent) != allLoads.end()) {
                    toKeep.insert(node);
                    break;
                }
            }
        }
        
        // Lastly, hold anything that might be used for an overlay
        if (curOvlLevel != targetLevel) {
            if (node.level == curOvlLevel && !it->second->getOvlCompObjs().empty()) {
                if (toKeep.find(node) == toKeep.end())
                    toKeep.insert(node);
            }
        }
    }
    
    return toKeep;
}

/// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
void QuadImageFrameLoader::builderLoad(QuadTileBuilder *builder,
                         const WhirlyKit::TileBuilderDelegateInfo &updates,
                         ChangeSet &changes)
{
    // Not initialized yet
    if (!this->builder)
        return;
    
    // Only handling loads and unloads for now
    if (updates.loadTiles.empty() && updates.unloadTiles.empty())
        return;
    
    bool somethingChanged = false;
    
    targetLevel = updates.targetLevel;
    
    QIFBatchOps *batchOps = makeBatchOps();
    
    // Add new tiles
    for (auto it = updates.loadTiles.rbegin(); it != updates.loadTiles.rend(); ++it) {
        auto tile = *it;
        // If it's already there, clear it out
        removeTile(tile->ident,batchOps,changes);
        
        // Create the new tile and put in the toLoad queue
        auto newTile = addNewTile(tile->ident, batchOps, changes);
        somethingChanged = true;
    }
    
    // Remove old tiles
    for (auto inTile: updates.unloadTiles) {
        auto it = tiles.find(inTile);
        // Don't know about this one.  Punt
        if (it == tiles.end())
            continue;
        auto tile = it->second;
        
        // Clear out any associated data and remove it from our list
        removeTile(inTile, batchOps, changes);
        somethingChanged = true;
    }
    
    // Process all the fetches and cancels at once
    processBatchOps(batchOps);
    delete batchOps;
    
    if (debugMode)
        wkLogLevel(Debug,"quadBuilder:updates:changes: changeRequests: %d",(int)changes.size());
    
    changesSinceLastFlush |= somethingChanged;
    
    loadingStatus = somethingChanged;
}

/// Called right before the layer thread flushes all its current changes
void QuadImageFrameLoader::builderPreSceneFlush(QuadTileBuilder *builder,ChangeSet &changes)
{
    // Not initialized yet
    if (!this->builder)
        return;

    if (!changesSinceLastFlush)
        return;
    
    // For multi-frame we need to generate the state and hand it over to the main
    //  thread which will pick out what it needs from the frames
    if (mode == MultiFrame) {
        buildRenderState(changes);
    } else {
        // For single frame (or object) mode we can just take action now
        updateRenderState(changes);
    }
    
    // Let's also generate the stats right here.  That should be often enough.
    makeStats();
    
    changesSinceLastFlush = false;
}

/// Shutdown called on the layer thread if you stuff to clean up
void QuadImageFrameLoader::builderShutdown(QuadTileBuilder *builder,ChangeSet &changes)
{
    if (lastRunReqFlag)
        *lastRunReqFlag = false;
}

bool QuadImageFrameLoader::builderIsLoading()
{
    return loadingStatus;
}

/// Returns true if there's an update to process
bool QuadImageFrameLoader::hasUpdate()
{
    return renderState.hasUpdate(curFrames);
}

/// Process the update
void QuadImageFrameLoader::updateForFrame(RendererFrameInfo *frameInfo)
{
    if (!renderState.hasUpdate(curFrames))
        return;

    ChangeSet changes;

    TimeInterval now = control->getScene()->getCurrentTime();
    renderState.updateScene(frameInfo->scene, curFrames, now, flipY, color, changes);

    frameInfo->scene->addChangeRequests(changes);
}

QuadImageFrameLoader::FrameStats::FrameStats()
: totalTiles(0), tilesToLoad(0)
{
}

QuadImageFrameLoader::Stats::Stats()
: numTiles(0)
{
}

void QuadImageFrameLoader::makeStats()
{
    Stats newStats;
    
    newStats.numTiles = tiles.size();
    int numFrames = getNumFrames();
    newStats.frameStats.resize(numFrames);
    for (auto it : tiles) {
        auto tileID = it.first;
        auto tile = it.second;
        
        for (int frameID = 0;frameID<numFrames;frameID++) {
            auto frame = tile->getFrame(frameID);
            auto &frameStat = newStats.frameStats[frameID];
            switch (frame->getState()) {
                case QIFFrameAsset::Empty:
                    break;
                case QIFFrameAsset::Loaded:
                    break;
                case QIFFrameAsset::Loading:
                    frameStat.tilesToLoad++;
                    break;
            }
            frameStat.totalTiles++;
        }
    }
    
    {
        std::lock_guard<std::mutex> guardLock(statsLock);
        stats = newStats;
    }
}
    
QuadImageFrameLoader::Stats QuadImageFrameLoader::getStats()
{
    {
        std::lock_guard<std::mutex> guardLock(statsLock);
        return stats;
    }
}
    
void QuadImageFrameLoader::cleanup(ChangeSet &changes)
{
    QIFBatchOps *batchOps = makeBatchOps();
    
    for (auto tile : tiles) {
        tile.second->clear(this, batchOps, changes);
    }
    tiles.clear();

    processBatchOps(batchOps);
    delete batchOps;
}

bool QuadImageFrameLoader::isFrameLoading(const QuadTreeIdentifier &ident,int frame)
{
    auto it = tiles.find(ident);
    if (it == tiles.end()) {
        return false;
    }
    auto tile = it->second;
    
    if (getNumFrames() > 0)
        return tile->isFrameLoading(frame);
    else
        return true;
}
    
bool QuadImageFrameLoader::mergeLoadedFrame(const QuadTreeIdentifier &ident,int frame,const RawDataRef &data,std::vector<RawDataRef> &allData)
{
    changesSinceLastFlush = true;

    auto it = tiles.find(ident);
    if (it == tiles.end()) {
        return false;
    }
    auto tile = it->second;

    // Single frame mode with multiple sources is the only case where we keep the data
    if (mode == SingleFrame && getNumFrames() > 1) {
        tile->mergeLoadedFrame(this,frame,data);
        
        // We need to put all the various data pieces into the load return for processing
        if (tile->allFramesLoaded()) {
            tile->getLoadedData(allData);
            return true;
        }
        
        return false;
    }
    
    return true;
}

}
