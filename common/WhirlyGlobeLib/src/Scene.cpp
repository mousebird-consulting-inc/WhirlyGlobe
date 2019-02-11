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
//#import "SphericalEarthChunkManager.h"
#import "LoftManager.h"
#import "ParticleSystemManager.h"
#import "BillboardManager.h"
#import "WideVectorManager.h"
#import "GeometryManager.h"
#import "FontTextureManager.h"

namespace WhirlyKit
{
    
Scene::Scene()
    : fontTextureManager(NULL)
{
}
    
void Scene::Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr)
{
    SetupDrawableStrings();

    pthread_mutex_init(&coordAdapterLock,NULL);
    pthread_mutex_init(&changeRequestLock,NULL);
    pthread_mutex_init(&subTexLock, NULL);
    pthread_mutex_init(&textureLock,NULL);
    pthread_mutex_init(&programLock,NULL);
    pthread_mutex_init(&managerLock,NULL);

    coordAdapter = adapter;
    
//    dispatchQueue = dispatch_queue_create("WhirlyKit Scene", 0);

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
//    addManager(kWKSphericalChunkManager, new SphericalChunkManager());
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
    
    overlapMargin = 0.0;
}

Scene::~Scene()
{
//    wkLogLevel(Verbose,"Shutting down scene");
    
    pthread_mutex_destroy(&coordAdapterLock);

    textures.clear();
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        delete it->second;
    managers.clear();
        
    pthread_mutex_destroy(&managerLock);
    pthread_mutex_destroy(&changeRequestLock);
    pthread_mutex_destroy(&subTexLock);
    pthread_mutex_destroy(&textureLock);
    pthread_mutex_destroy(&programLock);
    
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
    pthread_mutex_lock(&coordAdapterLock);
    coordAdapter = newCoordAdapter;
    pthread_mutex_unlock(&coordAdapterLock);
}
    
// Add change requests to our list
void Scene::addChangeRequests(const ChangeSet &newChanges)
{
    pthread_mutex_lock(&changeRequestLock);
    
    for (ChangeRequest *change : newChanges)
    {
        if (change && change->when > 0.0)
            timedChangeRequests.insert(change);
        else
            changeRequests.push_back(change);
    }
    
    pthread_mutex_unlock(&changeRequestLock);
}

// Add a single change request
void Scene::addChangeRequest(ChangeRequest *newChange)
{
    pthread_mutex_lock(&changeRequestLock);
    
    if (newChange && newChange->when > 0.0)
        timedChangeRequests.insert(newChange);
    else
        changeRequests.push_back(newChange);
    
    pthread_mutex_unlock(&changeRequestLock);
}

GLuint Scene::getGLTexture(SimpleIdentity texIdent)
{
    if (texIdent == EmptyIdentity)
        return 0;
    
    GLuint ret = 0;
    
    pthread_mutex_lock(&textureLock);
    // Might be a texture ref
    auto it = textures.find(texIdent);
    if (it != textures.end())
    {
        ret = it->second->getGLId();
    }
    
    pthread_mutex_unlock(&textureLock);
    
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
    pthread_mutex_lock(&managerLock);
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        it->second->setRenderer(renderer);
    
    pthread_mutex_unlock(&managerLock);
}
    
SceneManager *Scene::getManager(const char *name)
{
    SceneManager *ret = NULL;
    
    pthread_mutex_lock(&managerLock);
    
    std::map<std::string,SceneManager *>::iterator it = managers.find((std::string)name);
    if (it != managers.end())
        ret = it->second;
    
    pthread_mutex_unlock(&managerLock);
    
    return ret;
}

void Scene::addManager(const char *name,SceneManager *manager)
{
    pthread_mutex_lock(&managerLock);

    // If there's one here, we'll clear it out first
    std::map<std::string,SceneManager *>::iterator it = managers.find((std::string)name);
    if (it != managers.end())
        managers.erase(it);
    managers[(std::string)name] = manager;
    manager->setScene(this);
    
    pthread_mutex_unlock(&managerLock);
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
    pthread_mutex_lock(&textureLock);
    
    TextureBase *retTex = NULL;
    auto it = textures.find(texId);
    if (it != textures.end())
        retTex = it->second.get();
    
    pthread_mutex_unlock(&textureLock);
    
    return retTex;
}
    
