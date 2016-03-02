/*
 *  ElevationChunk.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "ElevationCesiumChunk.h"
#import "ElevationCesiumFormat.h"
#import "oct.h"
#import "WhirlyOctEncoding.h"
#import "GridClipper.h"
#import "Tesselator.h"

using namespace WhirlyKit;

static inline int16_t decodeZigZag(uint16_t encodedValue)
{
    return (encodedValue >> 1) ^ (-(encodedValue & 1));
}

static inline void decodeHighWaterMark(vector<uint32_t> encoded, vector<uint32_t> &decoded)
{
// Taken from
// https://books.google.es/books?id=0bTMBQAAQBAJ&lpg=PA450&ots=bxAyZK_cSz&dq=high%20water%20mark%20encoding&pg=PA448#v=onepage&q&f=false

	uint32_t nextHighWaterMark = 0;

	for (vector<uint32_t>::iterator it = encoded.begin();
			it != encoded.end(); ++it)
	{
		uint32_t code = *it;

		decoded.push_back(nextHighWaterMark - code);

		if (code == 0) ++nextHighWaterMark;
	}
}

static inline void oct_normalize(float vec[3]) {
	float len = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

static inline Point3f canonical_oct_decode(uint8_t x, uint8_t y)
{
	Snorm<snormSize> projected[2];
	projected[0] = Snorm<snormSize>::fromBits(x);
	projected[1] = Snorm<snormSize>::fromBits(y);

	float vec[3];

	octDecode(projected, vec);
	oct_normalize(vec);

	return Point3f(vec[0], vec[1], vec[2]);
}


@implementation WhirlyKitElevationCesiumChunk

- (id)initWithCesiumData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY
{
	if (self = [super init]) {
		_sizeX = sizeX;
		_sizeY = sizeY;

		[self readData:(uint8_t *) [data bytes] length:[data length]];
        _scale = 1.0;
	}

	return self;
}

- (void)setScale:(float)scale
{
    _scale = scale;
}

- (void)readData:(uint8_t *)data length:(size_t)length
{
	// This tool may be useful to compare values read with Cesium's JS code
	// https://github.com/jmnavarro/cesium-quantized-mesh-terrain-format-logger
	//

	uint8_t *startData = data;

	// QuantizedMeshHeader
	// ====================
	CesiumQuantizedMeshHeader header = *(CesiumQuantizedMeshHeader *)data;
	data += sizeof(CesiumQuantizedMeshHeader);

	// VertexData
	// ====================
	uint32_t vertexCount = CFSwapInt32LittleToHost(*(uint32_t *)data);
	data += sizeof(uint32_t);


	BOOL use32bits = NO;
	int indexSize = sizeof(uint16_t);

	if (vertexCount > 64 * 1024)
	{
		use32bits = YES;
		indexSize = sizeof(uint32_t);
	}

	_mesh = VectorTriangles::createTriangles();
	_mesh->pts.reserve(vertexCount);

	uint16_t *horizontalCoords = (uint16_t *)data;
	uint16_t *verticalCoords   = ((uint16_t *)data) + vertexCount;
	uint16_t *heights          = ((uint16_t *)data) + (vertexCount * 2);

	const double MaxValue = 32767.0;

	int16_t u = 0;	// horizontal value
	int16_t v = 0;	// vertical value
	int16_t h = 0;	// height value

	for (int i = 0; i < vertexCount; ++i)
	{
		// According to the forums, it's zig-zag AND delta encoded
		// https://groups.google.com/d/msg/cesium-dev/IpcBEvjt-DA/98D0E8c0ET0J

		u += decodeZigZag(CFSwapInt16LittleToHost(horizontalCoords[i]));
		v += decodeZigZag(CFSwapInt16LittleToHost(verticalCoords[i]));
		h += decodeZigZag(CFSwapInt16LittleToHost(heights[i]));

		int posX = _sizeX * (u / MaxValue);
		int posY = _sizeY * (v / MaxValue);

		// height holds the absolute elevation.
		// Documentation makes no mention of the unit
		double heightRatio = h / MaxValue;
		float height = heightRatio * (header.MaximumHeight - header.MinimumHeight) + header.MinimumHeight;

		Point3f pt(posX, posY, height);
		_mesh->pts.push_back(pt);
	}

	data += sizeof(uint16_t) * vertexCount * 3;


	// VertexData(16/32)
	// ====================

	// Handle padding
	uint32_t currentPosition = data - startData;

	if (currentPosition % indexSize != 0) {
		data += (indexSize - (currentPosition % indexSize));
	}

	uint32_t triangleCount = CFSwapInt32LittleToHost(*(uint32_t *)data);
	data += sizeof(uint32_t);

	uint32_t *indices32 = (uint32_t *)data;
	uint16_t *indices16 = (uint16_t *)data;

	// jump to end
	data += triangleCount * indexSize * 3;

	// store the raw serie in a vector, then decode the high water mark encoding
	vector<uint32_t> encodedInd;
	encodedInd.reserve(triangleCount * indexSize * 3);

	if (use32bits)
	{
		while ((uint8_t *)indices32 < data)
		{
			encodedInd.push_back(CFSwapInt32LittleToHost(indices32[0]));
			encodedInd.push_back(CFSwapInt32LittleToHost(indices32[1]));
			encodedInd.push_back(CFSwapInt32LittleToHost(indices32[2]));

			indices32 += 3;
		}
	}
	else
	{
		while ((uint8_t *)indices16 < data)
		{
			encodedInd.push_back(CFSwapInt16LittleToHost(indices16[0]));
			encodedInd.push_back(CFSwapInt16LittleToHost(indices16[1]));
			encodedInd.push_back(CFSwapInt16LittleToHost(indices16[2]));

			indices16 += 3;
		}
	}

	// decode high water mark encoding
	vector<uint32_t> decodedInd;
	decodedInd.reserve(triangleCount * indexSize * 3);

	decodeHighWaterMark(encodedInd, decodedInd);

	// dump to triangles
	_mesh->tris.reserve(triangleCount);

	for (vector<uint32_t>::iterator it = decodedInd.begin();
			it != decodedInd.end(); )
	{
		VectorTriangles::Triangle tri;
		tri.pts[0] = *it; it++;
		tri.pts[1] = *it; it++;
		tri.pts[2] = *it; it++;

		_mesh->tris.push_back(tri);
	}


	// EdgeIndices(16/32)
	// ====================

	_westVertices  = [self readVertexList:&data is32:use32bits];
	_southVertices = [self readVertexList:&data is32:use32bits];
	_eastVertices  = [self readVertexList:&data is32:use32bits];
	_northVertices = [self readVertexList:&data is32:use32bits];

	// Extensions
	// ===========
	const static int kOctEncodedVertexNormals = 1;

	while ((data - startData) < length) {
		uint8_t extensionId = *(uint8_t *)data;
		data += sizeof(uint8_t);
		uint32_t extensionLength = CFSwapInt32LittleToHost(*(uint32_t *)data);
		data += sizeof(uint32_t);

		switch (extensionId) {
			case kOctEncodedVertexNormals:
			{
				uint8_t *normals = (uint8_t *)data;

				_normals.reserve(vertexCount);

				for (int i = 0; i < 2*vertexCount; )
				{
					//TODO(JM) official implementation produces different values than Cesium's one
					// Which is the good one?
					Point3f n = OctDecode(normals[i], normals[i+1]);
					//Point3f n = canonical_oct_decode(normals[i], normals[i+1]);

//					NSLog(@"[%d, %d]%f, %f, %f", normals[i], normals[i+1], n.x(), n.y(), n.z());

					_normals.push_back(n);

					i += 2;
				}
				break;
			}
			default:
				break;
		}

		data += extensionLength;
	}
}

- (vector<uint32_t>)readVertexList:(Byte **)dataRef is32:(BOOL)is32
{
	vector<uint32_t> list;
	Byte *data = *dataRef;

	uint32_t vertexCount = CFSwapInt32LittleToHost(*(uint32_t *)data);
	data += sizeof(uint32_t);
	*dataRef += sizeof(uint32_t);

	if (is32)
	{
		uint32_t *indices = (uint32_t *)data;

		for (int i = 0; i < vertexCount; ++i)
			list.push_back(CFSwapInt32LittleToHost(indices[i]));

		*dataRef += vertexCount * sizeof(uint32_t);
	}
	else
	{
		uint16_t *indices = (uint16_t *)data;

		for (int i = 0; i < vertexCount; ++i)
			list.push_back(CFSwapInt16LittleToHost(indices[i]));

		*dataRef += vertexCount * sizeof(uint16_t);
	}

	return list;
}

- (float)elevationAtX:(int)x y:(int)y
{
	//TODO
	return -1;
}

- (float)interpolateElevationAtX:(float)x y:(float)y
{
	//TODO
	return -1;
}

- (void)generateDrawables:(WhirlyKit::ElevationDrawInfo *)drawInfo chunk:(BasicDrawable **)draw skirts:(BasicDrawable **)skirtDraw
{
    // Size of each chunk
    Point2f chunkSize = drawInfo->parentMbr.ur() - drawInfo->parentMbr.ll();
    
    // We need the corners in geographic for the cullable
    Point2d chunkLL(drawInfo->parentMbr.ll().x(),drawInfo->parentMbr.ll().y());
    Point2d chunkUR(drawInfo->parentMbr.ur().x(),drawInfo->parentMbr.ur().y());
    //    Point2d chunkMid = (chunkLL+chunkUR)/2.0;
    CoordSystem *sceneCoordSys = drawInfo->coordAdapter->getCoordSystem();
    GeoCoord geoLL(drawInfo->coordSys->localToGeographic(Point3d(chunkLL.x(),chunkLL.y(),0.0)));
    GeoCoord geoUR(drawInfo->coordSys->localToGeographic(Point3d(chunkUR.x(),chunkUR.y(),0.0)));

    // Texture increment for each "pixel"
    TexCoord texIncr(1.0/(float)_sizeX,1.0/(float)_sizeY);

    // We'll set up and fill in the drawable
    BasicDrawable *chunk = new BasicDrawable("Tile Quad Loader",_mesh->pts.size(),_mesh->tris.size());
    if (drawInfo->useTileCenters)
        chunk->setMatrix(&drawInfo->transMat);
    
    if (drawInfo->activeTextures > 0)
        chunk->setTexId(drawInfo->activeTextures-1, EmptyIdentity);
    chunk->setDrawOffset(drawInfo->drawOffset);
    chunk->setDrawPriority(drawInfo->drawPriority);
    chunk->setVisibleRange(drawInfo->minVis, drawInfo->maxVis);
    chunk->setAlpha(drawInfo->hasAlpha);
    chunk->setColor(drawInfo->color);
    chunk->setLocalMbr(Mbr(Point2f(geoLL.x(),geoLL.y()),Point2f(geoUR.x(),geoUR.y())));
    chunk->setProgram(drawInfo->programId);
    int elevEntry = 0;
    if (drawInfo->includeElev)
        elevEntry = chunk->addAttribute(BDFloatType, "a_elev");
    // Single level mode uses Z to sort out priority
    //        if (singleLevel != -1)
    //        {
    //            chunk->setRequestZBuffer(true);
    //            chunk->setWriteZBuffer(true);
    //        }
    
    chunk->setType(GL_TRIANGLES);
    
    // Work through the points
    for (unsigned int ip=0;ip<_mesh->pts.size();ip++)
    {
        // Convert the point to display space
        const Point3f &pt = _mesh->pts[ip];
        Point3d loc3d(chunkLL.x()+pt.x()/_sizeX * chunkSize.x(),chunkLL.y()+pt.y()/_sizeY * chunkSize.y(),pt.z()*_scale);
        Point3d disp3d = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(drawInfo->coordSys,sceneCoordSys,loc3d));
        Point3d normUp = drawInfo->coordAdapter->normalForLocal(loc3d);
        
        // Need some axes to reproject the normal
        Point3d east = Point3d(0,0,1).cross(normUp);
        Point3d north = normUp.cross(east);
        
        // Texture runs across the tile [0,1]
        TexCoord texCoord(texIncr.x()*pt.x()*drawInfo->texScale.x()+drawInfo->texOffset.x(),1.0-(texIncr.y()*pt.y()*drawInfo->texScale.y()+drawInfo->texOffset.y()));
        
        chunk->addPoint(disp3d);
        if (ip < _normals.size())
        {
            const Point3f &inNorm = _normals[ip];
            Point3d adjNorm = east * inNorm.x() + north * inNorm.y() + normUp * inNorm.z();
            adjNorm.normalize();
            chunk->addNormal(adjNorm);
        }
        chunk->addTexCoord(-1, texCoord);
        if (elevEntry != 0)
            chunk->addAttributeValue(elevEntry, pt.z());
    }

    // This is the parent tile, so all the triangles
    if (drawInfo->texScale.x() == 1.0)
    {
        for (unsigned int it=0;it<_mesh->tris.size();it++)
        {
            auto &tri = _mesh->tris[it];
            chunk->addTriangle(BasicDrawable::Triangle(tri.pts[0],tri.pts[1],tri.pts[2]));
        }
    } else {
        Mbr mbr;
        mbr.ll().x() = _sizeX * drawInfo->texOffset.x();
        mbr.ll().y() = _sizeY * drawInfo->texOffset.y();
        mbr.ur().x() = mbr.ll().x() + drawInfo->texScale.x() * _sizeX;
        mbr.ur().y() = mbr.ll().y() + drawInfo->texScale.y() * _sizeY;
        
        // Just include the triangles that overlap our section
        for (unsigned int it=0;it<_mesh->tris.size();it++)
        {
            auto &tri = _mesh->tris[it];
            Mbr triMbr;
            
            Point3f pts[3];
            Point3f norms[3];
            for (unsigned int ip=0;ip<3;ip++)
            {
                const Point3f &pt = _mesh->pts[tri.pts[ip]];
                triMbr.addPoint(Point2f(pt.x(),pt.y()));
                pts[ip] = pt;

                norms[ip] = _normals[tri.pts[ip]];
            }
            
            if (mbr.overlaps(triMbr))
            {
                // Just include the triangle as is
                if (mbr.contained(triMbr))
                    chunk->addTriangle(BasicDrawable::Triangle(tri.pts[0],tri.pts[1],tri.pts[2]));
                else {
                    // We need to clip it
                    VectorRing inPts;
                    std::vector<VectorRing> outLoops;
                    for (unsigned int ip=0;ip<3;ip++)
                    {
                        const Point3f &pt = pts[ip];
                        inPts.push_back(Point2f(pt.x(),pt.y()));
                    }
                    if (ClipLoopToMbr(inPts,mbr,true,outLoops))
                    {
                        // Work through the resulting loops
                        for (const VectorRing &loop : outLoops)
                        {
                            VectorTrianglesRef tris = VectorTriangles::createTriangles();
                            TesselateRing(loop,tris);
                            
                            int basePoint = chunk->getNumPoints();
                            for (const Point3f &pt : tris->pts)
                            {
                                // Calculate z for the point
                                double u,v,w;
                                BarycentricCoords(Point2d(pt.x(),pt.y()), Point2d(pts[0].x(),pts[0].y()), Point2d(pts[1].x(),pts[1].y()), Point2d(pts[2].x(),pts[2].y()), u, v, w);
                                double newZ = u*pts[0].z() + v*pts[1].z() + w*pts[2].z();
                                
                                // Reproject the point
                                Point3d loc3d(chunkLL.x()+pt.x()/_sizeX * chunkSize.x(),chunkLL.y()+pt.y()/_sizeY * chunkSize.y(),newZ*_scale);
                                Point3d disp3d = drawInfo->coordAdapter->localToDisplay(CoordSystemConvert3d(drawInfo->coordSys,sceneCoordSys,loc3d));
                                Point3d normUp = drawInfo->coordAdapter->normalForLocal(loc3d);
                                
                                // Calculate a normal for the point
                                Point3f iNorm = u*norms[0] + v*norms[1] + w*norms[2];

                                // Need some axes to reproject the normal
                                Point3d east = Point3d(0,0,1).cross(normUp);
                                Point3d north = normUp.cross(east);
                                Point3d adjNorm = east * iNorm.x() + north * iNorm.y() + normUp * iNorm.z();
                                adjNorm.normalize();

                                // Texture runs across the tile [0,1]
                                TexCoord texCoord(texIncr.x()*pt.x()*drawInfo->texScale.x()+drawInfo->texOffset.x(),1.0-(texIncr.y()*pt.y()*drawInfo->texScale.y()+drawInfo->texOffset.y()));
                                
                                chunk->addPoint(disp3d);
                                // Note: Should interpolate normal
                                chunk->addNormal(adjNorm);
                                chunk->addTexCoord(-1, texCoord);
                                // Note: Need to interpolate Z
                                //                            if (elevEntry != 0)
                                //                                chunk->addAttributeValue(elevEntry, pt.z());
                            }
                            
                            for (const VectorTriangles::Triangle &tri : tris->tris)
                                chunk->addTriangle(BasicDrawable::Triangle(tri.pts[0]+basePoint,tri.pts[2]+basePoint,tri.pts[1]+basePoint));
                        }
                    }
                }
            }
        }
    }
    
    *draw = chunk;
}


@end
