//
//  VectorDB.cpp
//  vector_dice
//
//  Created by Steve Gifford on 1/8/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#include "VectorDB.h"
#include "zlib.h"

using namespace Kompex;

namespace Maply
{
    
VectorDatabase::VectorDatabase()
: db(NULL), valid(false)
{
}
    
VectorDatabase::~VectorDatabase()
{
    flush();
}
    
bool VectorDatabase::setupDatabase(Kompex::SQLiteDatabase *inDb,const char *dbSrs,const char *tileSrs,double minX,double minY,double maxX,double maxY,int minLevel,int maxLevel,bool inCompress)
{
    db = inDb;
    compress = inCompress;
    
    // Create the manifest (table)
    try {
        SQLiteStatement stmt(db);

        // Manifest
        stmt.SqlStatement((std::string)"CREATE TABLE manifest (minx REAL, miny REAL, maxx REAL, maxy REAL);");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD compressed BOOLEAN DEFAULT false NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD minlevel INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD maxlevel INTEGER DEFAULT 0 NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD gridsrs TEXT DEFAULT '' NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD tilesrs TEXT DEFAULT '' NOT NULL;");
        stmt.SqlStatement((std::string)"ALTER TABLE manifest ADD version TEXT DEFAULT '' NOT NULL;");
        
        char stmtStr[4096];
        sprintf(stmtStr,"INSERT INTO manifest (minx,miny,maxx,maxy,compressed,minlevel,maxlevel,gridsrs,tilesrs,version) VALUES (%f,%f,%f,%f,%d,%d,%d,'%s','%s','%s');",minX,minY,maxX,maxY,compress,minLevel,maxLevel,(dbSrs ? dbSrs : ""),(tileSrs ? tileSrs : ""),"1.0");
        stmt.SqlStatement(stmtStr);
        
        // Layer table
        stmt.SqlStatement("CREATE TABLE layers (name TEXT);");
        
        // Attribute table
        stmt.SqlStatement("CREATE TABLE attributes (name TEXT,type INTEGER);");
        
        // Style table
        stmt.SqlStatement("CREATE TABLE styles (name TEXT,style TEXT);");
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }

    return true;
}
    
void VectorDatabase::addStyle(const char *name,const char *styleStr)
{
    try {
        SQLiteStatement stmt(db);
        stmt.SqlStatement((std::string)"INSERT INTO styles (name,style) VALUES ('" + name + "','" + styleStr + "');");
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write style to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }
}
    
int VectorDatabase::addVectorLayer(const char *name)
{
    try {
        SQLiteStatement stmt(db);
        
        std::string tableName = (std::string)name + "_table";
        stmt.SqlStatement("CREATE TABLE " + tableName + " (data BLOB,level INTEGER,x INTEGER,y INTEGER,quadindex INTEGER PRIMARY KEY);");
        
        // Create a new layer table and update the layer list
        layerNames.push_back(name);
        stmt.SqlStatement((std::string)"INSERT INTO layers (name) VALUES ('" + name + "');");
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }
    
    insertStmts.push_back(NULL);
    return (int)layerNames.size()-1;
}
    
bool VectorDatabase::getAttribute(const char *name,Attribute &attr)
{
    Attribute search;
    search.name = name;
    std::set<Attribute>::iterator it = attributes.find(search);
    if (it != attributes.end())
    {
        attr = *it;
        return true;
    }

    return false;
}
    
VectorDatabase::Attribute VectorDatabase::addAttribute(const char *name,VectorAttributeType type)
{
    Attribute search;
    search.name = name;
    std::set<Attribute>::iterator it = attributes.find(search);
    if (it != attributes.end())
        return *it;
    
    // Add the attribute
    search.index = (int)attributes.size();
    search.type = type;
    attributes.insert(search);
    try {
        SQLiteStatement stmt(db);
        stmt.SqlStatement((std::string)"INSERT INTO attributes (name,type) VALUES ('" + name + "'," + std::to_string(type) + ");");
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }
    
    return search;
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
    
// Vector data that shares exactly the same attributes
class VectorChunk
{
public:
    OGRFeature *mainFeature() const { return features[0]; }
    
    // Number of non-empty attributes (we hope)
    int numAttributes() const
    {
        return mainFeature()->GetFieldCount();
    }
    
    // Calculate the number of features, including individual elements of mult-features
    int numFeatures() const
    {
        int num = 0;
        for (unsigned int ii=0;ii<features.size();ii++)
        {
            OGRFeature *feat = features[ii];
            switch (feat->GetGeometryRef()->getGeometryType())
            {
                case wkbPoint:
                case wkbLineString:
                case wkbPolygon:
                    num++;
                    break;
                case wkbMultiPoint:
                case wkbMultiLineString:
                case wkbMultiPolygon:
                {
                    OGRGeometryCollection *geomC = (OGRGeometryCollection *)feat->GetGeometryRef();
                    num += geomC->getNumGeometries();
                }
                    break;
                default:
                    break;
            }
        }
        
        return num;
    }
    
    // Geometry type.  We disentable the multi-types.
    OGRwkbGeometryType geomType() const
    {
        switch (mainFeature()->GetGeometryRef()->getGeometryType())
        {
            case wkbPoint:
            case wkbMultiPoint:
                return wkbPoint;
                break;
            case wkbLineString:
            case wkbMultiLineString:
                return wkbLineString;
                break;
            case wkbPolygon:
            case wkbMultiPolygon:
                return wkbPolygon;
                break;
            default:
                return wkbUnknown;
        }
    }
    
    std::vector<OGRFeature *> features;
};
    
struct VectorChunkCompare
{
    // Comparison operator used to sort within a set
    bool operator() (const VectorChunk *chunkA,const VectorChunk *chunkB)
    {
        OGRFeature *featA = chunkA->mainFeature();
        OGRFeature *featB = chunkB->mainFeature();
        
        // First check geometry type
        OGRwkbGeometryType typeA = featA->GetGeometryRef()->getGeometryType();
        OGRwkbGeometryType typeB = featB->GetGeometryRef()->getGeometryType();
        if (typeA != typeB)
            return typeA < typeB;
        
        // Number of attributes
        int numA = chunkA->numAttributes();
        int numB = chunkB->numAttributes();
        if (numA != numB)
            return numA < numB;
        
        // Just evaluate the attributes next
        for (unsigned int ii=0;ii<featA->GetFieldCount();ii++)
        {
            OGRFieldDefn featADefn = featA->GetFieldDefnRef(ii);
            int fieldIdxB = featB->GetFieldIndex(featADefn.GetNameRef());
            if (fieldIdxB < 0)
                return false;
            OGRFieldDefn featBDefn = featB->GetFieldDefnRef(fieldIdxB);
            if (featADefn.GetType() != featBDefn.GetType())
                return featADefn.GetType() < featBDefn.GetType();
            
            OGRField *fieldA = featA->GetRawFieldRef(ii);
            OGRField *fieldB = featB->GetRawFieldRef(fieldIdxB);
            switch (featADefn.GetType())
            {
                case OFTInteger:
                    return fieldA->Integer < fieldB->Integer;
                    break;
                case OFTReal:
                    return fieldA->Real < fieldB->Real;
                    break;
                case OFTString:
                {
                    std::string strA = fieldA->String;
                    std::string strB = fieldB->String;
                    return strA < strB;
                }
                    break;
                default:
                    // Sure, why not?
                    break;
            }
        }
        
        return false;
    };
};
    
typedef std::set<VectorChunk *,VectorChunkCompare> VectorChunkSet;
    
// Add a 32 bit integer to the data
void AddToData(std::vector<unsigned char> &vecData,int iVal)
{
    const char *chars = (const char *)&iVal;
    for (unsigned int ii=0;ii<4;ii++)
        vecData.push_back(chars[ii]);
}
    
// Add a 32 bit float to the data
void AddToData(std::vector<unsigned char> &vecData,float fVal)
{
    const char *chars = (const char *)&fVal;
    for (unsigned int ii=0;ii<4;ii++)
        vecData.push_back(chars[ii]);
}
    
// Add a string to the data
void AddToData(std::vector<unsigned char> &vecData,const char *sVal)
{
    AddToData(vecData,(int)strlen(sVal));
    while (*sVal)
    {
        vecData.push_back(*sVal);
        sVal++;
    }
}
    
// Add a point to the data
void AddToData(std::vector<unsigned char> &vecData,OGRPoint *pt)
{
    // X and Y
    AddToData(vecData,(float)(pt->getX() / 180.0 * M_PI));
    AddToData(vecData,(float)(pt->getY() / 180.0 * M_PI));
}
    
// Add a line string to the data
void AddToData(std::vector<unsigned char> &vecData,OGRLineString *line)
{
    // Number of points and then points
    AddToData(vecData,(int)line->getNumPoints());
    for (unsigned int pp=0;pp<line->getNumPoints();pp++)
    {
        OGRPoint pt;
        line->getPoint(pp, &pt);
        AddToData(vecData,(float)(pt.getX() / 180.0 * M_PI));
        AddToData(vecData,(float)(pt.getY() / 180.0 * M_PI));
    }
}
    
// Add a polygon to the data
void AddToData(std::vector<unsigned char> &vecData,OGRPolygon *poly)
{
    // Number of loops
    int numLoops = 1+poly->getNumInteriorRings();
    AddToData(vecData,(int)numLoops);
    for (int ll=0;ll<numLoops;ll++)
    {
        OGRLinearRing *ring = (ll == 0) ? poly->getExteriorRing() : poly->getInteriorRing(ll-1);
        // Number of points and then points
        AddToData(vecData,(int)ring->getNumPoints());
        for (unsigned int pp=0;pp<ring->getNumPoints();pp++)
        {
            OGRPoint pt;
            ring->getPoint(pp, &pt);
            AddToData(vecData,(float)(pt.getX() / 180.0 * M_PI));
            AddToData(vecData,(float)(pt.getY() / 180.0 * M_PI));
        }
    }
}
    
// Add the attributes for a feature to the data
void AddToData(std::vector<unsigned char> &vecData,std::vector<const VectorDatabase::Attribute> &attrsPresent,OGRFeature *thisFeat)
{
    // First, the attributes
    for (int fi=0;fi<attrsPresent.size();fi++)
    {
        const VectorDatabase::Attribute &thisAttr = attrsPresent[fi];
        int fieldIdx = thisFeat->GetFieldIndex(thisAttr.name.c_str());
        if (fieldIdx < 0)
            throw (std::string)"Indexing problem when sorting out attributes.";
        switch (thisAttr.type)
        {
            case VectorAttrInt:
            {
                int iVal = thisFeat->GetFieldAsInteger(fieldIdx);
                AddToData(vecData,iVal);
            }
                break;
            case VectorAttrReal:
            {
                float fVal = thisFeat->GetFieldAsDouble(fieldIdx);
                AddToData(vecData,fVal);
            }
                break;
            case VectorAttrString:
            {
                const char *sVal = thisFeat->GetFieldAsString(fieldIdx);
                AddToData(vecData,sVal);
            }
                break;
        }
    }
    
}
    
void VectorDatabase::vectorToDBFormat(OGRLayer *srcLayer,std::vector<unsigned char> &vecData)
{
    VectorChunkSet vectorChunks;

    for (unsigned int ii=0;ii<srcLayer->GetFeatureCount();ii++)
    {
        OGRFeature *feature = srcLayer->GetFeature(ii);
        
        // Look for a match
        VectorChunk testChunk;
        testChunk.features.push_back(feature);
        VectorChunkSet::iterator it = vectorChunks.find(&testChunk);
        // Add it to the existing one
        if (it != vectorChunks.end())
        {
            VectorChunk *oldChunk = *it;
            oldChunk->features.push_back(feature);
        } else {
            // Add it new
            VectorChunk *newChunk = new VectorChunk();
            newChunk->features.push_back(feature);
            vectorChunks.insert(newChunk);
        }
    }
    
    if (vectorChunks.empty())
        return;

    // Start with the number of chunks
    AddToData(vecData,(int)vectorChunks.size());
    
    // Work through the chunks, writing as we go
    for (VectorChunkSet::iterator it = vectorChunks.begin(); it != vectorChunks.end(); ++it)
    {
        VectorChunk *chunk = *it;
        
        // Figure out which attributes are in this chunk
        std::vector<const Attribute> attrsPresent;
        std::vector<std::string> fieldNames;
        OGRFeature *feat = chunk->mainFeature();
        for (unsigned int fi=0;fi<feat->GetFieldCount();fi++)
        {
            OGRFieldDefn fieldDefn = feat->GetFieldDefnRef(fi);
            fieldNames.push_back(fieldDefn.GetNameRef());
            VectorAttributeType attrType;
            switch (fieldDefn.GetType())
            {
                case OFTInteger:
                    attrType = VectorAttrInt;
                    break;
                case OFTReal:
                    attrType = VectorAttrReal;
                    break;
                case OFTString:
                    attrType = VectorAttrString;
                    break;
                default:
                    throw (std::string)"Unhandled attribute type in vectorToDBFormat()";
                    break;
            }
            Attribute theAttr = addAttribute(fieldDefn.GetNameRef(), attrType);
            attrsPresent.push_back(theAttr);
        }
        
        // Number of attributes present and then the attribute indices
        AddToData(vecData,(int)attrsPresent.size());
        for (unsigned int fi=0;fi<attrsPresent.size();fi++)
            AddToData(vecData,(int)attrsPresent[fi].index);
        
        // The data type
        OGRwkbGeometryType geomType = chunk->geomType();
        AddToData(vecData,(int)geomType);
        
        // Number of features of this data type.  We merge the multi-types
        AddToData(vecData,(int)chunk->numFeatures());
        
        // Now for the features
        for (unsigned int ii=0;ii<chunk->features.size();ii++)
        {
            OGRFeature *thisFeat = chunk->features[ii];

            // Attributes for each piece of geometry, then geometry
            OGRGeometry *thisGeom = thisFeat->GetGeometryRef();
            switch (thisGeom->getGeometryType())
            {
                case wkbPoint:
                    AddToData(vecData,attrsPresent,thisFeat);
                    AddToData(vecData,(OGRPoint *)thisGeom);
                    break;
                case wkbLineString:
                    AddToData(vecData,attrsPresent,thisFeat);
                    AddToData(vecData,(OGRLineString *)thisGeom);
                    break;
                case wkbPolygon:
                    AddToData(vecData,attrsPresent,thisFeat);
                    AddToData(vecData,(OGRPolygon *)thisGeom);
                    break;
                case wkbMultiPoint:
                {
                    OGRMultiPoint *pts = (OGRMultiPoint *)thisGeom;
                    for (unsigned int mi=0;mi<pts->getNumGeometries();mi++)
                    {
                        AddToData(vecData,attrsPresent,thisFeat);
                        AddToData(vecData, (OGRPoint *)pts->getGeometryRef(mi));
                    }
                }
                    break;
                case wkbMultiLineString:
                {
                    OGRMultiLineString *lins = (OGRMultiLineString *)thisGeom;
                    for (unsigned int mi=0;mi<lins->getNumGeometries();mi++)
                    {
                        AddToData(vecData,attrsPresent,thisFeat);
                        AddToData(vecData, (OGRLineString *)lins->getGeometryRef(mi));
                    }
                }
                    break;
                case wkbMultiPolygon:
                {
                    OGRMultiPolygon *polys = (OGRMultiPolygon *)thisGeom;
                    for (unsigned int mi=0;mi<polys->getNumGeometries();mi++)
                    {
                        AddToData(vecData,attrsPresent,thisFeat);
                        AddToData(vecData, (OGRPolygon *)polys->getGeometryRef(mi));
                    }
                }
                    break;
                default:
                    throw (std::string)"Unhandled geometry type.";
                    break;
            }
        }
    }

    // Clean up the sorted vectors
    for (VectorChunkSet::iterator it = vectorChunks.begin(); it != vectorChunks.end(); ++it)
        delete *it;
}

bool VectorDatabase::addVectorTile(int x,int y,int level,int layerID,const char *data,unsigned int dataLen)
{
    // Calculate a quad index for later use
    int quadIndex = 0;
    for (int iq=0;iq<level;iq++)
        quadIndex += (1<<iq)*(1<<iq);
    quadIndex += y*(1<<level) + x;

    try {
        if (!insertStmts[layerID])
        {
            insertStmts[layerID] = new SQLiteStatement(db);
            insertStmts[layerID]->Sql((std::string)"INSERT INTO " + layerNames[layerID] + "_table (data,level,x,y,quadindex) VALUES (@data,@level,@x,@y,@quadinex);");
        }
        SQLiteStatement *insertStmt = insertStmts[layerID];

        // Compress if necessary
        if (compress)
        {
            void *compressOut;
            int compressSize=0;
            if (CompressData((void *)data, dataLen, &compressOut, compressSize))
            {
                // Now insert the samples into the database as a blob
                insertStmt->BindBlob(1, compressOut, compressSize);
                insertStmt->BindInt(2, level);
                insertStmt->BindInt(3, x);
                insertStmt->BindInt(4, y);
                insertStmt->BindInt(5, quadIndex);
                insertStmt->Execute();
                insertStmt->Reset();
            } else {
                // Insert the uncompressed data
                insertStmt->BindBlob(1, (void *)data, dataLen);
                insertStmt->BindInt(2, level);
                insertStmt->BindInt(3, x);
                insertStmt->BindInt(4, y);
                insertStmt->BindInt(5, quadIndex);
                insertStmt->Execute();
                insertStmt->Reset();
            }
        }
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }
    catch (...)
    {
        std::string errorStr = "Unknown error";
        valid = false;
        throw errorStr;
    }
    
    return true;
}

void VectorDatabase::flush()
{
    try {
        for (unsigned int ii=0;ii<insertStmts.size();ii++)
            if (insertStmts[ii])
            {
                delete insertStmts[ii];
                insertStmts[ii] = NULL;
            }
        db->Close();
    }
    catch (SQLiteException &exc)
    {
        std::string errorStr = (std::string)"Failed to write to database:\n" + exc.GetString();
        valid = false;
        throw errorStr;
    }
    catch (...)
    {
        std::string errorStr = (std::string)"Unknown error";
        valid = false;
        throw errorStr;
    }
}
    
}