const DrawableRefSet &Scene::getDrawables()
{
    return drawables;
}
    
int Scene::preProcessChanges(WhirlyKit::View *view,SceneRendererES *renderer,TimeInterval now)
{
    ChangeSet preRequests;
    
    pthread_mutex_lock(&changeRequestLock);
    // Just doing the ones that require a pre-process
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
    {
        ChangeRequest *req = changeRequests[ii];
        if (req && req->needPreExecute()) {
            preRequests.push_back(req);
            changeRequests[ii] = NULL;
        }
    }
    
    pthread_mutex_unlock(&changeRequestLock);

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
    pthread_mutex_lock(&changeRequestLock);
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
        
    pthread_mutex_unlock(&changeRequestLock);
}
    
bool Scene::hasChanges(TimeInterval now)
{
    bool changes = false;
    if (!pthread_mutex_trylock(&changeRequestLock))
    {
        changes = !changeRequests.empty();
        
        if (!changes)
            if (timedChangeRequests.size() > 0)
                changes = now >= (*timedChangeRequests.begin())->when;
        
        pthread_mutex_unlock(&changeRequestLock);            
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
    pthread_mutex_lock(&subTexLock);
    subTextureMap.insert(subTex);
    pthread_mutex_unlock(&subTexLock);
}

// Add a whole group of sub textures maps
void Scene::addSubTextures(const std::vector<SubTexture> &subTexes)
{
    pthread_mutex_lock(&subTexLock);
    subTextureMap.insert(subTexes.begin(),subTexes.end());
    pthread_mutex_unlock(&subTexLock);
}
    
void Scene::removeSubTexture(SimpleIdentity subTexID)
{
    pthread_mutex_lock(&subTexLock);
    SubTexture dumbTex(subTexID);
    SubTextureSet::iterator it = subTextureMap.find(dumbTex);
    if (it != subTextureMap.end())
        subTextureMap.erase(it);
    pthread_mutex_unlock(&subTexLock);
}

void Scene::removeSubTextures(const std::vector<SimpleIdentity> &subTexIDs)
{
    pthread_mutex_lock(&subTexLock);
    SubTexture dummySubTex;
    for (auto texID : subTexIDs)
    {
        dummySubTex.setId(texID);
        auto it = subTextureMap.find(dummySubTex);
        if (it != subTextureMap.end())
            subTextureMap.erase(it);
    }
    pthread_mutex_unlock(&subTexLock);
}

// Look for a sub texture by ID
SubTexture Scene::getSubTexture(SimpleIdentity subTexId)
{
    pthread_mutex_lock(&subTexLock);
    SubTexture dumbTex;
    dumbTex.setId(subTexId);
    SubTextureSet::iterator it = subTextureMap.find(dumbTex);
    if (it == subTextureMap.end())
    {
        SubTexture passTex;
        passTex.trans = passTex.trans.Identity();
        passTex.texId = subTexId;
        pthread_mutex_unlock(&subTexLock);
        return passTex;
    }
    
    pthread_mutex_unlock(&subTexLock);
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
    pthread_mutex_lock(&programLock);

    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end())
    {
        prog = *it;
    }
    
    pthread_mutex_unlock(&programLock);
        
    return prog;
}

void Scene::addProgram(OpenGLES2Program *prog)
{
    if (!prog) {
        wkLogLevel(Warn,"Tried to add NULL program to scene.  Ignoring.");
        return;
    }

    pthread_mutex_lock(&programLock);
    
    if (glPrograms.find(prog) == glPrograms.end())
        glPrograms.insert(prog);
    
    pthread_mutex_unlock(&programLock);
}

void Scene::removeProgram(SimpleIdentity progId)
{
    pthread_mutex_lock(&programLock);
    
    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end()) {
        prog = *it;
        glPrograms.erase(it);
        
        prog->cleanUp();
    }
        
    pthread_mutex_unlock(&programLock);
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
    pthread_mutex_lock(&scene->textureLock);
    auto it = scene->textures.find(texture);
    if (it != scene->textures.end())
    {
        TextureBaseRef tex = it->second;
        tex->destroyInGL(scene->getMemManager());
        scene->textures.erase(it);
    } else
        wkLogLevel(Warn,"RemTextureReq: No such texture.");
    pthread_mutex_unlock(&scene->textureLock);
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
