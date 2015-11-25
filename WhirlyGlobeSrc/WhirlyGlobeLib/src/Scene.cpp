/*
 *  Scene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2015 mousebird consulting
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
#import "Platform.h"
//#import "ViewPlacementGenerator.h"
#import "FontTextureManager.h"
#import "SelectionManager.h"
#import "LayoutManager.h"
//#import "ShapeManager.h"
#import "MarkerManager.h"
#import "LabelManager.h"
#import "VectorManager.h"
#import "SphericalEarthChunkManager.h"
//#import "LoftManager.h"
//#import "ParticleSystemManager.h"
//#import "BillboardManager.h"

namespace WhirlyKit
{
    
Scene::Scene()
    : fontTextureManager(NULL)
{
}
    
void Scene::Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr,unsigned int depth)
{
    pthread_mutex_init(&coordAdapterLock,NULL);
    coordAdapter = adapter;
    cullTree = new CullTree(adapter,localMbr,depth);

    pthread_mutex_init(&changeRequestLock,NULL);
    pthread_mutex_init(&subTexLock, NULL);
    pthread_mutex_init(&textureLock,NULL);
    pthread_mutex_init(&generatorLock,NULL);
    pthread_mutex_init(&programLock,NULL);
    
    // Note: Porting.  This won't work.  Need to instantiate a platform version
//    fontTextureManager = new FontTextureManager(this);
    
//    // And put in a UIView placement generator for use in the main thread
//    vpGen = new ViewPlacementGenerator(kViewPlacementGeneratorShared);
//    generators.insert(vpGen);
//
//    dispatchQueue = dispatch_queue_create("WhirlyKit Scene", 0);

    pthread_mutex_init(&managerLock,NULL);
    // Selection manager is used for object selection from any thread
    addManager(kWKSelectionManager,new SelectionManager(this,DeviceScreenScale()));
    // Layout manager handles text and icon layout
    addManager(kWKLayoutManager, new LayoutManager());
//    // Shape manager handles circles, spheres and such
//    addManager(kWKShapeManager, new ShapeManager());
    // Marker manager handles 2D and 3D markers
    addManager(kWKMarkerManager, new MarkerManager());
    // Label manager handes 2D and 3D labels
    addManager(kWKLabelManager, new LabelManager());
    // Vector manager handes vector features
    addManager(kWKVectorManager, new VectorManager());
    // Chunk manager handles geographic chunks that cover a large chunk of the globe
    addManager(kWKSphericalChunkManager, new SphericalChunkManager());
//    // Loft manager handles lofted polygon geometry
//    addManager(kWKLoftedPolyManager, new LoftManager());
//    // Particle system manager
//    addManager(kWKParticleSystemManager, new ParticleSystemManager());
//    // 3D billboards
//    addManager(kWKBillboardManager, new BillboardManager());
//    
//    // Font Texture manager is used from any thread
//    fontTexManager = [[WhirlyKitFontTextureManager alloc] initWithScene:this];
//    
//    activeModels = [NSMutableArray array];
}

Scene::~Scene()
{
    // This should block until the queue is empty
    // Note: Porting
//    dispatch_sync(dispatchQueue, ^{});
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 60000
//    dispatch_release(dispatchQueue);
#endif

    pthread_mutex_destroy(&coordAdapterLock);

    if (cullTree)
    {
        delete cullTree;
        cullTree = NULL;
    }
    for (TextureSet::iterator it = textures.begin(); it != textures.end(); ++it)
        delete *it;
    for (GeneratorSet::iterator it = generators.begin(); it != generators.end(); ++it)
        delete *it;
    
    for (std::map<std::string,SceneManager *>::iterator it = managers.begin();
         it != managers.end(); ++it)
        delete it->second;
    managers.clear();
    
    // Note: Porting
//    fontTexManager = nil;
    
    pthread_mutex_destroy(&managerLock);
    pthread_mutex_destroy(&changeRequestLock);
    pthread_mutex_destroy(&subTexLock);
    pthread_mutex_destroy(&textureLock);
    pthread_mutex_destroy(&generatorLock);
    pthread_mutex_destroy(&programLock);
    
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
    {
        // Note: Tear down change requests?
        delete changeRequests[ii];
    }
    changeRequests.clear();
    
    // Note: Porting
//    activeModels = nil;
    
    subTextureMap.clear();

    // Note: Should be clearing program out of context somewhere
    for (OpenGLES2ProgramSet::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        delete *it;
    glPrograms.clear();
    glProgramMap.clear();
    
    if (fontTextureManager)
        delete fontTextureManager;
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
    
    changeRequests.insert(changeRequests.end(),newChanges.begin(),newChanges.end());
    
    pthread_mutex_unlock(&changeRequestLock);
}

// Add a single change request
void Scene::addChangeRequest(ChangeRequest *newChange)
{
    pthread_mutex_lock(&changeRequestLock);
    
    changeRequests.push_back(newChange);
    
    pthread_mutex_unlock(&changeRequestLock);
}

GLuint Scene::getGLTexture(SimpleIdentity texIdent)
{
    if (texIdent == EmptyIdentity)
        return 0;
    
    GLuint ret = 0;
    
    pthread_mutex_lock(&textureLock);
    TextureBase dumbTex(texIdent);
    TextureSet::iterator it = textures.find(&dumbTex);
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
    
void Scene::setRenderer(WhirlyKit::SceneRendererES *renderer)
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

// Note: Porting
//void Scene::addActiveModel(NSObject<WhirlyKitActiveModel> *activeModel)
//{
//    [activeModels addObject:activeModel];
//    [activeModel startWithScene:this];
//}
    
// Note: Porting
//void Scene::removeActiveModel(NSObject<WhirlyKitActiveModel> *activeModel)
//{
//    if ([activeModels containsObject:activeModel])
//    {
//        [activeModels removeObject:activeModel];
//        [activeModel shutdown];
//    }
//}
    
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
    for (TextureSet::iterator it = textures.begin();
         it != textures.end(); ++it)
    {
        TextureBase *texture = *it;
        texture->destroyInGL(&memManager);
        delete texture;
    }
    textures.clear();
    
    memManager.clearBufferIDs();
    memManager.clearTextureIDs();
}

TextureBase *Scene::getTexture(SimpleIdentity texId)
{
    pthread_mutex_lock(&textureLock);
    
    TextureBase *retTex = NULL;
    TextureBase dumbTex(texId);
    Scene::TextureSet::iterator it = textures.find(&dumbTex);
    if (it != textures.end())
        retTex = *it;
    
    pthread_mutex_unlock(&textureLock);
    
    return retTex;
}
    
void Scene::addTexture(TextureBase *tex)
{
    pthread_mutex_lock(&textureLock);
    textures.insert(tex);
    pthread_mutex_unlock(&textureLock);
}

const DrawableRefSet &Scene::getDrawables()
{
    return drawables;
}

// Process outstanding changes.
// We'll grab the lock and we're only expecting to be called in the rendering thread
void Scene::processChanges(WhirlyKit::View *view,WhirlyKit::SceneRendererES *renderer)
{
    // We're not willing to wait in the rendering thread
    if (!pthread_mutex_trylock(&changeRequestLock))
    {
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
    
bool Scene::hasChanges()
{
    bool changes = false;
    if (!pthread_mutex_trylock(&changeRequestLock))
    {
        changes = !changeRequests.empty();
        
        pthread_mutex_unlock(&changeRequestLock);            
    }        
    if (changes)
        return true;
    
    // How about the active models?
    // Note: Porting
//    for (NSObject<WhirlyKitActiveModel> *model in activeModels)
//        if ([model hasUpdate])
//            return true;
    
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
    // Note: Porting
//    NSLog(@"Scene: %ld drawables",drawables.size());
//    NSLog(@"Scene: %d active models",[activeModels count]);
//    NSLog(@"Scene: %ld generators",generators.size());
//    NSLog(@"Scene: %ld textures",textures.size());
//    NSLog(@"Scene: %ld sub textures",subTextureMap.size());
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
    
void AddTextureReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    if (!tex->getGLId())
        tex->createInGL(scene->getMemManager());
    scene->addTexture(tex);
    tex = NULL;
}

void RemTextureReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    pthread_mutex_lock(&scene->textureLock);
    TextureBase dumbTex(texture);
    Scene::TextureSet::iterator it = scene->textures.find(&dumbTex);
    if (it != scene->textures.end())
    {
        TextureBase *tex = *it;
        tex->destroyInGL(scene->getMemManager());
        scene->textures.erase(it);
        delete tex;
    }
    pthread_mutex_unlock(&scene->textureLock);
}

void AddDrawableReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    // If this is an instance, deal with that madness
    BasicDrawableInstance *drawInst = dynamic_cast<BasicDrawableInstance *>(drawable);
    if (drawInst)
    {
        DrawableRef theDraw = scene->getDrawable(drawInst->getMasterID());
        BasicDrawableRef baseDraw = std::dynamic_pointer_cast<BasicDrawable>(theDraw);
        if (baseDraw)
            drawInst->setMaster(baseDraw);
        else {
            // Uh oh, dangling reference, just kill it
            delete drawable;
            return;
        }
    }

    DrawableRef drawRef(drawable);
    scene->addDrawable(drawRef);
        
    // Initialize any OpenGL foo
    WhirlyKitGLSetupInfo setupInfo;
    setupInfo.minZres = view->calcZbufferRes();
    drawable->setupGL(&setupInfo,scene->getMemManager());
    
    drawable->updateRenderer(renderer);
        
    drawable = NULL;
}

void RemDrawableReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    BasicDrawable *dumbDraw = new BasicDrawable("None");
    dumbDraw->setId(drawable);
    DrawableRefSet::iterator it = scene->drawables.find(DrawableRef(dumbDraw));
    if (it != scene->drawables.end())
    {
        // Teardown OpenGL foo
        (*it)->teardownGL(scene->getMemManager());

        scene->remDrawable(*it);        
    }
}

void AddGeneratorReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    // Add the generator
    scene->generators.insert(generator);
    
    generator = NULL;
}

void RemGeneratorReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
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

void AddProgramReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    scene->addProgram(sceneName,program);
    program = NULL;
}

void RemProgramReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    scene->removeProgram(programId);
}
    
// Note: Porting
//void RemBufferReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
//{
//    scene->getMemManager()->removeBufferID(bufID);
//    bufID = 0;
//}
//    
//NotificationReq::NotificationReq(NSString *inNoteName,NSObject *inNoteObj)
//{
//    noteName = inNoteName;
//    noteObj = inNoteObj;
//}
//
//NotificationReq::~NotificationReq()
//{
//    noteName = nil;
//    noteObj = nil;
//}
//
//void NotificationReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
//{
//    NSString *theNoteName = noteName;
//    NSObject *theNoteObj = noteObj;
//    
//    // Send out the notification on the main thread
//    dispatch_async(dispatch_get_main_queue(),
//                   ^{
//                       [[NSNotificationCenter defaultCenter] postNotificationName:theNoteName object:theNoteObj];
//                   });
//}
    
void SetProgramValueReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    OpenGLES2Program *prog = scene->getProgram(progID);
    if (prog)
    {
        prog->setUniform(u_name, u_val);
    }
}

}
