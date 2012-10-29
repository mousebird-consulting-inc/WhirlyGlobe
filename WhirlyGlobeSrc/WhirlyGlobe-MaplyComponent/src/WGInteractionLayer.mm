/*
 *  WGInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "WGInteractionLayer_private.h"
#import "MaplyScreenMarker.h"
#import "MaplyMarker.h"
#import "MaplyScreenLabel.h"
#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShape.h"
#import "WGCoordinate.h"
#import "ImageTexture_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircle(MaplyCoordinate startPt,MaplyCoordinate endPt,float height,std::vector<Point3f> &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
{
    Point3f p0 = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(startPt.x,startPt.y)));
    Point3f p1 = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(endPt.x,endPt.y)));
    
    // Note: Dumb approach.  Switch to an adaptive sampling
    int numSamples = 300;
    for (unsigned int ii=0;ii<=numSamples;ii++)
    {
        // Sample the point
        float t = (ii/(float)numSamples);
        Point3f pt = (p1-p0)*t + p0;
        // This puts us on the surface of the sphere
        pt.normalize();
        
        // Parabolic curve
        float b = 4*height;
        float a = -b;
        float thisHeight = a*(t*t) + b*t;
        
        pt *= 1.0+thisHeight;
        pts.push_back(pt);
    }
}

@implementation WGInteractionLayer
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyGlobe::GlobeScene *scene;
    ImageTextureSet imageTextures;
    AnimateViewMomentum *autoSpinner;
}

@synthesize markerLayer;
@synthesize labelLayer;
@synthesize vectorLayer;
@synthesize shapeLayer;
@synthesize selectLayer;
@synthesize glView;
@synthesize viewController;

// Initialize with the globeView
-(id)initWithGlobeView:(WhirlyGlobeView *)inGlobeView
{
    self = [super init];
    if (!self)
        return nil;
    globeView = inGlobeView;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = (WhirlyGlobe::GlobeScene *)inScene;
    userObjects = [NSMutableArray array];
    
    // Run the auto spin every so often
    [self performSelector:@selector(processAutoSpin:) withObject:nil afterDelay:1.0];
}

- (void)dealloc
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)shutdown
{
    layerThread = nil;
    scene = NULL;
    imageTextures.clear();
}

// Add an image to the cache, or find an existing one
// Called in the layer thread
- (SimpleIdentity)addImage:(UIImage *)image
{
    // Look for an existing one
    ImageTextureSet::iterator it = imageTextures.find(ImageTexture(image));
    if (it != imageTextures.end())
        return it->texID;
    
    // Add it and download it
    Texture *tex = new Texture(image);
//    tex->createInGL(YES, scene->getMemManager());
    scene->addChangeRequest(new AddTextureReq(tex));
    
    // Add to our cache
    ImageTexture newTex(image,tex->getId());
    imageTextures.insert(newTex);
    
    return newTex.texID;
}

- (void)setAutoRotateInterval:(float)inAutoRotateInterval degrees:(float)inAutoRotateDegrees
{
    autoRotateInterval = inAutoRotateInterval;
    autoRotateDegrees = inAutoRotateDegrees;
    if (autoSpinner)
    {
        if (autoRotateInterval == 0.0 || autoRotateDegrees == 0)
        {
            [globeView cancelAnimation];
            autoSpinner = nil;
        } else
            // Update the spin
            autoSpinner.velocity = autoRotateDegrees / 180.0 * M_PI;
    }
}

// Try to auto-spin every so often
-(void)processAutoSpin:(id)sender
{
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    
    if (autoSpinner && globeView.delegate != autoSpinner)
        autoSpinner = nil;
    
    if (autoRotateInterval > 0.0 && !autoSpinner)
    {
        if (now - globeView.lastChangedTime > autoRotateInterval &&
            now - lastTouched > autoRotateInterval)
        {
            float anglePerSec = autoRotateDegrees / 180.0 * M_PI;
            
            // Keep going in that direction
            Vector3f upVector(0,0,1);
            autoSpinner = [[AnimateViewMomentum alloc] initWithView:globeView velocity:anglePerSec accel:0.0 axis:upVector];
            globeView.delegate = autoSpinner;
        }
    }
    
    [self performSelector:@selector(processAutoSpin:) withObject:nil afterDelay:1.0];
}

// Actually add the markers.
// Called in the layer thread.
- (void)addScreenMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (WGScreenMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
            texID = [self addImage:marker.image];
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
    }

    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:desc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Called in the main thread.
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];    
    [self performSelector:@selector(addScreenMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the markers.
// Called in the layer thread.
- (void)addMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (WGMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
            texID = [self addImage:marker.image];
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
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:inDesc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Add 3D markers
- (WGComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addScreenLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (WGScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage)
            texID = [self addImage:label.iconImage];
        wgLabel.iconTexture = texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
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
    }
    
    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:desc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add screen space (2D) labels
- (WGComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];    
    [self performSelector:@selector(addScreenLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (WGLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage)
            texID = [self addImage:label.iconImage];
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
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:inDesc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add 3D labels
- (WGComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];    
    [self performSelector:@selector(addLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the vectors.
// Called in the layer thread.
- (void)addVectorsLayerThread:(NSArray *)argArray
{
    NSArray *vectors = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSDictionary *inDesc = [argArray objectAtIndex:2];    
    
    ShapeSet shapes;
    for (WGVectorObject *vecObj in vectors)
    {
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    SimpleIdentity vecID = [vectorLayer addVectors:&shapes desc:inDesc];
    compObj.vectorIDs.insert(vecID);
    
    [userObjects addObject:compObj];
}

// Add vectors
- (WGComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addVectorsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually do the vector change
// Called in the layer thread
- (void)changeVectorLayerThread:(NSArray *)argArray
{
    WGComponentObject *vecObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];

    for (SimpleIDSet::iterator it = vecObj.vectorIDs.begin();
         it != vecObj.vectorIDs.end(); ++it)
        [vectorLayer changeVector:*it desc:desc];
}

// Change vector representation
- (void)changeVectors:(WGComponentObject *)vecObj desc:(NSDictionary *)desc
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
    WGComponentObject *compObj = [argArray objectAtIndex:1];
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
            [ourShapes addObject:newCircle];
        } else if ([shape isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)shape;
            WhirlyKitSphere *newSphere = [[WhirlyKitSphere alloc] init];
            newSphere.loc.lon() = sphere.center.x;
            newSphere.loc.lat() = sphere.center.y;
            newSphere.radius = sphere.radius;
            newSphere.height = sphere.height;
            [ourShapes addObject:newSphere];
        } else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)shape;
            WhirlyKitCylinder *newCyl = [[WhirlyKitCylinder alloc] init];
            newCyl.loc.lon() = cyl.baseCenter.x;
            newCyl.loc.lat() = cyl.baseCenter.y;
            newCyl.radius = cyl.radius;
            newCyl.height = cyl.height;
            [ourShapes addObject:newCyl];
        } else if ([shape isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)shape;
            WhirlyKitShapeLinear *lin = [[WhirlyKitShapeLinear alloc] init];
            SampleGreatCircle(gc.startPt,gc.endPt,gc.height,lin.pts,globeView.coordAdapter);
            lin.lineWidth = gc.lineWidth;
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
                pt *= (1.0+coord.z);
                newLin.pts.push_back(pt);
            }
            newLin.lineWidth = lin.lineWidth;
            [ourShapes addObject:newLin];
        }
    }

    SimpleIdentity shapeID = [shapeLayer addShapes:ourShapes desc:inDesc];
    compObj.shapeIDs.insert(shapeID);
    
    [userObjects addObject:compObj];
}

// Add shapes
- (WGComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:shapes, compObj, [NSDictionary dictionaryWithDictionary:desc], nil];
    [self performSelector:@selector(addShapesLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Remove the object, but do it on the layer thread
- (void)removeObjectLayerThread:(NSArray *)userObjs
{
    // First, let's make sure we're representing it
    for (WGComponentObject *userObj in userObjs)
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
            
            [userObjects removeObject:userObj];
        }    
    }
}

// Remove data associated with a user object
- (void)removeObject:(WGComponentObject *)userObj
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
    
    for (WGComponentObject *userObj in userObjects)
    {
        if (userObj.vectors)
        {
            for (WGVectorObject *vecObj in userObj.vectors)
            {
                if (vecObj.selectable)
                {
                    // Note: Take visibility into account too
                    WGCoordinate coord;
                    coord.x = pt.x();
                    coord.y = pt.y();
                    if ([vecObj pointInAreal:coord])
                    {
                        selObj = vecObj;
                        break;
                    }
                }
            }
        }
    }
    
    return selObj;
}

// Do the logic for a selection
// Runs in the layer thread
- (void) userDidTapLayerThread:(WhirlyGlobeTapMessage *)msg
{
    lastTouched = CFAbsoluteTimeGetCurrent();
    if (autoSpinner)
    {
        if (globeView.delegate == autoSpinner)
        {
            autoSpinner = nil;
            globeView.delegate = nil;
        }
    }
    
    // First, we'll look for labels and markers
    SimpleIdentity selID = [selectLayer pickObject:Point2f(msg.touchLoc.x,msg.touchLoc.y) view:glView maxDist:10.0];

    NSObject *selObj;
    if (selID != EmptyIdentity)
    {       
        // Found something.  Now find the associated object
        SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
        if (it != selectObjectSet.end())
        {
            selObj = it->obj;
        }
    } else {
        // Next, try the vectors
        selObj = [self findVectorInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y())];
    }
    
    // Tell the view controller about it
    dispatch_async(dispatch_get_main_queue(),^
                   {
                       [viewController handleSelection:msg didSelect:selObj];
                   }
                   );
}

// Check for a selection
- (void) userDidTap:(WhirlyGlobeTapMessage *)msg
{
    // Pass it off to the layer thread
    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

@end