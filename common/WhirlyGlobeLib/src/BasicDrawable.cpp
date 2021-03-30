/*  BasicDrawable.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "Program.h"
#import "BasicDrawable.h"
#import "BasicDrawableInstance.h"
#import "ParticleSystemDrawable.h"
#import "SceneRenderer.h"
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
: Drawable(name), motion(false)
{
}

BasicDrawable::~BasicDrawable()
{
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        delete vertexAttributes[ii];
    vertexAttributes.clear();
}

void BasicDrawable::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    bool changes = false;
    
    if (which >= texInfo.size())
        return;
    
    TexInfo &ti = texInfo[which];
    if (ti.size != size || ti.borderTexel != borderTexel ||
        ti.relLevel != relLevel || ti.relX != relX || ti.relY != relY) {
        ti.size = size;
        ti.borderTexel = borderTexel;
        ti.relLevel = relLevel;
        ti.relX = relX;
        ti.relY = relY;
        changes = true;
    }
    
    if (changes)
        setTexturesChanged();
}

SimpleIdentity BasicDrawable::getProgram() const
{
    return programId;
}

void BasicDrawable::setProgram(SimpleIdentity progId)
{
    if (programId == progId)
        return;
    
    setValuesChanged();

    programId = progId;
}

int64_t BasicDrawable::getDrawOrder() const
{
    return drawOrder;
}

unsigned int BasicDrawable::getDrawPriority() const
{
    return drawPriority;
}

bool BasicDrawable::isOn(RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable)
            return false;
        if (endEnable != 0.0 && endEnable < frameInfo->currentTime)
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
    
    // Zoom based check.  We need to be in the current zoom range
    if (zoomSlot > -1 && zoomSlot <= MaplyMaxZoomSlots) {
        float zoom = frameInfo->scene->getZoomSlotValue(zoomSlot);
        if (zoom != MAXFLOAT) {
            if (minZoomVis != DrawVisibleInvalid && zoom < minZoomVis)
                return false;
            if (maxZoomVis != DrawVisibleInvalid && zoom >= maxZoomVis)
                return false;
        }
    }
    
    return true;
}

void BasicDrawable::setOnOff(bool onOff)
{
    if (on == onOff)
        return;
    
    setValuesChanged();

    on = onOff;
}
    
bool BasicDrawable::hasMotion() const
{
    return motion;
}

Mbr BasicDrawable::getLocalMbr() const
{
    return localMbr;
}

void BasicDrawable::setDrawOrder(int64_t newOrder)
{
    if (newOrder != drawOrder)
    {
        drawOrder = newOrder;
        setValuesChanged();
    }
}

void BasicDrawable::setDrawPriority(unsigned int newPriority)
{
    if (drawPriority == newPriority)
        return;
    
    setValuesChanged();

    drawPriority = newPriority;
}
    
void BasicDrawable::setMatrix(const Eigen::Matrix4d *inMat)
{
    setValuesChanged();

    mat = *inMat; hasMatrix = true;
}
    
const std::vector<BasicDrawable::TexInfo> &BasicDrawable::getTexInfo()
{
    return texInfo;
}

void BasicDrawable::setTexId(unsigned int which,SimpleIdentity inId)
{
    if (which >= 0 && which < texInfo.size()) {
        if (texInfo[which].texId == inId)
            return;

        texInfo[which].texId = inId;

        setTexturesChanged();
    }
}

void BasicDrawable::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    bool changes = false;
    
    for (unsigned int ii=0;ii<std::min(texIDs.size(),texInfo.size());ii++)
    {
        if (texInfo[ii].texId != texIDs[ii]) {
            texInfo[ii].texId = texIDs[ii];
            changes = true;
        }
    }

    if (changes)
        setTexturesChanged();
}
    
void BasicDrawable::setOverrideColor(RGBAColor inColor)
{
    if (hasOverrideColor && color == inColor)
        return;
    
    color = inColor;
    hasOverrideColor = true;

    setValuesChanged();
}

void BasicDrawable::setOverrideColor(unsigned char inColor[])
{
    setOverrideColor(RGBAColor(inColor[0],inColor[1],inColor[2],inColor[3]));
}

void BasicDrawable::setVisibleRange(float minVis,float maxVis,float minVisBand,float maxVisBand)
{
    if (minVisible == minVis && maxVisible == maxVis &&
        minVisibleFadeBand == minVisBand && maxVisibleFadeBand == maxVisBand)
        return;
    
    setValuesChanged();

    minVisible = minVis;  maxVisible = maxVis;  minVisibleFadeBand = minVisBand; maxVisibleFadeBand = maxVisBand;
}
    
void BasicDrawable::setViewerVisibility(double inMinViewerDist,double inMaxViewerDist,const Point3d &inViewerCenter)
{
    if (minViewerDist == inMinViewerDist && maxViewerDist == inMaxViewerDist &&
        viewerCenter == inViewerCenter)
        return;
    
    setValuesChanged();

    minViewerDist = inMinViewerDist;
    maxViewerDist = inMaxViewerDist;
    viewerCenter = inViewerCenter;
}

void BasicDrawable::setZoomInfo(int inZoomSlot,double inMinZoomVis,double inMaxZoomVis)
{
    if (zoomSlot == inZoomSlot && minZoomVis == inMinZoomVis && maxZoomVis == inMaxZoomVis)
        return;
    
    setValuesChanged();
    
    zoomSlot = inZoomSlot;
    minZoomVis = inMinZoomVis;
    maxZoomVis = inMaxZoomVis;
}

void BasicDrawable::setFade(TimeInterval inFadeDown,TimeInterval inFadeUp)
{
    if (fadeUp == inFadeUp && fadeDown == inFadeDown)
        return;
    
    setValuesChanged();

    fadeUp = inFadeUp;  fadeDown = inFadeDown;
}

void BasicDrawable::setLineWidth(float inWidth)
{
    if (lineWidth == inWidth)
        return;
    
    setValuesChanged();

    lineWidth = inWidth;
}

void BasicDrawable::setRequestZBuffer(bool val)
{
    if (requestZBuffer == val)
        return;
    
    setValuesChanged();

    requestZBuffer = val;
}

bool BasicDrawable::getRequestZBuffer() const
{ return requestZBuffer; }

void BasicDrawable::setWriteZBuffer(bool val)
{
    if (writeZBuffer == val)
        return;
    
    setValuesChanged();
    writeZBuffer = val;
}

bool BasicDrawable::getWriteZbuffer() const
{ return writeZBuffer; }
    
SimpleIdentity BasicDrawable::getRenderTarget() const
{
    return renderTargetID;
}
    
void BasicDrawable::setRenderTarget(SimpleIdentity newRenderTarget)
{
    if (renderTargetID == newRenderTarget)
        return;
    
    setValuesChanged();

    renderTargetID = newRenderTarget;
}
    
// If we're fading in or out, update the rendering window
void BasicDrawable::updateRenderer(SceneRenderer *renderer)
{
    renderer->setRenderUntil(fadeUp);
    renderer->setRenderUntil(fadeDown);
    
    if (startEnable != endEnable)
    {
        // Note: This still means we'll render until endEnable
        renderer->setRenderUntil(startEnable);
        renderer->setRenderUntil(endEnable);
    } else if (motion) {
        // Motion requires continuous rendering
        renderer->addContinuousRenderRequest(getId());
    }
    if (extraFrames > 0)
        renderer->addExtraFrameRenderRequest(getId(), extraFrames);
}

SimpleIdentity BasicDrawable::getCalculationProgram() const
{
    return EmptyIdentity;
}
        
/// Return the active transform matrix, if we have one
const Eigen::Matrix4d *BasicDrawable::getMatrix() const
{ if (hasMatrix) return &mat;  return NULL; }
    
void BasicDrawable::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    setValuesChanged();

    uniforms = newUniforms;
}

void BasicDrawable::setUniform(SimpleIdentity nameID, float val)
{
    SingleVertexAttribute attr;
    attr.nameID = nameID;
    attr.slot = -1;
    attr.type = BDFloatType;
    attr.data.floatVal = val;

    setUniform(attr);
}

void BasicDrawable::setUniform(SimpleIdentity nameID, int val)
{
    SingleVertexAttribute attr;
    attr.nameID = nameID;
    attr.slot = -1;
    attr.type = BDIntType;
    attr.data.intVal = val;

    setUniform(attr);
}

void BasicDrawable::setUniform(SimpleIdentity nameID, const Eigen::Vector2f &vec)
{
    SingleVertexAttribute attr;
    attr.nameID = nameID;
    attr.slot = -1;
    attr.type = BDFloat2Type;
    attr.data.vec2[0] = vec.x();
    attr.data.vec2[1] = vec.y();

    setUniform(attr);
}

void BasicDrawable::setUniform(SimpleIdentity nameID, const Eigen::Vector3f &vec)
{
    SingleVertexAttribute attr;
    attr.nameID = nameID;
    attr.slot = -1;
    attr.type = BDFloat2Type;
    attr.data.vec3[0] = vec.x();
    attr.data.vec3[1] = vec.y();
    attr.data.vec3[2] = vec.z();

    setUniform(attr);
}

void BasicDrawable::setUniform(SimpleIdentity nameID, const Eigen::Vector4f &vec)
{
    SingleVertexAttribute attr;
    attr.nameID = nameID;
    attr.slot = -1;
    attr.type = BDFloat2Type;
    attr.data.vec4[0] = vec.x();
    attr.data.vec4[1] = vec.y();
    attr.data.vec4[2] = vec.z();
    attr.data.vec4[3] = vec.w();

    setUniform(attr);
}

void BasicDrawable::setUniform(const SingleVertexAttribute &attr)
{
    auto it = uniforms.find(attr);
    if (it != uniforms.end())
        uniforms.erase(it);
    uniforms.insert(attr);
}

void BasicDrawable::setUniBlock(const UniformBlock &uniBlock)
{
    setValuesChanged();

    for (int ii=0;ii<uniBlocks.size();ii++)
        if (uniBlocks[ii].bufferID == uniBlock.bufferID) {
            uniBlocks[ii] = uniBlock;
            return;
        }
    
    uniBlocks.push_back(uniBlock);
}
    
void BasicDrawable::addTweaker(const DrawableTweakerRef &tweak)
{
    setValuesChanged();

    tweakers.insert(tweak);
}

void BasicDrawable::setValuesChanged()
{
    valuesChanged = true;
    if (renderTargetCon)
        renderTargetCon->modified = true;
}

void BasicDrawable::setTexturesChanged()
{
    texturesChanged = true;
    if (renderTargetCon)
        renderTargetCon->modified = true;
}

BasicDrawableTexTweaker::BasicDrawableTexTweaker(std::vector<SimpleIdentity> &&texIDs,TimeInterval startTime,double period)
    : texIDs(std::move(texIDs)), startTime(startTime), period(period)
{
}

BasicDrawableTexTweaker::BasicDrawableTexTweaker(const std::vector<SimpleIdentity> &texIDs,TimeInterval startTime,double period)
    : texIDs(texIDs), startTime(startTime), period(period)
{
}

void BasicDrawableTexTweaker::tweakForFrame(Drawable *draw,RendererFrameInfo *frame)
{
    BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(draw);
    
    double t = fmod(frame->currentTime-startTime,period)/period;
    int base = floor(t * texIDs.size());
    int next = (base+1)%texIDs.size();
    double interp = t*texIDs.size()-base;
    
    basicDraw->setTexId(0, texIDs[base]);
    basicDraw->setTexId(1, texIDs[next]);

    // Interpolation as well
    SingleVertexAttributeSet uniforms;
    uniforms.insert(SingleVertexAttribute(u_interpNameID,-1,(float)interp));
    basicDraw->setUniforms(uniforms);

    // This forces a redraw every frame
    // Note: There has to be a better way
    frame->scene->addChangeRequest(nullptr);
}
    
BasicDrawableScreenTexTweaker::BasicDrawableScreenTexTweaker(const Point3d &centerPt,const Point2d &texScale)
    : centerPt(centerPt), texScale(texScale)
{
}
    
void BasicDrawableScreenTexTweaker::tweakForFrame(Drawable *draw,RendererFrameInfo *frameInfo)
{
    BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(draw);

    if (basicDraw && frameInfo->program)
    {
        Vector4f screenPt = frameInfo->mvpMat * Vector4f(centerPt.x(),centerPt.y(),centerPt.z(),1.0);
        screenPt /= screenPt.w();
        
        Point2f u_scale = frameInfo->sceneRenderer->getFramebufferSize() / 2.f;
        Point2f newScreenPt(fmod(-screenPt.x()*texScale.x()*u_scale.x(),1.0),fmod(-screenPt.y()*texScale.y()*u_scale.y(),1.0));
        newScreenPt.x() /= texScale.x()*u_scale.x();
        newScreenPt.y() /= texScale.y()*u_scale.y();

        SingleVertexAttributeSet uniforms;
        uniforms.insert(SingleVertexAttribute(u_screenOriginNameID, -1, newScreenPt.x(),newScreenPt.y()));
        uniforms.insert(SingleVertexAttribute(u_ScaleNameID, -1, u_scale.x(), u_scale.y()));
        uniforms.insert(SingleVertexAttribute(u_texScaleNameID, -1, texScale.x(),texScale.y()));
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

void ColorChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    if (auto basicDrawable = dynamic_cast<BasicDrawable*>(draw.get()))
    {
        basicDrawable->setOverrideColor(color);
    }
    else if (auto basicDrawInst = dynamic_cast<BasicDrawableInstance*>(draw.get()))
    {
        basicDrawInst->setColor(RGBAColor(color[0],color[1],color[2],color[3]));
    }
}

OnOffChangeRequest::OnOffChangeRequest(SimpleIdentity drawId,bool OnOff)
: DrawableChangeRequest(drawId), newOnOff(OnOff)
{
}

void OnOffChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    if (auto basicDrawable = dynamic_cast<BasicDrawable*>(draw.get()))
    {
        basicDrawable->setOnOff(newOnOff);
    }
    else if (auto basicDrawInst = dynamic_cast<BasicDrawableInstance*>(draw.get()))
    {
        basicDrawInst->setEnable(newOnOff);
    }
    else if (auto partSys = dynamic_cast<ParticleSystemDrawable*>(draw.get()))
    {
        partSys->setOnOff(newOnOff);
    }
}

VisibilityChangeRequest::VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis)
: DrawableChangeRequest(drawId), minVis(minVis), maxVis(maxVis)
{
}

void VisibilityChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    if (auto basicDrawable = dynamic_cast<BasicDrawable*>(draw.get()))
    {
        basicDrawable->setVisibleRange(minVis,maxVis);
    }
    else if (auto basicDrawInst = dynamic_cast<BasicDrawableInstance*>(draw.get()))
    {
        basicDrawInst->setVisibleRange(minVis, maxVis);
    }
}

FadeChangeRequest::FadeChangeRequest(SimpleIdentity drawId,TimeInterval fadeUp,TimeInterval fadeDown)
: DrawableChangeRequest(drawId), fadeUp(fadeUp), fadeDown(fadeDown)
{
    
}

void FadeChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    // Fade it out, then remove it
    if (auto basicDrawable = dynamic_cast<BasicDrawable*>(draw.get()))
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
: DrawableChangeRequest(drawId), which(which), newTexId(newTexId), relSet(true), size(size), borderTexel(borderTexel), relLevel(relLevel), relX(relX), relY(relY)
{
}
    
void DrawTexChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
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
                if (orgDrawable->getTexInfo().size() < which) {
                    wkLogLevel(Error,"DrawTexChangeRequest: Asked to change texture entry that doesn't exit.");
                    return;
                }
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

void DrawTexturesChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setTexIDs(newTexIDs);
}

TransformChangeRequest::TransformChangeRequest(SimpleIdentity drawId,const Matrix4d *newMat)
: DrawableChangeRequest(drawId), newMat(*newMat)
{
}

void TransformChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDraw = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDraw.get())
        basicDraw->setMatrix(&newMat);
}

DrawOrderChangeRequest::DrawOrderChangeRequest(SimpleIdentity drawId,int64_t drawOrder)
: DrawableChangeRequest(drawId), drawOrder(drawOrder)
{
}

void DrawOrderChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{

    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable && basicDrawable->drawOrder != drawOrder) {
        renderer->removeDrawable(draw,false,NULL);
        basicDrawable->setDrawOrder(drawOrder);
        renderer->addDrawable(draw);
    } else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst && basicDrawInst->getDrawOrder() != drawOrder) {
            renderer->removeDrawable(draw,false,NULL);
            basicDrawInst->setDrawOrder(drawOrder);
            renderer->addDrawable(draw);
        }
    }
}

DrawPriorityChangeRequest::DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority)
: DrawableChangeRequest(drawId), drawPriority(drawPriority)
{
}

void DrawPriorityChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{

    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable && basicDrawable->drawPriority != drawPriority) {
        renderer->removeDrawable(draw,false,NULL);
        basicDrawable->setDrawPriority(drawPriority);
        renderer->addDrawable(draw);
    } else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst && basicDrawInst->getDrawPriority() != drawPriority) {
            renderer->removeDrawable(draw,false,NULL);
            basicDrawInst->setDrawPriority(drawPriority);
            renderer->addDrawable(draw);
        }
    }
}

LineWidthChangeRequest::LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth)
: DrawableChangeRequest(drawId), lineWidth(lineWidth)
{
}

void LineWidthChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
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
    : DrawableChangeRequest(drawID), attrs(attrs)
{
}
    
void DrawUniformsChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
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
: DrawableChangeRequest(drawID), targetID(targetID)
{
}
    
void RenderTargetChangeRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    renderer->removeDrawable(draw,false,NULL);
    
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
    
    renderer->addDrawable(draw);
}
    
UniformBlockSetRequest::UniformBlockSetRequest(SimpleIdentity drawID,const RawDataRef &uniData,int bufferID)
    : DrawableChangeRequest(drawID)
{
    uniBlock.blockData = uniData;
    uniBlock.bufferID = bufferID;
}
    
void UniformBlockSetRequest::execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setUniBlock(uniBlock);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setUniBlock(uniBlock);
        else {
            ParticleSystemDrawableRef partDrawable = std::dynamic_pointer_cast<ParticleSystemDrawable>(draw);
            if (partDrawable)
                partDrawable->setUniBlock(uniBlock);
        }
    }
}

}
