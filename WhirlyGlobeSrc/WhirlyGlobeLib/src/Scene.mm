/*
 *  Scene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011 mousebird consulting
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

namespace WhirlyKit
{
    
Scene::Scene(unsigned int numX, unsigned int numY,WhirlyKit::CoordSystem *coordSystem)
    : numX(numX), numY(numY), coordSystem(coordSystem)
{
    cullables = new Cullable [numX*numY];
    
    // Set up the various MBRs
    GeoCoord geoIncr(2*M_PI/numX,M_PI/numY);
    for (unsigned int iy=0;iy<numY;iy++)
    {
        for (unsigned int ix=0;ix<numX;ix++)
        {
            // Set up the extents for each cullable
            GeoCoord geoLL(-M_PI + ix*geoIncr.x(),-M_PI/2.0 + iy*geoIncr.y());
            GeoCoord geoUR(geoLL.x() + geoIncr.x(),geoLL.y() + geoIncr.y());
            Cullable &cullable = cullables[iy*numX+ix];
            cullable.setGeoMbr(GeoMbr(geoLL,geoUR),coordSystem);
        }
    }
    
    // Also toss in a screen space generator to share amongst the layers
    ScreenSpaceGenerator *ssGen = new ScreenSpaceGenerator(kScreenSpaceGeneratorShared);
    screenSpaceGeneratorID = ssGen->getId();
    generators.insert(ssGen);
    
    pthread_mutex_init(&changeRequestLock,NULL);
}

Scene::~Scene()
{
    delete [] cullables;
    for (DrawableSet::iterator it = drawables.begin(); it != drawables.end(); ++it)
        delete *it;
    for (TextureSet::iterator it = textures.begin(); it != textures.end(); ++it)
        delete *it;
    for (GeneratorSet::iterator it = generators.begin(); it != generators.end(); ++it)
        delete *it;
    
    pthread_mutex_destroy(&changeRequestLock);
    
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
        delete changeRequests[ii];
    changeRequests.clear();
    
    subTextureMap.clear();
}

// Remove the given drawable from all the cullables
// Note: Optimize this
void Scene::removeFromCullables(Drawable *drawable)
{
    for (unsigned int ii=0;ii<numX*numY;ii++)
    {
        Cullable &cullable = cullables[ii];
        cullable.remDrawable(drawable);
    }
}
    
SimpleIdentity Scene::getGeneratorIDByName(const std::string &name)
{
    for (GeneratorSet::iterator it = generators.begin();
         it != generators.end(); ++it)
    {
        Generator *gen = *it;
        if (!name.compare(gen->name))
            return gen->getId();
    }
    
    return EmptyIdentity;
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
    Texture dumbTex;
    dumbTex.setId(texIdent);
    TextureSet::iterator it = textures.find(&dumbTex);
    if (it != textures.end())
        return (*it)->getGLId();
    
    return 0;
}

Drawable *Scene::getDrawable(SimpleIdentity drawId)
{
    BasicDrawable dumbDraw;
    dumbDraw.setId(drawId);
    Scene::DrawableSet::iterator it = drawables.find(&dumbDraw);
    if (it != drawables.end())
        return *it;
    
    return NULL;
}

Generator *Scene::getGenerator(SimpleIdentity genId)
{
    Generator dumbGen;
    dumbGen.setId(genId);
    GeneratorSet::iterator it = generators.find(&dumbGen);
    if (it != generators.end())
        return *it;
    
    return NULL;
}

Texture *Scene::getTexture(SimpleIdentity texId)
{
    Texture dumbTex;
    dumbTex.setId(texId);
    Scene::TextureSet::iterator it = textures.find(&dumbTex);
    if (it != textures.end())
        return *it;
    
    return NULL;
}

// Process outstanding changes.
// We'll grab the lock and we're only expecting to be called in the rendering thread
void Scene::processChanges(WhirlyKitView *view)
{
    std::vector<Cullable *> foundCullables;
    
    // We're not willing to wait in the rendering thread
    if (!pthread_mutex_trylock(&changeRequestLock))
    {
        for (unsigned int ii=0;ii<changeRequests.size();ii++)
        {
            ChangeRequest *req = changeRequests[ii];
            req->execute(this,view);
            delete req;
        }
        changeRequests.clear();
        
        pthread_mutex_unlock(&changeRequestLock);
    }
}

// Add a single sub texture map
void Scene::addSubTexture(const SubTexture &subTex)
{
    subTextureMap.insert(subTex);
}

// Add a whole group of sub textures maps
void Scene::addSubTextures(const std::vector<SubTexture> &subTexes)
{
    subTextureMap.insert(subTexes.begin(),subTexes.end());
}

// Look for a sub texture by ID
SubTexture Scene::getSubTexture(SimpleIdentity subTexId)
{
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
    
void Scene::addDrawable(Drawable *drawable)
{
    // By default we'll add it without any thought
    drawables.insert(drawable);
}
    
SimpleIdentity Scene::getScreenSpaceGeneratorID()
{
    return screenSpaceGeneratorID;
}


void AddTextureReq::execute(Scene *scene,WhirlyKitView *view)
{
    if (!tex->getGLId())
        tex->createInGL(true);
    scene->textures.insert(tex);
    tex = NULL;
}

void RemTextureReq::execute(Scene *scene,WhirlyKitView *view)
{
    Texture dumbTex;
    dumbTex.setId(texture);
    Scene::TextureSet::iterator it = scene->textures.find(&dumbTex);
    if (it != scene->textures.end())
    {
        Texture *tex = *it;
        tex->destroyInGL();
        scene->textures.erase(it);
        delete tex;
    }
}

void AddDrawableReq::execute(Scene *scene,WhirlyKitView *view)
{
    scene->addDrawable(drawable);
        
    // Initialize any OpenGL foo
    // Note: Make the Z offset a parameter
    drawable->setupGL([view calcZbufferRes]);
    
    drawable = NULL;
}

void RemDrawableReq::execute(Scene *scene,WhirlyKitView *view)
{
    BasicDrawable dumbDraw;
    dumbDraw.setId(drawable);
    Scene::DrawableSet::iterator it = scene->drawables.find(&dumbDraw);
    if (it != scene->drawables.end())
    {
        Drawable *theDrawable = *it;
        scene->removeFromCullables(theDrawable);
        
        scene->drawables.erase(it);
        // Teardown OpenGL foo
        theDrawable->teardownGL();
        // And delete
        delete theDrawable;
    }
}

void AddGeneratorReq::execute(Scene *scene,WhirlyKitView *view)
{
    // Add the generator
    scene->generators.insert(generator);
    
    generator = NULL;
}

void RemGeneratorReq::execute(Scene *scene,WhirlyKitView *view)
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

void NotificationReq::execute(Scene *scene,WhirlyKitView *view)
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
