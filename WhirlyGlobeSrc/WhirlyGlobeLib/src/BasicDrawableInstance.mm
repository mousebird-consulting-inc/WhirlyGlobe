/*
 *  BasicDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import "GLUtils.h"
#import "BasicDrawableInstance.h"
#import "GlobeScene.h"
#import "UIImage+Stuff.h"
#import "SceneRendererES.h"
#import "TextureAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableInstance::BasicDrawableInstance(const std::string &name,SimpleIdentity masterID)
: Drawable(name), enable(true), masterID(masterID), requestZBuffer(false), writeZBuffer(true), startEnable(0.0), endEnable(0.0)
{
}

Mbr BasicDrawableInstance::getLocalMbr() const
{
    return basicDraw->getLocalMbr();
}

unsigned int BasicDrawableInstance::getDrawPriority() const
{
    if (hasDrawPriority)
        return drawPriority;
    return basicDraw->getDrawPriority();
}

SimpleIdentity BasicDrawableInstance::getProgram() const
{
    return basicDraw->getProgram();
}

bool BasicDrawableInstance::isOn(WhirlyKitRendererFrameInfo *frameInfo) const
{
    if (minVis == DrawVisibleInvalid || !enable)
        return enable;
    
    double visVal = [frameInfo.theView heightAboveSurface];
    
    bool test = ((minVis <= visVal && visVal <= maxVis) ||
                 (maxVis <= visVal && visVal <= minVis));
    return test;
}

GLenum BasicDrawableInstance::getType() const
{
    return basicDraw->getType();
}

bool BasicDrawableInstance::hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const
{
    return basicDraw->hasAlpha(frameInfo);
}

void BasicDrawableInstance::updateRenderer(WhirlyKitSceneRendererES *renderer)
{
    return basicDraw->updateRenderer(renderer);
}

const Eigen::Matrix4d *BasicDrawableInstance::getMatrix() const
{
    return basicDraw->getMatrix();
}

void BasicDrawableInstance::addInstances(const std::vector<SingleInstance> &insts)
{
    instances.insert(instances.end(), insts.begin(), insts.end());
}

void BasicDrawableInstance::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    whichInstance = -1;
    
    int oldDrawPriority = basicDraw->getDrawPriority();
    RGBAColor oldColor = basicDraw->getColor();
    float oldLineWidth = basicDraw->getLineWidth();
    float oldMinVis,oldMaxVis;
    basicDraw->getVisibleRange(oldMinVis, oldMaxVis);
    
    // Change the drawable
    if (hasDrawPriority)
        basicDraw->setDrawPriority(drawPriority);
    if (hasColor)
        basicDraw->setColor(color);
    if (hasLineWidth)
        basicDraw->setLineWidth(lineWidth);
    if (hasMinVis || hasMaxVis)
        basicDraw->setVisibleRange(minVis, maxVis);
    
    Matrix4f oldMvpMat = frameInfo.mvpMat;
    Matrix4f oldMvMat = frameInfo.viewAndModelMat;
    Matrix4f oldMvNormalMat = frameInfo.viewModelNormalMat;
    
    // No matrices, so just one instance
    if (instances.empty())
        basicDraw->draw(frameInfo,scene);
    else {
        // Run through the list of instances
        for (unsigned int ii=0;ii<instances.size();ii++)
        {
            // Change color
            const SingleInstance &singleInst = instances[ii];
            whichInstance = ii;
            if (singleInst.colorOverride)
                basicDraw->setColor(singleInst.color);
            else {
                if (hasColor)
                    basicDraw->setColor(color);
                else
                    basicDraw->setColor(oldColor);
            }
            
            // Note: Ignoring offsets, so won't work reliably in 2D
            Eigen::Matrix4d newMvpMat = frameInfo.projMat4d * frameInfo.viewTrans4d * frameInfo.modelTrans4d * singleInst.mat;
            Eigen::Matrix4d newMvMat = frameInfo.viewTrans4d * frameInfo.modelTrans4d * singleInst.mat;
            Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();
            
            // Inefficient, but effective
            Matrix4f mvpMat4f = Matrix4dToMatrix4f(newMvpMat);
            Matrix4f mvMat4f = Matrix4dToMatrix4f(newMvpMat);
            Matrix4f mvNormalMat4f = Matrix4dToMatrix4f(newMvNormalMat);
            frameInfo.mvpMat = mvpMat4f;
            frameInfo.viewAndModelMat = mvMat4f;
            frameInfo.viewModelNormalMat = mvNormalMat4f;
            
            basicDraw->draw(frameInfo,scene);
        }
    }
    
    frameInfo.mvpMat = oldMvpMat;
    frameInfo.viewAndModelMat = oldMvMat;
    frameInfo.viewModelNormalMat = oldMvNormalMat;
    
    // Set it back
    if (hasDrawPriority)
        basicDraw->setDrawPriority(oldDrawPriority);
    if (hasColor)
        basicDraw->setColor(oldColor);
    if (hasLineWidth)
        basicDraw->setLineWidth(oldLineWidth);
    if (hasMinVis || hasMaxVis)
        basicDraw->setVisibleRange(oldMinVis, oldMaxVis);
}
    
}
