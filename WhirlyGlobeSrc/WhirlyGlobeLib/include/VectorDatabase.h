/*
 *  VectorDatabase.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/12/11.
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

#import <math.h>
#import <set>
#import <map>
#import "VectorData.h"
#import "sqlite3.h"

namespace WhirlyKit
{
    
typedef std::set<unsigned int> UIntSet;

/** The Vector Database is used to keep vector data out of memory until needed.
    It will initialize itself if its cache files aren't there.
    That can be slow, so ideally initialize it offline.
    It builds both a file of MBRs and a sqlite database with the attributes
 */
class VectorDatabase
{
public:
    /// Either initialize from or build the cache info for a vector db.
    /// cacheDir is where the cache files are or you want them to be.
    /// baseName should correspond to the file being read.
    /// reader is a vector reader that can seek.  Will delete this on destruction.
    /// indices are sqlite columns to make indices if we're constructing this thing
    VectorDatabase(NSString *bundleDir,NSString *cacheDir,NSString *baseName,VectorReader *reader,const std::set<std::string> *indices,bool cache=false,bool autoload=false);
    ~VectorDatabase();

    /// Turn automatic loading on or off
    /// If it's on, you need to call process
    void setAutoload(bool autoload);
    
    /// Turn memory caching on or off
    void setMemCache(bool memCache);
    
    /// If you want the vector db to autoload, call this periodically
    void process();
    
    /// Total number of vectors in the database
    unsigned int numVectors();
    
    /// Fetch an MBR for the given vector.
    /// This is cheap because it's in memory
    GeoMbr getMbr(unsigned int);
    
    /** Fetch the corresponding vector.
        This probably isn't cheap since we're going out to "disk".
        We can also read the attributes, or not
        This is returning a reference counted shape so keep it around
        until you're done with it.
     */
    VectorShapeRef getVector(unsigned int,bool withAttributes=true);
    
    /// Return a list of all the features that overlap 
    void getVectorsWithinMbr(const GeoMbr &mbr,UIntSet &vecIds);
    
    /// Run a SQL query, returning the list of IDs that match.
    /// Pass in the where clause, essentially.
    /// This version returns a set of IDs
    void getMatchingVectors(NSString *query,UIntSet &vecIds);
    /// This version returns a set of shapes
    void getMatchingVectors(NSString *query,ShapeSet &shapes);

    /// Return any areals that surround the given point.
    /// The caller is responsible for deletion
    void findArealsForPoint(const GeoCoord &coord,ShapeSet &shapes);
    
    /// Return a pointer to the SQLite DB.  For read only please
    sqlite3 *getSqliteDb();
    
protected:
    bool buildCaches(NSString *mbrCache,NSString *sqlDb);
    bool readCaches(NSString *mbrCache,NSString *sqlDb);
    
    VectorReader *reader;
    
    /// Flat list of the vectors and their MBRs.
    /// Need a spatial index here, ideally an R-Tree
    std::vector<GeoMbr> mbrs;
    
    /// If we're caching in memory, this is the cache
    bool vecCacheOn;
    std::map<unsigned int,VectorShapeRef> vecCache;
    
    /// If we're slowly loading data in, this is how we keep track
    bool autoloadOn;
    int autoloadWhere;
    
    /// Open SQLite database
    sqlite3 *db;
};

}
