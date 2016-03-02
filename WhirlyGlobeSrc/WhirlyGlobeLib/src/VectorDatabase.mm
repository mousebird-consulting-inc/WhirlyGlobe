/*
 *  VectorDatabase.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/13/11.
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

#import <UIKit/UIKit.h>
#import "VectorDatabase.h"
#import "sqlhelpers.h"

namespace WhirlyKit
{

VectorDatabase::VectorDatabase(NSString *bundleDir,NSString *cacheDir,NSString *baseName,VectorReader *reader,const std::set<std::string> *indices,bool memCache,bool autoload)
    : reader(reader), db(NULL), autoloadOn(false), vecCacheOn(false)
{
    // Look for an existing MBR file and database
    NSString *mbrName0 = [NSString stringWithFormat:@"%@/%@.mbr",bundleDir,baseName];
    NSString *dbName0 = [NSString stringWithFormat:@"%@/%@.sqlite",bundleDir,baseName];
    NSString *mbrName1 = [NSString stringWithFormat:@"%@/%@.mbr",cacheDir,baseName];
    NSString *dbName1 = [NSString stringWithFormat:@"%@/%@.sqlite",cacheDir,baseName];
    
    bool needToBuild = true;
    
    // Look for existing versions in the bundle dir
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:mbrName0] && [fileManager fileExistsAtPath:dbName0])
    {
        needToBuild = false;
        if (!readCaches(mbrName0,dbName0))
            needToBuild = true;
    }
    
    // Look for existing versions in the cache dir
    if (needToBuild)
    {
        if ([fileManager fileExistsAtPath:mbrName1] && [fileManager fileExistsAtPath:dbName1])
        {
            needToBuild = false;
            if (!readCaches(mbrName1,dbName1))
                needToBuild = true;
        }
    }
    
    // Now maybe build the cache (hopefully not)
    if (needToBuild)
    {
        if (!buildCaches(mbrName1, dbName1))
            throw (std::string)"Failed to build vector cache.  Giving up.";
    }
    
    setMemCache(memCache);
    setAutoload(autoload);
}
    
VectorDatabase::~VectorDatabase()
{
    if (db)
        sqlite3_close(db);
    if (reader)
        delete reader;
}
    
// Turn automatic loading on or off
// If it's on, you need to call process
void VectorDatabase::setAutoload(bool autoload)
{
    // Only bother if we're changing status
    if (autoload != autoloadOn)
    {
        autoloadOn = autoload;
        // Just started
        if (autoloadOn)
            autoloadWhere = 0;
    }
}
    
// Turn memory caching on or off
void VectorDatabase::setMemCache(bool memCache)
{
    if (vecCacheOn != memCache)
    {
        vecCacheOn = memCache;
        // If we turned it off, clean it out
        if (!vecCacheOn)
            vecCache.clear();
    }
}
    
// If you want the vector db to autoload, call this periodically
void VectorDatabase::process()
{
    if (!vecCacheOn || !autoloadOn)
        return;
    
    if (autoloadWhere < reader->getNumObjects())
    {
        unsigned int vecId = autoloadWhere;
        // Load it and save it away
        VectorShapeRef newShape = getVector(vecId,true);
        vecCache[vecId] = newShape;
        autoloadWhere++;
    }
}
    
unsigned int VectorDatabase::numVectors()
{
    return reader->getNumObjects();
}
    
GeoMbr VectorDatabase::getMbr(unsigned int vecIndex)
{
    if (vecIndex >= reader->getNumObjects())
        return GeoMbr();
    
    return mbrs[vecIndex];
}
    
// Return a single vector by index
VectorShapeRef VectorDatabase::getVector(unsigned int vecIndex,bool withAttributes)
{
    if (vecIndex >= reader->getNumObjects())
        return VectorShapeRef();
    
    // Let's look in the cache first
    std::map<unsigned int,VectorShapeRef>::iterator it = vecCache.find(vecIndex);
    if (it != vecCache.end())
        return it->second;
    
    VectorShapeRef retShape;
    if (withAttributes)
        retShape = reader->getObjectByIndex(vecIndex,NULL);
    else {
        // Passing in an empty filter should net us no attributes
        StringSet filter;
        retShape = reader->getObjectByIndex(vecIndex, &filter);
    }
        
    return retShape;
}
    
// Return all the vectors that overlap the given Mbr
void VectorDatabase::getVectorsWithinMbr(const GeoMbr &mbr,UIntSet &vecIds)
{
    for (unsigned int ii=0;ii<mbrs.size();ii++)
        if (mbr.overlaps(mbrs[ii]))
            vecIds.insert(ii);
}
    
sqlite3 *VectorDatabase::getSqliteDb()
{
    return db;
}


// Build the MBR cache and the sqlite database
bool VectorDatabase::buildCaches(NSString *mbrCache,NSString *sqlDb)
{
    // If we're rebuiling the caches, just nuke everything
    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager removeItemAtPath:mbrCache error:NULL];
    [fileManager removeItemAtPath:sqlDb error:NULL];
    
    // Create a sqlite db
    if (sqlite3_open([sqlDb cStringUsingEncoding:NSASCIIStringEncoding],&db) != SQLITE_OK)
        return false;
    
    // Set up the one table we need table
    sqlhelpers::OneShot(db,@"CREATE TABLE vectors (vecid INTEGER PRIMARY KEY);");
    std::set<std::string> fields;
    std::set<std::string> ignoreFields;
    
    mbrs.resize(reader->getNumObjects());
    for (unsigned int ii=0;ii<mbrs.size();ii++)
    {
        VectorShapeRef theShape = getVector(ii,true);
        if (theShape.get())
        {
            mbrs[ii] = theShape->calcGeoMbr();

            std::vector<std::string> keys;
            NSMutableArray *data = [NSMutableArray array];

            // Build up a row to add
            for (NSString *key in [theShape->getAttrDict() allKeys])
            {
                std::string fieldName = [key cStringUsingEncoding:NSASCIIStringEncoding];
                std::string dataType = "";

                // Figure out the data type
                NSObject *obj = [theShape->getAttrDict() objectForKey:key];
                if ([obj isKindOfClass:[NSString class]])
                {
                    dataType = "VARCHAR(100)";  // Note: This may not be enough in all cases
                } else {
                    if ([obj isKindOfClass:[NSNumber class]])
                    {
                        NSNumber *num = (NSNumber *)obj;
                        if (!strcmp([num objCType], @encode(BOOL)))
                        {
                            dataType = "BOOLEAN";
                        } else {
                            if (!strcmp([num objCType], @encode(int)))
                            {
                                dataType = "INTEGER";
                            } else {
                                if (!strcmp([num objCType], @encode(float)) ||
                                    !strcmp([num objCType], @encode(double)))
                                {
                                    dataType = "REAL";
                                }
                            }
                        }
                    }
                }

                // Keep track of the field name/data
                if (!dataType.empty())
                {
                    keys.push_back(fieldName);
                    [data addObject:[theShape->getAttrDict() objectForKey:key]];
                }

                // Add the field if it isn't in the table
                if (fields.find(fieldName) == fields.end() &&
                    ignoreFields.find(fieldName) == ignoreFields.end())
                {                    
                    // Create the field
                    if (!dataType.empty())
                    {
                        sqlhelpers::OneShot(db,[NSString stringWithFormat:@"ALTER TABLE vectors ADD %@ %s;",key,dataType.c_str()]);                                                
                        fields.insert(fieldName);
                    } else
                        ignoreFields.insert(fieldName);
                }
            }

            // Build up the insert and kick it off
            NSMutableString *keyStr = [NSMutableString stringWithString:@"vecid, "];
            NSMutableString *valStr = [NSMutableString stringWithString:@"?, "];
            for (unsigned int jj=0;jj<keys.size();jj++)
            {
                [keyStr appendFormat:@"%s%s",keys[jj].c_str(),(jj==keys.size()-1 ? "" : ", ")];
                [valStr appendFormat:@"?%s",(jj==keys.size()-1 ? "" : ", ")];
            }
            sqlhelpers::StatementWrite insStmt(db,[NSString stringWithFormat:@"INSERT INTO vectors (%@) values (%@);",keyStr,valStr]);
            insStmt.add((int)ii);
            for (NSObject *obj in data)
            {
                if ([obj isKindOfClass:[NSString class]])
                    insStmt.add((NSString *)obj);
                else {
                    if ([obj isKindOfClass:[NSNumber class]])
                    {
                        NSNumber *num = (NSNumber *)obj;
                        if (!strcmp([num objCType], @encode(BOOL)))
                            insStmt.add([num boolValue]);
                        else {
                            if (!strcmp([num objCType], @encode(int)))
                                insStmt.add([num intValue]);
                            else {
                                if (!strcmp([num objCType], @encode(float)) ||
                                    !strcmp([num objCType], @encode(double)))
                                    insStmt.add([num floatValue]);
                            }
                        }
                    }
                }
            }
            insStmt.go();
        }
    }
    
    // Write the cache file
    //  Version
    //  Number of MBRs
    //  MBRs
    FILE *fp = fopen([mbrCache cStringUsingEncoding:NSASCIIStringEncoding],"wb");
    try
    {
        if (!fp)
            throw 1;
        unsigned int mbrVersion = 1;
        if (fwrite(&mbrVersion, sizeof(mbrVersion), 1, fp) != 1)
            throw 1;        
        unsigned int numMbrs = (unsigned int)mbrs.size();
        if (fwrite(&numMbrs, sizeof(numMbrs), 1, fp) != 1)
            throw 1;
        for (unsigned int ii=0;ii<mbrs.size();ii++)
        {
            GeoMbr &mbr = mbrs[ii];
            float ll_x=mbr.ll().x(),ll_y=mbr.ll().y(),ur_x=mbr.ur().x(),ur_y=mbr.ur().y();
            if (fwrite(&ll_x, sizeof(float), 1, fp) != 1 ||
                fwrite(&ll_y, sizeof(float), 1, fp) != 1 ||
                fwrite(&ur_x, sizeof(float), 1, fp) != 1 ||
                fwrite(&ur_y, sizeof(float), 1, fp) != 1)
                throw 1;
        }
        fclose(fp);
        fp = NULL;
    }
    catch (...)
    {
        if (fp)
            fclose(fp);
        return false;
    }
    if (fp)
        fclose(fp);
    
    return true;
}
    
// Read existing caches
bool VectorDatabase::readCaches(NSString *mbrCache, NSString *sqlDb)
{
    // MBR cache file
    FILE *fp = fopen([mbrCache cStringUsingEncoding:NSASCIIStringEncoding],"rb");
    try {
        if (!fp)
            throw 1;
        unsigned int mbrVersion;
        if (fread(&mbrVersion, sizeof(mbrVersion), 1, fp) != 1 ||
            mbrVersion != 1)
            throw 1;
        unsigned int numMbrs;
        if (fread(&numMbrs,sizeof(numMbrs), 1, fp) != 1)
            throw 1;
        mbrs.resize(numMbrs);
        for (unsigned int ii=0;ii<mbrs.size();ii++)
        {
            float ll_x,ll_y,ur_x,ur_y;
            if (fread(&ll_x, sizeof(float), 1, fp) != 1 ||
                fread(&ll_y, sizeof(float), 1, fp) != 1 ||
                fread(&ur_x, sizeof(float), 1, fp) != 1 ||
                fread(&ur_y, sizeof(float), 1, fp) != 1)
                throw 1;
            GeoMbr &mbr = mbrs[ii];
            mbr.ll().x() = ll_x;  mbr.ll().y() = ll_y;
            mbr.ur().x() = ur_x;  mbr.ur().y() = ur_y;
        }
    }
    catch (...)
    {
        if (fp)
            fclose(fp);
        return false;
    }
    fclose(fp);

    if (sqlite3_open([sqlDb cStringUsingEncoding:NSASCIIStringEncoding],&db) != SQLITE_OK)
        return false;
    
    return true;
}

// Look areals that pass that point in polygon test
void VectorDatabase::findArealsForPoint(const GeoCoord &coord,ShapeSet &shapes)
{
    for (unsigned int ii=0;ii<mbrs.size();ii++)
    {
        if (mbrs[ii].inside(coord))
        {
            // Load it in and see if it passes
            VectorShapeRef shape = reader->getObjectByIndex(ii, NULL);
            if (shape.get())
            {
                bool keep = false;
                VectorArealRef ar = boost::dynamic_pointer_cast<VectorAreal>(shape);
                if (ar && ar->pointInside(coord))
                    keep = true;
                // Hand it back to the caller.  Up to them to delete it
                if (keep)
                    shapes.insert(ar);
            }
        }
    }
}

// Return the list of IDs that match the given criteria
// We're passing the where clause.  The rest is a select
void VectorDatabase::getMatchingVectors(NSString *query,UIntSet &vecIds)
{
    try {
        sqlhelpers::StatementRead readStmt(db,
                                           [NSString stringWithFormat:@"SELECT vecid from vectors where %@;",query]);
        if (!readStmt.isValid())
            return;
        while (readStmt.stepRow())
            vecIds.insert(readStmt.getInt());
    } 
    catch (...) {
        // This shouldn't happen.  If it does there's some corruption
        // Just give up
    }    
}

// Return the actual vector data that match the criteria
// Caller responsible for deletion
void VectorDatabase::getMatchingVectors(NSString *query,ShapeSet &shapes)
{
    UIntSet vecIds;
    getMatchingVectors(query, vecIds);
    
    for (UIntSet::iterator it = vecIds.begin();it != vecIds.end(); ++it)
    {
        VectorShapeRef shape = getVector(*it,true);
        if (shape.get())
            shapes.insert(shape);
    }
}

}
