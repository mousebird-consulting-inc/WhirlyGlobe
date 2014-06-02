/*
 *  SphericalEarthChunkLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/23/13.
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

#import <queue>
#import <set>
#import "SphericalEarthChunkLayer.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitSphericalChunkLayer
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyKit::Scene *scene;
    // Our own (unshared) chunk manager
    SphericalChunkManager *chunkManager;
    DynamicTextureAtlas *texAtlas;
    DynamicDrawableAtlas *drawAtlas;
    // We gather changes between swaps when we're using atlases
    ChangeSet changes;
    bool sleeping;
    SimpleIDSet chunkIDs;
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _ignoreEdgeMatching = false;
    _useDynamicAtlas = true;
    
    return self;
}

- (void)dealloc
{
    if (chunkManager)
    {
        delete chunkManager;
        chunkManager = NULL;
    }
    for (unsigned int ii=0;ii<changes.size();ii++)
        delete changes[ii];
    changes.clear();
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    chunkManager = new SphericalChunkManager();
    chunkManager->setScene(inScene);
}

- (void)shutdown
{
    if (chunkManager)
    {
        chunkManager->removeChunks(chunkIDs,changes);
        delete chunkManager;
        chunkManager = NULL;
    }
    chunkIDs.clear();
    
    if (drawAtlas)
    {
        drawAtlas->shutdown(changes);
        delete drawAtlas;
    }
    drawAtlas = NULL;
    if (texAtlas)
    {
        texAtlas->shutdown(changes);
        delete texAtlas;
    }
    texAtlas = NULL;
    
    if (chunkManager)
    {
        delete chunkManager;
    }
    chunkManager = NULL;

    [layerThread addChangeRequests:changes];
    changes.clear();
    
    scene = NULL;
}

// Process outstanding requests
- (void)processQueue
{
    if (!scene)
        return;
        
    // If there's an outstanding swap going on, can't do this now, wait for the wakeUp
    if (sleeping || (drawAtlas && drawAtlas->waitingOnSwap()))
        return;
    
    if (chunkManager)
        chunkManager->processRequests(changes);

    bool addChanges = false;
    if (drawAtlas)
    {
        if (drawAtlas->hasUpdates() && !drawAtlas->waitingOnSwap())
        {
            drawAtlas->swap(changes, self, @selector(wakeUp));
            sleeping = true;
            addChanges = true;
        }
    } else
        addChanges = true;
    
    if (addChanges)
    {
        [layerThread addChangeRequests:changes];
        changes.clear();
    }
}

// Called by the main thread to wake us up after a buffer swap
- (void)wakeUp
{
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(wakeUp) onThread:layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    sleeping = false;
    [self processQueue];
}
// The vertex size is used for estimating
static const int SingleVertexSize = 3*sizeof(float) + 2*sizeof(float) +  4*sizeof(unsigned char) + 3*sizeof(float);
static const int SingleElementSize = sizeof(GLushort);

/// Add a single chunk on the spherical earth.  This returns and ID
///  we can use to remove it later.
- (WhirlyKit::SimpleIdentity)addChunk:(WhirlyKitSphericalChunk *)chunk enable:(bool)enable
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitSphericalChunk: addChunk called before layer initialized or in wrong thread.");
        return EmptyIdentity;
    }
    
    SimpleIdentity chunkID = EmptyIdentity;
    if (chunkManager)
    {
        // Build the atlases if we need them
        if (chunk.loadImage && !texAtlas && _useDynamicAtlas)
        {
            texAtlas = new DynamicTextureAtlas(2048,64,GL_UNSIGNED_BYTE);
            
            // At 256 pixels square we can hold 64 tiles in a texture atlas
            int DrawBufferSize = 2 * (8 + 1) * (8 + 1) * SingleVertexSize * 64;
            // Two triangles per grid cell in a tile
            int ElementBufferSize = 2 * 6 * (8 + 1) * (8 + 1) * SingleElementSize * 64;
            drawAtlas = new DynamicDrawableAtlas("Spherical Earth Chunk",SingleElementSize,DrawBufferSize,ElementBufferSize,scene->getMemManager());
            
            chunkManager->setAtlases(texAtlas, drawAtlas);
            chunkManager->setBorderTexel(1);
        }
        chunkID = chunkManager->addChunk(chunk,!_ignoreEdgeMatching,enable,changes);
        if (chunkID != EmptyIdentity)
            chunkIDs.insert(chunkID);
    }
    [self processQueue];
    
    return chunkID;
}

/// Remove the named chunk
- (void)removeChunk:(WhirlyKit::SimpleIdentity)chunkID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
//        NSLog(@"WhirlyKitSphericalChunk: removeChunk called before layer initialized or in wrong thread.");
        return;
    }

    if (chunkManager)
    {
        SimpleIDSet::iterator it = chunkIDs.find(chunkID);
        if (it != chunkIDs.end())
        {
            SimpleIDSet theIDs;
            theIDs.insert(chunkID);
            chunkManager->removeChunks(theIDs,changes);
            chunkIDs.erase(it);
        }
    }
    [self processQueue];
}

- (void)toogleChunk:(WhirlyKit::SimpleIdentity)chunkID enable:(bool)enable
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitSphericalChunk: toogleChunk called before layer initialized or in wrong thread.");
        return;
    }

    if (chunkManager)
    {
        SimpleIDSet::iterator it = chunkIDs.find(chunkID);
        if (it != chunkIDs.end())
            chunkManager->enableChunk(chunkID,enable,changes);
    }
    [self processQueue];
}

- (int)numChunk
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
        return 0;
    
    if (chunkManager)
        return chunkManager->getNumChunks();
    else
        return 0;
}

@end
