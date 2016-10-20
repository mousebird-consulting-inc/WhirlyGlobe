/*
 *  BasicDrawableInstance.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2013 mousebird consulting
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
#import "SceneRendererES.h"
#import "TextureAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableInstance::BasicDrawableInstance(const std::string &name,SimpleIdentity masterID,Style style)
: Drawable(name), programID(EmptyIdentity), enable(true), masterID(masterID), requestZBuffer(false), writeZBuffer(true), startEnable(0.0), endEnable(0.0), instBuffer(0), numInstances(0), vertArrayObj(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), minViewerDist(DrawVisibleInvalid), maxViewerDist(DrawVisibleInvalid), viewerCenter(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid), startTime(0), moving(false), instanceStyle(style)
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
    if (programID != EmptyIdentity)
        return programID;


    return basicDraw->getProgram();
}

bool BasicDrawableInstance::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable ||
            endEnable < frameInfo->currentTime)
            return false;
    }
    
    if (!enable)
        return false;
    
    double visVal = frameInfo->theView->heightAboveSurface();
    
    // Height based check
    if (minVis != DrawVisibleInvalid && maxVis != DrawVisibleInvalid)
    {
        if (!((minVis <= visVal && visVal <= maxVis) ||
              (maxVis <= visVal && visVal <= minVis)))
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
    
    return true;}

GLenum BasicDrawableInstance::getType() const
{
    return basicDraw->getType();
}

bool BasicDrawableInstance::hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    return basicDraw->hasAlpha(frameInfo);
}

void BasicDrawableInstance::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    if (moving)
    {
        // Motion requires continuous rendering
        renderer->addContinuousRenderRequest(getId());
    }
    
    return basicDraw->updateRenderer(renderer);
}

const Eigen::Matrix4d *BasicDrawableInstance::getMatrix() const
{
    return basicDraw->getMatrix();
}
    
void BasicDrawableInstance::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    uniforms = newUniforms;
}

void BasicDrawableInstance::addInstances(const std::vector<SingleInstance> &insts)
{
    instances.insert(instances.end(), insts.begin(), insts.end());
}

void BasicDrawableInstance::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    if (instBuffer)
        return;
    
    numInstances = instances.size();
    
    if (instances.empty())
        return;

    // Always doing color and position matrix
    // Note: Should allow for a list of optional attributes here
    centerSize = sizeof(GLfloat)*3;
    matSize = sizeof(GLfloat)*16;
    colorSize = sizeof(GLubyte)*4;
    if (moving)
    {
        modelDirSize = sizeof(GLfloat)*3;
    } else {
        modelDirSize = 0;
    }
    instSize = centerSize + matSize + colorSize + modelDirSize;
    int bufferSize = instSize * instances.size();
    
    instBuffer = memManager->getBufferID(bufferSize,GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, instBuffer);
    void *glMem = NULL;
    if (hasMapBufferSupport)
    {
//        EAGLContext *context = [EAGLContext currentContext];
//        if (context.API < kEAGLRenderingAPIOpenGLES3)
//            glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//        else
//            glMem = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT);        
    } else {
        glMem = (void *)malloc(bufferSize);
    }
    unsigned char *basePtr = (unsigned char *)glMem;
    for (unsigned int ii=0;ii<instances.size();ii++,basePtr+=instSize)
    {
        const SingleInstance &inst = instances[ii];
        Point3f center3f(inst.center.x(),inst.center.y(),inst.center.z());
        Matrix4f mat = Matrix4dToMatrix4f(inst.mat);
        RGBAColor locColor = inst.colorOverride ? inst.color : color;
        memcpy(basePtr, (void *)center3f.data(), centerSize);
        memcpy(basePtr+centerSize, (void *)mat.data(), matSize);
        memcpy(basePtr+centerSize+matSize, (void *)&locColor.r, colorSize);
        if (moving)
        {
            Point3d modelDir = (inst.endCenter - inst.center)/inst.duration;
            Point3f modelDir3f(modelDir.x(),modelDir.y(),modelDir.z());
            memcpy(basePtr+centerSize+matSize+colorSize, (void *)modelDir3f.data(), modelDirSize);
        }
    }
    
    if (hasMapBufferSupport)
    {
//        if (context.API < kEAGLRenderingAPIOpenGLES3)
//            glUnmapBufferOES(GL_ARRAY_BUFFER);
//        else
//            glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        glBufferData(GL_ARRAY_BUFFER, bufferSize, glMem, GL_STATIC_DRAW);
        free(glMem);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
    
void BasicDrawableInstance::teardownGL(OpenGLMemManager *memManage)
{
    if (instBuffer)
    {
        memManage->removeBufferID(instBuffer);
        instBuffer = 0;
    }
    
    if (vertArrayObj)
    {
        glDeleteVertexArrays(1, &vertArrayObj);
        vertArrayObj = 0;
    }
}

GLuint BasicDrawableInstance::setupVAO(OpenGLES2Program *prog)
{
    vertArrayObj = basicDraw->setupVAO(prog);
    
//    EAGLContext *context = [EAGLContext currentContext];
    
    glBindVertexArray(vertArrayObj);

    glBindBuffer(GL_ARRAY_BUFFER,instBuffer);
    const OpenGLESAttribute *centerAttr = prog->findAttribute("a_modelCenter");
    if (centerAttr)
    {
        glVertexAttribPointer(centerAttr->index, 3, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(0));
        CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
        // Note: Porting
//        if (context.API < kEAGLRenderingAPIOpenGLES3)
//            glVertexAttribDivisorEXT(centerAttr->index, 1);
//        else
            glVertexAttribDivisor(centerAttr->index, 1);
        glEnableVertexAttribArray(centerAttr->index);
        CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
    }
    const OpenGLESAttribute *matAttr = prog->findAttribute("a_singleMatrix");
    if (matAttr)
    {
        for (unsigned int im=0;im<4;im++)
        {
            glVertexAttribPointer(matAttr->index+im, 4, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(centerSize+im*(4*sizeof(GLfloat))));
            CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
            // Note: Porting
//            if (context.API < kEAGLRenderingAPIOpenGLES3)
//                glVertexAttribDivisorEXT(matAttr->index+im, 1);
//            else
                glVertexAttribDivisor(matAttr->index+im, 1);
            glEnableVertexAttribArray(matAttr->index+im);
            CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
        }
    }
    const OpenGLESAttribute *colorAttr = prog->findAttribute("a_color");
    if (colorAttr)
    {
        glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, instSize, (const GLvoid *)(long)(centerSize+matSize));
        CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
        // Note: Porting
//        if (context.API < kEAGLRenderingAPIOpenGLES3)
//            glVertexAttribDivisorEXT(colorAttr->index, 1);
//        else
            glVertexAttribDivisor(colorAttr->index, 1);
        glEnableVertexAttribArray(colorAttr->index);
        CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
    }
    const OpenGLESAttribute *dirAttr = prog->findAttribute("a_modelDir");
    if (moving && dirAttr)
    {
        glVertexAttribPointer(dirAttr->index, 3, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(centerSize+matSize+colorSize));
        CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
        // Note: Porting
//        if (context.API < kEAGLRenderingAPIOpenGLES3)
//            glVertexAttribDivisorEXT(dirAttr->index, 1);
//        else
            glVertexAttribDivisor(dirAttr->index, 1);
        glEnableVertexAttribArray(dirAttr->index);
        CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    return vertArrayObj;
}
    
// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)((long)(base) + (off)))

void BasicDrawableInstance::draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene)
{
    // The old style where we reuse the basic drawable
    if (instanceStyle == ReuseStyle)
    {
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
        basicDraw->setVisibleRange(minVis, maxVis);
        
        Matrix4f oldMvpMat = frameInfo->mvpMat;
        Matrix4f oldMvMat = frameInfo->viewAndModelMat;
        Matrix4f oldMvNormalMat = frameInfo->viewModelNormalMat;
        
        // No matrices, so just one instance
        if (instances.empty())
            basicDraw->draw(frameInfo,scene);
        else {
            // Run through the list of instances
            for (unsigned int ii=0;ii<instances.size();ii++)
            {
                // Change color
                const SingleInstance &singleInst = instances[ii];
                if (singleInst.colorOverride)
                    basicDraw->setColor(singleInst.color);
                else {
                    if (hasColor)
                        basicDraw->setColor(color);
                    else
                        basicDraw->setColor(oldColor);
                }
                
                // Note: Ignoring offsets, so won't work reliably in 2D
                Eigen::Matrix4d newMvpMat = frameInfo->projMat4d * frameInfo->viewTrans4d * frameInfo->modelTrans4d * singleInst.mat;
                Eigen::Matrix4d newMvMat = frameInfo->viewTrans4d * frameInfo->modelTrans4d * singleInst.mat;
                Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();
                
                // Inefficient, but effective
                Matrix4f mvpMat4f = Matrix4dToMatrix4f(newMvpMat);
                Matrix4f mvMat4f = Matrix4dToMatrix4f(newMvpMat);
                Matrix4f mvNormalMat4f = Matrix4dToMatrix4f(newMvNormalMat);
                frameInfo->mvpMat = mvpMat4f;
                frameInfo->viewAndModelMat = mvMat4f;
                frameInfo->viewModelNormalMat = mvNormalMat4f;
                
                basicDraw->draw(frameInfo,scene);
            }
        }
        
        // Set it back
        frameInfo->mvpMat = oldMvpMat;
        frameInfo->viewAndModelMat = oldMvMat;
        frameInfo->viewModelNormalMat = oldMvNormalMat;
        
        if (hasDrawPriority)
            basicDraw->setDrawPriority(oldDrawPriority);
        if (hasColor)
            basicDraw->setColor(oldColor);
        if (hasLineWidth)
            basicDraw->setLineWidth(oldLineWidth);
        basicDraw->setVisibleRange(oldMinVis, oldMaxVis);
        
    } else {
        // Note: Porting
        // New style makes use of OpenGL instancing and makes its own copy of the geometry
//        EAGLContext *context = [EAGLContext currentContext];
        OpenGLES2Program *prog = frameInfo->program;
        
        // Figure out if we're fading in or out
        float fade = 1.0;
        // Note: Time based fade isn't represented in the instance.  Probably should be.
        
        // Deal with the range based fade
        if (frameInfo->heightAboveSurface > 0.0)
        {
            float factor = 1.0;
            if (basicDraw->minVisibleFadeBand != 0.0)
            {
                float a = (frameInfo->heightAboveSurface - minVis)/basicDraw->minVisibleFadeBand;
                if (a >= 0.0 && a < 1.0)
                    factor = a;
            }
            if (basicDraw->maxVisibleFadeBand != 0.0)
            {
                float b = (maxVis - frameInfo->heightAboveSurface)/basicDraw->maxVisibleFadeBand;
                if (b >= 0.0 && b < 1.0)
                    factor = b;
            }
            
            fade = fade * factor;
        }
        
        // Time for motion
        if (moving)
            frameInfo->program->setUniform("u_time", (float)(frameInfo->currentTime - startTime));
        
        // GL Texture IDs
        bool anyTextures = false;
        std::vector<GLuint> glTexIDs;
        for (unsigned int ii=0;ii<basicDraw->texInfo.size();ii++)
        {
            const BasicDrawable::TexInfo &thisTexInfo = basicDraw->texInfo[ii];
            GLuint glTexID = EmptyIdentity;
            if (thisTexInfo.texId != EmptyIdentity)
            {
                glTexID = scene->getGLTexture(thisTexInfo.texId);
                anyTextures = true;
            }
            glTexIDs.push_back(glTexID);
        }
        
        // Model/View/Projection matrix
        prog->setUniform("u_mvpMatrix", frameInfo->mvpMat);
        prog->setUniform("u_mvMatrix", frameInfo->viewAndModelMat);
        prog->setUniform("u_mvNormalMatrix", frameInfo->viewModelNormalMat);
        prog->setUniform("u_mvpNormalMatrix", frameInfo->mvpNormalMat);
        prog->setUniform("u_pMatrix", frameInfo->projMat);
        
        // Fade is always mixed in
        prog->setUniform("u_fade", fade);
        
        // Let the shaders know if we even have a texture
        prog->setUniform("u_hasTexture", anyTextures);
        
        // If this is present, the drawable wants to do something based where the viewer is looking
        prog->setUniform("u_eyeVec", frameInfo->fullEyeVec);
        
        // The program itself may have some textures to bind
        bool hasTexture[WhirlyKitMaxTextures];
        int progTexBound = prog->bindTextures();
        for (unsigned int ii=0;ii<progTexBound;ii++)
            hasTexture[ii] = true;
        
        // Zero or more textures in the drawable
        for (unsigned int ii=0;ii<WhirlyKitMaxTextures-progTexBound;ii++)
        {
            GLuint glTexID = ii < glTexIDs.size() ? glTexIDs[ii] : 0;
            char baseMapName[40];
            sprintf(baseMapName,"s_baseMap%d",ii);
            const OpenGLESUniform *texUni = prog->findUniform(baseMapName);
            hasTexture[ii+progTexBound] = glTexID != 0 && texUni;
            if (hasTexture[ii+progTexBound])
            {
                frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii+progTexBound);
                glBindTexture(GL_TEXTURE_2D, glTexID);
                CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
                prog->setUniform(baseMapName, (int)ii+progTexBound);
                CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
            }
        }
        
        // If necessary, set up the VAO (once)
        if (vertArrayObj == 0 && basicDraw->sharedBuffer !=0)
            setupVAO(prog);
        
        // Figure out what we're using
        const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
        
        // Vertex array
        bool usedLocalVertices = false;
        if (vertAttr && !(basicDraw->sharedBuffer || basicDraw->pointBuffer))
        {
            usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &basicDraw->points[0]);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }
        
        // Other vertex attributes
        const OpenGLESAttribute *progAttrs[basicDraw->vertexAttributes.size()];
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
        {
            VertexAttribute *attr = basicDraw->vertexAttributes[ii];
            const OpenGLESAttribute *progAttr = prog->findAttribute(attr->name);
            progAttrs[ii] = NULL;
            if (progAttr)
            {
                // The data hasn't been downloaded, so hook it up directly here
                if (attr->buffer == 0)
                {
                    // We have a data array for it, so hand that over
                    if (attr->numElements() != 0)
                    {
                        glVertexAttribPointer(progAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), 0, attr->addressForElement(0));
                        CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
                        glEnableVertexAttribArray ( progAttr->index );
                        CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
                        
                        progAttrs[ii] = progAttr;
                    } else {
                        // The program is expecting it, so we need a default
                        // Note: Could be doing this in the VAO
                        attr->glSetDefault(progAttr->index);
                        CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                    }
                }
            }
        }
        
        // Let a subclass bind anything additional
        basicDraw->bindAdditionalRenderObjects(frameInfo,scene);
        
        // If there are no instances, fill in the identity
        if (!instBuffer)
        {
            // Set the singleMatrix attribute to identity
            const OpenGLESAttribute *matAttr = prog->findAttribute("a_singleMatrix");
            if (matAttr)
            {
                glVertexAttrib4f(matAttr->index,1.0,0.0,0.0,0.0);
                glVertexAttrib4f(matAttr->index+1,0.0,1.0,0.0,0.0);
                glVertexAttrib4f(matAttr->index+2,0.0,0.0,1.0,0.0);
                glVertexAttrib4f(matAttr->index+3,0.0,0.0,0.0,1.0);
            }
        }
        // No direction data, so provide an empty default
        if (!instBuffer || modelDirSize == 0)
        {
            const OpenGLESAttribute *dirAttr = prog->findAttribute("a_modelDir");
            if (dirAttr)
                glVertexAttrib3f(dirAttr->index, 0.0, 0.0, 0.0);
        }
        
        // If we're using a vertex array object, bind it and draw
        if (vertArrayObj)
        {
            glBindVertexArray(vertArrayObj);
            
            switch (basicDraw->type)
            {
                case GL_TRIANGLES:
                    if (instBuffer)
                    {
                        // Note: Porting
//                        if (context.API < kEAGLRenderingAPIOpenGLES3)
//                            glDrawElementsInstancedEXT(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(basicDraw->sharedBufferOffset,basicDraw->triBuffer), numInstances);
//                        else
                            glDrawElementsInstanced(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(basicDraw->sharedBufferOffset,basicDraw->triBuffer), numInstances);
                    } else
                        glDrawElements(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(basicDraw->sharedBufferOffset,basicDraw->triBuffer));
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    break;
                case GL_POINTS:
                case GL_LINES:
                case GL_LINE_STRIP:
                case GL_LINE_LOOP:
                    frameInfo->stateOpt->setLineWidth(lineWidth);
                    if (instBuffer)
                    {
                        // Note: Porting
//                        if (context.API < kEAGLRenderingAPIOpenGLES3)
//                            glDrawArraysInstancedEXT(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                        else
                            glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
                case GL_TRIANGLE_STRIP:
                    if (instBuffer)
                    {
                        // Note: Porting
//                        if (context.API < kEAGLRenderingAPIOpenGLES3)
//                            glDrawArraysInstancedEXT(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                        else
                            glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
            }
            
            glBindVertexArray(0);
        } else {
            // Draw without a VAO
            switch (basicDraw->type)
            {
                case GL_TRIANGLES:
                {
                    if (basicDraw->triBuffer)
                    {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basicDraw->triBuffer);
                        CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
                        if (instBuffer)
                        {
                            // Note: Porting
//                            if (context.API < kEAGLRenderingAPIOpenGLES3)
//                                glDrawElementsInstancedEXT(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, 0, numInstances);
//                            else
                                glDrawElementsInstanced(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, 0, numInstances);
                        } else
                            glDrawElements(GL_TRIANGLES, basicDraw->numTris*3, GL_UNSIGNED_SHORT, 0);
                        CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } else {
                        if (instBuffer)
                        {
                            // Note: Porting
//                            if (context.API < kEAGLRenderingAPIOpenGLES3)
//                                glDrawElementsInstancedEXT(GL_TRIANGLES, (GLsizei)basicDraw->tris.size()*3, GL_UNSIGNED_SHORT, &basicDraw->tris[0], numInstances);
//                            else
                                glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)basicDraw->tris.size()*3, GL_UNSIGNED_SHORT, &basicDraw->tris[0], numInstances);
                        } else
                            glDrawElements(GL_TRIANGLES, (GLsizei)basicDraw->tris.size()*3, GL_UNSIGNED_SHORT, &basicDraw->tris[0]);
                        CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    }
                }
                    break;
                case GL_POINTS:
                case GL_LINES:
                case GL_LINE_STRIP:
                case GL_LINE_LOOP:
                    frameInfo->stateOpt->setLineWidth(lineWidth);
                    CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
                    if (instBuffer)
                    {
                        // Note: Porting
//                        if (context.API < kEAGLRenderingAPIOpenGLES3)
//                            glDrawArraysInstancedEXT(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                        else
                            glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
                case GL_TRIANGLE_STRIP:
                    if (instBuffer)
                    {
                        // Note: Porting
//                        if (context.API < kEAGLRenderingAPIOpenGLES3)
//                            glDrawArraysInstancedEXT(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                        else
                            glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
            }
        }
        
        // Unbind any textures
        for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
            if (hasTexture[ii])
            {
                frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        
        // Tear down the various arrays, if we stood them up
        if (usedLocalVertices)
            glDisableVertexAttribArray(vertAttr->index);
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
            if (progAttrs[ii])
                glDisableVertexAttribArray(progAttrs[ii]->index);
        
        if (instBuffer)
        {
            const OpenGLESAttribute *centerAttr = prog->findAttribute("a_modelCenter");
            if (centerAttr)
            {
                glDisableVertexAttribArray(centerAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *matAttr = prog->findAttribute("a_singleMatrix");
            if (matAttr)
            {
                for (unsigned int im=0;im<4;im++)
                    glDisableVertexAttribArray(matAttr->index+im);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *colorAttr = prog->findAttribute("a_color");
            if (colorAttr)
            {
                glDisableVertexAttribArray(colorAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *dirAttr = prog->findAttribute("a_modelDir");
            if (dirAttr)
            {
                glDisableVertexAttribArray(dirAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
        }
        
        // Let a subclass clean up any remaining state
        basicDraw->postDrawCallback(frameInfo,scene);
    }
}

}
