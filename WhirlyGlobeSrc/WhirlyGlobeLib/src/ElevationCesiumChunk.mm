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

using namespace WhirlyKit;

static inline int16_t decodeZigZag(uint16_t encodedValue)
{
    return (encodedValue >> 1) ^ (-(encodedValue & 1));
}

static void decodeHighWaterMark(vector<uint32_t> encoded, vector<uint32_t> &decoded)
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



@implementation WhirlyKitElevationCesiumChunk

@synthesize sizeX = _sizeX, sizeY = _sizeY;


- (id)initWithCesiumData:(NSData *)data sizeX:(int)sizeX sizeY:(int)sizeY
{
	if (self = [super init]) {
		_sizeX = sizeX;
		_sizeY = sizeY;

		[self readData:(uint8_t *) [data bytes]];
	}

	return self;
}

- (void)readData:(uint8_t *)data
{
	// This tool may be useful to compare values read with Cesium's JS code
	// https://github.com/jmnavarro/cesium-quantized-mesh-terrain-format-logger
	//

	uint8_t *startData = data;

	// QuantizedMeshHeader
	// ====================
	struct QuantizedMeshHeader header = *(struct QuantizedMeshHeader *)data;
	data += sizeof(struct QuantizedMeshHeader);

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

	//TODO do we need extensions?
}

- (vector<uint32_t>)readVertexList:(Byte **)dataRef is32:(BOOL)is32
{
	vector<uint32_t> list;
	Byte *data = *dataRef;

	uint32_t vertexCount = CFSwapInt32LittleToHost(*(uint32_t *)data);
	data += sizeof(uint32_t);

	if (is32)
	{
		uint32_t *indices = (uint32_t *)data;

		for (int i = 0; i < vertexCount; ++i)
			list.push_back(CFSwapInt32LittleToHost(indices[i]));

		*dataRef += sizeof(uint32_t) + (vertexCount * sizeof(uint32_t));
	}
	else
	{
		uint16_t *indices = (uint16_t *)data;

		for (int i = 0; i < vertexCount; ++i)
			list.push_back(CFSwapInt16LittleToHost(indices[i]));

		*dataRef += sizeof(uint32_t) + (vertexCount * sizeof(uint16_t));
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

- (void)generateDrawables:(std::vector<BasicDrawable *> &)drawables
{
	//TODO
}


@end
