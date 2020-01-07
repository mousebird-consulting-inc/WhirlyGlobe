/*
 *  BasicDrawableInstanceGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
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

#import "BasicDrawableInstanceGLES.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawableInstanceGLES::BasicDrawableInstanceGLES(const std::string &name)
    : BasicDrawableInstance(name), Drawable(name), instBuffer(0), vertArrayObj(0)
{
}
    
GLuint BasicDrawableInstanceGLES::setupVAO(RendererFrameInfoGLES *frameInfo)
{
    ProgramGLES *prog = (ProgramGLES *)frameInfo->program;
    
    BasicDrawableGLES *basicDrawGL = dynamic_cast<BasicDrawableGLES *>(basicDraw.get());
    vertArrayObj = basicDrawGL->setupVAO(prog);
    vertArrayDefaults = basicDrawGL->vertArrayDefaults;
    
    glBindVertexArray(vertArrayObj);
    
    glBindBuffer(GL_ARRAY_BUFFER,instBuffer);
    {
        const OpenGLESAttribute *centerAttr = prog->findAttribute(a_modelCenterNameID);
        if (centerAttr)
        {
            glVertexAttribPointer(centerAttr->index, 3, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(0));
            CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
            glVertexAttribDivisor(centerAttr->index, 1);
            glEnableVertexAttribArray(centerAttr->index);
            CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
        }
    }
    {
        const OpenGLESAttribute *matAttr = prog->findAttribute(a_SingleMatrixNameID);
        if (matAttr)
        {
            for (unsigned int im=0;im<4;im++)
            {
                glVertexAttribPointer(matAttr->index+im, 4, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(centerSize+im*(4*sizeof(GLfloat))));
                CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
                glVertexAttribDivisor(matAttr->index+im, 1);
                glEnableVertexAttribArray(matAttr->index+im);
                CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
            }
        }
    }
    {
        const OpenGLESAttribute *useColorAttr = prog->findAttribute(a_useInstanceColorNameID);
        if (useColorAttr)
        {
            glVertexAttribPointer(useColorAttr->index, 1, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(centerSize+matSize));
            CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
            glVertexAttribDivisor(useColorAttr->index, 1);
            glEnableVertexAttribArray(useColorAttr->index);
            CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
        }
    }
    {
        const OpenGLESAttribute *colorAttr = prog->findAttribute(a_instanceColorNameID);
        if (colorAttr)
        {
            glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, instSize, (const GLvoid *)(long)(centerSize+matSize+colorInstSize));
            CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
            glVertexAttribDivisor(colorAttr->index, 1);
            glEnableVertexAttribArray(colorAttr->index);
            CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
        }
    }
    {
        const OpenGLESAttribute *dirAttr = prog->findAttribute(a_modelDirNameID);
        if (moving && dirAttr)
        {
            glVertexAttribPointer(dirAttr->index, 3, GL_FLOAT, GL_FALSE, instSize, (const GLvoid *)(long)(centerSize+matSize+colorInstSize+colorSize));
            CheckGLError("BasicDrawableInstance::draw glVertexAttribPointer");
            glVertexAttribDivisor(dirAttr->index, 1);
            glEnableVertexAttribArray(dirAttr->index);
            CheckGLError("BasicDrawableInstance::setupVAO glEnableVertexAttribArray");
        }
    }
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    return vertArrayObj;
}


/// Set up local rendering structures (e.g. VBOs)
void BasicDrawableInstanceGLES::setupForRenderer(const RenderSetupInfo *inSetupInfo)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    if (instBuffer)
        return;
    
    numInstances = (int)instances.size();
    
    if (instances.empty())
        return;
    
    // Always doing color and position matrix
    // Note: Should allow for a list of optional attributes here
    centerSize = sizeof(GLfloat)*3;
    matSize = sizeof(GLfloat)*16;
    colorInstSize = sizeof(GLfloat);
    colorSize = sizeof(GLubyte)*4;
    if (moving)
    {
        modelDirSize = sizeof(GLfloat)*3;
    } else {
        modelDirSize = 0;
    }
    instSize = centerSize + matSize + colorSize + colorInstSize + modelDirSize;
    int bufferSize = (int)(instSize * instances.size());
    
    instBuffer = setupInfo->memManager->getBufferID(bufferSize,GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, instBuffer);
    void *glMem = NULL;
    if (hasMapBufferSupport)
    {
        glMem = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT);
    } else {
        glMem = (void *)malloc(bufferSize);
    }
    unsigned char *basePtr = (unsigned char *)glMem;
    for (unsigned int ii=0;ii<instances.size();ii++,basePtr+=instSize)
    {
        const SingleInstance &inst = instances[ii];
        Point3f center3f(inst.center.x(),inst.center.y(),inst.center.z());
        Matrix4f mat = Matrix4dToMatrix4f(inst.mat);
        float colorInst = inst.colorOverride ? 1.0 : 0.0;
        RGBAColor locColor = colorInst ? inst.color : color;
        memcpy(basePtr, (void *)center3f.data(), centerSize);
        memcpy(basePtr+centerSize, (void *)mat.data(), matSize);
        memcpy(basePtr+centerSize+matSize, (void *)&colorInst, colorInstSize);
        memcpy(basePtr+centerSize+matSize+colorInstSize, (void *)&locColor.r, colorSize);
        if (moving)
        {
            Point3d modelDir = (inst.endCenter - inst.center)/inst.duration;
            Point3f modelDir3f(modelDir.x(),modelDir.y(),modelDir.z());
            memcpy(basePtr+centerSize+matSize+colorInstSize+colorSize, (void *)modelDir3f.data(), modelDirSize);
        }
    }
    
    if (hasMapBufferSupport)
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        glBufferData(GL_ARRAY_BUFFER, bufferSize, glMem, GL_STATIC_DRAW);
        free(glMem);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/// Clean up any rendering objects you may have (e.g. VBOs).
void BasicDrawableInstanceGLES::teardownForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    if (instBuffer)
    {
        setupInfo->memManager->removeBufferID(instBuffer);
        instBuffer = 0;
    }
    
    if (vertArrayObj)
    {
        glDeleteVertexArrays(1, &vertArrayObj);
        vertArrayObj = 0;
    }
}

void BasicDrawableInstanceGLES::calculate(RendererFrameInfoGLES *frameInfo,Scene *scene)
{
}

// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)((long)(base) + (off)))

void BasicDrawableInstanceGLES::draw(RendererFrameInfoGLES *frameInfo,Scene *inScene)
{
    BasicDrawableGLES *basicDrawGL = dynamic_cast<BasicDrawableGLES *>(basicDraw.get());
    SceneGLES *scene = (SceneGLES *)inScene;
    
    // Happens if we're deleting things out of order
    if (!basicDrawGL || !basicDrawGL->isSetupInGL())
        return;
    
    // The old style where we reuse the basic drawable
    if (instanceStyle == ReuseStyle)
    {
        // New style makes use of OpenGL instancing and makes its own copy of the geometry
        ProgramGLES *prog = (ProgramGLES *)frameInfo->program;

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
            ((ProgramGLES *)frameInfo->program)->setUniform(u_TimeNameID, (float)(frameInfo->currentTime - startTime));

        // GL Texture IDs
        bool anyTextures = false;
        std::vector<GLuint> glTexIDs;
        if (texInfo.empty()) {
            // Just run the ones from the basic drawable
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
        } else {
            // We have our own tex info to set up, but it does depend on the base drawable
            for (int ii=0;ii<texInfo.size();ii++)
            {
                SimpleIdentity texID = texInfo[ii].texId;

                GLuint glTexID = EmptyIdentity;
                if (texID != EmptyIdentity)
                {
                    glTexID = scene->getGLTexture(texID);\
                    if (glTexID != 0)
                    {
                        anyTextures = true;
                        glTexIDs.push_back(glTexID);
                    } else
                        wkLogLevel(Error,"BasicDrawableInstance: Missing texture %d",texID);
                }
            }
        }

        if (!anyTextures)
            wkLogLevel(Error,"BasicDrawableInstance: Drawable without textures");

        // Model/View/Projection matrix
        if (basicDraw->clipCoords)
        {
            Matrix4f identMatrix = Matrix4f::Identity();
            prog->setUniform(mvpMatrixNameID, identMatrix);
            prog->setUniform(mvMatrixNameID, identMatrix);
            prog->setUniform(mvNormalMatrixNameID, identMatrix);
            prog->setUniform(mvpNormalMatrixNameID, identMatrix);
            prog->setUniform(u_pMatrixNameID, identMatrix);
        } else {
            prog->setUniform(mvpMatrixNameID, frameInfo->mvpMat);
            prog->setUniform(mvMatrixNameID, frameInfo->viewAndModelMat);
            prog->setUniform(mvNormalMatrixNameID, frameInfo->viewModelNormalMat);
            prog->setUniform(mvpNormalMatrixNameID, frameInfo->mvpNormalMat);
            prog->setUniform(u_pMatrixNameID, frameInfo->projMat);
        }

        // Any uniforms we may want to apply to the shader
        for (auto const &attr : uniforms)
            prog->setUniform(attr);

        // Fade is always mixed in
        prog->setUniform(u_FadeNameID, fade);

        // Let the shaders know if we even have a texture
        prog->setUniform(u_HasTextureNameID, anyTextures);

        // If this is present, the drawable wants to do something based where the viewer is looking
        prog->setUniform(u_EyeVecNameID, frameInfo->fullEyeVec);

        // The program itself may have some textures to bind
        bool hasTexture[WhirlyKitMaxTextures];
        int progTexBound = prog->bindTextures();
        for (unsigned int ii=0;ii<progTexBound;ii++)
            hasTexture[ii] = true;

        // Zero or more textures in the drawable
        for (unsigned int ii=0;ii<WhirlyKitMaxTextures-progTexBound;ii++)
        {
            GLuint glTexID = ii < glTexIDs.size() ? glTexIDs[ii] : 0;
            auto baseMapNameID = baseMapNameIDs[ii];
            auto hasBaseMapNameID = hasBaseMapNameIDs[ii];
            auto texScaleNameID = texScaleNameIDs[ii];
            auto texOffsetNameID = texOffsetNameIDs[ii];
            const OpenGLESUniform *texUni = prog->findUniform(baseMapNameID);
            hasTexture[ii+progTexBound] = glTexID != 0 && texUni;
            if (hasTexture[ii+progTexBound])
            {
                glActiveTexture(GL_TEXTURE0+ii+progTexBound);
                glBindTexture(GL_TEXTURE_2D, glTexID);
                CheckGLError("BasicDrawableInstance::drawVBO2() glBindTexture");
                prog->setUniform(baseMapNameID, (int)ii+progTexBound);
                CheckGLError("BasicDrawableInstance::drawVBO2() glUniform1i");
                prog->setUniform(hasBaseMapNameID, 1);

                float texScale = 1.0;
                Vector2f texOffset(0.0,0.0);
                // Adjust for border pixels
                if (ii < texInfo.size()) {
                    auto thisTexInfo = texInfo[ii];
                    if (thisTexInfo.borderTexel > 0 && thisTexInfo.size > 0) {
                        texScale = (thisTexInfo.size - 2 * thisTexInfo.borderTexel) / (double)thisTexInfo.size;
                        float offset = thisTexInfo.borderTexel / (double)thisTexInfo.size;
                        texOffset = Vector2f(offset,offset);
                    }
                    // Adjust for a relative texture lookup (using lower zoom levels)
                    if (thisTexInfo.relLevel > 0) {
                        texScale = texScale/(1<<thisTexInfo.relLevel);
                        texOffset = Vector2f(texScale*thisTexInfo.relX,texScale*thisTexInfo.relY) + texOffset;
                    }
                    prog->setUniform(texScaleNameID, Vector2f(texScale, texScale));
                    prog->setUniform(texOffsetNameID, texOffset);
                }
                CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
            } else {
                prog->setUniform(hasBaseMapNameID, 0);
            }
        }

        // If necessary, set up the VAO (once)
        if (vertArrayObj == 0 && basicDrawGL->sharedBuffer !=0)
            setupVAO(frameInfo);

        // Figure out what we're using
        const OpenGLESAttribute *vertAttr = prog->findAttribute(a_PositionNameID);

        // Vertex array
        bool usedLocalVertices = false;
        if (vertAttr && !(basicDrawGL->sharedBuffer || basicDrawGL->pointBuffer))
        {
            usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &basicDrawGL->points[0]);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }

        // Other vertex attributes
        std::vector<const OpenGLESAttribute *> progAttrs;
        if (!vertArrayObj) {
            progAttrs.resize(basicDraw->vertexAttributes.size(),NULL);
            for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
            {
                VertexAttributeGLES *attr = (VertexAttributeGLES *)basicDraw->vertexAttributes[ii];
                const OpenGLESAttribute *progAttr = prog->findAttribute(attr->nameID);
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
                        }
                    }
                }
            }
        } else {
            // Vertex Array Objects can't hold the defaults, so we build them earlier
            // Note: We should override these that we need to from our own settings
            for (auto attrDef : vertArrayDefaults) {
                // The program is expecting it, so we need a default
                attrDef.attr.glSetDefault(attrDef.progAttrIndex);
                CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
            }
        }

        // Note: Something of a hack
        if (hasColor) {
            const OpenGLESAttribute *colorAttr = prog->findAttribute(a_colorNameID);
            if (colorAttr)
                glVertexAttrib4f(colorAttr->index, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
        }

        // If there are no instances, fill in the identity
        if (!instBuffer)
        {
            // Set the singleMatrix attribute to identity
            const OpenGLESAttribute *matAttr = prog->findAttribute(a_SingleMatrixNameID);
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
            const OpenGLESAttribute *dirAttr = prog->findAttribute(a_modelDirNameID);
            if (dirAttr)
                glVertexAttrib3f(dirAttr->index, 0.0, 0.0, 0.0);
        }

        // If we're using a vertex array object, bind it and draw
        if (vertArrayObj)
        {
            glBindVertexArray(vertArrayObj);

            switch (basicDraw->type)
            {
                case Triangles:
                    if (instBuffer)
                    {
                        glDrawElementsInstanced(GL_TRIANGLES, basicDrawGL->numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(0,basicDrawGL->triBuffer), numInstances);
                    } else
                        glDrawElements(GL_TRIANGLES, basicDrawGL->numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(0,basicDrawGL->triBuffer));
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    break;
                case Points:
                    if (instBuffer)
                    {
                        glDrawArraysInstanced(GL_POINTS, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(GL_POINTS, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
                case Lines:
                    glLineWidth(lineWidth);
                    if (instBuffer)
                    {
                        glDrawArraysInstanced(GL_LINES, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(GL_LINES, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
//                case GL_TRIANGLE_STRIP:
//                    if (instBuffer)
//                    {
//                        glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                    } else
//                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
//                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
//                    break;
            }

            glBindVertexArray(0);
        } else {
            // Draw without a VAO
            switch (basicDraw->type)
            {
                case Triangles:
                {
                    if (basicDrawGL->triBuffer)
                    {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basicDrawGL->triBuffer);
                        CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
                        if (instBuffer)
                        {
                            glDrawElementsInstanced(GL_TRIANGLES, basicDrawGL->numTris*3, GL_UNSIGNED_SHORT, 0, numInstances);
                        } else
                            glDrawElements(GL_TRIANGLES, basicDrawGL->numTris*3, GL_UNSIGNED_SHORT, 0);
                        CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } else {
                        if (instBuffer)
                        {
                            glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)basicDrawGL->tris.size()*3, GL_UNSIGNED_SHORT, &basicDrawGL->tris[0], numInstances);
                        } else
                            glDrawElements(GL_TRIANGLES, (GLsizei)basicDrawGL->tris.size()*3, GL_UNSIGNED_SHORT, &basicDrawGL->tris[0]);
                        CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    }
                }
                    break;
                case Points:
                    if (instBuffer)
                    {
                        glDrawArraysInstanced(GL_POINTS, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(GL_POINTS, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
                case GL_LINES:
                    glLineWidth(lineWidth);
                    CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
                    if (instBuffer)
                    {
                        glDrawArraysInstanced(GL_LINES, 0, basicDraw->numPoints, numInstances);
                    } else
                        glDrawArrays(GL_LINES, 0, basicDraw->numPoints);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                    break;
//                case GL_TRIANGLE_STRIP:
//                    if (instBuffer)
//                    {
//                        glDrawArraysInstanced(basicDraw->type, 0, basicDraw->numPoints, numInstances);
//                    } else
//                        glDrawArrays(basicDraw->type, 0, basicDraw->numPoints);
//                    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
//                    break;
            }
        }

        // Unbind any textures
        for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
            if (hasTexture[ii])
            {
                glActiveTexture(GL_TEXTURE0+ii);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

        // Tear down the various arrays, if we stood them up
        if (usedLocalVertices)
            glDisableVertexAttribArray(vertAttr->index);
        for (unsigned int ii=0;ii<progAttrs.size();ii++)
            if (progAttrs[ii])
                glDisableVertexAttribArray(progAttrs[ii]->index);

        if (instBuffer)
        {
            const OpenGLESAttribute *centerAttr = prog->findAttribute(a_modelCenterNameID);
            if (centerAttr)
            {
                glDisableVertexAttribArray(centerAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *matAttr = prog->findAttribute(a_SingleMatrixNameID);
            if (matAttr)
            {
                for (unsigned int im=0;im<4;im++)
                    glDisableVertexAttribArray(matAttr->index+im);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *colorAttr = prog->findAttribute(a_colorNameID);
            if (colorAttr)
            {
                glDisableVertexAttribArray(colorAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
            const OpenGLESAttribute *dirAttr = prog->findAttribute(a_modelDirNameID);
            if (dirAttr)
            {
                glDisableVertexAttribArray(dirAttr->index);
                CheckGLError("BasicDrawableInstance::draw() glDisableVertexAttribArray");
            }
        }
    } else {
        int oldDrawPriority = basicDraw->getDrawPriority();
        RGBAColor oldColor = basicDrawGL->color;
        float oldLineWidth = basicDrawGL->lineWidth;
        float oldMinVis = basicDrawGL->minVisible,oldMaxVis = basicDrawGL->maxVisible;
        auto oldUniforms = basicDrawGL->uniforms;
        std::vector<BasicDrawable::TexInfo> oldTexInfo = basicDraw->getTexInfo();

        // Change the drawable
        if (hasDrawPriority)
            basicDraw->setDrawPriority(drawPriority);
        if (hasColor)
            basicDrawGL->color = color;
        if (hasLineWidth)
            basicDraw->setLineWidth(lineWidth);
        if (!uniforms.empty()) {
            basicDraw->setUniforms(uniforms);
        }
        if (!texInfo.empty())
        {
            if (basicDraw->texInfo.size() < texInfo.size()) {
                wkLogLevel(Error,"BasicDrawableInstanceGLES: Tried to set missing texture entry");
//                basicDrawGL->setupTexCoordEntry(texInfo.size()-1, 0);
            }
            // Override the texture with different ID and relative coords
            if (texInfo.size() == basicDraw->texInfo.size()) {
                for (int ii=0;ii<basicDraw->texInfo.size();ii++)
                {
                    auto &newEntry = texInfo[ii];
                    auto &entry = basicDraw->texInfo[ii];
                    entry.texId = newEntry.texId;
                    entry.size = newEntry.size;
                    entry.borderTexel = newEntry.borderTexel;
                    entry.relX = newEntry.relX;
                    entry.relY = newEntry.relY;
                    entry.relLevel = newEntry.relLevel;
                }
            }
        }
        basicDraw->setVisibleRange(minVis, maxVis);

        Matrix4f oldMvpMat = frameInfo->mvpMat;
        Matrix4f oldMvMat = frameInfo->viewAndModelMat;
        Matrix4f oldMvNormalMat = frameInfo->viewModelNormalMat;

        // No matrices, so just one instance
        if (instances.empty())
            basicDrawGL->draw(frameInfo,scene);
        else {
            // Run through the list of instances
            for (unsigned int ii=0;ii<instances.size();ii++)
            {
                // Change color
                const SingleInstance &singleInst = instances[ii];
                if (singleInst.colorOverride)
                    basicDrawGL->color = singleInst.color;
                else {
                    if (hasColor)
                        basicDrawGL->color = color;
                    else
                        basicDrawGL->color = oldColor;
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

                basicDrawGL->draw(frameInfo,scene);
            }
        }

        // Set it back
        frameInfo->mvpMat = oldMvpMat;
        frameInfo->viewAndModelMat = oldMvMat;
        frameInfo->viewModelNormalMat = oldMvNormalMat;

        if (hasDrawPriority)
            basicDraw->setDrawPriority(oldDrawPriority);
        if (hasColor)
            basicDraw->color = oldColor;
        if (hasLineWidth)
            basicDraw->setLineWidth(oldLineWidth);
        if (!texInfo.empty())
            basicDraw->texInfo = oldTexInfo;
        if (!uniforms.empty())
            basicDraw->setUniforms(oldUniforms);
        basicDraw->setVisibleRange(oldMinVis, oldMaxVis);
    }
}

}
