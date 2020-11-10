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
    
QIFFrameAsset::QIFFrameAsset(QuadFrameInfoRef frameInfo)
: frameInfo(frameInfo), state(Empty), priority(0), importance(0.0), loadReturnSet(false)
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

QuadFrameInfoRef QIFFrameAsset::getFrameInfo()
{
    return frameInfo;
}
    
void QIFFrameAsset::setupFetch(QuadImageFrameLoader *loader)
{
    state = Loading;
}

void QIFFrameAsset::clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes) {
    state = Empty;
    for (auto texID : texIDs)
        changes.push_back(new RemTextureReq(texID));
    texIDs.clear();
}

bool QIFFrameAsset::updateFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,int newPriority,double newImportance)
{
    if (priority == newPriority && importance == newImportance)
        return false;
    priority = newPriority;
    importance = newImportance;
    
    return true;
}

void QIFFrameAsset::cancelFetch(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps)
{
    state = Empty;
}

void QIFFrameAsset::loadSuccess(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs)
{
    state = Loaded;
    texIDs.clear();
    for (auto tex : texs)
        texIDs.push_back(tex->getId());
}

void QIFFrameAsset::loadFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader)
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

void QIFFrameAsset::setLoadReturnRef(const QuadLoaderReturnRef &inLoadReturn)
{
    loadReturnRef = inLoadReturn;
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
    
void QIFTileAsset::setupFrames(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,int numFrames)
{
    frames.reserve(numFrames);
    for (unsigned int ii = 0; ii < numFrames; ii++) {
        frames.push_back(makeFrameAsset(threadInfo,loader->getFrameInfo(ii),loader));
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

void QIFTileAsset::setImportance(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,double import)
{
    for (auto frame : frames) {
        frame->updateFetching(threadInfo,loader, frame->getPriority(), import);
    }
    ident.importance = import;
}

// Clear out the individual frames, loads and all
void QIFTileAsset::clearFrames(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes)
{
    for (auto frame : frames)
        frame->clear(threadInfo,loader,batchOps, changes);
}

// Clear out geometry and all the frame info
void QIFTileAsset::clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps, ChangeSet &changes)
{
    clearFrames(threadInfo,loader,batchOps, changes);
    
    state = Waiting;
    for (auto drawIDs : instanceDrawIDs) {
        for (auto drawID : drawIDs) {
            changes.push_back(new RemDrawableReq(drawID));
        }
    }
    instanceDrawIDs.clear();
    
    if (!compObjs.empty()) {
        loader->compManager->removeComponentObjects(threadInfo,compObjs,changes);
        compObjs.clear();
    }
    if (!ovlCompObjs.empty()) {
        loader->compManager->removeComponentObjects(threadInfo,ovlCompObjs, changes);
        ovlCompObjs.clear();
    }

    shouldEnable = false;
}
    
void QIFTileAsset::startFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *inLoader,QuadFrameInfoRef frameToLoad,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    state = Active;
}

// Set up the geometry for this tile
void QIFTileAsset::setupContents(QuadImageFrameLoader *loader,
                                 LoadedTileNewRef loadedTile,
                                 int defaultDrawPriority,
                                 const std::vector<SimpleIdentity> &shaderIDs,
                                 ChangeSet &changes)
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
            if (loader->getNumFrames() > 1)
                drawInst->setTexId(1, EmptyIdentity);
            drawInst->setDrawPriority(newDrawPriority);
            drawInst->setOnOff(false);
            drawInst->setProgram(shaderIDs[focusID]);
            auto uniBlock = loader->getUniBlock();
            if (uniBlock.blockData)
                drawInst->setUniBlock(uniBlock);
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

// A single frame loaded successfully
bool QIFTileAsset::frameLoaded(PlatformThreadInfo *threadInfo,
                               QuadImageFrameLoader *loader,
                               QuadLoaderReturn *loadReturn,
                               std::vector<Texture *> &texs,
                               ChangeSet &changes) {
    // Sometimes changes are made directly with the managers and we need to reflect that
    //  even if those features are immediately deleted
    if (!loadReturn->changes.empty())
        changes.insert(changes.end(),loadReturn->changes.begin(),loadReturn->changes.end());
    
    auto frame = findFrameFor(loadReturn->frame);
    if (loadReturn->frame && !frame)
    {
        if (!loadReturn->compObjs.empty())
            loader->compManager->removeComponentObjects(threadInfo,loadReturn->compObjs, changes);
        if (!loadReturn->ovlCompObjs.empty())
            loader->compManager->removeComponentObjects(threadInfo,loadReturn->ovlCompObjs, changes);
        wkLogLevel(Warn,"QuadImageFrameLoader: Got frame back outside of range");
        return false;
    }
    
    // Check the generation.  This is how we catch old data that was in transit.
    if (loadReturn->generation < loader->getGeneration()) {
        if (!loadReturn->compObjs.empty())
            loader->compManager->removeComponentObjects(threadInfo,loadReturn->compObjs, changes);
        if (!loadReturn->ovlCompObjs.empty())
            loader->compManager->removeComponentObjects(threadInfo,loadReturn->ovlCompObjs, changes);
//        wkLogLevel(Debug, "QuadImageFrameLoader: Dropped an old loadReturn after a reload.");
        return true;
    }
    
    // We may be replacing data that's already there
    if (!compObjs.empty()) {
        loader->compManager->removeComponentObjects(threadInfo,compObjs, changes);
        compObjs.clear();
    }
    if (!ovlCompObjs.empty()) {
        loader->compManager->removeComponentObjects(threadInfo,ovlCompObjs, changes);
        ovlCompObjs.clear();
    }
    
    // Component objects (if there)
    for (ComponentObjectRef compObj : loadReturn->compObjs)
        compObjs.insert(compObj->getId());
    for (ComponentObjectRef ovlCompObj : loadReturn->ovlCompObjs)
        ovlCompObjs.insert(ovlCompObj->getId());
    
    if (frame) {
        // Clear out the old texture if it's there
        // Happens in the reload case
        if (!frame->getTexIDs().empty()) {
            for (auto texID : frame->getTexIDs())
                changes.push_back(new RemTextureReq(texID));
        }
        
        frame->loadSuccess(threadInfo,loader,texs);
    }
    
    // In single frame mode with multiple sources, we have to mark the rest of the frames done
    if (loader->getMode() == QuadImageFrameLoader::SingleFrame && frames.size() > 1) {
        std::vector<Texture *> emptyTex;
        for (auto frame: frames) {
            if (frame->getState() == QIFFrameAsset::Loading)
                frame->loadSuccess(threadInfo, loader, emptyTex);
        }
    }
    
    if (!texs.empty()) {
        for (auto tex : texs)
            changes.push_back(new AddTextureReq(tex));
    } else
        changes.push_back(NULL);
    
    return true;
}
    
void QIFTileAsset::mergeLoadedFrame(QuadImageFrameLoader *loader,QuadFrameInfoRef frameInfo,const RawDataRef &data)
{
    auto frame = findFrameFor(frameInfo);
    if (frame)
        frame->setLoadReturn(data);
}

// A single frame failed to load
void QIFTileAsset::frameFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QuadLoaderReturn *loadReturn,ChangeSet &changes) {
    if (frames.size() > 0 && (loadReturn->frame->frameIndex < 0 || loadReturn->frame->frameIndex >= frames.size()))
    {
        wkLogLevel(Warn,"MaplyQuadImageFrameLoader: Got frame back outside of range.");
        return;
    }
    
    auto frame = findFrameFor(loadReturn->frame);
    if (frame)
        frame->loadFailed(threadInfo,loader);
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

bool QIFTileAsset::anythingLoading() {
    for (const auto &frame : frames)
        if (frame->getState() == QIFFrameAsset::Loading)
            return true;

    return false;
}

// Find the frame corresponding to the given source
QIFFrameAssetRef QIFTileAsset::findFrameFor(QuadFrameInfoRef frameInfo) {
    for (const auto &frame : frames) {
        if (frame->getFrameInfo()->getId() == frameInfo->getId())
            return frame;
    }
    
    return QIFFrameAssetRef();
}

// Check if we're in the process of loading any of the given frames
bool QIFTileAsset::anyFramesLoading(const std::set<QuadFrameInfoRef> &frameInfos) {
    for (const auto &frame : frames) {
        if (frame->getState() == QIFFrameAsset::Loading && frameInfos.find(frame->getFrameInfo()) != frameInfos.end())
            return true;
    }
    
    return false;
}

// Check if we've already loaded any of the given frames
bool QIFTileAsset::anyFramesLoaded(const std::set<QuadFrameInfoRef> &frameInfos) {
    for (const auto &frame : frames) {
        if (frame->getState() == QIFFrameAsset::Loaded && frameInfos.find(frame->getFrameInfo()) != frameInfos.end())
            return true;
    }
    
    return false;
}
    
bool QIFTileAsset::isFrameLoading(QuadFrameInfoRef frameInfo)
{
    for (const auto &frame : frames) {
        if (frame->getFrameInfo()->getId() == frameInfo->getId()) {
            return frame->getState() == QIFFrameAsset::Loading;
        }
    }
    
    return false;
}

void QIFTileAsset::setLoadReturnRef(QuadFrameInfoRef frameInfo,QuadLoaderReturnRef loadReturnRef)
{
    for (const auto &frame : frames) {
        if (!frameInfo || frame->getFrameInfo()->getId() == frameInfo->getId())
            frame->setLoadReturnRef(loadReturnRef);
    }
}
    
void QIFTileAsset::cancelFetches(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QuadFrameInfoRef frameToCancel,QIFBatchOps *batchOps)
{
    if (!frameToCancel) {
        for (auto frame : frames) {
            frame->cancelFetch(threadInfo,loader,batchOps);
        }
    } else {
        QIFFrameAssetRef frame = findFrameFor(frameToCancel);
        if (frame)
            frame->cancelFetch(threadInfo, loader, batchOps);
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
: lastUpdate(0.0), lastRenderTime(0.0), texSize(0), borderSize(0)
{ }

QIFRenderState::QIFRenderState(int numFocus,int numFrames)
: texSize(0), borderSize(0)
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
                                changes.push_back(new DrawTexChangeRequest(drawID,ii,EmptyIdentity,texSize,borderSize,relLevel,relX,relY));
                            else
                                changes.push_back(new DrawTexChangeRequest(drawID,ii,frame.texIDs[0],texSize,borderSize,relLevel,relX,relY));
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
                attrs.insert(SingleVertexAttribute(u_interpNameID,-1,(float)t));
                attrs.insert(SingleVertexAttribute(u_colorNameID,-1,color4));
                
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
    texType(TexTypeUnsignedByte), texSize(0), borderSize(0), flipY(true),
    baseDrawPriority(100), drawPriorityPerLevel(1),
    colorChanged(false),
    color(RGBAColor(255,255,255,255)),
    control(NULL), builder(NULL),
    changesSinceLastFlush(true),
    compManager(NULL),
    generation(0),
    targetLevel(-1), curOvlLevel(-1), loadingStatus(true)
{
    lastRunReqFlag = std::make_shared<bool>(true);
    numFocus = 1;
    renderTargetIDs.push_back(EmptyIdentity);
    shaderIDs.push_back(EmptyIdentity);
    curFrames.push_back(0.0);
    
    updatePriorityDefaults();
    
    minZoom = params.minZoom;
    maxZoom = params.maxZoom;
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

const SamplingParams &QuadImageFrameLoader::getSamplingParams()
{
    return params;
}

void QuadImageFrameLoader::setZoomLimits(int inMinZoom,int inMaxZoom)
{
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
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
        for (auto const &it : tiles) {
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
    

void QuadImageFrameLoader::setUniBlock(BasicDrawable::UniformBlock &inUniBlock)
{
    uniBlock = inUniBlock;
}

BasicDrawable::UniformBlock &QuadImageFrameLoader::getUniBlock()
{
    return uniBlock;
}

void QuadImageFrameLoader::setTexType(TextureType inTexType)
{
    texType = inTexType;
}

void QuadImageFrameLoader::setTexSize(int inTexSize,int inBorderSize)
{
    texSize = inTexSize;
    borderSize = inBorderSize;
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

int QuadImageFrameLoader::getNumFrames()
{
    return frames.size();
}

QuadFrameInfoRef QuadImageFrameLoader::getFrameInfo(int which)
{
    if (which < 0 || which >= frames.size())
        return QuadFrameInfoRef();
    return frames[which];
}

void QuadImageFrameLoader::setFrames(const std::vector<QuadFrameInfoRef> &newFrames)
{
    frames = newFrames;
}
    
int QuadImageFrameLoader::getGeneration()
{
    return generation;
}

void QuadImageFrameLoader::reload(PlatformThreadInfo *threadInfo,int frameIndex, ChangeSet &changes)
{
    reload(threadInfo,frameIndex,nullptr,0,changes);
}

static MbrD GeoBoundToLocal(const Mbr &bound, const CoordSystem& cs)
{
    // These should be const
    const auto ll = const_cast<CoordSystem&>(cs).geographicToLocal(Point2d(bound.ll().x(), bound.ll().y()));
    const auto ur = const_cast<CoordSystem&>(cs).geographicToLocal(Point2d(bound.ur().x(), bound.ur().y()));
    return MbrD(Point2d(ll.x(), ll.y()), Point2d(ur.x(), ur.y()));
}

void QuadImageFrameLoader::reload(PlatformThreadInfo *threadInfo,int frameIndex,const Mbr* bounds,int boundCount,ChangeSet &changes)
{
    if (debugMode)
        wkLogLevel(Debug, "QuadImageFrameLoader: Starting reload of frame %d",frameIndex);
    
    loadingStatus = true;
    
    auto batchOps = std::unique_ptr<QIFBatchOps>(makeBatchOps(threadInfo));
    const auto frame = (frameIndex >= 0 && frameIndex < frames.size()) ? frames[frameIndex] : NULL;

    generation++;
    
    // Note: Deal with a load coming in that we might already have

    // If we're given bounds, convert them to the same coordinate system as the QuadTree will produce for the tiles
    auto const controller = getController();
    auto const quadTree = controller ? controller->getQuadTree() : nullptr;
    auto const coordSys = controller ? controller->getCoordSys() : nullptr;

    std::vector<Mbr> localBounds;
    if (bounds && 0 < boundCount && coordSys && quadTree) {
        localBounds.resize(boundCount);
        for (int i = 0; i < boundCount; ++i) {
            localBounds[i] = GeoBoundToLocal(bounds[i], *coordSys);
        }
    }

    // Look through the tiles and:
    //  Cancel outstanding fetches (that match our frame)
    //  Start new requests
    for (const auto &it : tiles) {
        const QIFTileAssetRef &tile = it.second;

        // We cancel everything, even if it's not being refreshed.
        // We've increased the generation, so the incoming data will be dropped.
        // It might be better to increment the generation of everything outstanding,
        // so that if other tiles are still loading they don't end up missing.
        tile->cancelFetches(threadInfo, this, frame, batchOps.get());

        // If bounding boxes were provided, check them
        if (!localBounds.empty()) {
            // Get the bounding box of the tile
            const auto tileBound = quadTree->generateMbrForNode(tile->getIdent());
            // If none of the areas overlap, skip this tile.
            if (std::none_of(localBounds.begin(), localBounds.end(), [&](const auto& b){ return tileBound.overlaps(b); })) {
                continue;
            }
        }

        tile->startFetching(threadInfo, this, frame, batchOps.get(), changes);
    }
    
    // Process all the fetches and cancels at once
    // We're not making any visual changes here, just messing with loading so no ChangeSet
    processBatchOps(threadInfo,batchOps.get());
}
    
QIFTileAssetRef QuadImageFrameLoader::addNewTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident,QIFBatchOps *batchOps,ChangeSet &changes)
{
    // Set up a new tile
    auto newTile = makeTileAsset(threadInfo,ident);
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
        wkLogLevel(Debug,"MaplyQuadImageLoader: Starting fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
    
    // Normal remote data fetching
    newTile->startFetching(threadInfo,this, NULL, batchOps, changes);
        
    return newTile;
}

void QuadImageFrameLoader::removeTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::Node &ident, QIFBatchOps *batchOps, ChangeSet &changes)
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        if (debugMode)
            wkLogLevel(Debug,"MaplyQuadImageLoader: Unloading tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        
        it->second->clear(threadInfo, this, batchOps, changes);
        
        batchOps->deletes.push_back(QuadTreeIdentifier(ident.x,ident.y,ident.level));
        
        tiles.erase(it);
    }
}
    
    
void QuadImageFrameLoader::mergeLoadedTile(PlatformThreadInfo *threadInfo,QuadLoaderReturn *loadReturn,ChangeSet &changes)
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
            tile->frameFailed(threadInfo, this, loadReturn, changes);
        } else {
            if (!tile->frameLoaded(threadInfo, this, loadReturn, texs, changes))
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
        compManager->removeComponentObjects(threadInfo, compObjs, changes);
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
            wkLogLevel(Debug, "MaplyQuadImageLoader: Picking new overlay level %d, targetLevel = %d",curOvlLevel,targetLevel);
    } else {
        if (allLoaded) {
            curOvlLevel = targetLevel;
            if (debugMode)
                wkLogLevel(Debug, "MaplyQuadImageLoader: Picking new overlay level %d, targetLevel = %d",curOvlLevel,targetLevel);
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

                // We'll want to match the draw priority of the tile we're changing to the texture we're using
                int newDrawPriority = baseDrawPriority + drawPriorityPerLevel * texNode.level;

                for (unsigned int focusID = 0;focusID<getNumFocus();focusID++) {
                    for (auto drawID : tile->getInstanceDrawIDs(focusID)) {
                        changes.push_back(new OnOffChangeRequest(drawID,true));
                        changes.push_back(new DrawPriorityChangeRequest(drawID,newDrawPriority));
                        int texIDCount = 0;
                        for (auto texID : texIDs) {
                            changes.push_back(new DrawTexChangeRequest(drawID,texIDCount,texID,texSize,borderSize,relLevel,relX,relY));
                            texIDCount++;
                        }
                    }
                }
            } else {
                int newDrawPriority = baseDrawPriority + drawPriorityPerLevel * tileID.level;
                for (unsigned int focusID = 0;focusID<getNumFocus();focusID++) {
                    for (auto drawID : tile->getInstanceDrawIDs(focusID)) {
                        changes.push_back(new OnOffChangeRequest(drawID,false));
                        changes.push_back(new DrawPriorityChangeRequest(drawID,newDrawPriority));
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
    newRenderState.texSize = texSize;
    newRenderState.borderSize = borderSize;
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
    
    auto theLastRunReqFlag = lastRunReqFlag;
    auto mergeReq = new RunBlockReq([this,newRenderState,theLastRunReqFlag](Scene *scene,SceneRenderer *renderer,View *view)
    {
        if (theLastRunReqFlag) {
            if (builder)
                renderState = newRenderState;
        }
    });
    
    changes.push_back(mergeReq);
}

const std::set<QuadFrameInfoRef> QuadImageFrameLoader::getActiveFrames()
{
    std::set<QuadFrameInfoRef> activeFrames;
    for (auto frame : frames)
        activeFrames.insert(frame);
    
    return activeFrames;
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
    
    auto theActiveFrames = getActiveFrames();
    
    // List all the tiles that we're going to load or are loading
    QuadTreeNew::NodeSet allLoads;
    for (auto node : loadTiles)
        allLoads.insert(node);
    for (auto node : tiles)
        if (node.second->anyFramesLoading(theActiveFrames))
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
                if (it != tiles.end() && it->second->anyFramesLoaded(theActiveFrames)) {
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
        if (!it->second->anyFramesLoaded(theActiveFrames))
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
void QuadImageFrameLoader::builderLoad(PlatformThreadInfo *threadInfo,
                                       QuadTileBuilder *builder,
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
    
    QIFBatchOps *batchOps = makeBatchOps(threadInfo);
    
    // Add new tiles
    for (auto it = updates.loadTiles.rbegin(); it != updates.loadTiles.rend(); ++it) {
        auto tile = *it;
        // If it's already there, clear it out
        removeTile(threadInfo,tile->ident,batchOps,changes);
        
        // Create the new tile and put in the toLoad queue
        auto newTile = addNewTile(threadInfo,tile->ident, batchOps, changes);
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
        removeTile(threadInfo,inTile, batchOps, changes);
        somethingChanged = true;
    }

    builderLoadAdditional(threadInfo,builder,updates,changes);
    
    // Process all the fetches and cancels at once
    processBatchOps(threadInfo,batchOps);
    delete batchOps;
    
    if (debugMode)
        wkLogLevel(Debug,"MaplyQuadImageLoader: changeRequests: %d",(int)changes.size());
    
    changesSinceLastFlush |= somethingChanged;
    
    updateLoadingStatus();
}

void QuadImageFrameLoader::builderLoadAdditional(PlatformThreadInfo *threadInfo,
                                                 QuadTileBuilder *builder,
                                                 const WhirlyKit::TileBuilderDelegateInfo &updates,
                                                 ChangeSet &changes)
{
}

void QuadImageFrameLoader::updateLoadingStatus()
{
    int numTilesLoading = 0;
    for (auto tile : tiles)
        if (tile.second->anythingLoading()) {
//            wkLogLevel(Debug,"  Tile %d: (%d,%d)  for %d",tile.first.level,tile.first.x,tile.first.y,(long)this);
            numTilesLoading++;
        }
    this->loadingStatus = numTilesLoading != 0;
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
void QuadImageFrameLoader::builderShutdown(PlatformThreadInfo *threadInfo,QuadTileBuilder *builder,ChangeSet &changes)
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
    if (!control)
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
            if (frame) {
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
    
void QuadImageFrameLoader::cleanup(PlatformThreadInfo *threadInfo,ChangeSet &changes)
{
    QIFBatchOps *batchOps = makeBatchOps(threadInfo);
    
    for (auto tile : tiles) {
        tile.second->clear(threadInfo,this, batchOps, changes);
    }
    tiles.clear();

    processBatchOps(threadInfo,batchOps);
    delete batchOps;
}

bool QuadImageFrameLoader::isFrameLoading(const QuadTreeIdentifier &ident,QuadFrameInfoRef frame)
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

void QuadImageFrameLoader::setLoadReturnRef(const QuadTreeIdentifier &ident,QuadFrameInfoRef frame,QuadLoaderReturnRef loadReturnRef)
{
    auto it = tiles.find(ident);
    if (it == tiles.end()) {
        return;
    }
    auto tile = it->second;

    tile->setLoadReturnRef(frame,loadReturnRef);
}
    
bool QuadImageFrameLoader::mergeLoadedFrame(const QuadTreeIdentifier &ident,QuadFrameInfoRef frame,const RawDataRef &data,std::vector<RawDataRef> &allData)
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
