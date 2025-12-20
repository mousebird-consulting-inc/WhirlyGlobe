/*  ParticleSystemDrawableGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
 *  Copyright 2011-2022 mousebird consulting
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

#import "ParticleSystemDrawableGLES.h"
#include "VertexAttributeGLES.h"
#import "SceneGLES.h"
#import "WhirlyKitLog.h"

#include <GLES3/gl32.h>

#ifndef DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
#define DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES 0
#endif
#ifndef DUMP_PARTICLE_VARYING_DATA_WEBGL
#define DUMP_PARTICLE_VARYING_DATA_WEBGL 0 // max number of floats to dump
#endif
#if DUMP_PARTICLE_VARYING_DATA_WEBGL
#include <webgl/webgl2.h>
#include <sstream>
#endif

namespace WhirlyKit
{

ParticleSystemDrawableGLES::ParticleSystemDrawableGLES(std::string name) :
    ParticleSystemDrawable(name),
    DrawableGLES(std::move(name))
{
}

void ParticleSystemDrawableGLES::setupForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    auto setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    if (pointBuffer != 0) {
        return;
    }

    const int pointVertexBytes = vertexSize * numTotalPoints;
    pointBuffer = setupInfo->memManager->getBufferID(pointVertexBytes, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, pointVertexBytes, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Set up rectangles
    if (useRectangles)
    {
        // Build up the coordinates for two rectangles
        if (useInstancing)
        {
            Point2f verts[2*6];
            verts[0] = Point2f(-1,-1);
            verts[1] = Point2f(0,0);
            verts[2] = Point2f(1,-1);
            verts[3] = Point2f(1.0,0);
            verts[4] = Point2f(1,1);
            verts[5] = Point2f(1.0,1.0);
            verts[6] = Point2f(-1,-1);
            verts[7] = Point2f(0,0);
            verts[8] = Point2f(1,1);
            verts[9] = Point2f(1.0,1.0);
            verts[10] = Point2f(-1,1);
            verts[11] = Point2f(0,1.0);
            
            int rectSize = 2*sizeof(float)*6*2;
            rectBuffer = setupInfo->memManager->getBufferID(0,GL_STATIC_DRAW);
            
            glBindBuffer(GL_ARRAY_BUFFER, rectBuffer);
            glBufferData(GL_ARRAY_BUFFER, rectSize, (const GLvoid *)&verts[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else {
            wkLogLevel(Error,"ParticleSystemDrawable: Can only do instanced rectangles at present.  This system can't handle instancing.");
        }
    }

    // If we have varyings we need buffers to hold them
    const GLuint varyingsSize = getTotalVaryingsSize();

    // Allocate two buffers for varying outputs
    if (varyingsSize > 0) {
        const GLuint totalVaryingsSize = varyingsSize * numTotalPoints;
        for (int i = 0; i < 2; ++i) {
            const auto buffer = setupInfo->memManager->getBufferID(totalVaryingsSize, GL_DYNAMIC_DRAW);
#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
            wkLogLevel(Debug,
                       "ParticleSystemDrawableGLES: Allocated varying buffer[%u]=%u of size %u bytes for %d vertices",
                       i,
                       buffer,
                       totalVaryingsSize,
                       batchSize);
#endif

            // Zero out the new buffers
            // That's how we signal that they're new
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, totalVaryingsSize, nullptr);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            varyBuffer.buffers[i] = buffer;
        }
    }
}

void ParticleSystemDrawableGLES::teardownForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene,RenderTeardownInfoRef teardown)
{
    auto setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    if (pointBuffer)
        setupInfo->memManager->removeBufferID(pointBuffer);
    pointBuffer = 0;
    if (rectBuffer) {
        setupInfo->memManager->removeBufferID(rectBuffer);
    }

    for (auto &buffer : varyBuffer.buffers) {
        setupInfo->memManager->removeBufferID(buffer);
        buffer = 0;
    }

    rectBuffer = 0;
    batches.clear();
    chunks.clear();
}

void ParticleSystemDrawableGLES::addAttributeData(const RenderSetupInfo *setupInfo,const std::vector<AttributeData> &attrData,const Batch &batch)
{
    if (attrData.size() != vertAttrs.size()) {
        wkLogLevel(
            Warn, "addAttributeData: Mismatched attribute data size (%zu vs %zu)", attrData.size(), vertAttrs.size());
        return;
    }

    // Note: Android.  Needs hasMapBufferSupport check
    // When the particles initialize themselves we don't have vertex data
    if (vertexSize > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
        std::vector<std::uint8_t> glMem(vertexSize * batchSize);

        // Work through the attribute blocks
        int attrOffset = 0;
        for (unsigned int ai=0;ai<vertAttrs.size();ai++)
        {
            const AttributeData &thisAttrData = attrData[ai];
            SingleVertexAttributeInfo &attrInfo = vertAttrs[ai];
            int attrSize = attrInfo.size();
            auto rawAttrData = (unsigned char *)thisAttrData.data;
            auto *ptr = &glMem[attrOffset];
            // Copy into each vertex
            for (unsigned int ii = 0; ii < batchSize; ii++) {
                memcpy(ptr, rawAttrData, attrSize);
                ptr += vertexSize;
                rawAttrData += attrSize;
            }

            attrOffset += attrSize;
        }

        glBufferSubData(GL_ARRAY_BUFFER, batch.batchID * vertexSize * batchSize, vertexSize * batchSize, glMem.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    std::lock_guard<std::mutex> guardLock(batchLock);
    batches[batch.batchID] = batch;
    batches[batch.batchID].active = true;
    chunksDirty = true;
}

void ParticleSystemDrawableGLES::drawSetupTextures(RendererFrameInfo *frameInfo,Scene *inScene,ProgramGLES *prog,bool hasTexture[],int &progTexBound)
{
    auto scene = (SceneGLES *)inScene;
    
    // GL Texture IDs
    //bool anyTextures = false;
    std::vector<GLuint> glTexIDs;
    for (SimpleIdentity texID : texIDs)
    {
        GLuint glTexID = scene->getGLTexture(texID);
        //anyTextures = true;
        glTexIDs.push_back(glTexID);
    }
    
    // The program itself may have some textures to bind
    progTexBound = prog->bindTextures();
    for (unsigned int ii=0;ii<progTexBound;ii++)
        hasTexture[ii] = true;
    
    // Zero or more textures in the drawable
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures-progTexBound;ii++)
    {
        GLuint glTexID = ii < glTexIDs.size() ? glTexIDs[ii] : 0;
        auto baseMapNameID = baseMapNameIDs[ii];
        auto hasBaseMapNameID = hasBaseMapNameIDs[ii];
        const OpenGLESUniform *texUni = prog->findUniform(baseMapNameID);
        hasTexture[ii+progTexBound] = glTexID != 0 && texUni;
        if (hasTexture[ii+progTexBound])
        {
            glActiveTexture(GL_TEXTURE0+ii+progTexBound);
            glBindTexture(GL_TEXTURE_2D, glTexID);
            CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
            prog->setUniform(baseMapNameID, (int)ii+progTexBound);
            CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
            prog->setUniform(hasBaseMapNameID, 1);
        } else {
            prog->setUniform(hasBaseMapNameID, 0);
        }
    }
}

void ParticleSystemDrawableGLES::drawTeardownTextures(RendererFrameInfo *frameInfo,Scene *scene,ProgramGLES *prog,bool hasTexture[],int progTexBound)
{
    // Unbind any textures
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
        if (hasTexture[ii])
        {
            glActiveTexture(GL_TEXTURE0+ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
}

void ParticleSystemDrawableGLES::drawSetupUniforms(RendererFrameInfo *frameInfo,Scene *scene,ProgramGLES *prog)
{
    const Point2f frameSize = frameInfo->sceneRenderer->getFramebufferSize();

    // Model/View/Projection matrix
    prog->setUniform(mvpMatrixNameID, frameInfo->mvpMat);
    prog->setUniform(mvpInvMatrixNameID, frameInfo->mvpInvMat);
    prog->setUniform(mvMatrixNameID, frameInfo->viewAndModelMat);
    prog->setUniform(mvNormalMatrixNameID, frameInfo->viewModelNormalMat);
    prog->setUniform(mvpNormalMatrixNameID, frameInfo->mvpNormalMat);
    prog->setUniform(u_pMatrixNameID, frameInfo->projMat);
    prog->setUniform(u_ScaleNameID, Point2f(2.f/frameSize.x(),2.f/frameSize.y()));
    
    // Size of a single pixel
    const Point2f pixDispSize = frameInfo->screenSizeInDisplayCoords.cast<float>().cwiseQuotient(frameSize);
    
    // If this is present, the drawable wants to do something based where the viewer is looking
    prog->setUniform(u_EyeVecNameID, frameInfo->fullEyeVec);
    prog->setUniform(u_EyePosNameID, Vector3dToVector3f(frameInfo->eyePos));
    
    prog->setUniform(u_SizeNameID, pointSize);
    prog->setUniform(u_TimeNameID, (float)(frameInfo->currentTime-baseTime));
    prog->setUniform(u_lifetimeNameID, (float)lifetime);
    prog->setUniform(u_pixDispSizeNameID, pixDispSize);
    prog->setUniform(u_frameLenID, (float)frameInfo->frameLen);
}

void ParticleSystemDrawableGLES::drawBindAttrs(RendererFrameInfo *,Scene *,ProgramGLES *prog,const BufferChunk &chunk,int vertexOffset,bool useInstancingHere)
{
    glBindBuffer(GL_ARRAY_BUFFER,pointBuffer);
    
    // Bind the various attributes to their offsets
    int attrOffset = 0;
    for (SingleVertexAttributeInfoGLES &attrInfo : vertAttrs) {
        const int attrSize = attrInfo.size();
        if (const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.nameID)) {
            // If this is an in-out, bind to the transform feedback output buffer instead of the attribute buffer
            const auto hit = std::find_if(inOutVaryings.begin(), inOutVaryings.end(), [&attrInfo](const auto &pair) {
                return attrInfo.nameID == pair.second;
            });
            if (hit != inOutVaryings.end()) {
                // Find the offset into the varying buffer
                GLuint varyOffset = 0;
                SingleVertexAttributeInfoGLES *targetVaryAttr = nullptr;
                for (auto i = varyAttrs.begin(); i != varyAttrs.end(); ++i) {
                    if (i->nameID == hit->first) {
                        targetVaryAttr = &*i;
                        break;
                    }
                    varyOffset += i->size();
                }
                if (targetVaryAttr) {
                    // Bind this attribute to the active transform feedback varying buffer instead
                    // Binding to the "other" buffer results in an error.
                    const GLuint varyingsStride = getTotalVaryingsSize();
                    varyOffset += vertexOffset * varyingsStride;

                    glBindBuffer(GL_ARRAY_BUFFER, varyBuffer.buffers[activeVaryBuffer]);
#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
                    std::string dataStr;
#if DUMP_PARTICLE_VARYING_DATA_WEBGL
                    std::vector<float> debugData(
                        std::min(DUMP_PARTICLE_VARYING_DATA_WEBGL, (int)(targetVaryAttr->size() / sizeof(float))));
                    glGetBufferSubData(GL_ARRAY_BUFFER, varyOffset, debugData.size() * sizeof(float), debugData.data());
                    std::ostringstream debugStr;
                    for (float val : debugData) {
                        debugStr << val << " ";
                    }
                    dataStr = ", data: " + debugStr.str();
#endif

                    wkLogLevel(Info,
                               "%s: Chunk %u-%u Binding attribute %s at index %u, size %u to varying %s buffer %u "
                               "offset %u stride %u%s",
                               prog->getName().c_str(),
                               chunk.vertexStart,
                               chunk.vertexStart + chunk.numVertices - 1,
                               StringIndexer::getString(attrInfo.nameID).c_str(),
                               thisAttr->index,
                               attrSize,
                               StringIndexer::getString(targetVaryAttr->nameID).c_str(),
                               varyBuffer.buffers[activeVaryBuffer],
                               varyOffset,
                               varyingsStride,
                               dataStr.c_str());
#endif

                    glVertexAttribPointer(thisAttr->index,
                                          attrInfo.glEntryComponents(),
                                          attrInfo.glType(),
                                          attrInfo.glNormalize(),
                                          varyingsStride,
                                          (const GLvoid *)(long)(varyOffset));
                    if (useInstancingHere) {
                        const int divisor = useInstancing ? 1 : 0;
                        glVertexAttribDivisor(thisAttr->index, divisor);
                    }
                    glEnableVertexAttribArray(thisAttr->index);
                    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
                    continue;
                }
            }

#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
            std::string dataStr;
#if DUMP_PARTICLE_VARYING_DATA_WEBGL
            std::vector<float> debugData(std::min(DUMP_PARTICLE_VARYING_DATA_WEBGL, (int)(attrSize / sizeof(float))));
            glGetBufferSubData(
                GL_ARRAY_BUFFER, attrOffset + chunk.bufferStart, debugData.size() * sizeof(float), debugData.data());
            std::ostringstream debugStr;
            for (float val : debugData) {
                debugStr << val << " ";
            }
            dataStr = ", data: " + debugStr.str();
#endif
            wkLogLevel(Info,
                       "%s: Chunk %u-%u Binding attribute %s at index %d, size %u, vertexOffset %d, buffer offset %d "
                       "stride %d%s",
                       prog->getName().c_str(),
                       chunk.vertexStart,
                       chunk.vertexStart + chunk.numVertices - 1,
                       StringIndexer::getString(attrInfo.nameID).c_str(),
                       thisAttr->index,
                       attrSize,
                       vertexOffset,
                       attrOffset + chunk.bufferStart,
                       vertexSize,
                       dataStr.c_str());
#endif
            glVertexAttribPointer(thisAttr->index,
                                  attrInfo.glEntryComponents(),
                                  attrInfo.glType(),
                                  attrInfo.glNormalize(),
                                  vertexSize,
                                  (const GLvoid *)(long)(attrOffset + chunk.bufferStart));

            if (useInstancingHere) {
                const int divisor = useInstancing ? 1 : 0;
                glVertexAttribDivisor(thisAttr->index, divisor);
            }
            glEnableVertexAttribArray(thisAttr->index);
        }

        attrOffset += attrSize;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!varyAttrs.empty()) {
        // Link the varying output to attribute array input
        glBindBuffer(GL_ARRAY_BUFFER, varyBuffer.buffers[activeVaryBuffer]);

        // The size of the varyings for a single vertex
        const GLuint varyingsStride = getTotalVaryingsSize();

        GLuint varyingOffset = chunk.vertexStart * varyingsStride;

        for (std::size_t varyWhich = 0; varyWhich < varyAttrs.size(); ++varyWhich) {
            const auto &varyInfo = varyAttrs[varyWhich];
            const GLuint size = varyInfo.size();
            if (const OpenGLESAttribute *thisAttr = prog->findAttribute(varyNames[varyWhich])) {
#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
                std::string dataStr;
#if DUMP_PARTICLE_VARYING_DATA_WEBGL
                std::vector<float> debugData(std::min(DUMP_PARTICLE_VARYING_DATA_WEBGL, (int)(size / sizeof(float))));
                glGetBufferSubData(GL_ARRAY_BUFFER, varyingOffset, debugData.size() * sizeof(float), debugData.data());
                std::ostringstream debugStr;
                for (float val : debugData) {
                    debugStr << val << " ";
                }
                dataStr = ", data: " + debugStr.str();
#endif
                wkLogLevel(Info,
                           "%s: Chunk %u-%u Binding varying to attribute %s (components=%d) at index %d, size %u, "
                           "vertexOffset %d, buffer offset %d, buffer %u, stride %u%s",
                           prog->getName().c_str(),
                           chunk.vertexStart,
                           chunk.vertexStart + chunk.numVertices - 1,
                           StringIndexer::getString(varyInfo.nameID).c_str(),
                           varyInfo.glEntryComponents(),
                           thisAttr->index,
                           size,
                           vertexOffset,
                           varyingOffset,
                           varyBuffer.buffers[activeVaryBuffer],
                           varyingsStride,
                           dataStr.c_str());
#endif

                glVertexAttribPointer(thisAttr->index,
                                      varyInfo.glEntryComponents(),
                                      varyInfo.glType(),
                                      varyInfo.glNormalize(),
                                      varyingsStride,
                                      (const GLvoid *)(long)varyingOffset);

                if (useInstancingHere) {
                    const int divisor = useInstancing ? 1 : 0;
                    glVertexAttribDivisor(thisAttr->index, divisor);
                }
                glEnableVertexAttribArray(thisAttr->index);
            }
            varyingOffset += size;
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ParticleSystemDrawableGLES::drawUnbindAttrs(ProgramGLES *prog)
{
    // Tear down the state
    for (SingleVertexAttributeInfo &attrInfo : vertAttrs)
    {
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.nameID);
        if (thisAttr) {
            glDisableVertexAttribArray(thisAttr->index);
            glVertexAttribDivisor(thisAttr->index, 0);
        }
    }
    for (const auto &name : varyNames)
    {
        const OpenGLESAttribute *thisAttr = prog->findAttribute(name);
        if (thisAttr)
        {
            glDisableVertexAttribArray(thisAttr->index);
            glVertexAttribDivisor(thisAttr->index, 0);
        }
    }
}

void ParticleSystemDrawableGLES::calculate(RendererFrameInfoGLES *frameInfo,Scene *scene)
{
    CheckGLError("BasicDrawable::calculate() glBeginTransformFeedback");
    
    updateBatches(frameInfo->currentTime);
    updateChunks();
    lastUpdateTime = frameInfo->currentTime;
    
    if (chunks.empty())
        return;
    
    auto prog = (ProgramGLES *)frameInfo->program;
    
    if (!prog)
        return;
    
    // Setup the textures for use and set the uniforms
    bool hasTexture[WhirlyKitMaxTextures];
    int progTexBound = 0;
    drawSetupTextures(frameInfo, scene, prog, hasTexture, progTexBound);
    drawSetupUniforms(frameInfo, scene, prog);
    
    // Work through the batches to assign vertex arrays
    for (const BufferChunk &chunk : chunks) {
        drawBindAttrs(frameInfo,scene,prog,chunk,chunk.vertexStart,false);

        // Now bind the varying outputs to their buffer
        const int outputVaryBuffer = (activeVaryBuffer == 0) ? 1 : 0;
        const auto totalVaryingSize = getTotalVaryingsSize();
        glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER,
                          0,
                          varyBuffer.buffers[outputVaryBuffer],
                          chunk.vertexStart * totalVaryingSize,
                          chunk.numVertices * totalVaryingSize);
#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
        wkLogLevel(Info,
                   "%s: Calculating chunk vertexStart = %d, numVertex = %d to buffer %u offset %u",
                   prog->getName().c_str(),
                   chunk.vertexStart,
                   chunk.numVertices,
                   varyBuffer.buffers[outputVaryBuffer],
                   chunk.vertexStart * totalVaryingSize);
#endif

        glBeginTransformFeedback(GL_POINTS);
        CheckGLError("BasicDrawable::calculate() glBeginTransformFeedback");
        
        glDrawArrays(GL_POINTS, 0, chunk.numVertices);
        CheckGLError("BasicDrawable::calculate() glDrawArrays");
        
        glEndTransformFeedback();
        CheckGLError("BasicDrawable::calculate() glEndTransformFeedback");

        glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0, 0, 0);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

#if DUMP_PARTICLE_VARYING_DATA_WEBGL
        std::vector<float> debugData(
            std::min(DUMP_PARTICLE_VARYING_DATA_WEBGL, (int)(totalVaryingSize * chunk.numVertices / sizeof(float))));
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, varyBuffer.buffers[outputVaryBuffer]);
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER,
                           chunk.vertexStart * totalVaryingSize,
                           debugData.size() * sizeof(float),
                           debugData.data());
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

        std::ostringstream debugStream;
        debugStream << prog->getName() << ": Output varying data for chunk starting at vertex " << chunk.vertexStart
                    << ", numVertices " << chunk.numVertices << " offset " << chunk.vertexStart * totalVaryingSize
                    << " to buffer " << varyBuffer.buffers[outputVaryBuffer] << ": ";
        for (std::size_t i = 0; i < debugData.size(); ++i) {
            debugStream << debugData[i] << " ";
        }
        wkLogLevel(Info, "%s", debugStream.str().c_str());
#endif

        drawUnbindAttrs(prog);
    }
    
    // Tear down textures we may have set up
    drawTeardownTextures(frameInfo, scene, prog, hasTexture, progTexBound);
    
    // Switch the active vary buffers (if we're using them)
    activeVaryBuffer = (activeVaryBuffer == 0) ? 1 : 0;
}

void ParticleSystemDrawableGLES::draw(RendererFrameInfoGLES *frameInfo,Scene *scene)
{
    if (lastUpdateTime < frameInfo->currentTime) {
        updateBatches(frameInfo->currentTime);
        updateChunks();
        lastUpdateTime = frameInfo->currentTime;
    }
    
    if (chunks.empty())
        return;
    
    auto prog = (ProgramGLES *)frameInfo->program;
    
    // Sometimes the program is deleted before the drawable (oops)
    if (!prog)
        return;
    
    bool hasTexture[WhirlyKitMaxTextures];
    int progTexBound = 0;
    
    // Setup the textures for use and set the uniforms
    drawSetupTextures(frameInfo, scene, prog, hasTexture, progTexBound);
    drawSetupUniforms(frameInfo, scene, prog);
    
    // Work through the batches
    for (const BufferChunk &chunk : chunks) {
        // Use the rectangle buffer for instancing
        if (rectBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER,rectBuffer);
            const OpenGLESAttribute *thisAttr = prog->findAttribute(a_offsetNameID);
            if (thisAttr)
            {
                glVertexAttribPointer(thisAttr->index, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (const GLvoid *)(long)0);
                CheckGLError("ParticleSystemDrawable::setupVAO glVertexAttribPointer");
                glVertexAttribDivisor(thisAttr->index, 0);
                glEnableVertexAttribArray(thisAttr->index);
                CheckGLError("ParticleSystemDrawable::setupVAO glEnableVertexAttribArray");
            }
            thisAttr = prog->findAttribute(a_texCoordNameID);
            if (thisAttr)
            {
                glVertexAttribPointer(thisAttr->index, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (const GLvoid *)(long)(2*sizeof(GLfloat)));
                CheckGLError("ParticleSystemDrawable::setupVAO glVertexAttribPointer");
                glVertexAttribDivisor(thisAttr->index, 0);
                glEnableVertexAttribArray(thisAttr->index);
                CheckGLError("ParticleSystemDrawable::setupVAO glEnableVertexAttribArray");
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        drawBindAttrs(frameInfo,scene,prog,chunk,chunk.vertexStart,true);

#if DEBUG_PARTICLE_SYSTEM_DRAWABLE_GLES
        wkLogLevel(Info,
                   "%s: Drawing chunk vertexStart = %d, numVertex = %d, instances = %d",
                   prog->getName().c_str(),
                   chunk.vertexStart,
                   chunk.numVertices,
                   rectBuffer ? chunk.numVertices : 1);
#endif

        if (rectBuffer)
        {
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk.numVertices);
            CheckGLError("BasicDrawable::drawVBO2() glDrawArraysInstanced");
        } else {
            glDrawArrays(GL_POINTS, 0, chunk.numVertices);
            CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
        }
        
        if (rectBuffer)
        {
            const OpenGLESAttribute *thisAttr = prog->findAttribute(a_offsetNameID);
            if (thisAttr)
            {
                glDisableVertexAttribArray(thisAttr->index);
                CheckGLError("ParticleSystemDrawable glDisableVertexAttribArray");
            }
            thisAttr = prog->findAttribute(a_texCoordNameID);
            if (thisAttr)
            {
                glDisableVertexAttribArray(thisAttr->index);
                CheckGLError("ParticleSystemDrawable glDisableVertexAttribArray");
            }
        }
        
        drawUnbindAttrs(prog);
    }

    // Tear down any textures we set up
    drawTeardownTextures(frameInfo, scene, prog, hasTexture, progTexBound);
}

static const char *vertexShaderTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_size;
uniform float u_time;

attribute vec3 a_position;
attribute vec4 a_color;
attribute vec3 a_dir;
attribute float a_startTime;

varying vec4 v_color;

void main()
{
    v_color = a_color;
    vec3 thePos = normalize(a_position + (u_time-a_startTime)*a_dir);
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(thePos,1.0);
    pt /= pt.w;
    // Make sure the object is facing the user
    vec4 testNorm = u_mvNormalMatrix * vec4(thePos,0.0);
    float dot_res = dot(-pt.xyz,testNorm.xyz);
    // Set the point size
    gl_PointSize = u_size;
    // Project the point into 3-space
    gl_Position = (dot_res > 0.0) ? u_mvpMatrix * vec4(thePos,1.0) : vec4(1000.0,1000.0,1000.0,0.0);
}
)";

static const char *fragmentShaderTri = R"(
precision highp float;

varying vec4      v_color;

void main()
{
    gl_FragColor = v_color;
}
)";

ProgramGLES *BuildParticleSystemProgramGLES(const std::string &name,SceneRenderer *)
{
    auto shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

}

