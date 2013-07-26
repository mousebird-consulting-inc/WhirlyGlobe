/*
 *  MaplyWMSTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/25/13.
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"
#import "DDXML.h"

/** A bounding box for a specific CRS in that coordinate
    system.
  */
@interface MaplyWMSLayerBoundingBox : NSObject

/// Coordinate Reference System
@property (nonatomic) NSString *crs;

/// Bounding box
@property (nonatomic) double minx,miny,maxx,maxy;

/// Generate the coordinate system, if we can
- (MaplyCoordinateSystem *)buildCoordinateSystem;

@end

/** Style of a WMS layer as returned by GetCapabilities
  */
@interface MaplyWMSStyle : NSObject

/// Name and title as returned
@property (nonatomic) NSString *name,*title;

@end

/** Description of a WMS layer as returned by a GetCapabilities call.
  */
@interface MaplyWMSLayer : NSObject

/// Name and title as returned by the service
@property (nonatomic) NSString *name,*title,*abstract;

/// Coordinate reference systems supported by the layer
@property (nonatomic) NSArray *coordRefSystems;

/// Styles we can choose
@property (nonatomic) NSArray *styles;

/// Bounding boxes for zero or more of the CRS'
@property (nonatomic) NSArray *boundingBoxes;

/// Lat/lon bounding box
@property (nonatomic) MaplyCoordinate ll,ur;

/// Try to build a coordinate system we understand
- (MaplyCoordinateSystem *)buildCoordSystem;

/// Find the style with the given name
- (MaplyWMSStyle *)findStyle:(NSString *)styleName;

@end

/** Encapsulates the capabilities coming back from a WMS server.
    We can query this to see what layers and coordinate systems are available.
  */
@interface MaplyWMSCapabilities : NSObject

/// We can fetch the capabilities from this URL
+ (NSString *)CapabilitiesURLFor:(NSString *)baseURL;

/// Name and title
@property (nonatomic) NSString *name,*title;

/// Available formats (strings)
@property (nonatomic) NSArray *formats;

/// Layers we can fetch from
@property (nonatomic) NSArray *layers;

/// Initialize capabitiles from an XML document
- (id)initWithXML:(DDXMLDocument *)xmlDoc;

/// Look for a layer with the given name
- (MaplyWMSLayer *)findLayer:(NSString *)name;

@end

/** Tile source implementation for WMS.
 
  */
@interface MaplyWMSTileSource : NSObject<MaplyTileSource>

/// Base URL for the Map Service
@property (nonatomic) NSString *baseURL;

/// Capabilities describing the service
@property (nonatomic) MaplyWMSCapabilities *capabilities;

/// Image type to request
@property (nonatomic) NSString *imageType;

/// Layer we're grabbing
@property (nonatomic) MaplyWMSLayer *layer;

/// Optional style we're using
@property (nonatomic) MaplyWMSStyle *style;

/// Zoom levels provided to caller
@property (nonatomic,readonly) int minZoom,maxZoom;

/// Tile size provided to caller
@property (nonatomic,readonly) int tileSize;

/// If set we'll ask for a transparent background from the server
@property (nonatomic) bool transparent;

/// Coordinate system (used to build URLs)
@property (nonatomic,readonly) MaplyCoordinateSystem *coordSys;

/// Construct with the base URL and the layers we should fetch
- (id)initWithBaseURL:(NSString *)baseURL capabilities:(MaplyWMSCapabilities *)cap layer:(MaplyWMSLayer *)layer style:(MaplyWMSStyle *)style coordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom tileSize:(int)tileSize;

@end

