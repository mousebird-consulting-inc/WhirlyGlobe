/*
 *  MaplyActiveSticker.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/8/13.
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

#import "MaplyActiveSticker.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"
#import "MaplySharedAttributes.h"
#import "SphericalEarthChunkManager.h"

using namespace WhirlyKit;

@implementation MaplyActiveSticker
{
    MaplySticker *sticker;
    NSDictionary *desc;
    UIImage *image;
    SimpleIdentity texId;
    SimpleIDSet chunkIDs;
}

- (void)setActiveSticker:(MaplySticker *)newSticker desc:(NSDictionary *)inDesc
{
    if (dispatchQueue)
    {
        dispatch_async(dispatchQueue,
                       ^{
                           @synchronized(self)
                           {
                               sticker = newSticker;
                               desc = inDesc;
                               [self update];
                           }
                       }
                       );
    } else {
        @synchronized(self)
        {
            sticker = newSticker;
            desc = inDesc;
            [self update];
        }
    }
}

// Flush out changes to the scene
- (void)update
{
    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    
    ChangeSet changes;
    if (chunkManager && !chunkIDs.empty())
        chunkManager->removeChunks(chunkIDs, changes);
    chunkIDs.clear();

    SimpleIdentity removeTexId = EmptyIdentity;
    if (sticker)
    {
        // Possibly get rid of the image too
        if (image && sticker.image != image)
        {
            removeTexId = texId;
            texId = EmptyIdentity;
            image = nil;
        }
        
        // And make a new one
        if (sticker.image != image)
        {
            Texture *tex = new Texture("Active Sticker",sticker.image);
            texId = tex->getId();
            image = sticker.image;
            changes.push_back(new AddTextureReq(tex));
            
            // Note: Debugging
//            NSData *imageData = UIImagePNGRepresentation(image);
//            if (imageData)
//            {
//                NSArray *myPathList = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
//                NSString *myPath    = [myPathList  objectAtIndex:0];
//                [imageData writeToFile:[NSString stringWithFormat:@"%@/%ld.png",myPath,texId] atomically:YES];
//            }
        }

        // Build the chunk
        WhirlyKitSphericalChunk *chunk = [[WhirlyKitSphericalChunk alloc] init];
        GeoMbr geoMbr = GeoMbr(GeoCoord(sticker.ll.x,sticker.ll.y), GeoCoord(sticker.ur.x,sticker.ur.y));
        chunk.mbr = geoMbr;
        chunk.texId = texId;
        chunk.drawOffset = [desc[@"drawOffset"] floatValue];
        chunk.drawPriority = [desc[@"drawPriority"] floatValue];
        chunk.sampleX = [desc[@"sampleX"] intValue];
        chunk.sampleY = [desc[@"sampleY"] intValue];
        chunk.rotation = sticker.rotation;
        NSNumber *bufRead = desc[kMaplyZBufferRead];
        if (bufRead)
            chunk.readZBuffer = [bufRead boolValue];
        NSNumber *bufWrite = desc[kMaplyZBufferWrite];
        if (bufWrite)
            chunk.writeZBuffer = [bufWrite boolValue];
        
        if (chunkManager)
        {
            SimpleIdentity chunkID = chunkManager->addChunk(chunk, false, true, changes);
            if (chunkID != EmptyIdentity)
                chunkIDs.insert(chunkID);
        }
    }

    if (removeTexId != EmptyIdentity)
        changes.push_back(new RemTextureReq(removeTexId));
    
    scene->addChangeRequests(changes);
}

- (void)shutdown
{
    @synchronized(self)
    {
        sticker = nil;
        desc = nil;
        [self update];
    }
}

@end
