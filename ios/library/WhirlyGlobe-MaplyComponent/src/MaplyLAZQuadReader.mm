//
//  LAZQuadReader.m
//  LidarViewer
//
//  Created by Steve Gifford on 4/13/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#include <fstream>
#include <iostream>
#import <set>
#include <string>
#include <iostream>
#include <sstream>

#import "MaplyLAZQuadReader.h"
#import "MaplyLAZShader.h"
#import "sqlite3.h"
#import "FMDatabase.h"
#import "FMDatabaseQueue.h"
#import "laszip_api.h"
#import "WhirlyGlobe.h"
#import "MaplyLAZMeshBuilder.h"
#import "private/WhirlyGlobeViewController_private.h"
#import "private/MaplyCoordinateSystem_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@interface MaplyLAZQuadReader()

- (bool) intersectWithRenderer:(WhirlyKitSceneRendererES *)renderer view:(WhirlyKitView *)theView touchPt:(const Point2f &)touchPt org:(const Point3d &)org dir:(const Point3d &)dir interPt:(Point3d &)iPt dist:(double &)dist;

@end

NSString * const kMaplyLAZReaderCoordSys = @"coordsys";
NSString * const kMaplyLAZReaderZOffset = @"zoffset";
NSString * const kMaplyLAZReaderColorScale = @"colorscale";

// Keep track of tile size (height in particular)
class TileBoundsInfo
{
public:
    TileBoundsInfo(MaplyTileID tileID) : tileID(tileID), minZ(0.0), maxZ(0.0) { }
    TileBoundsInfo() { }
    
    bool operator < (const TileBoundsInfo &that) const
    {
        if (tileID.level == that.tileID.level)
        {
            if (tileID.y == that.tileID.y)
                return tileID.x < that.tileID.x;
            return tileID.y < that.tileID.y;
        }
        return tileID.level < that.tileID.level;
    }
    
    MaplyTileID tileID;
    double minZ,maxZ;
    WhirlyKit::VectorTrianglesRef mesh;
};

/// Intersection handler for grabbing objects
class IntersectionHandler : public IntersectionManager::Intersectable
{
public:
    bool findClosestIntersection(WhirlyKitSceneRendererES *renderer,WhirlyKitView *theView,const Point2f &frameSize,const Point2f &touchPt,const Point3d &org,const Point3d &dir,Point3d &iPt,double &dist)
    {
        return [quadReader intersectWithRenderer:renderer view:theView touchPt:touchPt org:org dir:dir interPt:iPt dist:dist];
    }
    
    MaplyLAZQuadReader *quadReader;
};

typedef std::set<TileBoundsInfo> TileBoundsSet;

@implementation MaplyLAZQuadReader
{
    FMDatabase *db;
    FMDatabaseQueue *queue;
    std::ifstream *ifs;
    laszip_POINTER lazReader;
    TileBoundsSet tileSizes;
    int pointType;
    double colorScale;
    IntersectionHandler intersectionHandler;
    MaplyBaseViewController *viewC;
}

