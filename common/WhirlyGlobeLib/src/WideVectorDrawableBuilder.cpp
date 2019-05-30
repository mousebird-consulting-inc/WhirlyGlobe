/*
 *  WideVectorDrawable.mm
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

#import "WideVectorDrawableBuilder.h"
#import "SceneRenderer.h"
#import "FlatMath.h"
#import "ProgramGLES.h"

using namespace Eigen;

namespace WhirlyKit
{
    
// Modifies the uniform values of a given shader right
//  before the wide vector drawables are rendered
class WideVectorTweaker : public DrawableTweaker
{
public:
    void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
    {
        if (frameInfo->program)
        {
            float scale = std::max(frameInfo->sceneRenderer->framebufferWidth,frameInfo->sceneRenderer->framebufferHeight);
            float screenSize = frameInfo->screenSizeInDisplayCoords.x();
            float pixDispSize = std::min(frameInfo->screenSizeInDisplayCoords.x(),frameInfo->screenSizeInDisplayCoords.y()) / scale;
            if (realWidthSet)
            {
                frameInfo->program->setUniform(u_w2NameID, (float)(realWidth / pixDispSize));
                frameInfo->program->setUniform(u_Realw2NameID, (float)realWidth);
                frameInfo->program->setUniform(u_EdgeNameID, edgeSize);
            } else {
                frameInfo->program->setUniform(u_w2NameID, lineWidth);
                frameInfo->program->setUniform(u_Realw2NameID, pixDispSize * lineWidth);
                frameInfo->program->setUniform(u_EdgeNameID, edgeSize);
            }
            float texScale = scale/(screenSize*texRepeat);
            frameInfo->program->setUniform(u_texScaleNameID, texScale);
            frameInfo->program->setUniform(u_colorNameID, Vector4f(color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0));

            // Note: This calculation is out of date with respect to the shader
            // Redo the calculation for debugging
            //        NSLog(@"\n");
            //        for (unsigned int ii=0;ii<locPts.size();ii++)
            //        {
            //            float u_w2 = lineWidth/(2.f*scale);
            //            float u_real_w2 = pixDispSize * lineWidth;
            //            Point3f a_p0 = locPts[ii];
            //            Point3f a_p1 = p1[ii];
            //            Point2f a_t0_limit = t0_limits[ii];
            //            Point3f a_n0 = n0[ii];
            //            float a_c0 = c0[ii];
            //
            //            Vector4f screen_p0 = frameInfo.mvpMat * Vector4f(a_p0.x(),a_p0.y(),a_p0.z(),1.0);
            //            screen_p0 /= screen_p0.w();
            //            Vector4f screen_p1 = frameInfo.mvpMat * Vector4f(a_p1.x(),a_p1.y(),a_p1.z(),1.0);
            //            screen_p1 /= screen_p1.w();
            //            Point2f loc_p0(screen_p0.x(),screen_p0.y());
            //            Point2f loc_p1(screen_p1.x(),screen_p1.y());
            //
            //            Vector4f screen_n0 = frameInfo.mvpMat * Vector4f(a_n0.x(),a_n0.y(),a_n0.z(),0.0);
            //            Point2f loc_n0(screen_n0.x(),screen_n0.y());
            //
            //            float t0 = a_c0 * u_real_w2;
            //            Vector2f calcOff = (loc_p1-loc_p0) * t0 + loc_n0 * u_w2;
            //            Point2f finalPos2f = loc_p0 + calcOff;
            //
            //            NSLog(@"t0 = %f",t0);
            //            NSLog(@"finalPos = (%f,%f)",finalPos2f.x(),finalPos2f.y());
            //        }
        }
        
        //    for (unsigned int ii=0;ii<dirs.size();ii++)
        //    {
        //        double len = lens[ii];
        //        Point3d dir = dirs[ii];
        //        if (u_pixDispSize * dir.norm() * lineWidth > len)
        //        {
        //            NSLog(@"Dropping one");
        //        }
        //    }
    }
    
    bool realWidthSet;
    float realWidth;
    float edgeSize;
    float lineWidth;
    float texRepeat;
    RGBAColor color;
};
    
WideVectorDrawableBuilder::WideVectorDrawableBuilder()
    : texRepeat(1.0), edgeSize(1.0), realWidthSet(false), globeMode(true), color(255,255,255,255)
{
}
    
WideVectorDrawableBuilder::~WideVectorDrawableBuilder()
{
}
    
void WideVectorDrawableBuilder::Init(unsigned int numVert,unsigned int numTri,bool inGlobeMode)
{
    globeMode = inGlobeMode;
    
    BasicDrawableBuilder::Init();
    // Don't want standard attributes
    
    points.reserve(numVert);
    tris.reserve(numTri);
    
    lineWidth = 10.0/1024.0;
    if (globeMode)
        basicDraw->normalEntry = addAttribute(BDFloat3Type, a_normalNameID,numVert);
    basicDraw->colorEntry = addAttribute(BDChar4Type, a_colorNameID);
    p1_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_p1"),numVert);
    tex_index = addAttribute(BDFloat4Type, StringIndexer::getStringID("a_texinfo"),numVert);
    n0_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_n0"),numVert);
    c0_index = addAttribute(BDFloatType, StringIndexer::getStringID("a_c0"),numVert);
}
    
void WideVectorDrawableBuilder::setColor(RGBAColor inColor)
{
    color = inColor;
}
    
void WideVectorDrawableBuilder::setLineWidth(float inWidth)
{
    lineWidth = inWidth;
}
 
void WideVectorDrawableBuilder::setTexRepeat(float inTexRepeat)
    { texRepeat = inTexRepeat; }

void WideVectorDrawableBuilder::setEdgeSize(float inEdgeSize)
    { edgeSize = inEdgeSize; }

void WideVectorDrawableBuilder::setRealWorldWidth(double width)
    { realWidthSet = true;  realWidth = width; }

    
unsigned int WideVectorDrawableBuilder::addPoint(const Point3f &pt)
{
#ifdef WIDEVECDEBUG
    locPts.push_back(pt);
#endif
    return BasicDrawableBuilder::addPoint(pt);
}
    
void WideVectorDrawableBuilder::addNormal(const Point3f &norm)
{
    if (globeMode)
    {
        BasicDrawableBuilder::addNormal(norm);
    }
}

void WideVectorDrawableBuilder::addNormal(const Point3d &norm)
{
    if (globeMode)
    {
        BasicDrawableBuilder::addNormal(norm);
    }
}

void WideVectorDrawableBuilder::add_p1(const Point3f &pt)
{
    addAttributeValue(p1_index, pt);
#ifdef WIDEVECDEBUG
    p1.push_back(pt);
#endif
}

void WideVectorDrawableBuilder::add_texInfo(float texX,float texYmin,float texYmax,float texOffset)
{
    addAttributeValue(tex_index, Vector4f(texX,texYmin,texYmax,texOffset));
#ifdef WIDEVECDEBUG
#endif
}

void WideVectorDrawableBuilder::add_n0(const Point3f &dir)
{
    addAttributeValue(n0_index, dir);
#ifdef WIDEVECDEBUG
    n0.push_back(dir);
#endif
}

void WideVectorDrawableBuilder::add_c0(float val)
{
    addAttributeValue(c0_index, val);
#ifdef WIDEVECDEBUG
    c0.push_back(val);
#endif
}
    
void WideVectorDrawableBuilder::setupTweaker(BasicDrawable *theDraw)
{
    WideVectorTweaker *tweak = new WideVectorTweaker();
    tweak->realWidthSet = false;
    tweak->realWidth = realWidth;
    tweak->edgeSize = edgeSize;
    tweak->lineWidth = lineWidth;
    tweak->texRepeat = texRepeat;
    tweak->color = color;
    theDraw->addTweaker(DrawableTweakerRef(tweak));
}    
    
}
