/*
 *  WideVectorDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
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

#import "BasicDrawableBuilder.h"
#import "Program.h"
#import "BaseInfo.h"
#import "WideVectorManager.h"

namespace WhirlyKit
{
    
// Modifies the uniform values of a given shader right
//  before the wide vector drawables are rendered
class WideVectorTweaker : public DrawableTweaker
{
public:
    // Called right before the drawable is drawn
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) = 0;
    
    float edgeSize;
    float lineWidth;
    float texRepeat;
    RGBAColor color;

    FloatExpressionInfoRef widthExp;
};
    
// Used to debug the wide vectors
//#define WIDEVECDEBUG 1

/** This drawable adds convenience functions for wide vectors.
  */
class WideVectorDrawableBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    WideVectorDrawableBuilder(const std::string &name,const SceneRenderer *sceneRenderer,Scene *scene);
    virtual ~WideVectorDrawableBuilder();

    virtual void Init(unsigned int numVert,
                      unsigned int numTri,
                      unsigned int numCenterline,
                      WideVecImplType implType,
                      bool globeMode);
    
    /// Set the fade in and out
    void setFade(TimeInterval inFadeDown,TimeInterval inFadeUp);
    
    /// Set the bounding box for the data
    void setLocalMbr(Mbr mbr);

    virtual unsigned int addPoint(const Point3f &pt);
    // Next point, for calculating p1 - p0
    void add_p1(const Point3f &vec);
    // Texture calculation parameters
    void add_texInfo(float texX,float texYmin,float texYmax,float texOffset);
    // Vector for 90 deg from line
    void add_n0(const Point3f &vec);
    // Left or right direction vector (needed for offsets) [And a pointer about which side of the line this is on)
    void add_offset(const Point3f &nDir);
    // Complex constant we multiply by width for t
    void add_c0(float c);
    // Optional normal
    void addNormal(const Point3f &norm);
    void addNormal(const Point3d &norm);

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
    
    /// Add an integer value to the given attribute array
    virtual void addAttributeValue(int attrId,int val);
    
    /// Add an identity-type value to the given attribute array
    virtual void addAttributeValue(int attrId,int64_t val);
    
    /// Add a triangle.  Should point to the vertex IDs.
    virtual void addTriangle(BasicDrawable::Triangle tri);
    
    // We set color globally
    void setColor(RGBAColor inColor);
    
    // Line width for vectors is a bit different
    void setLineWidth(float inWidth);
    
    // Line offset for vectors
    void setLineOffset(float inOffset);
    
    /// How often the texture repeats
    void setTexRepeat(float inTexRepeat);
    
    /// Number of pixels to interpolate at the edges
    void setEdgeSize(float inEdgeSize);
    
    // Apply a dynamic color expression
    void setColorExpression(ColorExpressionInfoRef colorExp);
    
    // Apply a dynamic opacity expression
    void setOpacityExpression(FloatExpressionInfoRef opacityExp);

    // Apply a width expression
    void setWidthExpression(FloatExpressionInfoRef widthExp);
    
    // Apply an offset expression
    void setOffsetExpression(FloatExpressionInfoRef offsetExp);
    
    // The tweaker sets up uniforms before a given drawable draws
    void setupTweaker(BasicDrawable *theDraw);
    
    // For performance mode wide vectors (Metal for now), the center line instances
    void addCenterLine(const Point3d &centerPt,const Point3d &up,double len,const RGBAColor &color,const std::vector<SimpleIdentity> &maskIDs,int prev,int next);
    
    // Number of centerlines defined so far
    int getCenterLineCount();
    
    // We need slightly different tweakers for the rendering variants
    virtual WideVectorTweaker *makeTweaker() = 0;

    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot=-1,int numThings = -1) = 0;

    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);

    /// Set the active transform matrix
    virtual void setMatrix(const Eigen::Matrix4d *inMat);

    /// Number of points added so far
    unsigned int getNumPoints();
    
    /// Numer of triangles added so far
    unsigned int getNumTris();
    
    // Return the basic drawable for the simple and complex cases
    virtual BasicDrawableRef getBasicDrawable();
    
    virtual SimpleIdentity getBasicDrawableID();
    
    // Return the drawable instance for the complec case
    virtual BasicDrawableInstanceRef getInstanceDrawable();
    
    virtual SimpleIdentity getInstanceDrawableID();

protected:
    std::string name;
    Scene *scene;
    const SceneRenderer *renderer;
    
    // Controls whether we're building basic drawables or instances
    // We do instances on Metal
    WideVecImplType implType;
    BasicDrawableBuilderRef basicDrawable;
    BasicDrawableInstanceBuilderRef instDrawable;

    float lineWidth;
    float lineOffset;
    RGBAColor color;
    bool globeMode;
    bool snapTex;
    float texRepeat;
    float edgeSize;
    int p1_index;
    int n0_index;
    int offset_index;
    int c0_index;
    int tex_index;
    
    FloatExpressionInfoRef widthExp;
    FloatExpressionInfoRef offsetExp;
    FloatExpressionInfoRef opacityExp;
    ColorExpressionInfoRef colorExp;

    // Centerline structure (for Metal)
    typedef struct {
        Point3f center;
        Point3f up;
        float len;
        Point3f color;
        int prev,next;
        int maskIDs[2];
    } CenterPoint;
    std::vector<CenterPoint> centerline;
    
#ifdef WIDEVECDEBUG
    Point3fVector locPts;
    Point3fVector p1;
    Point2fVector t0_limits;
    Point3fVector n0;
    std::vector<float> c0;
#endif
};
    
typedef std::shared_ptr<WideVectorDrawableBuilder> WideVectorDrawableBuilderRef;
    
}