- (id)initWithDB:(NSString *)sqliteFileName desc:(NSDictionary *)desc viewC:(WhirlyGlobeViewController *)inViewC
{
    self = [super init];
    if (!self)
        return nil;
    viewC = inViewC;
    
    _zOffset = 0.0;
    _pointSize = 6.0;
//    colorScale = (1<<16)-1;
    colorScale = 255;
    
    NSString *sqlitePath = nil;
    // See if that was a direct path first
    if ([[NSFileManager defaultManager] fileExistsAtPath:sqliteFileName])
        sqlitePath = sqliteFileName;
    else {
        // Now try looking for it in the bundle
        sqlitePath = [[NSBundle mainBundle] pathForResource:sqliteFileName ofType:@"sqlite"];
        if (!sqlitePath)
            return nil;
    }
    
    // Open the sqlite index file
    db = [[FMDatabase alloc] initWithPath:sqlitePath];
    if (!db)
        return nil;
    
    [db openWithFlags:SQLITE_OPEN_READONLY];
    
    FMResultSet *res = [db executeQuery:@"SELECT minx,miny,minz,maxx,maxy,maxz,minlevel,maxlevel,minpoints,maxpoints,srs FROM manifest"];
    NSString *srs = nil;
    if ([res next])
    {
        _minX = [res doubleForColumn:@"minx"];
        _minY = [res doubleForColumn:@"miny"];
        _minZ = [res doubleForColumn:@"minz"];
        _maxX = [res doubleForColumn:@"maxx"];
        _maxY = [res doubleForColumn:@"maxy"];
        _maxZ = [res doubleForColumn:@"maxz"];
        self.minZoom = [res intForColumn:@"minlevel"];
        self.maxZoom = [res intForColumn:@"maxlevel"];
        _minTilePoints = [res intForColumn:@"minpoints"];
        _maxTilePoints = [res intForColumn:@"maxpoints"];
        srs = [res stringForColumn:@"srs"];
    }
    res = [db executeQuery:@"SELECT pointtype from manifest"];
    if ([res next])
        pointType = [res intForColumn:@"pointtype"];
    res = [db executeQuery:@"SELECT maxcolor from manifest"];
    if ([res next])
    {
        int maxColor = [res intForColumn:@"maxcolor"];
        if (maxColor > 300)
            colorScale = (1<<16)-1;
    }

    // Override the coordinate system
    if (desc[kMaplyLAZReaderCoordSys])
        srs = desc[kMaplyLAZReaderCoordSys];
    // The point size
    if (desc[kMaplyLAZShaderPointSize])
        _pointSize = [desc[kMaplyLAZShaderPointSize] floatValue];
    // Z offset
    if (desc[kMaplyLAZReaderZOffset])
        _zOffset = [desc[kMaplyLAZReaderZOffset] floatValue];
    // Color scale
    if (desc[kMaplyLAZReaderColorScale])
        colorScale = [desc[kMaplyLAZReaderColorScale] doubleValue];

    // Note: If this isn't set up right, we need to fake it
    if (srs && [srs length])
    {
        _coordSys = [[MaplyProj4CoordSystem alloc] initWithString:srs];
        MaplyCoordinate ll,ur;
        ll.x = _minX;  ll.y = _minY;  ur.x = _maxX;  ur.y = _maxY;
        [_coordSys setBoundsLL:&ll ur:&ur];
    } else {
        // Nebraska state plane.  Because why not.
        _coordSys = [[MaplyProj4CoordSystem alloc] initWithString:@"+proj=lcc +lat_1=43 +lat_2=40 +lat_0=39.83333333333334 +lon_0=-100 +x_0=500000.00001016 +y_0=0 +ellps=GRS80 +to_meter=0.3048006096012192 +no_defs "];
        MaplyCoordinate ll,ur;
        ll.x = _minX;  ll.y = _minY;  ur.x = _maxX;  ur.y = _maxY;
        [_coordSys setBoundsLL:&ll ur:&ur];
    }
    
    queue = [FMDatabaseQueue databaseQueueWithPath:sqlitePath];
    
    // Hook up an intersection handler
    intersectionHandler.quadReader = self;
    IntersectionManager *intersectMan = (IntersectionManager *)viewC->scene->getManager(kWKIntersectionManager);
    intersectMan->addIntersectable(&intersectionHandler);

    return self;
}

- (void)setZOffset:(double)zOffset
{
    _zOffset = zOffset;
}

- (void)setShader:(MaplyShader *)shader
{
    _shader = shader;
}

- (void)dealloc
{
    if (ifs)
    {
        delete ifs;
    }
}

