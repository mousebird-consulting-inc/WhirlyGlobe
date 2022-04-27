/*  BasicDrawableGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
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

#import "BasicDrawableGLES.h"
#import "WhirlyKitLog.h"
#import <cstdint>

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawableGLES::BasicDrawableGLES(std::string name) :
    BasicDrawable(name),
    DrawableGLES(std::move(name))
{
}

unsigned int BasicDrawableGLES::singleVertexSize()
{
    GLuint singleVertSize = points.empty() ? 0 : 3 * sizeof(GLfloat);

    // Now for the rest of the buffers
    for (auto &vertexAttribute : vertexAttributes)
    {
        auto *attr = (VertexAttributeGLES *)vertexAttribute;
        if (attr->numElements() != 0)
        {
            attr->buffer = singleVertSize;
            singleVertSize += attr->size();
        }
    }
    
    return singleVertSize;
}

void BasicDrawableGLES::addPointsToBuffer(uint8_t *basePtr, unsigned numVerts, const Point3d *center)
{
    for (int ii = 0; ii < numVerts; ii++, basePtr += vertexSize)
    {
        addPointToBuffer(basePtr, ii, center);
    }
}

// Adds the basic vertex data to an interleaved vertex buffer
void BasicDrawableGLES::addPointToBuffer(uint8_t *basePtr,int which,const Point3d *center)
{
    if (!points.empty())
    {
        const Point3f &pt = points[which];
        
        // If there's a center, we have to offset everything first
        if (center)
        {
            Vector4d pt3d;
            if (hasMatrix)
                pt3d = mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            else
                pt3d = Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            Point3f newPt(pt3d.x()-center->x(),pt3d.y()-center->y(),pt3d.z()-center->z());
            memcpy(basePtr, &newPt.x(), 3*sizeof(GLfloat));
        }
        else
        {
            // Otherwise, copy it straight in
            memcpy(basePtr, pt.data(), 3*sizeof(GLfloat));
        }
    }
    
    for (VertexAttribute *attr : vertexAttributes)
    {
        const auto *theAttr = (VertexAttributeGLES *)attr;
        if (attr->numElements() != 0 && theAttr->buffer != 0)
        {
            memcpy(basePtr + theAttr->buffer, attr->addressForElement(which), attr->size());
        }
    }
}

// Put data into a GL buffer, with or without MapBuffer.
// Buffer must already be bound.
static inline void FillGLBuffer(GLenum target, GLenum usage, GLsizeiptr bufferSize,
                                const std::function<void(uint8_t*)> &fillFunc)
{
    if (auto *basePtr = hasMapBufferSupport ?
                        (uint8_t*)glMapBufferRange(target, 0, bufferSize, GL_MAP_WRITE_BIT) : nullptr)
    {
        memset(basePtr, 0, bufferSize);
        fillFunc(basePtr);
        glUnmapBuffer(target);
        return;
    }

    std::vector<uint8_t> glMemBuf(bufferSize);
    fillFunc(&glMemBuf[0]);
    glBufferData(target, bufferSize, &glMemBuf[0], usage);
    CheckGLError("FillGLBuffer/glBufferData");
}

static inline void BindAndFillGLBuffer(GLenum target, GLenum usage,
                                       GLuint buffer, GLsizeiptr bufferSize,
                                       const std::function<void(uint8_t*)> &fillFunc)
{
    glBindBuffer(target, buffer);
    FillGLBuffer(target, usage, bufferSize, fillFunc);
    glBindBuffer(target, 0);
}

// Create VBOs and such
void BasicDrawableGLES::setupForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    auto *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    // If we're already setup, don't do it twice
    if (pointBuffer || sharedBuffer)
        return;

    // Offset the geometry upward by minZres units along the normals
    // Only do this once, obviously
    if (drawOffset != 0 && (points.size() == vertexAttributes[normalEntry]->numElements()))
    {
        float scale = setupInfo->minZres*drawOffset;
        Point3fVector &norms = *(Point3fVector *)vertexAttributes[normalEntry]->data;
        
        for (unsigned int ii=0;ii<points.size();ii++)
        {
            Vector3f pt = points[ii];
            points[ii] = norms[ii] * scale + pt;
        }
    }

    pointBuffer = triBuffer = 0;
    sharedBuffer = 0;
    
    numPoints = (int)points.size();
    numTris = (int)tris.size();

    const auto vertBufSize = (int)(vertexSize * numPoints);
    const auto triBufSize = (int)(numTris * sizeof(Triangle));
    const auto sharedSize = vertBufSize + triBufSize;

    if (hasSharedBufferSupport)
    {
        sharedBuffer = setupInfo->memManager->getBufferID(sharedSize, GL_STATIC_DRAW);
        if (sharedBuffer)
        {
            pointBuffer = 0;
            triBuffer = vertBufSize;
        }
        else
        {
            wkLogLevel(Error, "No buffer (%d) in BasicDrawable::setupGL()", sharedSize);
        }
    }

    if (!sharedBuffer)
    {
        pointBuffer = setupInfo->memManager->getBufferID(vertBufSize, GL_STATIC_DRAW);
        if (pointBuffer == 0)
        {
            wkLogLevel(Error, "Vertex buffer alloc (%d) failed in BasicDrawable::setupGL", vertBufSize);
            return;
        }
        if (numTris)
        {
            // Note that we must pass zero for size here in the non-sharing case, or the buffer
            // will be set up as a vertex buffer and will then not work as an element buffer.
            triBuffer = setupInfo->memManager->getBufferID(0);
            if (triBuffer == 0)
            {
                setupInfo->memManager->removeBufferID(pointBuffer);
                pointBuffer = 0;
                wkLogLevel(Error, "Triangle buffer alloc (%d) failed in BasicDrawable::setupGL", triBufSize);
                return;
            }
        }
    }

    // Now copy in the data
    if (sharedBuffer)
    {
        BindAndFillGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sharedBuffer, sharedSize, [=](uint8_t *buf) {
            addPointsToBuffer(buf + pointBuffer, numPoints, nullptr);
            memcpy(buf + triBuffer, &tris[0], triBufSize);
        });
    }
    else
    {
        BindAndFillGLBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, pointBuffer, vertBufSize, [=](uint8_t *buf) {
            addPointsToBuffer(buf, numPoints, nullptr);
        });

        if (numTris)
        {
            BindAndFillGLBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, triBuffer, triBufSize, [=](uint8_t *buf) {
                memcpy(buf, &tris[0], triBufSize);
            });
        }
    }
    
    // Clear out the arrays, since we won't need them again
    points.clear();
    tris.clear();
    for (auto *vertexAttribute : vertexAttributes)
    {
        vertexAttribute->clear();
    }
    
    isSetupGL = true;
}

// Tear down the VBOs we set up
void BasicDrawableGLES::teardownForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene,RenderTeardownInfoRef teardown)
{
    auto *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    isSetupGL = false;
    if (vertArrayObj)
    {
        glDeleteVertexArrays(1,&vertArrayObj);
        vertArrayObj = 0;
    }

    if (sharedBuffer)
    {
        setupInfo->memManager->removeBufferID(sharedBuffer);
        sharedBuffer = 0;
    }
    else
    {
        if (pointBuffer)
            setupInfo->memManager->removeBufferID(pointBuffer);
        if (triBuffer)
            setupInfo->memManager->removeBufferID(triBuffer);

    }
    pointBuffer = 0;
    triBuffer = 0;

    for (auto *vertexAttribute : vertexAttributes)
    {
        ((VertexAttributeGLES *)vertexAttribute)->buffer = 0;
    }
}

// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)((uintptr_t)(base)) + (off))


// Called once to set up a Vertex Array Object
GLuint BasicDrawableGLES::setupVAO(ProgramGLES *prog)
{
    GLuint theVertArrayObj;
    const OpenGLESAttribute *vertAttr = prog->findAttribute(a_PositionNameID);
    
    glGenVertexArrays(1, &theVertArrayObj);
    glBindVertexArray(theVertArrayObj);
    
    // We're using a single buffer for all of our vertex attributes
    if (sharedBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
        CheckGLError("BasicDrawable::setupVAO() shared glBindBuffer");
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
        CheckGLError("BasicDrawable::setupVAO() point glBindBuffer");
    }

    // Vertex attributes start 3 floats after each point, which are separated by `vertexSize`
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, nullptr);
        glEnableVertexAttribArray(vertAttr->index);
    }
    
    // All the rest of the attributes
    const OpenGLESAttribute *progAttrs[vertexAttributes.size()];
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        progAttrs[ii] = nullptr;
        auto *attr = (VertexAttributeGLES *)vertexAttributes[ii];
        if (const OpenGLESAttribute *thisAttr = prog->findAttribute(attr->nameID))
        {
            if (attr->buffer != 0 || attr->numElements() != 0)
            {
                glEnableVertexAttribArray(thisAttr->index);
                glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(),
                                      attr->glNormalize(), vertexSize, CALCBUFOFF(0,attr->buffer));
                progAttrs[ii] = thisAttr;
            }
            else
            {
                VertAttrDefault attrDef(thisAttr->index,*attr);
                vertArrayDefaults.push_back(attrDef);
            }
        }
    }
    
    // Bind the element array
    bool boundElements = false;
    if (type == Triangles && (sharedBuffer || triBuffer))
    {
        boundElements = true;
        if (sharedBuffer)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
            CheckGLError("BasicDrawable::setupVAO() shared tri glBindBuffer");
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
            CheckGLError("BasicDrawable::setupVAO() tri glBindBuffer");
        }
    }
    
    glBindVertexArray(0);
    
    // Now tear down all that state
    if (vertAttr)
        glDisableVertexAttribArray(vertAttr->index);
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        if (progAttrs[ii])
            glDisableVertexAttribArray(progAttrs[ii]->index);
    if (boundElements)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (sharedBuffer)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return theVertArrayObj;
}
    
bool BasicDrawableGLES::isSetupInGL()
{
    return isSetupGL;
}

// Draw Vertex Buffer Objects, OpenGL 2.0+
void BasicDrawableGLES::draw(RendererFrameInfoGLES *frameInfo,Scene *inScene)
{
    auto *prog = (ProgramGLES *)frameInfo->program;
    auto *scene = (SceneGLES *)inScene;

    // Figure out if we're fading in or out
    float fade = 1.0;
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < fadeDown)
            fade = 0.0;
        else
            if (frameInfo->currentTime > fadeUp)
                fade = 1.0;
            else
                fade = (frameInfo->currentTime - fadeDown)/(fadeUp - fadeDown);
    } else {
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < fadeUp)
                fade = 1.0;
            else
                if (frameInfo->currentTime > fadeDown)
                    fade = 0.0;
                else
                    fade = 1.0f-(frameInfo->currentTime - fadeUp)/(fadeDown - fadeUp);
        }
    }
    // Deal with the range based fade
    if (frameInfo->heightAboveSurface > 0.0)
    {
        float factor = 1.0;
        if (minVisibleFadeBand != 0.0)
        {
            float a = (frameInfo->heightAboveSurface - minVisible)/minVisibleFadeBand;
            if (a >= 0.0 && a < 1.0)
                factor = a;
        }
        if (maxVisibleFadeBand != 0.0)
        {
            float b = (maxVisible - frameInfo->heightAboveSurface)/maxVisibleFadeBand;
            if (b >= 0.0 && b < 1.0)
                factor = b;
        }
        
        fade = fade * factor;
    }
    
    // GL Texture IDs
    bool anyTextures = false;
    std::vector<GLuint> glTexIDs;
    for (auto & thisTexInfo : texInfo)
    {
        GLuint glTexID = EmptyIdentity;
        if (thisTexInfo.texId != EmptyIdentity)
        {
            glTexID = scene->getGLTexture(thisTexInfo.texId);
            anyTextures = true;
        }
        glTexIDs.push_back(glTexID);
    }
    
    // Model/View/Projection matrix
    if (clipCoords)
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
    
    // Fill the a_singleMatrix attribute with default values
    const OpenGLESAttribute *matAttr = prog->findAttribute(a_SingleMatrixNameID);
    if (matAttr)
    {
        glVertexAttrib4f(matAttr->index,1.0,0.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+1,0.0,1.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+2,0.0,0.0,1.0,0.0);
        glVertexAttrib4f(matAttr->index+3,0.0,0.0,0.0,1.0);
    }
    
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
            const auto &thisTexInfo = texInfo[ii];
            glActiveTexture(GL_TEXTURE0+ii+progTexBound);
            glBindTexture(GL_TEXTURE_2D, glTexID);
            CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
            prog->setUniform(baseMapNameID, (int)ii+progTexBound);
            prog->setUniform(hasBaseMapNameID, 1);
            float texScale = 1.0;
            Vector2f texOffset(0.0,0.0);
            // Adjust for border pixels
            if (thisTexInfo.borderTexel > 0 && thisTexInfo.size > 0) {
                texScale = (float)(thisTexInfo.size - 2 * thisTexInfo.borderTexel) / (float)thisTexInfo.size;
                const float offset = (float)thisTexInfo.borderTexel / (float)thisTexInfo.size;
                texOffset = Vector2f(offset,offset);
            }
            // Adjust for a relative texture lookup (using lower zoom levels)
            if (thisTexInfo.relLevel > 0) {
                texScale = texScale/(float)(1U<<thisTexInfo.relLevel);
                texOffset = Vector2f(texScale*thisTexInfo.relX,texScale*thisTexInfo.relY) + texOffset;
            }
            prog->setUniform(texScaleNameID, Vector2f(texScale, texScale));
            prog->setUniform(texOffsetNameID, texOffset);
            CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
        } else {
            prog->setUniform(hasBaseMapNameID, 0);
        }
    }
    
    const OpenGLESAttribute *vertAttr = nullptr;
    bool boundElements = false;
    bool usedLocalVertices = false;
    std::vector<const OpenGLESAttribute *> progAttrs;
    
    if (hasVertexArraySupport)
    {
        // If necessary, set up the VAO (once)
        if (vertArrayObj == 0 && (sharedBuffer != 0 || pointBuffer))
        {
            vertArrayObj = setupVAO(prog);
        }
        
        // Figure out what we're using
        vertAttr = prog->findAttribute(a_PositionNameID);
        
        // Vertex array
        if (vertAttr && !sharedBuffer && !pointBuffer)
        {
            usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }
        
        // Other vertex attributes
        if (!vertArrayObj)
        {
            progAttrs.resize(vertexAttributes.size(),nullptr);
            
            for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            {
                auto *attr = (VertexAttributeGLES *)vertexAttributes[ii];
                const OpenGLESAttribute *progAttr = prog->findAttribute(attr->nameID);
                if (progAttr)
                {
                    // The data hasn't been downloaded, so hook it up directly here
                    if (attr->buffer == 0)
                    {
                        // We have a data array for it, so hand that over
                        if (attr->numElements() != 0)
                        {
                            glVertexAttribPointer(progAttr->index, attr->glEntryComponents(), attr->glType(),
                                                  attr->glNormalize(), 0, attr->addressForElement(0));
                            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
                            glEnableVertexAttribArray ( progAttr->index );
                            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
                            
                            progAttrs[ii] = progAttr;
                        }
                        else
                        {
                            // The program is expecting it, so we need a default
                            attr->glSetDefault(progAttr->index);
                            CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                        }
                    }
                }
            }
        }
        else
        {
            // Vertex Array Objects can't hold the defaults, so we build them earlier
            for (const auto& attrDef : vertArrayDefaults)
            {
                // The program is expecting it, so we need a default
                attrDef.attr.glSetDefault(attrDef.progAttrIndex);
                CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
            }
        }
    }
    else
    {
        vertAttr = prog->findAttribute(a_PositionNameID);
        progAttrs.resize(vertexAttributes.size(),nullptr);
        
        // We're using a single buffer for all of our vertex attributes
        if (sharedBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO2() shared glBindBuffer");
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, nullptr);
        }
        else if (pointBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
            CheckGLError("BasicDrawable::drawVBO2() point glBindBuffer");
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, nullptr);
        }
        else
        {
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
        }

        usedLocalVertices = true;
        glEnableVertexAttribArray(vertAttr->index);

        // All the rest of the attributes
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            const auto attr = (VertexAttributeGLES *)vertexAttributes[ii];
            progAttrs[ii] = nullptr;
            if (const OpenGLESAttribute *progAttr = prog->findAttribute(attr->nameID))
            {
                // The data hasn't been downloaded, so hook it up directly here
                if (attr->buffer != 0 || attr->numElements() != 0)
                {
                    const auto stride = attr->buffer ? vertexSize : 0;
                    const auto ptr = attr->buffer ? CALCBUFOFF(0,attr->buffer) : attr->addressForElement(0);
                    glVertexAttribPointer(progAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), stride, ptr);
                    CheckGLError("BasicDrawable::draw glVertexAttribPointer");
                    glEnableVertexAttribArray(progAttr->index);
                    CheckGLError("BasicDrawable::draw glEnableVertexAttribArray");
                    progAttrs[ii] = progAttr;
                }
                else
                {
                    // The program is expecting it, so we need a default
                    attr->glSetDefault(progAttr->index);
                    CheckGLError("BasicDrawable::draw glSetDefault");
                }
            }
        }
        
        // Bind the element array
        if (type == Triangles && triBuffer)
        {
            boundElements = true;
            if (sharedBuffer)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
                CheckGLError("BasicDrawable::drawVBO2() shared tri glBindBuffer");
            }
            else
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
                CheckGLError("BasicDrawable::drawVBO2() tri glBindBuffer");
            }
        }
    }
    
    // Color has been overridden, so don't use the embedded ones
    if (hasOverrideColor)
    {
        if (const OpenGLESAttribute *colorAttr = prog->findAttribute(a_colorNameID))
        {
            glDisableVertexAttribArray(colorAttr->index);
            glVertexAttrib4f(colorAttr->index, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,color.a / 255.0f);
        }
    }

    // If we're using a vertex array object, bind it and draw
    if (vertArrayObj)
    {
        glBindVertexArray(vertArrayObj);
        switch (type)
        {
            case Triangles:
            {
                const auto offset = CALCBUFOFF(nullptr, sharedBuffer ? triBuffer : 0);
                glDrawElements(GL_TRIANGLES, (GLsizei)numTris * 3, GL_UNSIGNED_SHORT, offset);
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                break;
            }
            case Points:
                glDrawArrays(GL_POINTS, 0, (GLsizei)numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
            case Lines:
                if (lineWidth > 0)
                {
                    glLineWidth(lineWidth);
                }
                glDrawArrays(GL_LINES, 0, (GLsizei)numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
//            case GL_TRIANGLE_STRIP:
//                glDrawArrays(type, 0, numPoints);
//                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
//                break;
        }
        glBindVertexArray(0);
    }
    else
    {
        // Draw without a VAO
        switch (type)
        {
        case Triangles:
            if (boundElements)
            {
                const auto offset = CALCBUFOFF(nullptr, sharedBuffer ? triBuffer : 0);
                glDrawElements(GL_TRIANGLES, (GLsizei)numTris*3, GL_UNSIGNED_SHORT, offset);
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
            }
            else
            {
                glDrawElements(GL_TRIANGLES, (GLsizei)tris.size()*3, GL_UNSIGNED_SHORT, &tris[0]);
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
            }
            break;
        case Points:
            glDrawArrays(GL_POINTS, 0, (GLsizei)numPoints);
            CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
            break;
        case Lines:
            if (lineWidth > 0)
            {
                glLineWidth(lineWidth);
            }
            CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
            glDrawArrays(GL_LINES, 0, (GLsizei)numPoints);
            CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
            break;
//            case GL_TRIANGLE_STRIP:
//                glDrawArrays(type, 0, numPoints);
//                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
//                break;
        }
    }
    
    // Unbind any textures
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
    {
        if (hasTexture[ii])
        {
            glActiveTexture(GL_TEXTURE0 + ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    // Tear down the various arrays, if we stood them up
    if (boundElements)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    if (usedLocalVertices)
    {
        glDisableVertexAttribArray(vertAttr->index);
    }
    if (!vertArrayObj)
    {
        for (auto &progAttr : progAttrs)
        {
            if (progAttr)
            {
                glDisableVertexAttribArray(progAttr->index);
            }
        }
    }
    
    if (!hasVertexArraySupport)
    {
        // Now tear down all that state
        if (vertAttr)
        {
            glDisableVertexAttribArray(vertAttr->index);
        }
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            if (auto *attr = progAttrs[ii])
            {
                glDisableVertexAttribArray(attr->index);
            }
        }
        if (sharedBuffer || pointBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}

}
