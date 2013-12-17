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

/** A bounding box for a specific CRS in that coordinate
    system.  This is part of the Web Map Server parser.
  */
@interface MaplyWMSLayerBoundingBox : NSObject

/// Coordinate Reference System
@property (nonatomic) NSString *crs;

/// Left side of the bounding box
@property (nonatomic) double minx;
/// Bottom of the bounding box
@property (nonatomic) double miny;
/// Right side of the bounding box
@property (nonatomic) double maxx;
/// Top of the bounding box
@property (nonatomic) double maxy;

/// Generate the coordinate system, if we can
- (MaplyCoordinateSystem *)buildCoordinateSystem;

@end

/** Style of a WMS layer as returned by GetCapabilities.
    This is part of the Web Map Service parser.
  */
@interface MaplyWMSStyle : NSObject

/// Name and title as returned
@property (nonatomic) NSString *name,*title;

@end

/** Description of a WMS layer as returned by a GetCapabilities call.
    This is part of the Web Map Service parser.
  */
@interface MaplyWMSLayer : NSObject

/// The name as returned by the service
@property (nonatomic) NSString *name;
/// The title as returned by the service
@property (nonatomic) NSString *title;
/// The abstract as returned by the service
@property (nonatomic) NSString *abstract;

/// Coordinate reference systems supported by the layer
@property (nonatomic) NSArray *coordRefSystems;

/// Styles we can choose
@property (nonatomic) NSArray *styles;

/// Bounding boxes for zero or more of the CRS'
@property (nonatomic) NSArray *boundingBoxes;

/// Lower left corner in longitude/latitude
@property (nonatomic) MaplyCoordinate ll;
/// Upper right corner in longitude/latitude
@property (nonatomic) MaplyCoordinate ur;

/// Try to build a coordinate system we understand
- (MaplyCoordinateSystem *)buildCoordSystem;

/// Find the style with the given name
- (MaplyWMSStyle *)findStyle:(NSString *)styleName;

@end

@class DDXMLDocument;

/** Encapsulates the capabilities coming back from a WMS server.
    We can query this to see what layers and coordinate systems are available.
    Part of the Web Map Service parser.
  */
@interface MaplyWMSCapabilities : NSObject

/// We can fetch the capabilities from this URL
+ (NSString *)CapabilitiesURLFor:(NSString *)baseURL;

/// The name as returned by the service
@property (nonatomic) NSString *name;
/// The title as returned by the service
@property (nonatomic) NSString *title;

/// Available formats (strings)
@property (nonatomic) NSArray *formats;

/// Layers we can fetch from
@property (nonatomic) NSArray *layers;

/// This constructor will initialize with an XML document that
///  we've fetched from the server, presumably.
- (id)initWithXML:(DDXMLDocument *)xmlDoc;

/// Look for a layer with the given name.
- (MaplyWMSLayer *)findLayer:(NSString *)name;

@end

/** This is a MaplyTileSource that works with a remote
    Web Map Service implementation.  WMS is not the most
    efficient way to access remote image data, but there
    are still a few places that use it.
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

/// Minimum zoom level we'll expect
@property (nonatomic,readonly) int minZoom;
/// Maximum zoom level we'll expect
@property (nonatomic,readonly) int maxZoom;

/// Tile size provided to caller
@property (nonatomic,readonly) int tileSize;

/// If set we'll ask for a transparent background from the server
@property (nonatomic) bool transparent;

/// Coordinate system (used to build URLs)
@property (nonatomic,readonly) MaplyCoordinateSystem *coordSys;

/// If set, we'll cache the images locally (a good idea with WMS)
@property (nonatomic) NSString *cacheDir;

/** Initialize with the parameters the WMS server is going to want.
    @param baseURL The main URL we'll use to construct queries.
    @param cap The capabilities as parsed from the service.
    @param layer The layer we'll access.  There can be multiple and it's
            up to you to pick one.
    @param style The style variant of the layer we want.  Again there can
            be multiple and it's up to you to pick.
    @param coordSys The coordinate system we're expecting to work in.
    @param minZoom The min zoom level we want.  Note that WMS doesn't handle
            this directly.  Our tile source just controls what areas it
            asks for based on the overall extents and the zoom levels.
    @param maxZoom The max zoom level we'll query.
  */
- (id)initWithBaseURL:(NSString *)baseURL capabilities:(MaplyWMSCapabilities *)cap layer:(MaplyWMSLayer *)layer style:(MaplyWMSStyle *)style coordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom tileSize:(int)tileSize;

@end