- (bool)hasColor
{
    int thisPointType = pointType;
    
    if (lazReader)
    {
        laszip_header *header;
        laszip_get_header_pointer(lazReader,&header);
        thisPointType = header->point_data_format;
    }
    
    bool hasColor = false;
    switch (thisPointType)
    {
        case 0:
            hasColor = false;
            break;
        case 1:
            hasColor = false;
            break;
        case 2:
            hasColor = false;
            break;
        case 3:
            hasColor = true;
            break;
        case 4:
            hasColor = false;
            break;
        case 5:
            hasColor = true;
            break;
        case 6:
            hasColor = false;
            break;
        case 7:
            hasColor = true;
            break;
        case 8:
            hasColor = true;
            colorScale = (1<<16)-1;
            break;
        case 9:
            hasColor = false;
            break;
        case 10:
            hasColor = true;
            colorScale = (1<<16)-1;
            break;
        default:
            break;
    }
    
    return thisPointType > 1;
}

- (int)getNumTilesFromMaxPoints:(int)maxPoints
{
    int numTiles = maxPoints/_minTilePoints;
    if (numTiles < 1)  numTiles = 1;
    if (numTiles > 256)  numTiles = 256;
    
    return numTiles;
}

- (MaplyCoordinate)getCenter
{
    // Figure out the bounds
    double midX = (_minX+_maxX)/2.0;
    double midY = (_minY+_maxY)/2.0;
    
    return MaplyCoordinateMake(midX,midY);
}

- (void)getBoundsLL:(MaplyCoordinate3dD *)ll ur:(MaplyCoordinate3dD *)ur
{
    ll->x = _minX;  ll->y = _minY;  ll->z = _minZ+_zOffset;
    ur->x = _maxX;  ur->y = _maxY;  ur->z = _maxZ+_zOffset;
}

- (void)getBoundingBox:(MaplyTileID)tileID ll:(MaplyCoordinate3dD *)ll ur:(MaplyCoordinate3dD *)ur
{
    @synchronized (self) {
        TileBoundsInfo dummy(tileID);
        TileBoundsSet::iterator it = tileSizes.find(dummy);
        if (it != tileSizes.end())
        {
            ll->z = it->minZ;
            ur->z = it->maxZ;
        } else {
            // Didn't find it, so look for the parent
            MaplyTileID parentTileID;
            parentTileID.x = tileID.x/2;  parentTileID.y = tileID.y/2;  parentTileID.level = tileID.level-1;
            TileBoundsInfo dummy(parentTileID);
            TileBoundsSet::iterator it = tileSizes.find(dummy);
            if (it != tileSizes.end())
            {
                ll->z = it->minZ;
                ur->z = it->maxZ;
            }
        }
    }
}

// Look for a valid intersection with any of our meshes
- (bool) intersectWithRenderer:(WhirlyKitSceneRendererES *)renderer view:(WhirlyKitView *)theView touchPt:(const Point2f &)touchPt org:(const Point3d &)org dir:(const Point3d &)dir interPt:(Point3d &)iPt dist:(double &)dist
{
    double minDist = std::numeric_limits<double>::max();
    Point3d minPt;
    CoordSystemDisplayAdapter *coordAdapter = viewC->visualView.coordAdapter;

    // Note: Naive implementation
    @synchronized (self) {
        for (auto tileBounds : tileSizes)
        {
            double thisT;
            Point3d thisPt;
            if (VectorTrianglesRayIntersect(org,dir,*(tileBounds.mesh),&thisT,&thisPt))
            {
                double thisDist = (thisPt-org).norm();
                if (thisDist < minDist)
                {
                    // Project point back to source system
                    Point3d localPt = coordAdapter->displayToLocal(thisPt);
                    Point3d srcPt = CoordSystemConvert3d(coordAdapter->getCoordSystem(), _coordSys->coordSystem, localPt);
                    
                    // We found an intersection, but let's check if a higher res tile is loaded
                    int numTiles = 1<<(tileBounds.tileID.level+1);
                    Point2d tileSize((self.maxX-self.minX)/numTiles,(self.maxY-self.minY)/numTiles);
                    MaplyTileID subTile;
                    subTile.x = (srcPt.x() - self.minX)/tileSize.x();
                    subTile.y = (srcPt.y() - self.minY)/tileSize.y();
                    subTile.level = tileBounds.tileID.level+1;
                    
                    // If the higher res tile is loaded, then we have to trust its surface
                    // Odds are that our lower res surface missed some dips.
                    TileBoundsInfo dummyTile(subTile);
                    auto it = tileSizes.find(dummyTile);
                    if (it == tileSizes.end())
                    {
                        minDist = thisDist;
                        minPt = thisPt;
                    }
                }
            }
        }
    }
    
    if (minDist != std::numeric_limits<double>::max())
    {
        iPt = minPt;
        dist = minDist;
        return true;
    }
    
    return false;
}

