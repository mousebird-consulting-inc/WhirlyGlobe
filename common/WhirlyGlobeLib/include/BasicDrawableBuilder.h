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
#import "TextureAtlas.h"

namespace WhirlyKit
{
    
/** Used to construct a BasicDrawable.
    This is abstracted away from the BasicDrawable itself so we can
    build drawables for the different renderers.
 */
class BasicDrawableBuilder
{
public:
    /// Construct empty
    BasicDrawableBuilder(const std::string &name);
    virtual ~BasicDrawableBuilder();
    
    /// Reserve the given amount of space (cuts down on reallocs)
    void reserve(int numPoints,int numTris);
        
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff);
    
    /// Set the time range for enable
    void setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable);
    
    /// Set the fade in and out
    void setFade(TimeInterval inFadeDown,TimeInterval inFadeUp);

    /// Set local extents
    void setLocalMbr(Mbr mbr);
    const Mbr &getLocalMbr();
    
    /// Set the viewer based visibility
    virtual void setViewerVisibility(double minViewerDist,double maxViewerDist,const Point3d &viewerCenter);
    
    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    virtual void setVisibleRange(float minVis,float maxVis,float minVisBand=0.0,float maxVisBand=0.0);
    
    /// Set the alpha sorting on or off
    void setAlpha(bool onOff);

    /// Set the draw offset.  This is an integer offset from the base terrain.
    /// Geometry is moved upward by a certain number of units.
    virtual void setDrawOffset(float newOffset);
    
    /// Draw priority used for sorting
    virtual void setDrawPriority(unsigned int newPriority);

    /// Set the active transform matrix
    virtual void setMatrix(const Eigen::Matrix4d *inMat);

    /// Resulting drawable wants the Z buffer for comparison
    virtual void setRequestZBuffer(bool val);

    /// Resulting drawable writes to the Z buffer
    virtual void setWriteZBuffer(bool val);

    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget);

    /// Set the line width (if using lines)
    virtual void setLineWidth(float inWidth);
    virtual float getLineWidth();
        
    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);

    /// Return the ID for the texture, if it's there
    virtual SimpleIdentity getTexId(unsigned int which);

    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &texIDs);
    
    /// Set the relative offsets for texture usage.
    /// We use these to look up parts of a texture at a higher level
    virtual void setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY);

    
    /// For OpenGLES2, you can set the program to use in rendering
    void setProgram(SimpleIdentity progId);

    /// Add a tweaker to this list to be run each frame
    void addTweaker(DrawableTweakerRef tweakRef);

    /// Set the geometry type.  Probably triangles.
    virtual void setType(GeometryType inType);
    
    /// Set the color as an RGB color
    virtual void setColor(RGBAColor inColor);
    
    /// Set the color as an array.
    virtual void setColor(unsigned char inColor[]);
    
    /// Number of extra frames to draw after we'd normally stop
    virtual void setExtraFrames(int numFrames);
    
    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings = -1) = 0;
    
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
    
    /// Number of points added so far
    virtual unsigned int getNumPoints();
    
    /// Numer of triangles added so far
    virtual unsigned int getNumTris();
    
    /// Return a given point
    virtual Point3d getPoint(int which);
    
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
    virtual void addTriangle(BasicDrawable::Triangle tri);
    
    /// Set the uniforms applied to the Program before rendering
    virtual void setUniforms(const SingleVertexAttributeSet &uniforms);
    
    /// Run the texture and texture coordinates based on a SubTexture
    virtual void applySubTexture(int which,SubTexture subTex,int startingAt=0);
        
    /// Constructs the remaining pieces of the drawable and returns it
    /// Caller is responsible for deletion
    virtual BasicDrawable *getDrawable() = 0;
    
    /// Return just the ID of the drawable being created.
    /// This avoids flushing things out
    virtual SimpleIdentity getDrawableID();
    
    /// Return just the draw priority of the drawable being created
    virtual int getDrawablePriority();

    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);

public:
    std::string name;
    
    // This version is only used by subclasses
    BasicDrawableBuilder();
    void setName(const std::string &name);
    // Used by subclasses to do the standard init
    virtual void Init();
    // Set up the standard vertex attributes we use
    virtual void setupStandardAttributes(int numReserve=0);
    
    // The basic drawable we're building up
    BasicDrawable *basicDraw;

    // Unprocessed data arrays
    std::vector<Eigen::Vector3f> points;
    std::vector<BasicDrawable::Triangle> tris;
};

typedef std::shared_ptr<BasicDrawableBuilder> BasicDrawableBuilderRef;

}
