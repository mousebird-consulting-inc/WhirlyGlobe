/*
 *  Scene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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

#import "WhirlyKitLog.h"
#import "Scene.h"
#import "GlobeView.h"
#import "GlobeMath.h"
#import "TextureAtlas.h"
#import "Platform.h"
#import "FontTextureManager.h"
#import "SelectionManager.h"
#import "IntersectionManager.h"
#import "LayoutManager.h"
#import "ShapeManager.h"
#import "MarkerManager.h"
#import "LabelManager.h"
#import "VectorManager.h"
#import "WideVectorManager.h"
#import "SphericalEarthChunkManager.h"
#import "LoftManager.h"
#import "ParticleSystemManager.h"
#import "BillboardManager.h"
#import "WideVectorManager.h"
#import "GeometryManager.h"
#import "FontTextureManager.h"
#import "ComponentManager.h"

namespace WhirlyKit
{

SceneManager::SceneManager()
: scene(NULL), renderer(NULL)
{
}

void SceneManager::setRenderer(SceneRenderer *inRenderer)
{
//    std::lock_guard<std::mutex> guardLock(lock);
    renderer = inRenderer;
}

void SceneManager::setScene(Scene *inScene)
{
//    std::lock_guard<std::mutex> guardLock(lock);
    scene = inScene;
}

Scene *SceneManager::getScene()
{
    return scene;
}

SceneRenderer *SceneManager::getSceneRenderer()
{
    return renderer;
}

Scene::Scene(CoordSystemDisplayAdapter *adapter)
    : setupInfo(nullptr)
    , currentTime(0.0)
    , coordAdapter(adapter)
{
    SetupDrawableStrings();
    
    // Selection manager is used for object selection from any thread
    addManager(kWKSelectionManager,std::make_shared<SelectionManager>(this));
    // Intersection handling
    addManager(kWKIntersectionManager, std::make_shared<IntersectionManager>(this));
    // Layout manager handles text and icon layout
    addManager(kWKLayoutManager, std::make_shared<LayoutManager>());
    // Shape manager handles circles, spheres and such
    addManager(kWKShapeManager, std::make_shared<ShapeManager>());
    // Marker manager handles 2D and 3D markers
    addManager(kWKMarkerManager, std::make_shared<MarkerManager>());
    // Label manager handes 2D and 3D labels
    addManager(kWKLabelManager, std::make_shared<LabelManager>());
    // Vector manager handes vector features
    addManager(kWKVectorManager, std::make_shared<VectorManager>());
    // Chunk manager handles geographic chunks that cover a large chunk of the globe
    addManager(kWKSphericalChunkManager, std::make_shared<SphericalChunkManager>());
    // Loft manager handles lofted polygon geometry
    addManager(kWKLoftedPolyManager, std::make_shared<LoftManager>());
    // Particle system manager
    addManager(kWKParticleSystemManager, std::make_shared<ParticleSystemManager>());
    // 3D billboards
    addManager(kWKBillboardManager, std::make_shared<BillboardManager>());
    // Widened vectors
    addManager(kWKWideVectorManager, std::make_shared<WideVectorManager>());
    // Raw Geometry
    addManager(kWKGeometryManager, std::make_shared<GeometryManager>());
    // Components (groups of things)
    addManager(kWKComponentManager, MakeComponentManager());
    
    overlapMargin = 0.0;
    baseTime = TimeGetCurrent();
    
    for (unsigned int ii=0;ii<MaplyMaxZoomSlots;ii++)
        zoomSlots[ii] = MAXFLOAT;
}

Scene::~Scene()
{
//    wkLogLevel(Verbose,"Shutting down scene");
    
    textures.clear();
    
    managers.clear();
    
    auto theChangeRequests = changeRequests;
    changeRequests.clear();
    for (unsigned int ii=0;ii<theChangeRequests.size();ii++)
    {
        // Note: Tear down change requests?
        delete theChangeRequests[ii];
    }
    
    activeModels.clear();
    
    subTextureMap.clear();

    programs.clear();
    
    fontTextureManager = NULL;
}
    
CoordSystemDisplayAdapter *Scene::getCoordAdapter()
{
    return coordAdapter;
}
    
void Scene::setDisplayAdapter(CoordSystemDisplayAdapter *newCoordAdapter)
{
    std::lock_guard<std::mutex> guardLock(coordAdapterLock);
    coordAdapter = newCoordAdapter;
}
    
// Add change requests to our list
void Scene::addChangeRequests(const ChangeSet &newChanges)
{
    std::lock_guard<std::mutex> guardLock(changeRequestLock);
    
    for (ChangeRequest *change : newChanges)
    {
        if (change && change->when > 0.0)
            timedChangeRequests.insert(change);
        else
            changeRequests.push_back(change);
    }
}

// Add a single change request
void Scene::addChangeRequest(ChangeRequest *newChange)
{
    std::lock_guard<std::mutex> guardLock(changeRequestLock);

    if (newChange && newChange->when > 0.0)
        timedChangeRequests.insert(newChange);
    else
        changeRequests.push_back(newChange);
}

int Scene::getNumChangeRequests()
{
    std::lock_guard<std::mutex> guardLock(changeRequestLock);

    return changeRequests.size();
}

DrawableRef Scene::getDrawable(SimpleIdentity drawId)
{
    std::lock_guard<std::mutex> guardLock(drawablesLock);
    
    const auto it = drawables.find(drawId);
    return (it != drawables.end()) ? it->second : DrawableRef();
}
    
void Scene::addLocalMbr(const Mbr &localMbr)
{
    Point3f ll,ur;
    coordAdapter->getBounds(ll,ur);
    
    // Note: This will only get bigger, never smaller
    if (localMbr.ll().x() < ll.x() && localMbr.ur().x() > ll.x())
    {
        const double dx1 = ll.x() - localMbr.ll().x();
        const double dx2 = localMbr.ur().x() - ll.x();
        overlapMargin = std::max(overlapMargin, std::max(dx1, dx2));
    }
}

void Scene::setRenderer(SceneRenderer *renderer)
{
    setupInfo = renderer->getRenderSetupInfo();
    
    std::lock_guard<std::mutex> guardLock(managerLock);

    for (const auto &kvp : managers)
    {
        kvp.second->setRenderer(renderer);
    }
}
    
SceneManagerRef Scene::getManager(const std::string &name)
{
    std::lock_guard<std::mutex> guardLock(managerLock);
    return getManagerNoLock(name);
}
    
SceneManagerRef Scene::getManagerNoLock(const std::string &name)
{
    const auto it = managers.find(name);
    return (it != managers.end()) ? it->second : SceneManagerRef();
}

void Scene::addManager(const std::string &name,const SceneManagerRef &manager)
{
    std::lock_guard<std::mutex> guardLock(managerLock);

    // If there's one here, we'll clear it out first
    const auto result = managers.insert(std::make_pair(name, manager));
    if (!result.second)
    {
        // Entry was already present, replace it.
        // Previous manager reference is released, possibly destroying it.
        result.first->second = manager;
    }
    manager->setScene(this);
}

void Scene::addActiveModel(ActiveModelRef activeModel)
{
    activeModels.push_back(activeModel);
    activeModel->startWithScene(this);
}

void Scene::removeActiveModel(ActiveModelRef activeModel)
{
    int which = 0;

    for (auto theModel : activeModels) {
        if (theModel == activeModel) {
            break;
        }
        which++;
    }
    if (which < activeModels.size()) {
        activeModels.erase(activeModels.begin() + which);
        activeModel->teardown();
    }
}

TextureBaseRef Scene::getTexture(SimpleIdentity texId)
{
    std::lock_guard<std::mutex> guardLock(textureLock);
    
    const auto it = textures.find(texId);
    return (it != textures.end()) ? it->second : TextureBaseRef();
}

const std::vector<Drawable *> Scene::getDrawables()
{
    std::vector<Drawable *> retDraws;
    
    std::lock_guard<std::mutex> guardLock(drawablesLock);
    for (auto it : drawables) {
        retDraws.push_back(it.second.get());
    }
    
    return retDraws;
}

void Scene::setCurrentTime(TimeInterval newTime)
{
    currentTime = newTime;
}

void Scene::markProgramsUnchanged()
{
    std::lock_guard<std::mutex> guardLock(programLock);

    for (auto it: programs) {
        auto prog = it.second;
        prog->changed = false;
    }
}

TimeInterval Scene::getCurrentTime()
{
    return (currentTime == 0.0) ? TimeGetCurrent() : currentTime;
}

TimeInterval Scene::getBaseTime()
{
    return baseTime;
}
    
int Scene::preProcessChanges(WhirlyKit::View *view,SceneRenderer *renderer,TimeInterval now)
{
    ChangeSet preRequests;

    {
        std::lock_guard<std::mutex> guardLock(changeRequestLock);
        // Just doing the ones that require a pre-process
        for (unsigned int ii=0;ii<changeRequests.size();ii++)
        {
            ChangeRequest *req = changeRequests[ii];
            if (req && req->needPreExecute()) {
                preRequests.push_back(req);
                changeRequests[ii] = NULL;
            }
        }
    }

    // Run these outside of the lock, since they might use the lock
    for (auto req : preRequests) {
        req->execute(this,renderer,view);
        delete req;
    }
    
    return preRequests.size();
}

// Process outstanding changes.
// We'll grab the lock and we're only expecting to be called in the rendering thread
int Scene::processChanges(WhirlyKit::View *view,SceneRenderer *renderer,TimeInterval now)
{
    std::lock_guard<std::mutex> guardLock(changeRequestLock);
    // See if any of the timed changes are ready
    std::vector<ChangeRequest *> toMove;
    for (ChangeRequest *req : timedChangeRequests)
    {
        if (now >= req->when)
            toMove.push_back(req);
        else
            break;
    }
    for (ChangeRequest *req : toMove)
    {
        timedChangeRequests.erase(req);
        changeRequests.push_back(req);
    }
    
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
    {
        ChangeRequest *req = changeRequests[ii];
        if (req) {
            req->execute(this,renderer,view);
            delete req;
        }
    }
    int numChanges = changeRequests.size();
    changeRequests.clear();
    
    return numChanges;
}
    
bool Scene::hasChanges(TimeInterval now)
{
    bool changes = false;
    if (changeRequestLock.try_lock())
    {
        changes = !changeRequests.empty();
        
        if (!changes)
            if (timedChangeRequests.size() > 0)
                changes = now >= (*timedChangeRequests.begin())->when;

        changeRequestLock.unlock();
    }
    
    // How about the active models?
    bool activeModelsUpdates = false;
    for (auto model : activeModels)
        if (model->hasUpdate()) {
            //activeModelsUpdates = true;
            return true;
        }
    
    return changes || activeModelsUpdates;
}

// Add a single sub texture map
void Scene::addSubTexture(const SubTexture &subTex)
{
    std::lock_guard<std::mutex> guardLock(subTexLock);
    subTextureMap.insert(subTex);
}

// Add a whole group of sub textures maps
void Scene::addSubTextures(const std::vector<SubTexture> &subTexes)
{
    std::lock_guard<std::mutex> guardLock(subTexLock);
    subTextureMap.insert(subTexes.begin(),subTexes.end());
}
    
void Scene::removeSubTexture(SimpleIdentity subTexID)
{
    std::lock_guard<std::mutex> guardLock(subTexLock);
    SubTexture dumbTex(subTexID);
    SubTextureSet::iterator it = subTextureMap.find(dumbTex);
    if (it != subTextureMap.end())
        subTextureMap.erase(it);
}

void Scene::removeSubTextures(const std::vector<SimpleIdentity> &subTexIDs)
{
    std::lock_guard<std::mutex> guardLock(subTexLock);
    SubTexture dummySubTex;
    for (auto texID : subTexIDs)
    {
        dummySubTex.setId(texID);
        auto it = subTextureMap.find(dummySubTex);
        if (it != subTextureMap.end())
            subTextureMap.erase(it);
    }
}

// Look for a sub texture by ID
SubTexture Scene::getSubTexture(SimpleIdentity subTexId)
{
    std::lock_guard<std::mutex> guardLock(subTexLock);
    SubTexture dumbTex;
    dumbTex.setId(subTexId);
    SubTextureSet::iterator it = subTextureMap.find(dumbTex);
    if (it == subTextureMap.end())
    {
        SubTexture passTex;
        passTex.trans = passTex.trans.Identity();
        passTex.texId = subTexId;
        return passTex;
    }
    
    return *it;
}
    
void Scene::addDrawable(DrawableRef draw)
{
    std::lock_guard<std::mutex> guardLock(drawablesLock);

    drawables[draw->getId()] = draw;
}
    
void Scene::remDrawable(DrawableRef draw)
{
    std::lock_guard<std::mutex> guardLock(drawablesLock);

    auto it = drawables.find(draw->getId());
    if (it != drawables.end())
        drawables.erase(it);
}

void Scene::addTexture(TextureBaseRef texRef)
{
    std::lock_guard<std::mutex> guardLock(textureLock);
    
    textures[texRef->getId()] = texRef;
}

bool Scene::removeTexture(SimpleIdentity texID)
{
    std::lock_guard<std::mutex> guardLock(textureLock);
    
    auto it = textures.find(texID);
    if (it != textures.end()) {
        textures.erase(it);
        return true;
    }
    
    return false;
}
    
void Scene::dumpStats()
{
    wkLogLevel(Verbose,"Scene: %ld drawables",drawables.size());
    wkLogLevel(Verbose,"Scene: %d active models",(int)activeModels.size());
    wkLogLevel(Verbose,"Scene: %ld textures",textures.size());
    wkLogLevel(Verbose,"Scene: %ld sub textures",subTextureMap.size());
}
    
void Scene::setFontTextureManager(const FontTextureManagerRef &newManager)
{
    fontTextureManager = newManager;
}

Program *Scene::getProgram(SimpleIdentity progId)
{
    std::lock_guard<std::mutex> guardLock(programLock);

    Program *prog = NULL;
    auto it = programs.find(progId);
    if (it != programs.end())
        prog = it->second.get();
    
    return prog;
}
    
Program *Scene::findProgramByName(const std::string &name)
{
    std::lock_guard<std::mutex> guardLock(programLock);
    
    Program *prog = NULL;
    for (auto it = programs.rbegin(); it != programs.rend(); ++it) {
        if (it->second->getName() == name)
        {
            prog = it->second.get();
            break;
        }
    }
    
    return prog;
}

void Scene::addProgram(ProgramRef prog)
{
    if (!prog) {
        wkLogLevel(Warn,"Tried to add NULL program to scene.  Ignoring.");
        return;
    }

    std::lock_guard<std::mutex> guardLock(programLock);

    programs[prog->getId()] = prog;
}

void Scene::removeProgram(SimpleIdentity progId,RenderTeardownInfoRef teardown)
{
    std::lock_guard<std::mutex> guardLock(programLock);

    auto it = programs.find(progId);
    if (it != programs.end()) {
        it->second->teardownForRenderer(setupInfo,this,NULL);
        programs.erase(it);
    }
}

int Scene::retainZoomSlot()
{
    std::lock_guard<std::mutex> guardLock(zoomSlotLock);
    
    // Look for an open slot
    int found = -1;
    for (int ii=0;ii<MaplyMaxZoomSlots;ii++) {
        if (zoomSlots[ii] == MAXFLOAT) {
            found = ii;
            zoomSlots[ii] = 0.0;  // Means we're retaining it for use
            break;
        }
    }

    return found;
}

void Scene::releaseZoomSlot(int zoomSlot)
{
    std::lock_guard<std::mutex> guardLock(zoomSlotLock);

    zoomSlots[zoomSlot] = MAXFLOAT;
}

void Scene::setZoomSlotValue(int zoomSlot,float zoom)
{
    std::lock_guard<std::mutex> guardLock(zoomSlotLock);

    zoomSlots[zoomSlot] = zoom;
}

float Scene::getZoomSlotValue(int zoomSlot)
{
    std::lock_guard<std::mutex> guardLock(zoomSlotLock);
    
    if (zoomSlot < 0 || zoomSlot >= MaplyMaxZoomSlots)
        return 0.0;

    return zoomSlots[zoomSlot];
}

void Scene::copyZoomSlots(float *dest)
{
    std::lock_guard<std::mutex> guardLock(zoomSlotLock);

    memcpy(dest, &zoomSlots[0], sizeof(float)*MaplyMaxZoomSlots);
}

void AddTextureReq::setupForRenderer(const RenderSetupInfo *setupInfo,Scene *scene)
{
    if (texRef)
        texRef->createInRenderer(setupInfo);
}
    
TextureBase *AddTextureReq::getTex()
{
    return texRef.get();
}
    
AddTextureReq::~AddTextureReq()
{
    texRef = NULL;
}
    
void AddTextureReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    texRef->createInRenderer(renderer->getRenderSetupInfo());
    scene->addTexture(texRef);
    texRef = NULL;
}

void RemTextureReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    TextureBaseRef tex = scene->getTexture(texture);
    if (tex)
    {
        if (renderer->teardownInfo)
            renderer->teardownInfo->destroyTexture(renderer,tex);
        scene->removeTexture(texture);
    } else
        wkLogLevel(Warn,"RemTextureReq: No such texture.");
}
    
void AddDrawableReq::setupForRenderer(const RenderSetupInfo *setupInfo,Scene *scene)
{
    if (drawRef) {
        drawRef->setupForRenderer(setupInfo,scene);
        
        // Add it to the scene, even if we're on another thread
        scene->addDrawable(drawRef);
    }
}

AddDrawableReq::~AddDrawableReq()
{
    drawRef = NULL;
}

void AddDrawableReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    // If this is an instance, deal with that madness
    BasicDrawableInstance *drawInst = dynamic_cast<BasicDrawableInstance *>(drawRef.get());
    if (drawInst)
    {
        DrawableRef theDraw = scene->getDrawable(drawInst->getMasterID());
        BasicDrawableRef baseDraw = std::dynamic_pointer_cast<BasicDrawable>(theDraw);
        if (baseDraw)
            drawInst->setMaster(baseDraw);
        else {
            return;
        }
    }

    scene->addDrawable(drawRef);
    renderer->addDrawable(drawRef);
    
    if (drawRef->getLocalMbr().valid())
        scene->addLocalMbr(drawRef->getLocalMbr());
            
    drawRef = NULL;
}

RemDrawableReq::RemDrawableReq(SimpleIdentity drawId) : drawID(drawId)
{
}

RemDrawableReq::RemDrawableReq(SimpleIdentity drawId,TimeInterval inWhen)
: drawID(drawId)
{
    when = inWhen;
}

void RemDrawableReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawID);
    if (draw)
    {
        renderer->removeDrawable(draw, true, renderer->teardownInfo);
        scene->remDrawable(draw);
    } else
        wkLogLevel(Warn,"Missing drawable for RemDrawableReq: %llu", drawID);
}

void AddProgramReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    scene->addProgram(program);
    program = NULL;
}

void RemProgramReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    scene->removeProgram(programId,renderer->teardownInfo);
}
    
RunBlockReq::RunBlockReq(BlockFunc newFunc) : func(newFunc)
{
}
    
RunBlockReq::~RunBlockReq()
{
}
    
void RunBlockReq::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
    func(scene,renderer,view);
}
    
SetZoomSlotReq::SetZoomSlotReq(int zoomSlot,float zoomVal)
: zoomSlot(zoomSlot), zoomVal(zoomVal)
{
}

void SetZoomSlotReq::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    if (zoomVal == MAXFLOAT)
        scene->releaseZoomSlot(zoomSlot);
    else
        scene->setZoomSlotValue(zoomSlot, zoomVal);
}



}