- (void)tileDidUnload:(MaplyTileID)tileID
{
    @synchronized (self) {
        TileBoundsInfo dummy(tileID);
        TileBoundsSet::iterator it = tileSizes.find(dummy);
        if (it != tileSizes.end())
            tileSizes.erase(it);
    }
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *__nonnull)layer
{
    MaplyCoordinate ll,ur;
    [layer boundsforTile:tileID ll:&ll ur:&ur];
    
//    NSLog(@"Tile %d: (%d,%d)  ll = (%f,%f),  ur (%f,%f)",tileID.level,tileID.x,tileID.y,ll.x,ll.y,ur.x,ur.y);
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
   ^{
       // Put together the precalculated quad index.  This is faster
       //  than x,y,level
       int quadIdx = 0;
       for (int iq=0;iq<tileID.level;iq++)
           quadIdx += (1<<iq)*(1<<iq);
       quadIdx += tileID.y*(1<<tileID.level)+tileID.x;

       // Information set up from the database or from the global file
       laszip_POINTER __block thisReader = NULL;
       std::stringstream * __block tileStream = NULL;
       MaplyComponentObject * __block compObj = nil;
       NSData * __block data = nil;
       long long __block pointStart = 0;
       int __block count = 0;
       bool __block hasColors = false;
       
       // We're either using the index with an external LAZ files or we're grabbing the raw data itself
       [queue inDatabase:^(FMDatabase *theDb) {
           FMResultSet *res = nil;
           if (lazReader)
               res = [db executeQuery:[NSString stringWithFormat:@"SELECT start,count FROM tileaddress WHERE quadindex=%d;",quadIdx]];
           else
               res = [db executeQuery:[NSString stringWithFormat:@"SELECT data FROM lidartiles WHERE quadindex=%d;",quadIdx]];
           if ([res next])
           {
               if (lazReader)
               {
                   pointStart = [res longLongIntForColumn:@"start"];
                   count = [res intForColumn:@"count"];
                   thisReader = lazReader;
                   laszip_header_struct *header;
                   laszip_get_header_pointer(thisReader,&header);
                   hasColors = header->point_data_format > 1;
               } else {
                   data = [res dataForColumn:@"data"];
                   tileStream = new std::stringstream();
                   tileStream->write(reinterpret_cast<const char *>([data bytes]),[data length]);

                   laszip_BOOL is_compressed;
                   laszip_create(&thisReader);
                   laszip_open_stream_reader(thisReader,tileStream,&is_compressed);
                   laszip_header_struct *header;
                   laszip_get_header_pointer(thisReader,&header);
                   hasColors = header->point_data_format > 1;
                   count = header->number_of_point_records;
               }

               [res close];
           }
       }];
       
       if (thisReader)
       {
           MaplyPoints *points = [[MaplyPoints alloc] initWithNumPoints:count];
           int elevID = [points addAttributeType:@"a_elev" type:MaplyShaderAttrTypeFloat];
           
           // Center the coordinates around the tile center
           MaplyCoordinate3dD tileCenter;
           laszip_header_struct *header;
           laszip_get_header_pointer(thisReader,&header);
           tileCenter.x = (header->min_x+header->max_x)/2.0;
           tileCenter.y = (header->min_y+header->max_y)/2.0;
           tileCenter.z = 0.0;
           MaplyCoordinate3dD tileCenterDisp = [layer.viewC displayCoordD:tileCenter fromSystem:_coordSys];
           points.transform = [[MaplyMatrix alloc] initWithTranslateX:tileCenterDisp.x y:tileCenterDisp.y z:tileCenterDisp.z];
           
           // We generate a triangle mesh underneath a given tile to provide something to grab
           MaplyLAZMeshBuilder meshBuilder(10,10,Point2d(header->min_x,header->min_y),Point2d(header->max_x,header->max_y),self.coordSys);
           
           long long which = 0;
           double minZ=MAXFLOAT,maxZ=-MAXFLOAT;
           while (which < count)
           {
               // Get the point and convert to geocentric
               if (lazReader)
               {
                   laszip_seek_point(thisReader,(which+pointStart));
                   laszip_read_point(thisReader);
               } else {
                   laszip_seek_point(thisReader,(which+pointStart));
                   laszip_read_point(thisReader);
               }
               laszip_point_struct *p;
               laszip_get_point_pointer(thisReader, &p);
               //                double x,y,z;
               //                x = p.GetX(), y = p.GetY(); z = p.GetZ();
               //                trans->TransformEx(1, &x, &y, &z);
               MaplyCoordinate3dD coord;
               coord.x = p->X * header->x_scale_factor + header->x_offset;
               coord.y = p->Y * header->y_scale_factor + header->y_offset;
               coord.z = p->Z * header->z_scale_factor + header->z_offset;
               coord.z += _zOffset;
               
               minZ = std::min(coord.z,minZ);
               maxZ = std::max(coord.z,maxZ);

               MaplyCoordinate3dD dispCoord = [layer.viewC displayCoordD:coord fromSystem:_coordSys];
               MaplyCoordinate3dD dispCoordCenter = MaplyCoordinate3dDMake(dispCoord.x-tileCenterDisp.x, dispCoord.y-tileCenterDisp.y, dispCoord.z-tileCenterDisp.z);
               
               float red = 1.0,green = 1.0, blue = 1.0;
               if (hasColors)
               {
                   red = p->rgb[0] / colorScale;
                   green = p->rgb[1] / colorScale;
                   blue = p->rgb[2] / colorScale;
               }
               [points addDispCoordDoubleX:dispCoordCenter.x y:dispCoordCenter.y z:dispCoordCenter.z];
               [points addColorR:red g:green b:blue a:1.0];
               [points addAttribute:elevID fVal:coord.z];
               
               meshBuilder.addPoint(Point3d(coord.x,coord.y,coord.z));
               
               which++;
           }
           
           // Keep track of tile size
           if (minZ == maxZ)
               maxZ += 1.0;
           @synchronized (self) {
               TileBoundsInfo tileInfo(tileID);
               tileInfo.mesh = meshBuilder.makeMesh(layer.viewC);
               tileInfo.minZ = minZ;  tileInfo.maxZ = maxZ;
               tileSizes.insert(tileInfo);
           }
           
//           NSLog(@"Loaded tile %d: (%d,%d) with %d points",tileID.level,tileID.x,tileID.y,count);

           compObj = [layer.viewC addPoints:@[points] desc:
                                            @{kMaplyColor: [UIColor redColor],
                                              kMaplyDrawPriority: @(10000000),
                                              kMaplyShader: _shader.name,
                                              kMaplyShaderUniforms:
                                                  @{kMaplyLAZShaderZMin: @(_minZ+_zOffset),
                                                    kMaplyLAZShaderZMax: @(_maxZ+_zOffset),
                                                    kMaplyLAZShaderPointSize: @(_pointSize)
                                                    },
                                              kMaplyZBufferRead: @(YES),
                                              kMaplyZBufferWrite: @(YES)
                                              }
                                                mode:MaplyThreadCurrent];
           [layer addData:@[compObj] forTile:tileID style:MaplyDataStyleAdd];
       }

       if (!lazReader)
       {
           if (thisReader)
           {
               laszip_close_reader(thisReader);
               laszip_destroy(thisReader);
               delete tileStream;
           }
       }
       
       if (compObj)
           [layer tileDidLoad:tileID];
       else
           [layer tileFailedToLoad:tileID];
   });
}

@end
