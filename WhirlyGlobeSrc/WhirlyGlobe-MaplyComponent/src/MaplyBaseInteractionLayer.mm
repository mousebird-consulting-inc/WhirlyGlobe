/*
 *  MaplyBaseInteractionLayer.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012 mousebird consulting
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

#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyScreenMarker.h"
#import "MaplyMarker.h"
#import "MaplyScreenLabel.h"
#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShape.h"
#import "MaplySticker.h"
#import "MaplyCoordinate.h"
#import "ImageTexture_private.h"
#import "MaplySharedAttributes.h"

using namespace Eigen;
using namespace WhirlyKit;

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircle(MaplyCoordinate startPt,MaplyCoordinate endPt,float height,std::vector<Point3f> &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
{
    bool isFlat = coordAdapter->isFlat();

    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(startPt.x,startPt.y)));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(endPt.x,endPt.y)));
    } else {
        VectorRing inPts;
        inPts.push_back(Point2f(startPt.x,startPt.y));
        inPts.push_back(Point2f(endPt.x,endPt.y));
        SubdivideEdgesToSurfaceGC(inPts, pts, false, coordAdapter, 0.001);

        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3f &pt = pts[ii];
            float len = (pts[ii+1]-pt).norm();
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;        
        }
    }
}

@implementation MaplyBaseInteractionLayer

- (id)initWithView:(WhirlyKitView *)inVisualView
{
    self = [super init];
    if (!self)
        return nil;
    visualView = inVisualView;
    pthread_mutex_init(&selectLock, NULL);
    pthread_mutex_init(&imageLock, NULL);
    pthread_mutex_init(&userLock, NULL);
    
    return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&selectLock);
    pthread_mutex_destroy(&imageLock);
    pthread_mutex_destroy(&userLock);
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = (WhirlyGlobe::GlobeScene *)inScene;
    userObjects = [NSMutableArray array];    
}

- (void)shutdown
{
    layerThread = nil;
    scene = NULL;
    imageTextures.clear();
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

// Add an image to the cache, or find an existing one
// Called in the layer thread
- (SimpleIdentity)addImage:(UIImage *)image mode:(MaplyThreadMode)threadMode
{
    SimpleIdentity texID = EmptyIdentity;
    
    pthread_mutex_lock(&imageLock);
    
    // Look for an existing one
    MaplyImageTextureSet::iterator it = imageTextures.find(MaplyImageTexture(image));
    if (it != imageTextures.end())
    {
        // Increment the reference count
        MaplyImageTexture copyTex(*it);
        copyTex.refCount++;
        imageTextures.erase(it);
        imageTextures.insert(copyTex);
        
        texID = copyTex.texID;
    }

    if (texID == EmptyIdentity)
    {
        // Add it and download it
        Texture *tex = new Texture("MaplyBaseInteraction",image,true);
        
        ChangeSet changes;
        changes.push_back(new AddTextureReq(tex));
        switch (threadMode)
        {
            case MaplyThreadCurrent:
                scene->addChangeRequests(changes);
                break;
            case MaplyThreadAny:
                [layerThread addChangeRequests:changes];
                break;
        }
        
        // Add to our cache
        MaplyImageTexture newTex(image,tex->getId());
        newTex.refCount = 1;
        imageTextures.insert(newTex);
        texID = newTex.texID;
    }

    pthread_mutex_unlock(&imageLock);
    
    return texID;
}

// Remove an image for the cache, or just decrement its reference count
- (void)removeImage:(UIImage *)image
{
    pthread_mutex_lock(&imageLock);
    
    // Look for an existing one
    MaplyImageTextureSet::iterator it = imageTextures.find(MaplyImageTexture(image));
    if (it != imageTextures.end())
    {
        // Decrement the reference count
        if (it->refCount > 1)
        {
            MaplyImageTexture copyTex(*it);
            copyTex.refCount--;
            imageTextures.insert(copyTex);
        } else {
            // Note: This time is a hack.  Should look at the fade out.
            [self performSelector:@selector(delayedRemoveTexture:) withObject:@(it->texID) afterDelay:1.0];
            imageTextures.erase(it);
        }
    }
    
    pthread_mutex_unlock(&imageLock);
}

// Remove the given Texture ID after a delay
// Note: This is a hack to work around fade problems
- (void)delayedRemoveTexture:(NSNumber *)texID
{
    [layerThread addChangeRequest:(new RemTextureReq([texID integerValue]))];
}

// We flush out changes in different ways depending on the thread mode
- (void)flushChanges:(ChangeSet &)changes mode:(MaplyThreadMode)threadMode
{
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            scene->addChangeRequests(changes);
            break;
        case MaplyThreadAny:
            [layerThread addChangeRequests:changes];
            break;
    }
}

// We can refer to shaders by ID or by name.  Figure that out.
- (NSDictionary *)resolveShader:(NSDictionary *)inDesc
{
    NSObject *shader = inDesc[kMaplyShader];
    if (shader)
    {
        // Translate the shader into an ID
        if ([shader isKindOfClass:[NSString class]])
        {
            NSString *shaderName = (NSString *)shader;
            NSMutableDictionary *newDesc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
            SimpleIdentity shaderID = scene->getProgramIDBySceneName([shaderName cStringUsingEncoding:NSASCIIStringEncoding]);
            if (shaderID == EmptyIdentity)
                [newDesc removeObjectForKey:@"shader"];
            else
                newDesc[kMaplyShader] = @(shaderID);

            return newDesc;
        }
    }
    
    return inDesc;
}

// Actually add the markers.
// Called in an unknown thread
- (void)addScreenMarkersRun:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];

    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyScreenMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
        {
            texID = [self addImage:marker.image mode:threadMode];
            compObj.images.insert(marker.image);
        }
        if (texID != EmptyIdentity)
            wgMarker.texIDs.push_back(texID);
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        if (marker.selectable)
        {
            wgMarker.isSelectable = true;
            wgMarker.selectID = Identifiable::genId();
        }
        wgMarker.layoutImportance = marker.layoutImportance;
        
        [wgMarkers addObject:wgMarker];
        
        if (marker.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgMarker.selectID);
        }
    }
    
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    
    if (markerManager)
    {
        // Set up a description and create the markers in the marker layer
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
        [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
        ChangeSet changes;
        SimpleIdentity markerID = markerManager->addMarkers(wgMarkers, desc, changes);
        if (markerID != EmptyIdentity)
            compObj.markerIDs.insert(markerID);
        [self flushChanges:changes mode:threadMode];
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Called in the main thread.
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[markers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addScreenMarkersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addScreenMarkersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the markers.
// Called in an unknown thread.
- (void)addMarkersRun:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];

    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
        {
            texID = [self addImage:marker.image mode:threadMode];
            compObj.images.insert(marker.image);
        }
        if (texID != EmptyIdentity)
            wgMarker.texIDs.push_back(texID);
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        if (marker.selectable)
        {
            wgMarker.isSelectable = true;
            wgMarker.selectID = Identifiable::genId();
        }
        
        [wgMarkers addObject:wgMarker];
        
        if (marker.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgMarker.selectID);
        }
    }
    
    // Set up a description and create the markers in the marker layer
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    if (markerManager)
    {
        ChangeSet changes;
        SimpleIdentity markerID = markerManager->addMarkers(wgMarkers, inDesc, changes);
        if (markerID != EmptyIdentity)
            compObj.markerIDs.insert(markerID);
        [self flushChanges:changes mode:threadMode];
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[markers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addMarkersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addMarkersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the labels.
// Called in an unknown thread.
- (void)addScreenLabelsRun:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];

    // Convert to WG screen labels
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.rotation = label.rotation;
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage) {
            texID = [self addImage:label.iconImage mode:threadMode];
            compObj.images.insert(label.iconImage);
        }
        wgLabel.iconTexture = texID;
        wgLabel.iconSize = label.iconSize;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
        if (label.layoutImportance != MAXFLOAT)
        {
            [desc setObject:@(YES) forKey:@"layout"];
            [desc setObject:@(label.layoutImportance) forKey:@"layoutImportance"];
            [desc setObject:@(label.layoutPlacement) forKey:@"layoutPlacement"];
        }
        wgLabel.screenOffset = label.offset;
        if (label.selectable)
        {
            wgLabel.isSelectable = true;
            wgLabel.selectID = Identifiable::genId();
        }
        if ([desc count] > 0)
            wgLabel.desc = desc;
        
        [wgLabels addObject:wgLabel];
        
        if (label.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgLabel.selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        // Set up a description and create the markers in the marker layer
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
        [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
        ChangeSet changes;
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, desc, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj.labelIDs.insert(labelID);
    }

    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[labels, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addScreenLabelsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addScreenLabelsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the labels.
// Called in an unknown thread.
- (void)addLabelsRun:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];

    // Convert to WG labels
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage) {
            texID = [self addImage:label.iconImage mode:threadMode];
            compObj.images.insert(label.iconImage);
        }
        wgLabel.iconTexture = texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
        if (label.selectable)
        {
            wgLabel.isSelectable = true;
            wgLabel.selectID = Identifiable::genId();
        }
        switch (label.justify)
        {
            case MaplyLabelJustifyLeft:
                [desc setObject:@"left" forKey:@"justify"];
                break;
            case MaplyLabelJustiyMiddle:
                [desc setObject:@"middle" forKey:@"justify"];
                break;
            case MaplyLabelJustifyRight:
                [desc setObject:@"right" forKey:@"justify"];
                break;
        }
        wgLabel.desc = desc;
        
        [wgLabels addObject:wgLabel];
        
        if (label.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgLabel.selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    
    if (labelManager)
    {
        ChangeSet changes;
        // Set up a description and create the markers in the marker layer
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, inDesc, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj.labelIDs.insert(labelID);
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[labels, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addLabelsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addLabelsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the vectors.
// Called in an unknown.
- (void)addVectorsRun:(NSArray *)argArray
{
    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:4] intValue];
    
    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];

    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        // Maybe need to make a copy if we're going to sample
        if (inDesc[kMaplySubdivEpsilon])
        {
            float eps = [inDesc[kMaplySubdivEpsilon] floatValue];
            NSString *subdivType = vecObj.attributes[kMaplySubdivType];
            bool greatCircle = ![subdivType compare:kMaplySubdivGreatCircle];
            MaplyVectorObject *newVecObj = [vecObj deepCopy];
            if (greatCircle)
                [newVecObj subdivideToGlobeGreatCircle:eps];
            else
                [newVecObj subdivideToGlobe:eps];

            shapes.insert(newVecObj.shapes.begin(),newVecObj.shapes.end());
        } else
            // We'll just reference it
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    if (makeVisible)
    {
        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
        
        if (vectorManager)
        {
            ChangeSet changes;
            SimpleIdentity vecID = vectorManager->addVectors(&shapes, inDesc, changes);
            [self flushChanges:changes mode:threadMode];
            if (vecID != EmptyIdentity)
                compObj.vectorIDs.insert(vecID);
        }
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addVectorsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addVectorsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Add vectors that we'll only use for selection
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = false;
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:NO], @(MaplyThreadCurrent)];
    [self addVectorsRun:argArray];
    
    return compObj;
}

// Actually do the vector change
// Called in the layer thread
- (void)changeVectorRun:(NSArray *)argArray
{
    MaplyComponentObject *vecObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];
    
    @synchronized(vecObj)
    {
        bool isHere = false;
        pthread_mutex_lock(&userLock);
        isHere = [userObjects containsObject:vecObj];
        pthread_mutex_unlock(&userLock);
        
        if (isHere)
            return;

        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);

        if (vectorManager)
        {
            ChangeSet changes;
            for (SimpleIDSet::iterator it = vecObj.vectorIDs.begin();
                 it != vecObj.vectorIDs.end(); ++it)
                vectorManager->changeVectors(*it, desc, changes);
            [self flushChanges:changes mode:threadMode];
        }
    }
}

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (!vecObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = @[vecObj, desc, @(threadMode)];
    
    // If the object is under construction, toss this over to the layer thread
    if (vecObj.underConstruction)
        threadMode = MaplyThreadAny;
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self changeVectorRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(changeVectorRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

// Called in the layer thread
- (void)addShapesRun:(NSArray *)argArray
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    NSArray *shapes = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];

    // Need to convert shapes to the form the API is expecting
    NSMutableArray *ourShapes = [NSMutableArray array];
    for (NSObject *shape in shapes)
    {
        if ([shape isKindOfClass:[MaplyShapeCircle class]])
        {
            MaplyShapeCircle *circle = (MaplyShapeCircle *)shape;
            WhirlyKitCircle *newCircle = [[WhirlyKitCircle alloc] init];
            newCircle.loc.lon() = circle.center.x;
            newCircle.loc.lat() = circle.center.y;
            newCircle.radius = circle.radius;
            newCircle.height = circle.height;
            if (circle.color)
            {
                newCircle.useColor = true;
                RGBAColor color = [circle.color asRGBAColor];
                newCircle.color = color;
            }
            [ourShapes addObject:newCircle];
        } else if ([shape isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)shape;
            WhirlyKitSphere *newSphere = [[WhirlyKitSphere alloc] init];
            newSphere.loc.lon() = sphere.center.x;
            newSphere.loc.lat() = sphere.center.y;
            newSphere.radius = sphere.radius;
            newSphere.height = sphere.height;
            if (sphere.color)
            {
                newSphere.useColor = true;
                RGBAColor color = [sphere.color asRGBAColor];
                newSphere.color = color;
            }
            if (sphere.selectable)
            {
                newSphere.isSelectable = true;
                newSphere.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newSphere.selectID,sphere));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newSphere.selectID);
            }
            [ourShapes addObject:newSphere];
        } else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)shape;
            WhirlyKitCylinder *newCyl = [[WhirlyKitCylinder alloc] init];
            newCyl.loc.lon() = cyl.baseCenter.x;
            newCyl.loc.lat() = cyl.baseCenter.y;
            newCyl.baseHeight = cyl.baseHeight;
            newCyl.radius = cyl.radius;
            newCyl.height = cyl.height;
            if (cyl.color)
            {
                newCyl.useColor = true;
                RGBAColor color = [cyl.color asRGBAColor];
                newCyl.color = color;
            }
            if (cyl.selectable)
            {
                newCyl.isSelectable = true;
                newCyl.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newCyl.selectID,cyl));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newCyl.selectID);
            }
            [ourShapes addObject:newCyl];
        } else if ([shape isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)shape;
            WhirlyKitShapeLinear *lin = [[WhirlyKitShapeLinear alloc] init];
            SampleGreatCircle(gc.startPt,gc.endPt,gc.height,lin.pts,visualView.coordAdapter);
            lin.lineWidth = gc.lineWidth;
            if (gc.color)
            {
                lin.useColor = true;
                RGBAColor color = [gc.color asRGBAColor];
                lin.color = color;
            }
            [ourShapes addObject:lin];
        } else if ([shape isKindOfClass:[MaplyShapeLinear class]])
        {
            MaplyShapeLinear *lin = (MaplyShapeLinear *)shape;
            WhirlyKitShapeLinear *newLin = [[WhirlyKitShapeLinear alloc] init];
            MaplyCoordinate3d *coords = NULL;
            int numCoords = [lin getCoords:&coords];
            for (unsigned int ii=0;ii<numCoords;ii++)
            {
                MaplyCoordinate3d &coord = coords[ii];
                Point3f pt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(coord.x,coord.y)));
                if (coordAdapter->isFlat())
                    pt.z() = coord.z;
                else
                    pt *= (1.0+coord.z);
                newLin.pts.push_back(pt);
            }
            newLin.lineWidth = lin.lineWidth;
            if (lin.color)
            {
                newLin.useColor = true;
                RGBAColor color = [lin.color asRGBAColor];
                newLin.color = color;
            }
            [ourShapes addObject:newLin];
        }
    }
    
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    if (shapeManager)
    {
        ChangeSet changes;
        SimpleIdentity shapeID = shapeManager->addShapes(ourShapes, inDesc, changes);
        if (shapeID != EmptyIdentity)
            compObj.shapeIDs.insert(shapeID);
        [self flushChanges:changes mode:threadMode];
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[shapes, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addShapesRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addShapesRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Called in the layer thread
- (void)addStickersRun:(NSArray *)argArray
{
    NSArray *stickers = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // Might be a custom shader on these
    inDesc = [self resolveShader:inDesc];

    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    
    for (MaplySticker *sticker in stickers)
    {
        SimpleIdentity texId = EmptyIdentity;
        if (sticker.image) {
            texId = [self addImage:sticker.image mode:threadMode];
            compObj.images.insert(sticker.image);
        }
        WhirlyKitSphericalChunk *chunk = [[WhirlyKitSphericalChunk alloc] init];
        GeoMbr geoMbr = GeoMbr(GeoCoord(sticker.ll.x,sticker.ll.y), GeoCoord(sticker.ur.x,sticker.ur.y));
        chunk.mbr = geoMbr;
        chunk.texId = texId;
        chunk.drawOffset = [inDesc[@"drawOffset"] floatValue];
        chunk.drawPriority = [inDesc[@"drawPriority"] floatValue];
        chunk.sampleX = [inDesc[@"sampleX"] intValue];
        chunk.sampleY = [inDesc[@"sampleY"] intValue];
        NSNumber *bufRead = inDesc[kMaplyZBufferRead];
        if (bufRead)
            chunk.readZBuffer = [bufRead boolValue];
        NSNumber *bufWrite = inDesc[kMaplyZBufferWrite];
        if (bufWrite)
            chunk.writeZBuffer = [bufWrite boolValue];
        chunk.rotation = sticker.rotation;
        if (chunkManager)
        {
            ChangeSet changes;
            SimpleIdentity chunkID = chunkManager->addChunk(chunk, false, true, changes);
            if (chunkID != EmptyIdentity)
                compObj.chunkIDs.insert(chunkID);
            [self flushChanges:changes mode:threadMode];
        }
    }
    
    pthread_mutex_lock(&userLock);
    [userObjects addObject:compObj];
    compObj.underConstruction = false;
    pthread_mutex_unlock(&userLock);
}

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[stickers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addStickersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addStickersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the lofted polys.
// Called in the layer thread.
- (void)addLoftedPolysLayerThread:(NSArray *)argArray
{
//    NSArray *vectors = [argArray objectAtIndex:0];
//    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
//    compObj.vectors = vectors;
//    NSDictionary *inDesc = [argArray objectAtIndex:2];
//    NSString *key = argArray[3];
//    NSObject<WhirlyKitLoftedPolyCache> *cache = argArray[4];
//    
//    ShapeSet shapes;
//    for (MaplyVectorObject *vecObj in vectors)
//    {
//        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
//    }
//    
//    SimpleIdentity loftID = [_loftLayer addLoftedPolys:&shapes desc:inDesc cacheName:key cacheHandler:cache];
//    compObj.loftIDs.insert(loftID);
//    compObj.isSelectable = false;
//    
//    [userObjects addObject:compObj];
}

// Add lofted polys
// Note: Need to handle the thread mode
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc key:(NSString *)key cache:(NSObject<WhirlyKitLoftedPolyCache> *)cache mode:(MaplyThreadMode)threadMode
{
    return nil;
    
//    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
//    
//    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], (key ? key : [NSNull null]), (cache ? cache : [NSNull null])];
//    [self performSelector:@selector(addLoftedPolysLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
//    
//    return compObj;
}


// Remove the object, but do it on the layer thread
- (void)removeObjectRun:(NSArray *)argArray
{
    NSArray *userObjs = argArray[0];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:1] intValue];
    
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);

    ChangeSet changes;
        
    // First, let's make sure we're representing it
    for (MaplyComponentObject *userObj in userObjs)
    {
        bool isHere = false;
        pthread_mutex_lock(&userLock);
        isHere = [userObjects containsObject:userObj];
        pthread_mutex_unlock(&userLock);
        if (isHere)
        {
            @synchronized(userObj)
            {
                // Get rid of the various layer objects
                if (markerManager)
                    markerManager->removeMarkers(userObj.markerIDs, changes);
                if (labelManager)
                    labelManager->removeLabels(userObj.labelIDs, changes);
                if (vectorManager)
                    vectorManager->removeVectors(userObj.vectorIDs, changes);
                if (shapeManager)
                    shapeManager->removeShapes(userObj.shapeIDs, changes);
//                for (SimpleIDSet::iterator it = userObj.loftIDs.begin();
//                     it != userObj.loftIDs.end(); ++it)
//                    [_loftLayer removeLoftedPoly:*it];
                if (chunkManager)
                    chunkManager->removeChunks(userObj.chunkIDs, changes);
                // And associated textures
                for (std::set<UIImage *>::iterator it = userObj.images.begin(); it != userObj.images.end(); ++it)
                    [self removeImage:*it];
                // And any references to selection objects
                pthread_mutex_lock(&selectLock);
                for (SimpleIDSet::iterator it = userObj.selectIDs.begin();
                     it != userObj.selectIDs.end(); ++it)
                {
                    SelectObjectSet::iterator sit = selectObjectSet.find(SelectObject(*it));
                    if (sit != selectObjectSet.end())
                        selectObjectSet.erase(sit);
                }
                pthread_mutex_unlock(&selectLock);
                
            }
            
            pthread_mutex_lock(&userLock);
            [userObjects removeObject:userObj];
            pthread_mutex_unlock(&userLock);
        } else {
//            NSLog(@"Tried to delete object that doesn't exist");
        }
    }
    
    [self flushChanges:changes mode:threadMode];
}

// Remove a group of objects at once
- (void)removeObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    NSArray *argArray = @[userObjs, @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self removeObjectRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(removeObjectRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

- (void)enableObjectsRun:(NSArray *)argArray
{
    NSArray *theObjs = argArray[0];
    bool enable = [argArray[1] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];

    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);

    ChangeSet changes;
    for (MaplyComponentObject *compObj in theObjs)
    {
        bool isHere = false;
        pthread_mutex_lock(&userLock);
        isHere = [userObjects containsObject:compObj];
        pthread_mutex_unlock(&userLock);

        if (isHere)
        {
            // Enable vectors
            for (SimpleIDSet::iterator it = compObj.vectorIDs.begin();
                 it != compObj.vectorIDs.end(); ++it)
                vectorManager->enableVector(*it, enable, changes);
            // Note: Need to implement the rest of the enable/disables
        }
    }

    [self flushChanges:changes mode:threadMode];
}

// Enable objects
- (void)enableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    NSArray *argArray = @[userObjs, @(true), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self enableObjectsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(enableObjectsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }    
}

// Disable objects
- (void)disableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    NSArray *argArray = @[userObjs, @(false), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self enableObjectsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(enableObjectsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}


// Search for a point inside any of our vector objects
// Runs in layer thread
- (NSObject *)findVectorInPoint:(Point2f)pt
{
    NSObject *selObj = nil;
    
    for (MaplyComponentObject *userObj in userObjects)
    {
        if (userObj.vectors && userObj.isSelectable)
        {
            for (MaplyVectorObject *vecObj in userObj.vectors)
            {
                if (vecObj.selectable)
                {
                    // Note: Take visibility into account too
                    MaplyCoordinate coord;
                    coord.x = pt.x();
                    coord.y = pt.y();
                    if ([vecObj pointInAreal:coord])
                    {
                        selObj = vecObj;
                        break;
                    }
                }
            }
            
            if (selObj)
                break;
        }
    }
    
    return selObj;
}

- (NSObject *)getSelectableObject:(WhirlyKit::SimpleIdentity)objId
{
    NSObject *ret = nil;
    
    pthread_mutex_lock(&selectLock);
    SelectObjectSet::iterator sit = selectObjectSet.find(SelectObject(objId));
    if (sit != selectObjectSet.end())
        ret = sit->obj;

    pthread_mutex_unlock(&selectLock);
    
    return ret;
}


@end
