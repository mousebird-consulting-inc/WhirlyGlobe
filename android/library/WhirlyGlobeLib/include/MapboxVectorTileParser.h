/*
 *  MapboxVectorTileParser.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/25/16.
 *  Copyright 2011-2016 mousebird consulting
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

#import "vector_tile.pb.h"
#import "VectorObject.h"

namespace WhirlyKit
{

typedef enum {
    GeomTypeUnknown = 0,
    GeomTypePoint = 1,
    GeomTypeLineString = 2,
    GeomTypePolygon = 3
} MapnikGeometryType;

typedef enum {
    SEG_END    = 0,
    SEG_MOVETO = 1,
    SEG_LINETO = 2,
    SEG_CLOSE = (0x40 | 0x0f)
} MapnikCommandType;

/** This object parses the data in Mapbox Vector Tile format.
  */
class MapboxVectorTileParser
{
public:
    MapboxVectorTileParser();
    ~MapboxVectorTileParser();
    
    // Parse the vector tile and return a list of vectors.
    // Returns false on failure.
    bool parseVectorTile(RawData *rawData,std::vector<VectorObject *> &vecObjs,const Mbr &mbr);
};

}
