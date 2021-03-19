/*  WideVectorDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "BasicDrawableBuilder.h"
#import "Program.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

// Modifies the uniform values of a given shader right
//  before the wide vector drawables are rendered
struct WideVectorTweaker : public BasicDrawableTweaker
{
    // Called right before the drawable is drawn
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) = 0;

    float edgeSize = 0.0f;
    float lineWidth = 0.0f;
    float texRepeat = 0.0f;
    float offset = 0.0f;
    bool offsetSet = false;

    FloatExpressionInfoRef widthExp;
    FloatExpressionInfoRef offsetExp;
};

// Used to debug the wide vectors
//#define WIDEVECDEBUG 1

/** This drawable adds convenience functions for wide vectors.
  */
class WideVectorDrawableBuilder : virtual public BasicDrawableBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    WideVectorDrawableBuilder() = default;
    virtual ~WideVectorDrawableBuilder() = default;

    virtual void Init(unsigned int numVert,unsigned int numTri,bool globeMode);
    
    virtual unsigned int addPoint(const Point3f &pt) override;
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
    virtual void addNormal(const Point3f &norm) override;
    virtual void addNormal(const Point3d &norm) override;

    // Line width for vectors is a bit different
    virtual void setLineWidth(float inWidth) override;
    
    // Line offset for vectors
    void setLineOffset(float inOffset);
    
    /// How often the texture repeats
    void setTexRepeat(float inTexRepeat);
    
    /// Number of pixels to interpolate at the edges
    void setEdgeSize(float inEdgeSize);

    // Apply a width expression
    void setWidthExpression(const FloatExpressionInfoRef &widthExp);
    
    // Apply an offset expression
    void setOffsetExpression(const FloatExpressionInfoRef &offsetExp);
    
    // The tweaker sets up uniforms before a given drawable draws
    virtual void setupTweaker(const DrawableTweakerRef &inTweaker) const override;

protected:
    float lineWidth = 1.0f;
    float lineOffset = 0.0f;
    bool lineOffsetSet = false;
    bool globeMode = true;
    float texRepeat = 1.0f;
    float edgeSize = 1.0f;
    int p1_index = -1;
    int n0_index = -1;
    int offset_index = -1;
    int c0_index = -1;
    int tex_index = -1;
    
    FloatExpressionInfoRef widthExp;
    FloatExpressionInfoRef offsetExp;
    
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
