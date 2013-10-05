//
//  ElevationPyramid.cpp
//  elev_assemble_pyramid
//
//  Created by Steve Gifford on 9/6/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#include "ElevationPyramid.h"
#include "zlib.h"

using namespace Kompex;

ElevationPyramid::ElevationPyramid(Kompex::SQLiteDatabase *db,const char *srs,GDALDataType dataType,double minX,double minY,double maxX,double maxY,unsigned int tileSizeX,unsigned int tileSizeY,bool compress,int minLevel,int maxLevel)
: db(db), dataType(dataType), compress(compress), tileSizeX(tileSizeX), tileSizeY(tileSizeY), insertStmt(NULL),
    minLevel(minLevel), maxLevel(maxLevel), minx(minX), miny(minY), maxx(maxX), maxy(maxY), srs(srs)
{
    SQLiteStatement stmt(db);
    
    // Create the manifest (table)
    try {
        stmt.SqlStatement((std::string)"CREATE TABLE manifest (minx REAL, miny REAL, maxx REAL, maxy REAL);");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD tilesizex INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD tilesizey INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD compressed BOOLEAN DEFAULT false NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD format TEXT DEFAULT '' NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD minlevel INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD maxlevel INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD srs TEXT DEFAULT '' NOT NULL;");
        
        char stmtStr[1024];
        sprintf(stmtStr,"INSERT INTO manifest (minx,miny,maxx,maxy,tilesizex,tilesizey,compressed,format,minlevel,maxlevel,srs) VALUES (%f,%f,%f,%f,%d,%d,%d,'%s',%d,%d,'%s');",minX,minY,maxX,maxY,tileSizeX,tileSizeY,1,"int16",minLevel,maxLevel,(srs ? srs : ""));
        stmt.SqlStatement(stmtStr);
        
        stmt.SqlStatement("CREATE TABLE elevationtiles (data BLOB,level INTEGER,x INTEGER,y INTEGER,quadindex INTEGER PRIMARY KEY);");
    } catch (SQLiteException &exc) {
        fprintf(stderr,"Failed to write to database:\n%s\n",exc.GetString().c_str());
        valid = false;
    }
    
    valid = true;
}

ElevationPyramid::ElevationPyramid(Kompex::SQLiteDatabase *db,int newMaxLevel)
: db(db)
{
    SQLiteStatement stmt(db);
    
    try {
        stmt.Sql((std::string)"SELECT * FROM manifest where rowid = 1;");
        if (stmt.FetchRow())
        {
            minx = stmt.GetColumnDouble("minx");
            miny = stmt.GetColumnDouble("miny");
            maxx = stmt.GetColumnDouble("maxx");
            maxy = stmt.GetColumnDouble("maxy");
            tileSizeX = stmt.GetColumnInt("tilesizex");
            tileSizeY = stmt.GetColumnInt("tilesizey");
            compress = stmt.GetColumnInt("compressed");
            minLevel = stmt.GetColumnInt("minlevel");
            maxLevel = stmt.GetColumnInt("minlevel");
            srs = stmt.GetColumnString("srs");
        } else
            valid = false;
        
        // Might need to update the levels
        if (newMaxLevel > maxLevel)
        {
            maxLevel = newMaxLevel;
            SQLiteStatement manStmt(db);
            char stmtStr[1024];
            sprintf(stmtStr, "UPDATE manifest SET maxlevel = %d WHERE rowid = 1;", maxLevel);
            manStmt.Sql((std::string)stmtStr);
            manStmt.Execute();
        }
        
        valid = true;
    } catch (SQLiteException &exc) {
        fprintf(stderr,"Failed to read from database:\n%s\n",exc.GetString().c_str());
        valid = false;
    }
}

// Compress data
// Courtesy: http://stackoverflow.com/questions/6466178/how-to-convert-an-nsdata-to-a-zip-file-with-objective-c/6466832#6466832
bool CompressData(void *data,int dataLen,void **retData,int &retDataLen)
{
    if (dataLen == 0)
        return false;
    
    z_stream strm;
    
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.total_out = 0;
    strm.next_in=(Bytef *)data;
    strm.avail_in = (unsigned int)dataLen;
    
    // Compresssion Levels:
    //   Z_NO_COMPRESSION
    //   Z_BEST_SPEED
    //   Z_BEST_COMPRESSION
    //   Z_DEFAULT_COMPRESSION
    
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY) != Z_OK) return false;
    
    // Should be more than enough
    *retData = malloc(2*dataLen);
    
    do {
        strm.next_out = (Bytef *)*retData;
        strm.avail_out = (unsigned int)2*dataLen;
        
        deflate(&strm, Z_FINISH);
        
    } while (strm.avail_out == 0);
    
    deflateEnd(&strm);
    
    retDataLen = (int)strm.total_out;
    
    return true;
}

bool ElevationPyramid::addElevationTile(void *tileData,int x,int y,int level)
{
    // Calculate a quad index for later use
    int quadIndex = 0;
    for (int iq=0;iq<level;iq++)
        quadIndex += (1<<iq)*(1<<iq);
    quadIndex += y*(1<<level) + x;
    
    if (!insertStmt)
    {
        insertStmt = new SQLiteStatement(db);
        insertStmt->Sql("INSERT INTO elevationtiles (data,level,x,y,quadindex) VALUES (@data,@level,@x,@y,@quadinex);");
    }
    
    // Here we've got data to insert
    if (tileData)
    {
        unsigned int dataSize = sizeof(unsigned short)*tileSizeX*tileSizeY;
        
        if (compress)
        {
            void *compressOut;
            int compressSize=0;
            if (CompressData((void *)tileData, dataSize, &compressOut, compressSize))
            {
                // Now insert the samples into the database as a blob
                try {
                    insertStmt->BindBlob(1, compressOut, compressSize);
                    insertStmt->BindInt(2, level);
                    insertStmt->BindInt(3, x);
                    insertStmt->BindInt(4, y);
                    insertStmt->BindInt(5, quadIndex);
                    insertStmt->Execute();
                    insertStmt->Reset();
                }
                catch (SQLiteException &except)
                {
                    fprintf(stderr,"Failed to write blob to database:\n%s\n",except.GetString().c_str());
                    return false;
                }            
            } else
                return false;
        }
    } else {
        // No data, so leave the blob empty.  This means the tile is all at zero.
        try {
            insertStmt->BindBlob(1, NULL, 0);
            insertStmt->BindInt(2, level);
            insertStmt->BindInt(3, x);
            insertStmt->BindInt(4, y);
            insertStmt->BindInt(5, quadIndex);
            insertStmt->Execute();
            insertStmt->Reset();
        }
        catch (SQLiteException &except)
        {
            fprintf(stderr,"Failed to write blob to database:\n%s\n",except.GetString().c_str());
            return false;
        }
    }
    
//    if (insertStmt)
//        delete insertStmt;
//    insertStmt = NULL;
    
    return true;
}

void ElevationPyramid::flush()
{
    if (insertStmt)
        delete insertStmt;
    insertStmt = NULL;
}

void ElevationPyramid::createIndex()
{
//    try {
//        SQLiteStatement stmt(db);
//        stmt.SqlStatement((std::string)"CREATE INDEX quadindex on elevationtiles (quadindex);");
//    }
//    
//    catch (SQLiteException &exc)
//    {
//        fprintf(stderr,"Failed to write to database:\n%s\n",exc.GetString().c_str());
//    }
}
