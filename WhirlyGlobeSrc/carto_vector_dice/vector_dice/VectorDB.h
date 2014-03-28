//
//  VectorDB.h
//  vector_dice
//
//  Created by Steve Gifford on 1/8/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#ifndef __vector_dice__VectorDB__
#define __vector_dice__VectorDB__

#include <iostream>
#include <vector>
#include <set>
#include "gdal.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"
#include "KompexSQLitePrerequisites.h"
#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include "KompexSQLiteStreamRedirection.h"
#include "KompexSQLiteBlob.h"
#include "KompexSQLiteException.h"

namespace Maply
{

// Vector attribute types
typedef enum {VectorAttrInt=0,VectorAttrString,VectorAttrReal} VectorAttributeType;

class VectorDatabase
{
public:
    VectorDatabase();
    ~VectorDatabase();

    // Set up the data structures we need in the SQLite database
    bool setupDatabase(Kompex::SQLiteDatabase *db,const char *dbSrs,const char *tileSrs,double minX,double minY,double maxX,double maxY,int minLevel,int maxLevel,bool compress);
    
    // Add a style definition (just json, basically)
    void addStyle(const char *name,const char *styleStr);
    
    // Add a layer.  We store those in individual tables.
    int addVectorLayer(const char *);
    
    // Convert OGR vector data to our raw format
    void vectorToDBFormat(std::vector<OGRFeature *> &features,std::vector<unsigned char> &vecData);
    
    // Add the data for a vector tile
    bool addVectorTile(int x,int y,int level,int layerID,const char *data,unsigned int dataLen);
    
    // Create the quadIndex index
    void createIndex();
    
    // Close any open statements and such
    void flush();
    
    // Check this after opening
    bool isValid() { return valid; }
    
protected:
    
    Kompex::SQLiteDatabase *db;
    std::vector<std::string> layerNames;
    
    double minx,miny,maxx,maxy;
    int minLevel,maxLevel;
    bool compress;
    bool valid;
    
    // Precompiled insert statement
    std::vector<Kompex::SQLiteStatement *> insertStmts;
};
    
// Simple compression routine.  Release data when done.
    bool CompressData(void *data,int dataLen,void **retData,int &retDataLen);
    
}

#endif /* defined(__vector_dice__VectorDB__) */
