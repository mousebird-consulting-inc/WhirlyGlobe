/*
 *  WideVectorDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
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

#import "BasicDrawable.h"

namespace WhirlyKit
{
    
// Shader name
#define kWideVectorShaderName "Wide Vector Shader"
    
/// Construct and return the Billboard shader program
OpenGLES2Program *BuildWideVectorProgram();
    
// Used to debug the wide vectors
//#define WIDEVECDEBUG 1

/** This drawable adds convenience functions for wide vectors.
  */
class WideVectorDrawable : public BasicDrawable
{
public:
    WideVectorDrawable(const std::string &name,unsigned int numVert,unsigned int numTri);
    
    virtual unsigned int addPoint(const Point3f &pt);
    // Next point, for calculating p1 - p0
    void add_p1(const Point3f &vec);
    // Texture calculation parameters
    void add_texInfo(float texX,float texYmin,float texYmax,float texOffset);
    // Vector for 90 deg from line
    void add_n0(const Point3f &vec);
    // Complex constant we multiply by width for t
    void add_c0(float c);
    
    /// How often the texture repeats
    void setTexRepeat(float inTexRepeat) { texRepeat = inTexRepeat; }
    
    /// Number of pixels to interpolate at the edges
    void setEdgeSize(float inEdgeSize) { edgeSize = inEdgeSize; }
    
    /// Fix the width to a real world value, rather than letting it change
    void setRealWorldWidth(double width) { realWidthSet = true;  realWidth = width; }
    
    /// We override draw so we can set our own values
    virtual void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
    // We don't want the standard attributes
    virtual void setupStandardAttributes(int numReserve=0);

protected:
    bool realWidthSet;
    double realWidth;
    bool snapTex;
    float texRepeat;
    float edgeSize;
    int p1_index;
    int n0_index;
    int c0_index;
    int tex_index;
    
#ifdef WIDEVECDEBUG
    std::vector<Point3f> locPts;
    std::vector<Point3f> p1;
    std::vector<Point2f> t0_limits;
    std::vector<Point3f> n0;
    std::vector<float> c0;
#endif
};
    
}
