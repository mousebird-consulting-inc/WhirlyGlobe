/*
 *  ElevationCesiumChunk.h
 *  WhirlyGlobeLib
 *
 *  Created by @jmnavarro on 6/16/15.
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

// Take from
// http://cesiumjs.org/data-and-assets/terrain/formats/quantized-mesh-1.0.html

typedef struct _CesiumQuantizedMeshHeader
{
    // The center of the tile in Earth-centered Fixed coordinates.
    double CenterX;
    double CenterY;
    double CenterZ;
    
    // The minimum and maximum heights in the area covered by this tile.
    // The minimum may be lower and the maximum may be higher than
    // the height of any vertex in this tile in the case that the min/max vertex
    // was removed during mesh simplification, but these are the appropriate
    // values to use for analysis or visualization.
    float MinimumHeight;
    float MaximumHeight;

    // The tile’s bounding sphere.  The X,Y,Z coordinates are again expressed
    // in Earth-centered Fixed coordinates, and the radius is in meters.
    double BoundingSphereCenterX;
    double BoundingSphereCenterY;
    double BoundingSphereCenterZ;
    double BoundingSphereRadius;

    // The horizon occlusion point, expressed in the ellipsoid-scaled Earth-centered Fixed frame.
    // If this point is below the horizon, the entire tile is below the horizon.
    // See http://cesiumjs.org/2013/04/25/Horizon-culling/ for more information.
    double HorizonOcclusionPointX;
    double HorizonOcclusionPointY;
    double HorizonOcclusionPointZ;
} CesiumQuantizedMeshHeader;


/*
struct VertexData
{
    uint32_t vertexCount;
    uint16_t u[vertexCount];
    uint16_t v[vertexCount];
    uint16_t height[vertexCount];
};

struct IndexData16
{
    uint32_t triangleCount;
    uint16_t indices[triangleCount * 3];
}

struct IndexData32
{
    uint32_t triangleCount;
    uint32_t indices[triangleCount * 3];
}

struct EdgeIndices16
{
    uint32_t westVertexCount;
    uint16_t westIndices[westVertexCount];

    uint32_t southVertexCount;
    uint16_t southIndices[southVertexCount];

    uint32_t eastVertexCount;
    uint16_t eastIndices[eastVertexCount];

    uint32_t northVertexCount;
    uint16_t northIndices[northVertexCount];
}

struct EdgeIndices32
{
    uint32_t westVertexCount;
    uint32_t westIndices[westVertexCount];

    uint32_t southVertexCount;
    uint32_t southIndices[southVertexCount];

    uint32_t eastVertexCount;
    uint32_t eastIndices[eastVertexCount];

    uint32_t northVertexCount;
    uint32_t northIndices[northVertexCount];
}

struct CesiumExtensionHeader
{
    uint8_t extensionId;
    uint32_t extensionLength;
}
*/
