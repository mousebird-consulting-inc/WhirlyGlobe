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
            
            switch (featADefn.GetType())
            {
                case OFTInteger:
                {
                    int intA = featA->GetFieldAsInteger(ii);
                    int intB = featB->GetFieldAsInteger(fieldIdxB);
                    if (intA != intB)
                        return intA < intB;
                }
                    break;
                case OFTReal:
                {
                    double realA = featA->GetFieldAsDouble(ii);
                    double realB = featB->GetFieldAsDouble(fieldIdxB);
                    if (realA != realB)
                        return realA < realB;
                }
                    break;
                case OFTString:
                {
                    std::string strA = featA->GetFieldAsString(ii);
                    std::string strB = featB->GetFieldAsString(fieldIdxB);
                    if (strA != strB)
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
    int len = (int)strlen(sVal);
    int extra = (4 - len % 4) % 4;
    AddToData(vecData,len+extra);
    while (*sVal)
    {
        vecData.push_back(*sVal);
        sVal++;
    }
    for (unsigned int ii=0;ii<extra;ii++)
        vecData.push_back(0);
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
    
// Information about attributes in a given tile
class VectorTileAttrInfo
{
public:
    // Attribute table
    typedef std::map<std::string,unsigned int> StringValMap;
    StringValMap stringValMap;
    
    // Find or add a string
    unsigned int getAddString(const std::string &str)
    {
        StringValMap::iterator it = stringValMap.find(str);
        if (it != stringValMap.end())
            return it->second;
        unsigned int idx = (unsigned int)stringValMap.size();
        stringValMap[str] = idx;
        return idx;
    }
    
    // Add a given attribute to the data stream
    void addAttribute(OGRFeature *feat,int fieldID,std::vector<unsigned char> &data)
    {
        // The name points into an index
        int nameIdx = getAddString(feat->GetFieldDefnRef(fieldID)->GetNameRef());
        AddToData(data,nameIdx);
        OGRFieldType fieldType = feat->GetFieldDefnRef(fieldID)->GetType();

        // Field type and value
        switch (fieldType)
        {
            case OFTInteger:
            {
                AddToData(data, (int)0);
                int iVal = feat->GetFieldAsInteger(fieldID);
                AddToData(data,iVal);
            }
                break;
            case OFTReal:
            {
                AddToData(data, (int)1);
                float fVal = feat->GetFieldAsDouble(fieldID);
                AddToData(data,fVal);
            }
                break;
            case OFTString:
            {
                AddToData(data, (int)2);
                int valIdx = getAddString(feat->GetFieldAsString(fieldID));
                AddToData(data,(int)valIdx);
            }
                break;
            default:
                throw (std::string)"Unhandled attribute type in vectorToDBFormat()";
                break;
        }
    }
    
    // Convert the attribute data into raw data
    void convert(std::vector<unsigned char> &data)
    {
        // Strings first
        std::vector<std::string> strs(stringValMap.size());
        for (StringValMap::iterator it = stringValMap.begin(); it != stringValMap.end(); ++it)
            strs[it->second] = it->first;
        AddToData(data, (int)strs.size());
        for (unsigned int ii=0;ii<strs.size();ii++)
            AddToData(data, strs[ii].c_str());
    }
};
    
void VectorDatabase::vectorToDBFormat(std::vector<OGRFeature *> &features,std::vector<unsigned char> &retData)
{
    VectorChunkSet vectorChunks;

    for (unsigned int ii=0;ii<features.size();ii++)
    {
        OGRFeature *feature = features[ii];
        
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
    
    // Used to consolidate attribute and string values in front
    VectorTileAttrInfo attrInfo;
    
    std::vector<unsigned char> vecData;
    // Start with the number of chunks
    AddToData(vecData,(int)vectorChunks.size());
    
    // Work through the chunks, writing as we go
    for (VectorChunkSet::iterator it = vectorChunks.begin(); it != vectorChunks.end(); ++it)
    {
        VectorChunk *chunk = *it;

        // Write all the attributes for this chunk.  They'll all be identical
        OGRFeature *feat = chunk->mainFeature();
        AddToData(vecData, (int)feat->GetFieldCount());
        for (unsigned int fi=0;fi<feat->GetFieldCount();fi++)
        {
            attrInfo.addAttribute(feat,fi,vecData);
        }
        
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
                    AddToData(vecData,(OGRPoint *)thisGeom);
                    break;
                case wkbLineString:
                    AddToData(vecData,(OGRLineString *)thisGeom);
                    break;
                case wkbPolygon:
                    AddToData(vecData,(OGRPolygon *)thisGeom);
                    break;
                case wkbMultiPoint:
                {
                    OGRMultiPoint *pts = (OGRMultiPoint *)thisGeom;
                    for (unsigned int mi=0;mi<pts->getNumGeometries();mi++)
                    {
                        AddToData(vecData, (OGRPoint *)pts->getGeometryRef(mi));
                    }
                }
                    break;
                case wkbMultiLineString:
                {
                    OGRMultiLineString *lins = (OGRMultiLineString *)thisGeom;
                    for (unsigned int mi=0;mi<lins->getNumGeometries();mi++)
                    {
                        AddToData(vecData, (OGRLineString *)lins->getGeometryRef(mi));
                    }
                }
                    break;
                case wkbMultiPolygon:
                {
                    OGRMultiPolygon *polys = (OGRMultiPolygon *)thisGeom;
                    for (unsigned int mi=0;mi<polys->getNumGeometries();mi++)
                    {
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

    attrInfo.convert(retData);
    retData.insert(retData.end(), vecData.begin(), vecData.end());
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

                // Note: Eh?
//                free(compressOut);
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
