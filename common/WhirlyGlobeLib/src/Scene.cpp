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
    
Scene::Scene()
    : fontTextureManager(NULL)
{
}
    
void Scene::Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr)
{
    SetupDrawableStrings();

    coordAdapter = adapter;
    
    // Selection manager is used for object selection from any thread
    addManager(kWKSelectionManager,new SelectionManager(this,DeviceScreenScale()));
    // Intersection handling
    addManager(kWKIntersectionManager, new IntersectionManager(this));
    // Layout manager handles text and icon layout
    addManager(kWKLayoutManager, new LayoutManager());
    // Shape manager handles circles, spheres and such
    addManager(kWKShapeManager, new ShapeManager());
    // Marker manager handles 2D and 3D markers
    addManager(kWKMarkerManager, new MarkerManager());
    // Label manager handes 2D and 3D labels
    addManager(kWKLabelManager, new LabelManager());
    // Vector manager handes vector features
    addManager(kWKVectorManager, new VectorManager());
    // Chunk manager handles geographic chunks that cover a large chunk of the globe
    addManager(kWKSphericalChunkManager, new SphericalChunkManager());
    // Loft manager handles lofted polygon geometry
    addManager(kWKLoftedPolyManager, new LoftManager());
    // Particle system manager
    addManager(kWKParticleSystemManager, new ParticleSystemManager());
    // 3D billboards
    addManager(kWKBillboardManager, new BillboardManager());
    // Widened vectors
    addManager(kWKWideVectorManager, new WideVectorManager());
    // Raw Geometry
    addManager(kWKGeometryManager, new GeometryManager());
    // Components (groups of things)
    addManager(kWKComponentManager, MakeComponentManager());
    
    overlapMargin = 0.0;
}

Scene::~Scene()
{
//    wkLogLevel(Verbose,"Shutting down scene");
    
    textures.clear();
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        delete it->second;
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

    // Note: Should be clearing program out of context somewhere
    for (OpenGLES2ProgramSet::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        delete *it;
    glPrograms.clear();
    
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

GLuint Scene::getGLTexture(SimpleIdentity texIdent)
{
    if (texIdent == EmptyIdentity)
        return 0;
    
    GLuint ret = 0;
    
    std::lock_guard<std::mutex> guardLock(textureLock);
    // Might be a texture ref
    auto it = textures.find(texIdent);
    if (it != textures.end())
    {
        ret = it->second->getGLId();
    }
    
    return ret;
}

DrawableRef Scene::getDrawable(SimpleIdentity drawId)
{
    auto it = drawables.find(drawId);
    if (it != drawables.end())
        return it->second;
    
    return DrawableRef();
}
    
void Scene::addLocalMbr(const Mbr &localMbr)
{
    Point3f ll,ur;
    coordAdapter->getBounds(ll,ur);
    
    // Note: This will only get bigger, never smaller
    if (localMbr.ll().x() < ll.x() && localMbr.ur().x() > ll.x())
    {
        double dx1 = ll.x() - localMbr.ll().x();
        double dx2 = localMbr.ur().x() - ll.x();
        double dx = std::max(dx1,dx2);
        overlapMargin = std::max(overlapMargin,dx);
    }
}
    
void Scene::setRenderer(SceneRendererES *renderer)
{
    std::lock_guard<std::mutex> guardLock(managerLock);
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        it->second->setRenderer(renderer);
}
    
SceneManager *Scene::getManager(const char *name)
{
    std::lock_guard<std::mutex> guardLock(managerLock);

    return getManagerNoLock(name);
}
    
SceneManager *Scene::getManagerNoLock(const char *name)
{
    SceneManager *ret = NULL;

    std::map<std::string,SceneManager *>::iterator it = managers.find((std::string)name);
    if (it != managers.end())
        ret = it->second;

    return ret;
}

void Scene::addManager(const char *name,SceneManager *manager)
{
    std::lock_guard<std::mutex> guardLock(managerLock);

    // If there's one here, we'll clear it out first
    std::map<std::string,SceneManager *>::iterator it = managers.find((std::string)name);
    if (it != managers.end())
        managers.erase(it);
    managers[(std::string)name] = manager;
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
    
void Scene::teardownGL()
{
    for (auto it : drawables)
        it.second->teardownGL(&memManager);
    drawables.clear();
    for (auto it : textures) {
        it.second->destroyInGL(&memManager);
    }
    textures.clear();
    
    memManager.clearBufferIDs();
    memManager.clearTextureIDs();
}

TextureBase *Scene::getTexture(SimpleIdentity texId)
{
    std::lock_guard<std::mutex> guardLock(textureLock);
    
    TextureBase *retTex = NULL;
    auto it = textures.find(texId);
    if (it != textures.end())
        retTex = it->second.get();
    
    return retTex;
}
    
const DrawableRefSet &Scene::getDrawables()
{
    return drawables;
}
    
int Scene::preProcessChanges(WhirlyKit::View *view,SceneRendererES *renderer,TimeInterval now)
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
void Scene::processChanges(WhirlyKit::View *view,SceneRendererES *renderer,TimeInterval now)
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
    changeRequests.clear();
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
    if (changes)
        return true;
    
    // How about the active models?
    for (auto model : activeModels)
        if (model->hasUpdate())
            return true;
    
    return changes;
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
    
void Scene::dumpStats()
{
    wkLogLevel(Verbose,"Scene: %ld drawables",drawables.size());
    wkLogLevel(Verbose,"Scene: %d active models",(int)activeModels.size());
    wkLogLevel(Verbose,"Scene: %ld textures",textures.size());
    wkLogLevel(Verbose,"Scene: %ld sub textures",subTextureMap.size());
    memManager.dumpStats();
}
    
void Scene::setFontTextureManager(FontTextureManagerRef newManager)
{
    fontTextureManager = newManager;
}

OpenGLES2Program *Scene::getProgram(SimpleIdentity progId)
{
    std::lock_guard<std::mutex> guardLock(programLock);

    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end())
    {
        prog = *it;
    }
    
    return prog;
}

void Scene::addProgram(OpenGLES2Program *prog)
{
    if (!prog) {
        wkLogLevel(Warn,"Tried to add NULL program to scene.  Ignoring.");
        return;
    }

    std::lock_guard<std::mutex> guardLock(programLock);

    if (glPrograms.find(prog) == glPrograms.end())
        glPrograms.insert(prog);
}

void Scene::removeProgram(SimpleIdentity progId)
{
    std::lock_guard<std::mutex> guardLock(programLock);

    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end()) {
        prog = *it;
        glPrograms.erase(it);
        
        prog->cleanUp();
    }
}
    
AddTextureReq::~AddTextureReq()
{
    texRef = NULL;
}
    
void AddTextureReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    if (!texRef->getGLId())
        texRef->createInGL(scene->getMemManager());
    scene->textures[texRef->getId()] = texRef;
    texRef = NULL;
}

void RemTextureReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    std::lock_guard<std::mutex> guardLock(scene->textureLock);
    auto it = scene->textures.find(texture);
    if (it != scene->textures.end())
    {
        TextureBaseRef tex = it->second;
        tex->destroyInGL(scene->getMemManager());
        scene->textures.erase(it);
    } else
        wkLogLevel(Warn,"RemTextureReq: No such texture.");
}

AddDrawableReq::~AddDrawableReq()
{
    drawRef = NULL;
}

void AddDrawableReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
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
    
    if (drawRef->getLocalMbr().valid())
        scene->addLocalMbr(drawRef->getLocalMbr());
    
    // Initialize any OpenGL foo
    WhirlyKitGLSetupInfo setupInfo;
    setupInfo.minZres = view->calcZbufferRes();
    setupInfo.glesVersion = renderer->glesVersion;
    drawRef->setupGL(&setupInfo,scene->getMemManager());
    
    drawRef->updateRenderer(renderer);
        
    drawRef = NULL;
}

void RemDrawableReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    auto it = scene->drawables.find(drawable);
    if (it != scene->drawables.end())
    {
        renderer->removeContinuousRenderRequest(it->second->getId());
        // Teardown OpenGL foo
        it->second->teardownGL(scene->getMemManager());

        scene->remDrawable(it->second);
    } else
        wkLogLevel(Warn,"Missing drawable for RemDrawableReq: %llu", drawable);
}

void AddProgramReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    scene->addProgram(program);
    program = NULL;
}

void RemProgramReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    scene->removeProgram(programId);
}
    
RunBlockReq::RunBlockReq(BlockFunc newFunc) : func(newFunc)
{
}
    
RunBlockReq::~RunBlockReq()
{
}
    
void RunBlockReq::execute(Scene *scene,SceneRendererES *renderer,WhirlyKit::View *view)
{
    func(scene,renderer,view);
}
    
}
