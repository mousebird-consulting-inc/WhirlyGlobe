/*
 *  BasicDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "BasicDrawable.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawable::Triangle::Triangle()
{
}

BasicDrawable::Triangle::Triangle(unsigned short v0,unsigned short v1,unsigned short v2)
{
    verts[0] = v0;  verts[1] = v1;  verts[2] = v2;
}
    
BasicDrawable::BasicDrawable(const std::string &name)
: Drawable(name)
{
    basicDrawableInit();
    
    setupStandardAttributes();
}

BasicDrawable::BasicDrawable(const std::string &name,unsigned int numVert,unsigned int numTri)
: Drawable(name)
{
    basicDrawableInit();
    
    points.reserve(numVert);
    tris.reserve(numTri);
    setupStandardAttributes(numVert);
}

BasicDrawable::~BasicDrawable()
{
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        delete vertexAttributes[ii];
    vertexAttributes.clear();
}

void BasicDrawable::setupTexCoordEntry(int which,int numReserve)
{
    if (which < texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)texInfo.size();ii<=which;ii++)
    {
        TexInfo newInfo;
        char attributeName[40];
        sprintf(attributeName,"a_texCoord%d",ii);
        newInfo.texCoordEntry = addAttribute(BDFloat2Type,StringIndexer::getStringID(attributeName));
        vertexAttributes[newInfo.texCoordEntry]->setDefaultVector2f(Vector2f(0.0,0.0));
        vertexAttributes[newInfo.texCoordEntry]->reserve(numReserve);
        texInfo.push_back(newInfo);
    }
}

void BasicDrawable::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    if (which >= texInfo.size())
        return;
    
    TexInfo &ti = texInfo[which];
    ti.size = size;
    ti.borderTexel = borderTexel;
    ti.relLevel = relLevel;
    ti.relX = relX;
    ti.relY = relY;
}

void BasicDrawable::setupStandardAttributes(int numReserve)
{
//    setupTexCoordEntry(0,numReserve);
    
    colorEntry = addAttribute(BDChar4Type,a_colorNameID);
    vertexAttributes[colorEntry]->setDefaultColor(RGBAColor(255,255,255,255));
    vertexAttributes[colorEntry]->reserve(numReserve);
    
    normalEntry = addAttribute(BDFloat3Type,a_normalNameID);
    vertexAttributes[normalEntry]->setDefaultVector3f(Vector3f(1.0,1.0,1.0));
    vertexAttributes[normalEntry]->reserve(numReserve);
}

SimpleIdentity BasicDrawable::getProgram() const
{
    return programId;
}

void BasicDrawable::setProgram(SimpleIdentity progId)
{
    programId = progId;
}

unsigned int BasicDrawable::getDrawPriority() const
{
    return drawPriority;
}

bool BasicDrawable::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable ||
            endEnable < frameInfo->currentTime)
            return false;
    }
    
    if (!on)
        return false;
    
    double visVal = frameInfo->theView->heightAboveSurface();

    // Height based check
    if (minVisible != DrawVisibleInvalid && maxVisible != DrawVisibleInvalid)
    {
        if (!((minVisible <= visVal && visVal <= maxVisible) ||
                (maxVisible <= visVal && visVal <= minVisible)))
            return false;
    }
    
    // Viewer based check
    if (minViewerDist != DrawVisibleInvalid && maxViewerDist != DrawVisibleInvalid &&
        viewerCenter.x() != DrawVisibleInvalid)
    {
        double dist2 = (viewerCenter - frameInfo->eyePos).squaredNorm();
        if (!(minViewerDist*minViewerDist < dist2 && dist2 <= maxViewerDist*maxViewerDist))
            return false;
    }
    
    return true;
}

void BasicDrawable::setOnOff(bool onOff)
{
    on = onOff;
}

bool BasicDrawable::hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (isAlpha)
        return true;
    
    // We don't need to get tricky unless we're z buffering this data
    if (!requestZBuffer)
        return false;
    
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < fadeDown)
            return false;
        else
            if (frameInfo->currentTime > fadeUp)
                return false;
            else
                return true;
    } else
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < fadeUp)
                return false;
            else
                if (frameInfo->currentTime > fadeDown)
                    return false;
                else
                    return true;
        }
    
    WhirlyGlobe::GlobeView *globeView = dynamic_cast<WhirlyGlobe::GlobeView *>(frameInfo->theView);
    if ((minVisibleFadeBand != 0.0 || maxVisibleFadeBand != 0.0) && globeView)
    {
        float height = globeView->heightAboveSurface();
        if (height > minVisible && height < minVisible + minVisibleFadeBand)
        {
            return true;
        } else if (height > maxVisible - maxVisibleFadeBand && height < maxVisible)
        {
            return true;
        }
    }
    
    return false;
}

void BasicDrawable::setAlpha(bool onOff)
{
    isAlpha = onOff;
}

Mbr BasicDrawable::getLocalMbr() const
{
    return localMbr;
}


void BasicDrawable::setLocalMbr(Mbr mbr)
{
    localMbr = mbr;
}

void BasicDrawable::setDrawPriority(unsigned int newPriority)
{
    drawPriority = newPriority;
}

unsigned int BasicDrawable::getDrawPriority()
{
    return drawPriority;
}

void BasicDrawable::setDrawOffset(float newOffset)
{
    drawOffset = newOffset;
}

float BasicDrawable::getDrawOffset()
{
    return drawOffset;
}

void BasicDrawable::setType(GLenum inType)
{
    type = inType;
    if (type == GL_LINES)
        writeZBuffer = false;
}

GLenum BasicDrawable::getType() const
{
    return type;
}

void BasicDrawable::setTexId(unsigned int which,SimpleIdentity inId)
{
    if (which >= 0 && which < texInfo.size())
        texInfo[which].texId = inId;
}

void BasicDrawable::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<std::min(texIDs.size(),texInfo.size());ii++)
    {
        setupTexCoordEntry(ii, 0);
        texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawable::setColor(RGBAColor inColor)
{
    if (colorEntry >= 0)
        vertexAttributes[colorEntry]->setDefaultColor(color);
}

/// Set the color as an array.
void BasicDrawable::setColor(unsigned char inColor[])
{
    if (colorEntry >= 0)
        vertexAttributes[colorEntry]->setDefaultColor(color);
}
    
void BasicDrawable::setOverrideColor(RGBAColor inColor)
{
    color = inColor;
    hasOverrideColor = true;
}

void BasicDrawable::setOverrideColor(unsigned char inColor[])
{
    color.r = inColor[0];  color.g = inColor[1];  color.b = inColor[2];  color.a = inColor[3];
    hasOverrideColor = true;
}

RGBAColor BasicDrawable::getColor() const
{
    return color;
}

void BasicDrawable::setVisibleRange(float minVis,float maxVis,float minVisBand,float maxVisBand)
{ minVisible = minVis;  maxVisible = maxVis;  minVisibleFadeBand = minVisBand; maxVisibleFadeBand = maxVisBand; }

void BasicDrawable::getVisibleRange(float &minVis,float &maxVis)
{ minVis = minVisible;  maxVis = maxVisible; }

void BasicDrawable::getVisibleRange(float &minVis,float &maxVis,float &minVisBand,float &maxVisBand)
{ minVis = minVisible; maxVis = maxVisible;  minVisBand = minVisibleFadeBand; maxVisBand = maxVisibleFadeBand; }
    
void BasicDrawable::setViewerVisibility(double inMinViewerDist,double inMaxViewerDist,const Point3d &inViewerCenter)
{
    minViewerDist = inMinViewerDist;
    maxViewerDist = inMaxViewerDist;
    viewerCenter = inViewerCenter;
}

void BasicDrawable::getViewerVisibility(double &outMinViewerDist,double &outMaxViewerDist,Point3d &outViewerCenter)
{
    outMinViewerDist = minViewerDist;
    outMaxViewerDist = maxViewerDist;
    outViewerCenter = viewerCenter;
}
void BasicDrawable::setFade(TimeInterval inFadeDown,TimeInterval inFadeUp)
{ fadeUp = inFadeUp;  fadeDown = inFadeDown; }

void BasicDrawable::setLineWidth(float inWidth)
{ lineWidth = inWidth; }

float BasicDrawable::getLineWidth()
{ return lineWidth; }

void BasicDrawable::setRequestZBuffer(bool val)
{ requestZBuffer = val; }

bool BasicDrawable::getRequestZBuffer() const
{ return requestZBuffer; }

void BasicDrawable::setWriteZBuffer(bool val)
{ writeZBuffer = val; }

bool BasicDrawable::getWriteZbuffer() const
{ return writeZBuffer; }
    
void BasicDrawable::setClipCoords(bool inClipCoords)
{
    clipCoords = inClipCoords;
}

SimpleIdentity BasicDrawable::getTexId(unsigned int which)
{
    SimpleIdentity texId = EmptyIdentity;
    if (which < texInfo.size())
        texId = texInfo[which].texId;
    
    return texId;
}

// If we're fading in or out, update the rendering window
void BasicDrawable::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    renderer->setRenderUntil(fadeUp);
    renderer->setRenderUntil(fadeDown);
}

/// Return the active transform matrix, if we have one
const Eigen::Matrix4d *BasicDrawable::getMatrix() const
{ if (hasMatrix) return &mat;  return NULL; }
    
void BasicDrawable::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    uniforms = newUniforms;
}
    
SingleVertexAttributeSet BasicDrawable::getUniforms() const
{
    return uniforms;
}


const std::vector<VertexAttribute *> &BasicDrawable::getVertexAttributes()
{
    return vertexAttributes;
}

BasicDrawableTexTweaker::BasicDrawableTexTweaker(const std::vector<SimpleIdentity> &texIDs,TimeInterval startTime,double period)
: texIDs(texIDs), startTime(startTime), period(period)
{
}

void BasicDrawableTexTweaker::tweakForFrame(Drawable *draw,RendererFrameInfo *frame)
{
    BasicDrawable *basicDraw = (BasicDrawable *)draw;
    
    double t = fmod(frame->currentTime-startTime,period)/period;
    int base = floor(t * texIDs.size());
    int next = (base+1)%texIDs.size();
    double interp = t*texIDs.size()-base;
    
    basicDraw->setTexId(0, texIDs[base]);
    basicDraw->setTexId(1, texIDs[next]);

    // Interpolation as well
    SingleVertexAttributeSet uniforms;
    uniforms.insert(SingleVertexAttribute(u_interpNameID,(float)interp));
    basicDraw->setUniforms(uniforms);

    // This forces a redraw every frame
    // Note: There has to be a better way
    frame->scene->addChangeRequest(NULL);
}
    
BasicDrawableScreenTexTweaker::BasicDrawableScreenTexTweaker(const Point3d &centerPt,const Point2d &texScale)
    : centerPt(centerPt), texScale(texScale)
{
}
    
void BasicDrawableScreenTexTweaker::tweakForFrame(Drawable *draw,WhirlyKit::RendererFrameInfo *frameInfo)
{
    BasicDrawable *basicDraw = (BasicDrawable *)draw;

    if (frameInfo->program)
    {
        Vector4f screenPt = frameInfo->mvpMat * Vector4f(centerPt.x(),centerPt.y(),centerPt.z(),1.0);
        screenPt /= screenPt.w();
        
        Point2f u_scale = frameInfo->sceneRenderer->getFramebufferSize() / 2.f;
        Point2f newScreenPt(fmod(-screenPt.x()*texScale.x()*u_scale.x(),1.0),fmod(-screenPt.y()*texScale.y()*u_scale.y(),1.0));
        newScreenPt.x() /= texScale.x()*u_scale.x();
        newScreenPt.y() /= texScale.y()*u_scale.y();

        SingleVertexAttributeSet uniforms;
        uniforms.insert(SingleVertexAttribute(u_screenOriginNameID, newScreenPt.x(),newScreenPt.y()));
        uniforms.insert(SingleVertexAttribute(u_ScaleNameID, u_scale.x(), u_scale.y()));
        uniforms.insert(SingleVertexAttribute(u_texScaleNameID, texScale.x(),texScale.y()));
        basicDraw->setUniforms(uniforms);
    }
}

ColorChangeRequest::ColorChangeRequest(SimpleIdentity drawId,RGBAColor inColor)
: DrawableChangeRequest(drawId)
{
    color[0] = inColor.r;
    color[1] = inColor.g;
    color[2] = inColor.b;
    color[3] = inColor.a;
}

void ColorChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
    {
        basicDrawable->setOverrideColor(color);
    } else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setColor(RGBAColor(color[0],color[1],color[2],color[3]));
    }
}

OnOffChangeRequest::OnOffChangeRequest(SimpleIdentity drawId,bool OnOff)
: DrawableChangeRequest(drawId), newOnOff(OnOff)
{
    
}

void OnOffChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable) {
        basicDrawable->setOnOff(newOnOff);
    }
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setEnable(newOnOff);
    }
}

VisibilityChangeRequest::VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis)
: DrawableChangeRequest(drawId), minVis(minVis), maxVis(maxVis)
{
}

void VisibilityChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setVisibleRange(minVis,maxVis);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        basicDrawInst->setVisibleRange(minVis, maxVis);
    }
}

FadeChangeRequest::FadeChangeRequest(SimpleIdentity drawId,TimeInterval fadeUp,TimeInterval fadeDown)
: DrawableChangeRequest(drawId), fadeUp(fadeUp), fadeDown(fadeDown)
{
    
}

void FadeChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    // Fade it out, then remove it
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
    {
        basicDrawable->setFade(fadeDown, fadeUp);
    }
    
    // And let the renderer know
    renderer->setRenderUntil(fadeDown);
    renderer->setRenderUntil(fadeUp);
}

DrawTexChangeRequest::DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId)
: DrawableChangeRequest(drawId), which(which), newTexId(newTexId), relSet(false), relLevel(0), relX(0), relY(0)
{
}

DrawTexChangeRequest::DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId,int size,int borderTexel,int relLevel,int relX,int relY)
: WhirlyKit::DrawableChangeRequest(drawId), which(which), newTexId(newTexId), relSet(true), size(size), borderTexel(borderTexel), relLevel(relLevel), relX(relX), relY(relY)
{
}
    
void DrawTexChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable) {
        basicDrawable->setTexId(which,newTexId);
        if (relSet)
            basicDrawable->setTexRelative(which, size, borderTexel, relLevel, relX, relY);
        else
            basicDrawable->setTexRelative(which, 0, 0, 0, 0, 0);
    } else {
        BasicDrawableInstanceRef refDrawable = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (refDrawable) {
            BasicDrawableRef orgDrawable = refDrawable->getMaster();
            if (orgDrawable) {
                if (orgDrawable->texInfo.size() < which)
                    orgDrawable->setupTexCoordEntry(which, 0);
                refDrawable->setTexId(which,newTexId);
                if (relSet)
                    refDrawable->setTexRelative(which, size, borderTexel, relLevel, relX, relY);
                else
                    refDrawable->setTexRelative(which, 0, 0, 0, 0, 0);
            }
        }
    }
}

DrawTexturesChangeRequest::DrawTexturesChangeRequest(SimpleIdentity drawId,const std::vector<SimpleIdentity> &newTexIDs)
: DrawableChangeRequest(drawId), newTexIDs(newTexIDs)
{
}

void DrawTexturesChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setTexIDs(newTexIDs);
}

TransformChangeRequest::TransformChangeRequest(SimpleIdentity drawId,const Matrix4d *newMat)
: DrawableChangeRequest(drawId), newMat(*newMat)
{
}

void TransformChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDraw = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDraw.get())
        basicDraw->setMatrix(&newMat);
}

DrawPriorityChangeRequest::DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority)
: DrawableChangeRequest(drawId), drawPriority(drawPriority)
{
}

void DrawPriorityChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setDrawPriority(drawPriority);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setDrawPriority(drawPriority);
    }
}

LineWidthChangeRequest::LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth)
: DrawableChangeRequest(drawId), lineWidth(lineWidth)
{
}

void LineWidthChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setLineWidth(lineWidth);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setLineWidth(lineWidth);
    }
}
    
DrawUniformsChangeRequest::DrawUniformsChangeRequest(SimpleIdentity drawID,const SingleVertexAttributeSet &attrs)
    : WhirlyKit::DrawableChangeRequest(drawID), attrs(attrs)
{
}
    
void DrawUniformsChangeRequest::execute2(Scene *scene,SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setUniforms(attrs);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setUniforms(attrs);
    }
}

RenderTargetChangeRequest::RenderTargetChangeRequest(SimpleIdentity drawID,SimpleIdentity targetID)
: WhirlyKit::DrawableChangeRequest(drawID), targetID(targetID)
{
}
    
void RenderTargetChangeRequest::execute2(Scene *scene,SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setRenderTarget(targetID);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setRenderTarget(targetID);
        else {
            ParticleSystemDrawableRef partDrawable = std::dynamic_pointer_cast<ParticleSystemDrawable>(draw);
            if (partDrawable)
                partDrawable->setRenderTarget(targetID);
        }
    }
}

}
