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

@synthesize markerLayer;
@synthesize labelLayer;
@synthesize vectorLayer;
@synthesize shapeLayer;
@synthesize chunkLayer;
@synthesize loftLayer;
@synthesize selectLayer;
@synthesize glView;

- (id)initWithView:(WhirlyKitView *)inVisualView
{
    self = [super init];
    if (!self)
        return nil;
    visualView = inVisualView;
    
    return self;
}

- (void)dealloc
{
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
- (SimpleIdentity)addImage:(UIImage *)image
{
    // Look for an existing one
    MaplyImageTextureSet::iterator it = imageTextures.find(MaplyImageTexture(image));
    if (it != imageTextures.end())
    {
        // Increment the reference count
        MaplyImageTexture copyTex(*it);
        copyTex.refCount++;
        imageTextures.erase(it);
        imageTextures.insert(copyTex);
        return copyTex.texID;
    }
    
    // Add it and download it
    [EAGLContext setCurrentContext:layerThread.glContext];
    Texture *tex = new Texture(image,true);
    tex->createInGL(YES, scene->getMemManager());
    scene->addChangeRequest(new AddTextureReq(tex));
    
    // Add to our cache
    MaplyImageTexture newTex(image,tex->getId());
    newTex.refCount = 1;
    imageTextures.insert(newTex);
    
    return newTex.texID;
}

// Remove an image for the cache, or just decrement its reference count
- (void)removeImage:(UIImage *)image
{
    // Look for an existing one
    MaplyImageTextureSet::iterator it = imageTextures.find(MaplyImageTexture(image));
    if (it != imageTextures.end())
    {
        // Decrement the reference count
        if (it->refCount > 1)
        {
            MaplyImageTexture copyTex(*it);
            copyTex.refCount--;
            imageTextures.erase(it);
            imageTextures.insert(copyTex);
        } else {
            // Note: This time is a hack.  Should look at the fade out.
            [self performSelector:@selector(delayedRemoveTexture:) withObject:@(it->texID) afterDelay:1.0];
            imageTextures.erase(it);
        }
    }
}

// Remove the given Texture ID after a delay
// Note: This is a hack to work around fade problems
- (void)delayedRemoveTexture:(NSNumber *)texID
{
    scene->addChangeRequest(new RemTextureReq([texID integerValue]));
}

// Actually add the markers.
// Called in the layer thread.
- (void)addScreenMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyScreenMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
        {
            texID = [self addImage:marker.image];
            compObj.images.push_back(marker.image);
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
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));

        if (wgMarker.selectID != EmptyIdentity)
            compObj.selectIDs.insert(wgMarker.selectID);
    }
    
    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:desc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Called in the main thread.
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addScreenMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the markers.
// Called in the layer thread.
- (void)addMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
        {
            texID = [self addImage:marker.image];
            compObj.images.push_back(marker.image);
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
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));

        if (wgMarker.selectID != EmptyIdentity)
            compObj.selectIDs.insert(wgMarker.selectID);
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:inDesc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addScreenLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage) {
            texID = [self addImage:label.iconImage];
            compObj.images.push_back(label.iconImage);
        }
        wgLabel.iconTexture = texID;
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
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));

        if (wgLabel.selectID != EmptyIdentity)
            compObj.selectIDs.insert(wgLabel.selectID);
    }
    
    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:desc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addScreenLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage) {
            texID = [self addImage:label.iconImage];
            compObj.images.push_back(label.iconImage);
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
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
        
        if (wgLabel.selectID != EmptyIdentity)
            compObj.selectIDs.insert(wgLabel.selectID);
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:inDesc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the vectors.
// Called in the layer thread.
- (void)addVectorsLayerThread:(NSArray *)argArray
{
    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    if (makeVisible)
    {
        SimpleIdentity vecID = [vectorLayer addVectors:&shapes desc:inDesc];
        compObj.vectorIDs.insert(vecID);
    }
    
    [userObjects addObject:compObj];
}

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], nil];
    [self performSelector:@selector(addVectorsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Add vectors that we'll only use for selection
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:NO], nil];
    [self performSelector:@selector(addVectorsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually do the vector change
// Called in the layer thread
- (void)changeVectorLayerThread:(NSArray *)argArray
{
    MaplyComponentObject *vecObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    
    for (SimpleIDSet::iterator it = vecObj.vectorIDs.begin();
         it != vecObj.vectorIDs.end(); ++it)
        [vectorLayer changeVector:*it desc:desc];
}

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc
{
    if (!vecObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = [NSArray arrayWithObjects:vecObj, desc, nil];
    
    [self performSelector:@selector(changeVectorLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
}

// Called in the layer thread
- (void)addShapesLayerThread:(NSArray *)argArray
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    NSArray *shapes = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
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
    
    SimpleIdentity shapeID = [shapeLayer addShapes:ourShapes desc:inDesc];
    compObj.shapeIDs.insert(shapeID);
    
    [userObjects addObject:compObj];
}

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:shapes, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addShapesLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Called in the layer thread
- (void)addStickersLayerThread:(NSArray *)argArray
{
    NSArray *stickers = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    
    for (MaplySticker *sticker in stickers)
    {
        SimpleIdentity texId = EmptyIdentity;
        if (sticker.image) {
            texId = [self addImage:sticker.image];
            compObj.images.push_back(sticker.image);
        }
        WhirlyKitSphericalChunk *chunk = [[WhirlyKitSphericalChunk alloc] init];
        GeoMbr geoMbr = GeoMbr(GeoCoord(sticker.ll.x,sticker.ll.y), GeoCoord(sticker.ur.x,sticker.ur.y));
        chunk.mbr = geoMbr;
        chunk.texId = texId;
        chunk.drawOffset = [inDesc[@"drawOffset"] floatValue];
        chunk.drawPriority = [inDesc[@"drawPriority"] floatValue];
        chunk.sampleX = [inDesc[@"sampleX"] intValue];
        chunk.sampleY = [inDesc[@"sampleY"] intValue];
        chunk.rotation = sticker.rotation;
        SimpleIdentity chunkId = [chunkLayer addChunk:chunk enable:true];
        compObj.chunkIDs.insert(chunkId);
    }
    
    [userObjects addObject:compObj];
}

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = @[stickers, compObj, [NSDictionary dictionaryWithDictionary:desc]];
    [self performSelector:@selector(addStickersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the lofted polys.
// Called in the layer thread.
- (void)addLoftedPolysLayerThread:(NSArray *)argArray
{
    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    NSString *key = argArray[3];
    NSObject<WhirlyKitLoftedPolyCache> *cache = argArray[4];
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    SimpleIdentity loftID = [loftLayer addLoftedPolys:&shapes desc:inDesc cacheName:key cacheHandler:cache];
    compObj.loftIDs.insert(loftID);
    compObj.isSelectable = false;
    
    [userObjects addObject:compObj];
}

// Add lofted polys
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc key:(NSString *)key cache:(NSObject<WhirlyKitLoftedPolyCache> *)cache
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] init];
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], (key ? key : [NSNull null]), (cache ? cache : [NSNull null])];
    [self performSelector:@selector(addLoftedPolysLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}


// Remove the object, but do it on the layer thread
- (void)removeObjectLayerThread:(NSArray *)userObjs
{
    // First, let's make sure we're representing it
    for (MaplyComponentObject *userObj in userObjs)
    {
        if ([userObjects containsObject:userObj])
        {
            // Get rid of the various layer objects
            for (SimpleIDSet::iterator it = userObj.markerIDs.begin();
                 it != userObj.markerIDs.end(); ++it)
                [markerLayer removeMarkers:*it];
            for (SimpleIDSet::iterator it = userObj.labelIDs.begin();
                 it != userObj.labelIDs.end(); ++it)
                [labelLayer removeLabel:*it];
            for (SimpleIDSet::iterator it = userObj.vectorIDs.begin();
                 it != userObj.vectorIDs.end(); ++it)
                [vectorLayer removeVector:*it];
            for (SimpleIDSet::iterator it = userObj.shapeIDs.begin();
                 it != userObj.shapeIDs.end(); ++it)
                [shapeLayer removeShapes:*it];
            for (SimpleIDSet::iterator it = userObj.loftIDs.begin();
                 it != userObj.loftIDs.end(); ++it)
                [loftLayer removeLoftedPoly:*it];
            for (SimpleIDSet::iterator it = userObj.chunkIDs.begin();
                 it != userObj.chunkIDs.end(); ++it)
                [chunkLayer removeChunk:*it];
            // And associated textures
            for (unsigned int ii=0;ii<userObj.images.size();ii++)
                [self removeImage:userObj.images.at(ii)];
            
            // And any references to selection objects
            for (SimpleIDSet::iterator it = userObj.selectIDs.begin();
                 it != userObj.selectIDs.end(); ++it)
            {
                SelectObjectSet::iterator sit = selectObjectSet.find(SelectObject(*it));
                if (sit != selectObjectSet.end())
                    selectObjectSet.erase(sit);
            }
            
            [userObjects removeObject:userObj];
        } else {
//            NSLog(@"Tried to delete object that doesn't exist");
        }
    }
}

// Remove data associated with a user object
- (void)removeObject:(MaplyComponentObject *)userObj
{
    if (userObj != nil)
        [self performSelector:@selector(removeObjectLayerThread:) onThread:layerThread withObject:[NSArray arrayWithObject:userObj] waitUntilDone:NO];
}

// Remove a group of objects at once
- (void)removeObjects:(NSArray *)userObjs
{
    [self performSelector:@selector(removeObjectLayerThread:) onThread:layerThread withObject:userObjs waitUntilDone:NO];
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

@end
