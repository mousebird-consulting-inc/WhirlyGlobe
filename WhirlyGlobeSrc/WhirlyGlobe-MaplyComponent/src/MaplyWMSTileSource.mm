/*
 *  MaplyWMSTileSource.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/25/13.
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

#import "MaplyWMSTileSource.h"
#import "DDXMLElementAdditions.h"
#import "DDXML.h"

@implementation MaplyWMSLayerBoundingBox

- (id)initWithXML:(DDXMLElement *)el
{
    self = [super init];
    _crs = [[el attributeForName:@"CRS"] stringValue];
    if (!_crs)
        _crs = [[el attributeForName:@"SRS"] stringValue];
    _minx = [[[el attributeForName:@"minx"] stringValue] doubleValue];
    _miny = [[[el attributeForName:@"miny"] stringValue] doubleValue];
    _maxx = [[[el attributeForName:@"maxx"] stringValue] doubleValue];
    _maxy = [[[el attributeForName:@"maxy"] stringValue] doubleValue];
    
    return self;
}

/// Generate the coordinate system, if we can
- (MaplyCoordinateSystem *)buildCoordinateSystem
{
    MaplyCoordinateSystem *crs = nil;
    
    // Note: This is just a hack for the moment
    if (![_crs compare:@"EPSG:4326"])
    {
        MaplyBoundingBox cbbx;
        cbbx.ll = MaplyCoordinateMakeWithDegrees(_minx, _miny);
        cbbx.ur = MaplyCoordinateMakeWithDegrees(_maxx, _maxy);
        crs = [[MaplyPlateCarree alloc] initWithBoundingBox:cbbx];
    } else if (![_crs compare:@"EPSG:3857"])
    {
        MaplySphericalMercator *crs = [[MaplySphericalMercator alloc] initWebStandard];
        MaplyCoordinate ll = MaplyCoordinateMakeWithDegrees(_minx, _miny);
        MaplyCoordinate ur = MaplyCoordinateMakeWithDegrees(_maxx, _maxy);
        [crs setBoundsLL:&ll ur:&ur];
    }
    
    return crs;
}

@end

@implementation MaplyWMSStyle

- (id)initWithXML:(DDXMLElement *)el
{
    self = [super init];
    
    _name = [[el elementForName:@"Name"] stringValue];
    _title = [[el elementForName:@"Title"] stringValue];
    
    return self;
}

@end

@implementation MaplyWMSLayer

- (id)initWithXML:(DDXMLElement *)el parent:(MaplyWMSLayer *)parent
{
    self = [super init];
    
    // Copy data from the parent first
    _name = parent.name;
    _title = parent.title;
    NSMutableArray *outCrss = [NSMutableArray array];
    if (parent.coordRefSystems)
        [outCrss addObjectsFromArray:parent.coordRefSystems];
    NSMutableArray *outBoxes = [NSMutableArray array];
    if (parent.boundingBoxes)
        [outBoxes addObjectsFromArray:parent.boundingBoxes];
    NSMutableArray *outStyles = [NSMutableArray array];
    if (parent.styles)
        [outStyles addObjectsFromArray:parent.styles];
    
    _name = [[el elementForName:@"Name"] stringValue];
    _title = [[el elementForName:@"Title"] stringValue];
    _abstract = [[el elementForName:@"Abstract"] stringValue];
    
    // Lat/lon bounding box
    _ll.x = _ll.y = _ur.x = _ur.y = 0.0;
    DDXMLElement *simpleBbox = [el elementForName:@"LatLonBoundingBox"];
    if (simpleBbox)
    {
        float llx = [[[simpleBbox attributeForName:@"minx"] stringValue] doubleValue];
        float lly = [[[simpleBbox attributeForName:@"miny"] stringValue] doubleValue];
        float urx = [[[simpleBbox attributeForName:@"maxx"] stringValue] doubleValue];
        float ury = [[[simpleBbox attributeForName:@"maxy"] stringValue] doubleValue];
        _ll = MaplyCoordinateMakeWithDegrees(llx, lly);
        _ll.x = MAX(_ll.x,-M_PI);
        _ll.y = MAX(_ll.y,-M_PI/2.0);
        _ur = MaplyCoordinateMakeWithDegrees(urx, ury);
        _ur.x = MIN(_ur.x,M_PI);
        _ur.y = MIN(_ur.y,M_PI/2.0);
    }
    
    // The various coordinate reference systems the layer supports
    NSArray *crss = [el elementsForName:@"CRS"];
    for (DDXMLElement *crs in crss)
        [outCrss addObject:[crs stringValue]];
    NSArray *srss = [el elementsForName:@"SRS"];
    for (DDXMLElement *srs in srss)
        [outCrss addObject:[srs stringValue]];
    _coordRefSystems = outCrss;
    
    // Bounding boxes for the some of the CRS'
    NSArray *boxes = [el elementsForName:@"BoundingBox"];
    for (DDXMLElement *box in boxes)
    {
        MaplyWMSLayerBoundingBox *newBox = [[MaplyWMSLayerBoundingBox alloc] initWithXML:box];
        if (newBox)
            [outBoxes addObject:newBox];
    }
    _boundingBoxes = outBoxes;
    
    // Styles
    NSArray *styles = [el elementsForName:@"Style"];
    for (DDXMLElement *style in styles)
    {
        MaplyWMSStyle *newStyle = [[MaplyWMSStyle alloc] initWithXML:style];
        if (newStyle)
            [outStyles addObject:newStyle];
    }
    _styles = outStyles;
    
    return self;
}

- (MaplyCoordinateSystem *)buildCoordSystem
{
    // See if there's one in the bounding boxes we like
    for (MaplyWMSLayerBoundingBox *bbox in _boundingBoxes)
    {
        MaplyCoordinateSystem *coordSys = [bbox buildCoordinateSystem];
        if (coordSys)
            return coordSys;
    }
    
    // Okay, nothing there so try the ones we know
    // Plate Carree (e.g. lat/lon)
    if ([_coordRefSystems containsObject:@"EPSG:4326"])
    {
        MaplyPlateCarree *crs = [[MaplyPlateCarree alloc] initFullCoverage];
        if (_ll.x != _ur.x)
            [crs setBoundsLL:&_ll ur:&_ur];
        return crs;
    }
    // Everyone's favorite spherical mercator
    if ([_coordRefSystems containsObject:@"EPSG:3857"])
    {
        MaplySphericalMercator *crs = [[MaplySphericalMercator alloc] initWebStandard];
        if (_ll.x != _ur.x)
            [crs setBoundsLL:&_ll ur:&_ur];
        return crs;
    }
    
    return nil;
}

- (MaplyWMSStyle *)findStyle:(NSString *)styleName
{
    for (MaplyWMSStyle *style in _styles)
    {
        if (![style.name compare:styleName])
            return style;
    }
    
    return nil;
}

@end

@implementation MaplyWMSCapabilities

+ (NSString *)CapabilitiesURLFor:(NSString *)baseURL
{
    return [NSString stringWithFormat:@"%@?Service=WMS&Request=GetCapabilities&Version=1.1.1",baseURL];
}

- (id)initWithXML:(DDXMLDocument *)xmlDoc
{
    self = [super init];
    
    DDXMLElement *top = [xmlDoc rootElement];
    DDXMLElement *service = [top elementForName:@"Service"];
    
//    NSArray *services = [xmlDoc nodesForXPath:@"//Service" error:&error];
//    DDXMLElement *service = [services objectAtIndex:0];
    if (service)
    {
        DDXMLElement *name = [service elementForName:@"Name"];
        _name = [name stringValue];
        DDXMLElement *title = [service elementForName:@"Title"];
        _title = [title stringValue];
    }
    
    DDXMLElement *cap = [top elementForName:@"Capability"];
    if (!cap) return nil;

    // We want the formats we can ask for
    DDXMLElement *request = [cap elementForName:@"Request"];
    DDXMLElement *getMap = [request elementForName:@"GetMap"];
    if (!getMap) return nil;
    if (getMap)
    {
        NSArray *formats = [getMap elementsForName:@"Format"];
        NSMutableArray *outFormats = [NSMutableArray array];
        for (DDXMLElement *format in formats)
            [outFormats addObject:[format stringValue]];
        _formats = outFormats;
    }
    
    // Now for the layers
    NSArray *layers = [cap elementsForName:@"Layer"];
    if (!layers)  return nil;
    NSMutableArray *outLayers = [NSMutableArray array];
    for (DDXMLElement *layer in layers)
        [self parseLayers:layer parent:nil outLayers:outLayers];
    _layers = outLayers;
    
    return self;
}

// Recursively (kill me) parse layers
- (void)parseLayers:(DDXMLElement *)layer parent:(MaplyWMSLayer *)parentLayer outLayers:(NSMutableArray *)outLayers
{
    MaplyWMSLayer *newLayer = [[MaplyWMSLayer alloc] initWithXML:layer parent:parentLayer];
    // If there are children, we keep going down
    NSArray *layers = [layer elementsForName:@"Layer"];
    if ([layers count] > 0)
    {
        for (DDXMLElement *child in layers)
            [self parseLayers:child parent:newLayer outLayers:outLayers];
    } else
        [outLayers addObject:newLayer];
}

- (MaplyWMSLayer *)findLayer:(NSString *)name
{
    for (MaplyWMSLayer *layer in _layers)
    {
        if (![layer.name compare:name])
            return layer;
    }
    
    return nil;
}

@end

@implementation MaplyWMSTileSource
{
    bool cacheInit;
}

- (id)initWithBaseURL:(NSString *)baseURL capabilities:(MaplyWMSCapabilities *)cap layer:(MaplyWMSLayer *)layer style:(MaplyWMSStyle *)style coordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom tileSize:(int)tileSize
{
    self = [super init];
    
    if (!layer.name)
        return nil;
    
    _capabilities = cap;
    _baseURL = baseURL;
    _minZoom = minZoom;
    _maxZoom = maxZoom;
    _coordSys = coordSys;
    _layer = layer;
    _style = style;
    _tileSize = 256;
    _imageType = @"image/png";
    _transparent = false;
    
    // Note: Should check the image type is there
    
    return self;
}

// Figure out the name for the tile, if it's local
- (NSString *)cacheFileForTile:(MaplyTileID)tileID
{
    // If there's a cache dir, make sure it's there
    if (!cacheInit)
    {
        if (_cacheDir)
        {
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:_cacheDir withIntermediateDirectories:YES attributes:nil error:&error];
        }
        cacheInit = true;
    }
    
    NSString *localName = [NSString stringWithFormat:@"%@/%d_%d_%d",_cacheDir,tileID.level,tileID.x,tileID.y];
    return localName;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    if (!_cacheDir)
    return false;
    
    NSString *fileName = [self cacheFileForTile:tileID];
    if ([[NSFileManager defaultManager] fileExistsAtPath:fileName])
    return true;
    
    return false;
}

/// Return the image for a given tile
- (id)imageForTile:(MaplyTileID)tileID
{
    NSData *imgData = nil;
    bool wasCached = false;
    NSString *fileName = nil;
    // Look for the image in the cache first
    if (_cacheDir)
    {
        fileName = [self cacheFileForTile:tileID];
        imgData = [NSData dataWithContentsOfFile:fileName];
        wasCached = true;
    }
    
    if (!imgData)
    {
        // SRS string for the coordinate system
        NSString *srsStr = [_coordSys getSRS];

        // Coordinates of the tile we're asking for
        MaplyCoordinate ll,ur;
        [self getBoundsLL:&ll ur:&ur];
        MaplyCoordinate tileLL,tileUR;
        int numSide = 1<<tileID.level;
        tileLL.x = tileID.x * (ur.x-ll.x)/numSide + ll.x;
        tileLL.y = tileID.y * (ur.y-ll.y)/numSide + ll.y;
        tileUR.x = (tileID.x+1) * (ur.x-ll.x)/numSide + ll.x;
        tileUR.y = (tileID.y+1) * (ur.y-ll.y)/numSide + ll.y;
        
        // Put the layer request together
        NSMutableString *layerStr = [NSMutableString string];
        [layerStr appendString:_layer.name];
        
        NSMutableString *reqStr = [NSMutableString stringWithFormat:@"%@?SERVICE=WMS&VERSION=1.1.1&REQUEST=GetMap&LAYERS=%@&STYLES=&SRS=%@&BBOX=%f,%f,%f,%f&WIDTH=%d&HEIGHT=%d&FORMAT=%@&TRANSPARENT=%@",_baseURL,layerStr,srsStr,tileLL.x,tileLL.y,tileUR.x,tileUR.y,_tileSize,_tileSize,_imageType,(_transparent ? @"true" : @"false")];
        if (_style)
            [reqStr appendFormat:@"&STYLES=%@",_style.name];
        NSString *fullReqStr = [reqStr stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
        NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullReqStr]];
        
        // Fetch the image synchronously
        NSURLResponse *resp = nil;
        NSError *error = nil;
        imgData = [NSURLConnection sendSynchronousRequest:urlReq returningResponse:&resp error:&error];
        if (error || !imgData)
        {
            NSLog(@"Failed to fetch image at: %@",reqStr);
            return nil;
        }
        
        // some wms servers will response with 200 OK, but with text error.
        if (![[resp MIMEType] hasPrefix:@"image/"])
        {
            NSLog(@"Failed to fetch image at: %@. Got mime type %@ - expected %@",
                  reqStr, [resp MIMEType], _imageType);
            return nil;
        }
        
    }
    
    // Let's also write it back out for the cache
    if (_cacheDir && !wasCached)
        [imgData writeToFile:fileName atomically:YES];
    
    return imgData;
}

// get the bounds of the most common tiling schema for the coordSys in units usable for WMS BBOX
- (void)getBoundsLL:(MaplyCoordinate *)ret_ll ur:(MaplyCoordinate *)ret_ur
{
    if ([_coordSys isKindOfClass:[MaplyPlateCarree class]])
    {
        MaplyCoordinate ll,ur;
        [_coordSys getBoundsLL:&ll ur:&ur];
        ret_ll->x = 180 * ll.x / M_PI;  ret_ll->y = 180 * ll.y / M_PI;
        ret_ur->x = 180 * ur.x / M_PI;  ret_ur->y = 180 * ur.y / M_PI;
    }
    else if ([_coordSys isKindOfClass:[MaplySphericalMercator class]])
    {
        // http://docs.openlayers.org/library/spherical_mercator.html
        ret_ll->x = -20037508.34; ret_ll->y = -20037508.34;
        ret_ur->x =  20037508.34; ret_ur->y =  20037508.34;
    }
    else
    {
        // fallback. might not work..
        [_coordSys getBoundsLL:ret_ll ur:ret_ur];
    }
}

@end
