/*
 *  Scene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2012 mousebird consulting
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
#import "ScreenSpaceGenerator.h"
#import "ViewPlacementGenerator.h"
#import "FontTextureManager.h"

namespace WhirlyKit
{
    
Scene::Scene()
    : defaultProgramTri(EmptyIdentity), defaultProgramLine(EmptyIdentity), selectManager(NULL)
{
    
}
    
void Scene::Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr,unsigned int depth)
{
    coordAdapter = adapter;
    cullTree = new CullTree(adapter,localMbr,depth);
    
    // Also toss in a screen space generator to share amongst the layers
    ssGen = new ScreenSpaceGenerator(kScreenSpaceGeneratorShared,Point2f(0.1,0.1));
    screenSpaceGeneratorID = ssGen->getId();
    generators.insert(ssGen);
    // And put in a UIView placement generator for use in the main thread
    vpGen = new ViewPlacementGenerator(kViewPlacementGeneratorShared);
    generators.insert(vpGen);
    
    // Selection manager is used for object selection from any thread
    selectManager = new SelectionManager(this,[UIScreen mainScreen].scale);
    
    // Font Texture manager is used from any thread
    fontTexManager = [[WhirlyKitFontTextureManager alloc] initWithScene:this];
    
    activeModels = [NSMutableArray array];
    
    pthread_mutex_init(&changeRequestLock,NULL);        
    pthread_mutex_init(&subTexLock, NULL);
    pthread_mutex_init(&textureLock,NULL);
    pthread_mutex_init(&generatorLock,NULL);
    pthread_mutex_init(&programLock,NULL);
}

Scene::~Scene()
{
    if (cullTree)
    {
        delete cullTree;
        cullTree = NULL;
    }
    for (TextureSet::iterator it = textures.begin(); it != textures.end(); ++it)
        delete *it;
    for (GeneratorSet::iterator it = generators.begin(); it != generators.end(); ++it)
        delete *it;
    
    if (selectManager)
    {
        delete selectManager;
        selectManager = NULL;
    }
    
    fontTexManager = nil;
    
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
    
    activeModels = nil;
    
    subTextureMap.clear();

    // Note: Should be clearing program out of context somewhere
    for (std::set<OpenGLES2Program *,IdentifiableSorter>::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        delete *it;
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
void Scene::addChangeRequests(const std::vector<ChangeRequest *> &newChanges)
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
        [activeModel shutdown];
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

// Process outstanding changes.
// We'll grab the lock and we're only expecting to be called in the rendering thread
void Scene::processChanges(WhirlyKitView *view,WhirlyKitSceneRendererES *renderer)
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
    NSLog(@"Scene: %d active models",[activeModels count]);
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
    std::set<OpenGLES2Program *,IdentifiableSorter>::iterator it = glPrograms.find(&dummy);
    if (it != glPrograms.end())
        prog = *it;
    
    pthread_mutex_unlock(&programLock);
        
    return prog;
}
    
OpenGLES2Program *Scene::getProgram(const std::string &name)
{
    pthread_mutex_lock(&programLock);
    
    OpenGLES2Program *prog = NULL;
    for (std::set<OpenGLES2Program *,IdentifiableSorter>::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        if ((*it)->getName() == name)
            prog = *it;
            
    pthread_mutex_unlock(&programLock);
    
    return prog;
}

SimpleIdentity Scene::getProgramId(const std::string &name)
{
    pthread_mutex_lock(&programLock);
    
    OpenGLES2Program *prog = NULL;
    for (std::set<OpenGLES2Program *,IdentifiableSorter>::iterator it = glPrograms.begin();
         it != glPrograms.end(); ++it)
        if ((*it)->getName() == name)
            prog = *it;
    
    pthread_mutex_unlock(&programLock);
    
    if (prog)
        return prog->getId();
    return EmptyIdentity;
}

void Scene::addProgram(OpenGLES2Program *prog)
{
    pthread_mutex_lock(&programLock);
    
    glPrograms.insert(prog);
    
    pthread_mutex_unlock(&programLock);
}
    
void Scene::removeProgram(SimpleIdentity progId)
{
    pthread_mutex_lock(&programLock);

    std::set<OpenGLES2Program *,IdentifiableSorter>::iterator it;
    for (it = glPrograms.begin();it != glPrograms.end(); ++it)
        if ((*it)->getId() == progId)
            break;
    
    if (it != glPrograms.end())
        glPrograms.erase(it);
    
    pthread_mutex_unlock(&programLock);
}

void Scene::setDefaultPrograms(OpenGLES2Program *progTri,OpenGLES2Program *progLine)
{
    if (progTri)
    {
        addProgram(progTri);
        defaultProgramTri = progTri->getId();
    }
    if (progLine)
    {
        defaultProgramLine = progLine->getId();
        addProgram(progLine);
    }
}
    
void Scene::getDefaultProgramIDs(SimpleIdentity &triShader,SimpleIdentity &lineShader)
{
    pthread_mutex_lock(&programLock);

    triShader = defaultProgramTri;
    lineShader = defaultProgramLine;

    pthread_mutex_unlock(&programLock);
}

void AddTextureReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    if (!tex->getGLId())
        tex->createInGL(scene->getMemManager());
    scene->textures.insert(tex);
    tex = NULL;
}

void RemTextureReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    TextureBase dumbTex(texture);
    Scene::TextureSet::iterator it = scene->textures.find(&dumbTex);
    if (it != scene->textures.end())
    {
        TextureBase *tex = *it;
        tex->destroyInGL(scene->getMemManager());
        scene->textures.erase(it);
        delete tex;
    }
}

void AddDrawableReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    DrawableRef drawRef(drawable);
    scene->addDrawable(drawRef);
        
    // Initialize any OpenGL foo
    WhirlyKitGLSetupInfo *setupInfo = [[WhirlyKitGLSetupInfo alloc] init];
    setupInfo->minZres = [view calcZbufferRes];
    drawable->setupGL(setupInfo,scene->getMemManager());
    
    drawable->updateRenderer(renderer);
        
    drawable = NULL;
}

void RemDrawableReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
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
    scene->addProgram(program);
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
