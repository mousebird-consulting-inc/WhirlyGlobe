/*
 *  BasicDrawableBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/9/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import <vector>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "GlobeView.h"
#import "Drawable.h"

namespace WhirlyKit
{
    
/** Used to construct a BasicDrawable.
 */
class BasicDrawableBuilder : public Identifiable
{
public:
    /// Set local extents
    void setLocalMbr(Mbr mbr);
    
    /// Set the geometry type.  Probably triangles.
    virtual void setType(GLenum inType);
    
    /// Set the color as an RGB color
    virtual void setColor(RGBAColor inColor);
    
    /// Set the color as an array.
    virtual void setColor(unsigned char inColor[]);
    
    /// Set the active transform matrix
    virtual void setMatrix(const Eigen::Matrix4d *inMat);
    
    /// Size of a single vertex used in creating an interleaved buffer.
    virtual unsigned int singleVertexSize();
    
    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings = -1);
    
    /// Reserve the extra space for points
    virtual void reserveNumPoints(int numPoints);
    
    /// Reserve the extra space for triangles
    virtual void reserveNumTris(int numTris);
    
    /// Reserve extra space for texture coordinates
    virtual void reserveNumTexCoords(unsigned int which,int numCoords);
    
    /// Reserve extra space for normals
    virtual void reserveNumNorms(int numNorms);
    
    /// Reserve extra space for colors
    virtual void reserveNumColors(int numColors);
    
    /// If true the geometry is already in clip coordinates, so we won't transform it
    virtual void setClipCoords(bool clipCoords);
    
    /// Add a point when building up geometry.  Returns the index.
    virtual unsigned int addPoint(const Point3f &pt);
    virtual unsigned int addPoint(const Point3d &pt);
    
    /// Return a given point
    virtual Point3f getPoint(int which);
    
    /// Add a texture coordinate. -1 means we add the same
    ///  texture coordinate to all the available texture coordinate sets
    virtual void addTexCoord(int which,TexCoord coord);
    
    /// Add a color
    virtual void addColor(RGBAColor color);
    
    /// Add a normal
    virtual void addNormal(const Point3f &norm);
    virtual void addNormal(const Point3d &norm);
    
    /// Decide if the given list of vertex attributes is the same as the one we have
    bool compareVertexAttributes(const SingleVertexAttributeSet &attrs);
    
    /// Set up the required vertex attribute arrays from the given list
    void setVertexAttributes(const SingleVertexAttributeInfoSet &attrs);
    
    /// Add the given vertex attributes for the given vertex
    void addVertexAttributes(const SingleVertexAttributeSet &attrs);
    
    /// Add a 2D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector2f &vec);
    
    /// Add a 3D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector3f &vec);
    
    /// Add a 4D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector4f &vec);
    
    /// Add a 4 component char array to the given attribute array
    virtual void addAttributeValue(int attrId,const RGBAColor &color);
    
    /// Add a float to the given attribute array
    virtual void addAttributeValue(int attrId,float val);
    
    /// Add a triangle.  Should point to the vertex IDs.
    virtual void addTriangle(Triangle tri);
    
    /// Add a single point to the GL Buffer.
    /// Override this to add your own data to interleaved vertex buffers.
    virtual void addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center);
    
    virtual void setUniforms(const SingleVertexAttributeSet &uniforms);
    
    /// Run the texture and texture coordinates based on a SubTexture
    virtual void applySubTexture(int which,SubTexture subTex,int startingAt=0);
    
    /// Copy the vertex data into an NSData object and return it
    virtual RawDataRef asData(bool dupStart,bool dupEnd);
    
    /// Copy vertex and element data into appropriate NSData objects
    virtual void asVertexAndElementData(MutableRawDataRef retVertData,RawDataRef retElementData,int singleElementSize,const Point3d *center);
    
public:
    // Used by subclasses to do the standard init
    void basicDrawableInit();
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    
    // Attributes associated with each vertex, some standard some not
    std::vector<VertexAttribute *> vertexAttributes;
    // Entries for the standard attributes we create on startup
    int colorEntry,normalEntry;
    // Set up the standard vertex attributes we use
    virtual void setupStandardAttributes(int numReserve=0);
    
    // We'll nuke the data arrays when we hand over the data to GL
    unsigned int numPoints, numTris;
    std::vector<Eigen::Vector3f> points;
    std::vector<Triangle> tris;
    SimpleIdentity renderTargetID;
    // If the drawable has a matrix, we'll transform by that before drawing
    Eigen::Matrix4d mat;
    // Uniforms to apply to shader
    SingleVertexAttributeSet uniforms;
    
    // Size for a single vertex w/ all its data.  Used by shared buffer
    int vertexSize;
    GLuint pointBuffer,triBuffer,sharedBuffer;
    GLuint vertArrayObj;
    std::vector<BasicDrawable::VertAttrDefault> vertArrayDefaults;
    
    // If set the geometry is already in OpenGL clip coordinates, so no transform
    bool clipCoords;
};
    
}
