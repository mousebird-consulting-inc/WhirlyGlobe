/*
 *  LAZQuadReader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
#import <jni.h>
#import "laszip/laszip_api.h"
#import "Maply_jni.h"
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_LAZQuadReader.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

class LAZQuadReader
{
public:
    LAZQuadReader()
    : coordSys(NULL)
    {        
    }
    
    Point3d ll,ur;
    double zOffset;
    int minZoom,maxZoom;
    int minTilePoints,maxTilePoints;
    float pointSize;
    int colorScale;
    int pointType;
    CoordSystem *coordSys;
};

typedef JavaClassInfo<LAZQuadReader> LAZQuadReaderClassInfo;
template<> LAZQuadReaderClassInfo *LAZQuadReaderClassInfo::classInfoObj = NULL;


JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_nativeInit
(JNIEnv *env, jclass cls)
{
    LAZQuadReaderClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReader *readerInst = new LAZQuadReader();
        LAZQuadReaderClassInfo::getClassInfo()->setHandle(env,obj,readerInst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::dispose()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setBounds
(JNIEnv *env, jobject obj, jdouble minX, jdouble minY, jdouble minZ, jdouble maxX, jdouble maxY, jdouble maxZ)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->ll = Point3d(minX,minY,minZ);
        lazReader->ur = Point3d(maxX,maxY,maxZ);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setBounds()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LAZQuadReader_getLL
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return NULL;
        
        return MakePoint3d(env,lazReader->ll);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getLL()");
    }
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LAZQuadReader_getUR
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return NULL;

        return MakePoint3d(env,lazReader->ur);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getUR()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setZOffset
(JNIEnv *env, jobject obj, jdouble zOffset)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->zOffset = zOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setZOffset()");
    }

}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_LAZQuadReader_getZOffset
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0.0;
        
        return lazReader->zOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getZOffset()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setZoomLevels
(JNIEnv *env, jobject obj, jint minZoom, jint maxZoom)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->minZoom = minZoom;
        lazReader->maxZoom = maxZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setZoomLevels()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMinZoom
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->minZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMinZoom()");
    }
    
    return -1;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMaxZoom
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->maxZoom;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMaxZoom()");
    }
    
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setTilePoints
(JNIEnv *env, jobject obj, jint minTilePoints, jint maxTilePoints)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->minTilePoints = minTilePoints;
        lazReader->maxTilePoints = maxTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setTilePoints()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMinTilePoints
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->minTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMinTilePoints()");
    }
    
    return -1;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getMaxTilePoints
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return -1;
        
        return lazReader->maxTilePoints;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getMaxTilePoints()");
    }
    
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setPointSize
(JNIEnv *env, jobject obj, jfloat pointSize)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->pointSize = pointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setPointSize()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_LAZQuadReader_getPointSize
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0.0;
        
        return lazReader->pointSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getPointSize()");
    }
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setColorScale
(JNIEnv *env, jobject obj, jint colorScale)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->colorScale = colorScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setColorScale()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getColorScale
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0;
        
        return lazReader->colorScale;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getColorScale()");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setPointType
(JNIEnv *env, jobject obj, jint pointType)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return;
        
        lazReader->pointType = pointType;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setPointType()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LAZQuadReader_getPointType
(JNIEnv *env, jobject obj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        if (!lazReader)
            return 0;
        
        return lazReader->pointType;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::getPointType()");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_setCoordSystemNative
(JNIEnv *env, jobject obj, jobject coordSysObj)
{
    try
    {
        LAZQuadReaderClassInfo *classInfo = LAZQuadReaderClassInfo::getClassInfo();
        LAZQuadReader *lazReader = classInfo->getObject(env,obj);
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        if (!lazReader || !coordSys)
            return;

        lazReader->coordSys = coordSys;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::setCoordSystemNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LAZQuadReader_processTileNative
(JNIEnv *env, jobject coordAdaptObj, jobject lazObj, jbyteArray data, jobject pointsObj, jobject tileCenterObj)
{
    try
    {
        CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdaptObj);
        LAZQuadReader *lazReader = LAZQuadReaderClassInfo::getClassInfo()->getObject(env,lazObj);
        GeometryRawPoints *points = GeometryRawPointsClassInfo::getClassInfo()->getObject(env,pointsObj);
        Point3d *tileCenterDisp = Point3dClassInfo::getClassInfo()->getObject(env,tileCenterObj);
        if (!coordAdapter || !lazReader || !points || !tileCenterDisp)
            return;

        std::stringstream tileStream;
        jbyte *bytes = env->GetByteArrayElements(data,NULL);
        tileStream.write(reinterpret_cast<const char *>(bytes),env->GetArrayLength(data));
        
        laszip_POINTER thisReader = NULL;
        laszip_BOOL is_compressed;
        laszip_create(&thisReader);
        laszip_open_stream_reader(thisReader,&tileStream,&is_compressed);
        laszip_header_struct *header;
        laszip_get_header_pointer(thisReader,&header);
        bool hasColors = header->point_data_format > 1;
        int count = header->number_of_point_records;

        int vertIdx = points->addAttribute("a_position",GeomRawFloat3Type);
        int elevIdx = points->addAttribute("a_elev",GeomRawFloatType);
        int colorIdx = hasColors ? points->addAttribute("a_color",GeomRawFloat4Type) : -1;

        // Center the coordinates around the tile center
        Point3d locTileCenter((header->min_x+header->max_x)/2.0,(header->min_y+header->max_y)/2.0,0.0);
        Point3d loc3d = CoordSystemConvert3d(lazReader->coordSys, coordAdapter->getCoordSystem(), locTileCenter);
        *tileCenterDisp = coordAdapter->localToDisplay(loc3d);
        
        // We generate a triangle mesh underneath a given tile to provide something to grab
//        MeshBuilder meshBuilder(10,10,Point2d(header->min_x,header->min_y),Point2d(header->max_x,header->max_y),self.coordSys);
        
        long long which = 0;
        double minZ=MAXFLOAT,maxZ=-MAXFLOAT;
        int pointStart = 0;
        while (which < count)
        {
            // Get the point and convert to geocentric
            laszip_seek_point(thisReader,(which+pointStart));
            laszip_read_point(thisReader);
            laszip_point_struct *p;
            laszip_get_point_pointer(thisReader, &p);
            //                double x,y,z;
            //                x = p.GetX(), y = p.GetY(); z = p.GetZ();
            //                trans->TransformEx(1, &x, &y, &z);
            Point3d coord;
            coord.x() = p->X * header->x_scale_factor + header->x_offset;
            coord.y() = p->Y * header->y_scale_factor + header->y_offset;
            coord.z() = p->Z * header->z_scale_factor + header->z_offset;
            coord.z() += lazReader->zOffset;
            
            minZ = std::min(coord.z(),minZ);
            maxZ = std::max(coord.z(),maxZ);
            
            Point3d loc3d = CoordSystemConvert3d(lazReader->coordSys, coordAdapter->getCoordSystem(), coord);
            Point3d dispCoord = coordAdapter->localToDisplay(loc3d);
            Point3d dispCoordCenter = dispCoord - *tileCenterDisp;
            
            float red = 1.0,green = 1.0, blue = 1.0;
            if (hasColors)
            {
                red = p->rgb[0] / lazReader->colorScale;
                green = p->rgb[1] / lazReader->colorScale;
                blue = p->rgb[2] / lazReader->colorScale;
            }
            
            points->addPoint(vertIdx,dispCoordCenter);
            if (hasColors)
                points->addPoint(colorIdx,Vector4f(red,green,blue,1.0));
            points->addValue(elevIdx,(float)coord.z());
            
//            meshBuilder.addPoint(Point3d(coord.x,coord.y,coord.z));
            
            which++;
        }
        
        // Keep track of tile size
        if (minZ == maxZ)
            maxZ += 1.0;
//        @synchronized (self) {
//            TileBoundsInfo tileInfo(tileID);
//            tileInfo.mesh = meshBuilder.makeMesh(layer.viewC);
//            tileInfo.minZ = minZ;  tileInfo.maxZ = maxZ;
//            tileSizes.insert(tileInfo);
//        }
        
        env->ReleaseByteArrayElements(data,bytes, 0);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LAZQuadReader::processTileNative()");
    }
}
