/*
 *  Scene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2017 mousebird consulting
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

#import "Scene.h"
#import "GlobeView.h"
#import "GlobeMath.h"
#import "TextureAtlas.h"
#import "ViewPlacementGenerator.h"
#import "FontTextureManager.h"
#import "SelectionManager.h"
#import "IntersectionManager.h"
#import "LayoutManager.h"
#import "ShapeManager.h"
#import "MarkerManager.h"
#import "LabelManager.h"
#import "VectorManager.h"
#import "SphericalEarthChunkManager.h"
#import "LoftManager.h"
#import "ParticleSystemManager.h"
#import "BillboardManager.h"
#import "WideVectorManager.h"
#import "GeometryManager.h"

namespace WhirlyKit
{
    
Scene::Scene()
    : ssGen(NULL)
{
}
    
void Scene::Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr,unsigned int depth)
{
    pthread_mutex_init(&coordAdapterLock,NULL);
    pthread_mutex_init(&changeRequestLock,NULL);
    pthread_mutex_init(&subTexLock, NULL);
    pthread_mutex_init(&textureLock,NULL);
    pthread_mutex_init(&generatorLock,NULL);
    pthread_mutex_init(&programLock,NULL);
    pthread_mutex_init(&managerLock,NULL);

    ssGen = NULL;
    
    coordAdapter = adapter;
    cullTree = new CullTree(adapter,localMbr,depth);
    
    // And put in a UIView placement generator for use in the main thread
    vpGen = new ViewPlacementGenerator(kViewPlacementGeneratorShared);
    generators.insert(vpGen);

    dispatchQueue = dispatch_queue_create("WhirlyKit Scene", 0);

    // Selection manager is used for object selection from any thread
    addManager(kWKSelectionManager,new SelectionManager(this,[UIScreen mainScreen].scale));
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
    
    // Font Texture manager is used from any thread
    fontTexManager = [[WhirlyKitFontTextureManager alloc] initWithScene:this];
    
    activeModels = [NSMutableArray array];
    
    overlapMargin = 0.0;
}

Scene::~Scene()
{
    pthread_mutex_destroy(&coordAdapterLock);

    if (cullTree)
    {
        delete cullTree;
        cullTree = NULL;
    }
    textures.clear();
    for (GeneratorSet::iterator it = generators.begin(); it != generators.end(); ++it)
        delete *it;
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        delete it->second;
    managers.clear();
    
    fontTexManager = nil;
    
    pthread_mutex_destroy(&managerLock);
    pthread_mutex_destroy(&changeRequestLock);
    pthread_mutex_destroy(&subTexLock);
    pthread_mutex_destroy(&textureLock);
    pthread_mutex_destroy(&generatorLock);
    pthread_mutex_destroy(&programLock);
    
    auto theChangeRuquests = changeRequests;
    changeRequests.clear();
    for (unsigned int ii=0;ii<theChangeRuquests.size();ii++)
    {
        // Note: Tear down change requests?
        delete theChangeRuquests[ii];
    }
    
    activeModels = nil;
    
    subTextureMap.clear();

    // Note: Should be clearing program out of context somewhere
    for (OpenGLES2ProgramSet::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        delete *it;
    glPrograms.clear();
    glProgramMap.clear();
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
    
SimpleIdentity Scene::getGeneratorIDByName(const std::string &name)
{
    pthread_mutex_lock(&generatorLock);
    
    SimpleIdentity retId = EmptyIdentity;
    for (GeneratorSet::iterator it = generators.begin();
         it != generators.end(); ++it)
    {
        Generator *gen = *it;
        if (!name.compare(gen->name))
        {
            retId = gen->getId();
            break;
        }
    }
    
    pthread_mutex_unlock(&generatorLock);

    return retId;
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
    TextureBaseRef dumbTex(new TextureBase(texIdent));
    Scene::TextureRefSet::iterator it = textures.find(dumbTex);
    if (it != textures.end())
    {
        ret = (*it)->getGLId();
    }
    
    pthread_mutex_unlock(&textureLock);
    
    return ret;
}

DrawableRef Scene::getDrawable(SimpleIdentity drawId)
{
    BasicDrawable *dumbDraw = new BasicDrawable("None");
    dumbDraw->setId(drawId);
    DrawableRefSet::iterator it = drawables.find(DrawableRef(dumbDraw));
    if (it != drawables.end())
        return *it;
    
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

Generator *Scene::getGenerator(SimpleIdentity genId)
{
    pthread_mutex_lock(&generatorLock);
    
    Generator *retGen = NULL;
    Generator dumbGen;
    dumbGen.setId(genId);
    GeneratorSet::iterator it = generators.find(&dumbGen);
    if (it != generators.end())
    {
        retGen = *it;
    }
    
    pthread_mutex_unlock(&generatorLock);
    
    return retGen;
}
    
void Scene::setRenderer(WhirlyKitSceneRendererES *renderer)
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

void Scene::addActiveModel(NSObject<WhirlyKitActiveModel> *activeModel)
{
    [activeModels addObject:activeModel];
    [activeModel startWithScene:this];
}
    
void Scene::removeActiveModel(NSObject<WhirlyKitActiveModel> *activeModel)
{
    if ([activeModels containsObject:activeModel])
    {
        [activeModels removeObject:activeModel];
        [activeModel teardown];
    }
}
    
void Scene::teardownGL()
{
    // Note: Tear down generators
    // Note: Tear down active models
    for (DrawableRefSet::iterator it = drawables.begin();
         it != drawables.end(); ++it)
        (*it)->teardownGL(&memManager);
    if (cullTree)
    {
        delete cullTree;
        cullTree = NULL;
    }
    drawables.clear();
    for (TextureRefSet::iterator it = textures.begin();
         it != textures.end(); ++it)
    {
        TextureBaseRef texRef = *it;
        texRef->destroyInGL(&memManager);
    }
    textures.clear();
    
    memManager.clearBufferIDs();
    memManager.clearTextureIDs();
}

TextureBase *Scene::getTexture(SimpleIdentity texId)
{
    pthread_mutex_lock(&textureLock);
    
    TextureBase *retTex = NULL;
    TextureBaseRef dumbTex(new TextureBase(texId));
    Scene::TextureRefSet::iterator it = textures.find(dumbTex);
    if (it != textures.end())
        retTex = it->get();
    
    pthread_mutex_unlock(&textureLock);
    
    return retTex;
}
    
const DrawableRefSet &Scene::getDrawables()
{
    return drawables;
}

// Process outstanding changes.
// We'll grab the lock and we're only expecting to be called in the rendering thread
void Scene::processChanges(WhirlyKitView *view,WhirlyKitSceneRendererES *renderer,NSTimeInterval now)
{
    // We're not willing to wait in the rendering thread
    if (!pthread_mutex_trylock(&changeRequestLock))
    {
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
}
    
bool Scene::hasChanges(NSTimeInterval now)
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
    for (NSObject<WhirlyKitActiveModel> *model in activeModels)
        if ([model hasUpdate])
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
    
SimpleIdentity Scene::getScreenSpaceGeneratorID()
{
    return screenSpaceGeneratorID;
}

void Scene::dumpStats()
{
    NSLog(@"Scene: %ld drawables",drawables.size());
    NSLog(@"Scene: %d active models",(int)[activeModels count]);
    NSLog(@"Scene: %ld generators",generators.size());
    NSLog(@"Scene: %ld textures",textures.size());
    NSLog(@"Scene: %ld sub textures",subTextureMap.size());
    cullTree->dumpStats();
    memManager.dumpStats();
    for (GeneratorSet::iterator it = generators.begin();
         it != generators.end(); ++it)
        (*it)->dumpStats();
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
    
OpenGLES2Program *Scene::getProgramBySceneName(const std::string &sceneName)
{
    OpenGLES2Program *prog = NULL;

    pthread_mutex_lock(&programLock);
    
    OpenGLES2ProgramMap::iterator it = glProgramMap.find(sceneName);
    if (it != glProgramMap.end())
        prog = it->second;
            
    pthread_mutex_unlock(&programLock);
    
    return prog;
}

SimpleIdentity Scene::getProgramIDBySceneName(const std::string &sceneName)
{
    OpenGLES2Program *prog = getProgramBySceneName(sceneName);
    return prog ? prog->getId() : EmptyIdentity;
}
    
OpenGLES2Program *Scene::getProgramByName(const std::string &name)
{
    OpenGLES2Program *prog = NULL;
    
    pthread_mutex_lock(&programLock);
    
    OpenGLES2ProgramMap::iterator it = glProgramMap.find(name);
    if (it != glProgramMap.end())
        prog = it->second;
    
    pthread_mutex_unlock(&programLock);
    
    return prog;
}
    
/// Search for a shader program by its name (not the scene name)
SimpleIdentity Scene::getProgramIDByName(const std::string &name)
{
    OpenGLES2Program *prog = getProgramByName(name);
    return prog ? prog->getId() : EmptyIdentity;
}
    
void Scene::addProgram(OpenGLES2Program *prog)
{
    pthread_mutex_lock(&programLock);
    
    if (glPrograms.find(prog) == glPrograms.end())
        glPrograms.insert(prog);
    
    pthread_mutex_unlock(&programLock);
}


void Scene::addProgram(const std::string &sceneName,OpenGLES2Program *prog)
{
    pthread_mutex_lock(&programLock);

    if (glPrograms.find(prog) == glPrograms.end())
        glPrograms.insert(prog);
    glProgramMap[sceneName] = prog;
    
    pthread_mutex_unlock(&programLock);
}
    
void Scene::setSceneProgram(const std::string &sceneName,SimpleIdentity progId)
{
    pthread_mutex_lock(&programLock);
    
    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end())
    {
        prog = *it;
    }
    
    if (prog)
        glProgramMap[sceneName] = prog;
    
    pthread_mutex_unlock(&programLock);
}
    
void Scene::removeProgram(SimpleIdentity progId)
{
    pthread_mutex_lock(&programLock);
    
    OpenGLES2Program *prog = NULL;
    OpenGLES2Program dummy(progId);
    OpenGLES2ProgramSet::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end())
        prog = *it;
    
    if (prog)
    {
        // Remove references in the map
        std::vector<OpenGLES2ProgramMap::iterator> toErase;
        OpenGLES2ProgramMap::iterator it2;
        for (it2 = glProgramMap.begin();it2 != glProgramMap.end(); ++it2)
            if (it2->second == prog)
                toErase.push_back(it2);
        for (unsigned int ii=0;ii<toErase.size();ii++)
            glProgramMap.erase(toErase[ii]);
        
        // And get rid of it in the list of programs
        glPrograms.erase(it);
    }
    
    pthread_mutex_unlock(&programLock);
}
    
AddTextureReq::~AddTextureReq()
{
    texRef = NULL;
}
    
void AddTextureReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    if (!texRef->getGLId())
        texRef->createInGL(scene->getMemManager());
    scene->textures.insert(texRef);
    texRef = NULL;
}

void RemTextureReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    pthread_mutex_lock(&scene->textureLock);
    TextureBaseRef dumpTexRef(new TextureBase(texture));
    Scene::TextureRefSet::iterator it = scene->textures.find(dumpTexRef);
    if (it != scene->textures.end())
    {
        TextureBaseRef tex = *it;
        tex->destroyInGL(scene->getMemManager());
        scene->textures.erase(it);
    } else
        NSLog(@"RemTextureReq: No such texture.");
    pthread_mutex_unlock(&scene->textureLock);
}

AddDrawableReq::~AddDrawableReq()
{
    drawRef = NULL;
}

void AddDrawableReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
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
    WhirlyKitGLSetupInfo *setupInfo = [[WhirlyKitGLSetupInfo alloc] init];
    setupInfo->minZres = [view calcZbufferRes];
    drawRef->setupGL(setupInfo,scene->getMemManager());
    
    drawRef->updateRenderer(renderer);
        
    drawRef = NULL;
}

void RemDrawableReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    BasicDrawable *dumbDraw = new BasicDrawable("None");
    dumbDraw->setId(drawable);
    DrawableRefSet::iterator it = scene->drawables.find(DrawableRef(dumbDraw));
    if (it != scene->drawables.end())
    {
        [renderer removeContinuousRenderRequest:(*it)->getId()];
        
        // Teardown OpenGL foo
        (*it)->teardownGL(scene->getMemManager());

        scene->remDrawable(*it);        
    } else
        NSLog(@"Missing drawable for RemDrawableReq: %llu", drawable);
}

void AddGeneratorReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    // Add the generator
    scene->generators.insert(generator);
    
    generator = NULL;
}

void RemGeneratorReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    Generator dumbGen;
    dumbGen.setId(genId);
    GeneratorSet::iterator it = scene->generators.find(&dumbGen);
    if (it != scene->generators.end())
    {
        Generator *theGenerator = *it;
        scene->generators.erase(it);
        
        delete theGenerator;
    }
}

void AddProgramReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    scene->addProgram(sceneName,program);
    program = NULL;
}

void RemProgramReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    scene->removeProgram(programId);
}
    
void RemBufferReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    scene->getMemManager()->removeBufferID(bufID);
    bufID = 0;
}
    
NotificationReq::NotificationReq(NSString *inNoteName,NSObject *inNoteObj)
{
    noteName = inNoteName;
    noteObj = inNoteObj;
}

NotificationReq::~NotificationReq()
{
    noteName = nil;
    noteObj = nil;
}

void NotificationReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    NSString *theNoteName = noteName;
    NSObject *theNoteObj = noteObj;
    
    // Send out the notification on the main thread
    dispatch_async(dispatch_get_main_queue(),
                   ^{
                       [[NSNotificationCenter defaultCenter] postNotificationName:theNoteName object:theNoteObj];
                   });
}
    
}
