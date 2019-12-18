/*
 *  BasicDrawableGLES.cpp
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

#import "BasicDrawableGLES.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawableGLES::BasicDrawableGLES(const std::string &name)
: BasicDrawable(name), Drawable(name), isSetupGL(false), usingBuffers(false), vertexSize(-1),
    pointBuffer(0), triBuffer(0), sharedBuffer(0), vertArrayObj(0)
{
}

BasicDrawableGLES::~BasicDrawableGLES()
{
}

unsigned int BasicDrawableGLES::singleVertexSize()
{
    GLuint singleVertSize = 0;
    
    // Always have points
    if (!points.empty())
    {
        pointBuffer = singleVertSize;
        singleVertSize += 3*sizeof(GLfloat);
    }
    
    // Now for the rest of the buffers
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttributeGLES *attr = (VertexAttributeGLES *)vertexAttributes[ii];
        if (attr->numElements() != 0)
        {
            attr->buffer = singleVertSize;
            singleVertSize += attr->size();
        }
    }
    
    return singleVertSize;
}
    
// Adds the basic vertex data to an interleaved vertex buffer
void BasicDrawableGLES::addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center)
{
    if (!points.empty())
    {
        Point3f &pt = points[which];
        
        // If there's a center, we have to offset everything first
        if (center)
        {
            Vector4d pt3d;
            if (hasMatrix)
                pt3d = mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            else
                pt3d = Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            Point3f newPt(pt3d.x()-center->x(),pt3d.y()-center->y(),pt3d.z()-center->z());
            memcpy(basePtr+pointBuffer, &newPt.x(), 3*sizeof(GLfloat));
        } else {
            // Otherwise, copy it straight in
            memcpy(basePtr+pointBuffer, &pt.x(), 3*sizeof(GLfloat));
        }
    }
    
    for (VertexAttribute *attr : vertexAttributes)
    {
        VertexAttributeGLES *theAttr = (VertexAttributeGLES *)attr;
        if (attr->numElements() != 0 && theAttr->buffer != pointBuffer)
            memcpy(basePtr+theAttr->buffer, attr->addressForElement(which), attr->size());
    }
}

// Create VBOs and such
void BasicDrawableGLES::setupForRenderer(const RenderSetupInfo *inSetupInfo)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    // If we're already setup, don't do it twice
    if (pointBuffer || sharedBuffer)
        return;
    
    //    if ([NSThread currentThread] == [NSThread mainThread]) {
    //        NSLog(@"Hey why are we doing setupGL on the main thread? %s",name.c_str());
    //    }
    
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
    
    // We'll set up a single buffer for everything.
    // The other buffer pointers are now strides
    // Size of a single vertex entry
    int numVerts = (int)points.size();
    
    // Set up the buffer
    int bufferSize = vertexSize*numVerts;
    if (!tris.empty())
    {
        bufferSize += tris.size()*sizeof(Triangle);
    }
    sharedBuffer = setupInfo->memManager->getBufferID(bufferSize,GL_STATIC_DRAW);
    if (!sharedBuffer)
        wkLogLevel(Error, "Empty buffer in BasicDrawable::setupGL()");
    
    // Now copy in the data
    glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
    if (hasMapBufferSupport) {
        void *glMem = NULL;
        glMem = glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT);
        unsigned char *basePtr = (unsigned char *)glMem;
        for (unsigned int ii=0;ii<numVerts;ii++,basePtr+=vertexSize)
            addPointToBuffer(basePtr,ii,NULL);
        
        // And copy in the element buffer
        if (tris.size())
        {
            triBuffer = vertexSize*numVerts;
            unsigned char *basePtr = (unsigned char *)glMem + triBuffer;
            for (unsigned int ii=0;ii<tris.size();ii++,basePtr+=sizeof(Triangle))
                memcpy(basePtr, &tris[ii], sizeof(Triangle));
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        bufferSize = numVerts*vertexSize+tris.size()*sizeof(Triangle);
        
        // Gotta do this the hard way
        unsigned char *glMem = (unsigned char *)malloc(bufferSize);
        unsigned char *basePtr = glMem;
        for (unsigned int ii=0;ii<numVerts;ii++,basePtr+=vertexSize)
            addPointToBuffer(basePtr, ii,NULL);
        
        // Now the element buffer
        triBuffer = numVerts*vertexSize;
        for (unsigned int ii=0;ii<tris.size();ii++,basePtr+=sizeof(Triangle))
            memcpy(basePtr, &tris[ii], sizeof(Triangle));
        
        glBufferData(GL_ARRAY_BUFFER, bufferSize, glMem, GL_STATIC_DRAW);
        free(glMem);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Clear out the arrays, since we won't need them again
    numPoints = (int)points.size();
    points.clear();
    numTris = (int)tris.size();
    tris.clear();
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        vertexAttributes[ii]->clear();
    
    usingBuffers = true;
    isSetupGL = true;
}

// Tear down the VBOs we set up
void BasicDrawableGLES::teardownForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    isSetupGL = false;
    if (vertArrayObj)
        glDeleteVertexArrays(1,&vertArrayObj);
    vertArrayObj = 0;
    
    if (sharedBuffer)
    {
        setupInfo->memManager->removeBufferID(sharedBuffer);
        sharedBuffer = 0;
    } else {
        if (pointBuffer)
            setupInfo->memManager->removeBufferID(pointBuffer);
        if (triBuffer)
            setupInfo->memManager->removeBufferID(triBuffer);
    }
    pointBuffer = 0;
    triBuffer = 0;
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        ((VertexAttributeGLES *)vertexAttributes[ii])->buffer = 0;
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
        glBindBuffer(GL_ARRAY_BUFFER,sharedBuffer);
        CheckGLError("BasicDrawable::setupVAO() shared glBindBuffer");
    }
    
    // Vertex array
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray ( vertAttr->index );
    }
    
    // All the rest of the attributes
    const OpenGLESAttribute *progAttrs[vertexAttributes.size()];
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        progAttrs[ii] = NULL;
        VertexAttributeGLES *attr = (VertexAttributeGLES *)vertexAttributes[ii];
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attr->nameID);
        if (thisAttr) {
            if (attr->buffer != 0 || attr->numElements() != 0) {
                glEnableVertexAttribArray(thisAttr->index);
                glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), vertexSize, CALCBUFOFF(0,attr->buffer));
                progAttrs[ii] = thisAttr;
            } else {
                VertAttrDefault attrDef(thisAttr->index,*attr);
                vertArrayDefaults.push_back(attrDef);
            }
        }
    }
    
    // Bind the element array
    bool boundElements = false;
    if (type == Triangles && triBuffer)
    {
        boundElements = true;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
        CheckGLError("BasicDrawable::setupVAO() glBindBuffer");
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
    ProgramGLES *prog = (ProgramGLES *)frameInfo->program;
    SceneGLES *scene = (SceneGLES *)inScene;
    
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
                    fade = 1.0-(frameInfo->currentTime - fadeUp)/(fadeDown - fadeUp);
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
    for (unsigned int ii=0;ii<texInfo.size();ii++)
    {
        const TexInfo &thisTexInfo = texInfo[ii];
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
            auto thisTexInfo = texInfo[ii];
            glActiveTexture(GL_TEXTURE0+ii+progTexBound);
            glBindTexture(GL_TEXTURE_2D, glTexID);
            CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
            prog->setUniform(baseMapNameID, (int)ii+progTexBound);
            prog->setUniform(hasBaseMapNameID, 1);
            float texScale = 1.0;
            Vector2f texOffset(0.0,0.0);
            // Adjust for border pixels
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
            CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
        } else {
            prog->setUniform(hasBaseMapNameID, 0);
        }
    }
    
    const OpenGLESAttribute *vertAttr = NULL;
    bool boundElements = false;
    bool usedLocalVertices = false;
    std::vector<const OpenGLESAttribute *> progAttrs;
    
    if (hasVertexArraySupport)
    {
        // If necessary, set up the VAO (once)
        if (vertArrayObj == 0 && sharedBuffer != 0)
            vertArrayObj = setupVAO(prog);
        
        // Figure out what we're using
        vertAttr = prog->findAttribute(a_PositionNameID);
        
        // Vertex array
        bool usedLocalVertices = false;
        if (vertAttr && !(sharedBuffer || pointBuffer))
        {
            usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }
        
        // Other vertex attributes
        if (!vertArrayObj) {
            progAttrs.resize(vertexAttributes.size(),NULL);
            
            for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            {
                VertexAttributeGLES *attr = (VertexAttributeGLES *)vertexAttributes[ii];
                const OpenGLESAttribute *progAttr = prog->findAttribute(attr->nameID);
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
                            attr->glSetDefault(progAttr->index);
                            CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                        }
                    }
                }
            }
        } else {
            // Vertex Array Objects can't hold the defaults, so we build them earlier
            for (auto attrDef : vertArrayDefaults) {
                // The program is expecting it, so we need a default
                attrDef.attr.glSetDefault(attrDef.progAttrIndex);
                CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
            }
        }
    } else {
        vertAttr = prog->findAttribute(a_PositionNameID);
        progAttrs.resize(vertexAttributes.size(),NULL);
        
        // We're using a single buffer for all of our vertex attributes
        if (sharedBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER,sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO2() shared glBindBuffer");
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
        } else {
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
        }
        usedLocalVertices = true;
        glEnableVertexAttribArray ( vertAttr->index );
        //        WHIRLYKIT_LOGD("BasicDrawable glEnableVertexAttribArray %d",vertAttr->index);
        
        // All the rest of the attributes
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            progAttrs[ii] = NULL;
            VertexAttributeGLES *attr = (VertexAttributeGLES *)vertexAttributes[ii];
            const OpenGLESAttribute *thisAttr = prog->findAttribute(attr->nameID);
            if (thisAttr)
            {
                if (attr->buffer != 0 || attr->numElements() != 0)
                {
                    if (attr->buffer)
                        glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), vertexSize, CALCBUFOFF(0,attr->buffer));
                    else
                        glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), 0, attr->addressForElement(0));
                    glEnableVertexAttribArray(thisAttr->index);
                    //                    WHIRLYKIT_LOGD("BasicDrawable glEnableVertexAttribArray %d",thisAttr->index);
                    progAttrs[ii] = thisAttr;
                } else {
                    // The program is expecting it, so we need a default
                    attr->glSetDefault(thisAttr->index);
                    CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                }
            }
        }
        
        // Bind the element array
        if (type == Triangles && sharedBuffer)
        {
            boundElements = true;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
            //            WHIRLYKIT_LOGD("BasicDrawable glBindBuffer %d",sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
        }
    }
    
    // Color has been overriden, so don't use the embedded ones
    if (hasOverrideColor) {
        const OpenGLESAttribute *colorAttr = prog->findAttribute(a_colorNameID);
        if (colorAttr)
            glVertexAttrib4f(colorAttr->index, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
    }
    
    
    // If we're using a vertex array object, bind it and draw
    if (vertArrayObj)
    {
        glBindVertexArray(vertArrayObj);
        switch (type)
        {
            case Triangles:
                glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(0,triBuffer));
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                break;
            case Points:
                glDrawArrays(GL_POINTS, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
            case Lines:
                glLineWidth(lineWidth);
                glDrawArrays(GL_LINES, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
//            case GL_TRIANGLE_STRIP:
//                glDrawArrays(type, 0, numPoints);
//                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
//                break;
        }
        glBindVertexArray(0);
    } else {
        // Draw without a VAO
        switch (type)
        {
            case Triangles:
            {
                if (triBuffer)
                {
                    if (!boundElements)
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
                    CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
                    glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, (void *)((uintptr_t)triBuffer));
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                } else {
                    if (!boundElements)
                        glDrawElements(GL_TRIANGLES, (GLsizei)tris.size()*3, GL_UNSIGNED_SHORT, &tris[0]);
                    else
                        glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, 0);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                }
            }
                break;
            case Points:
                glDrawArrays(GL_POINTS, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
            case Lines:
                glLineWidth(lineWidth);
                CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
                glDrawArrays(GL_LINES, 0, numPoints);
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
        if (hasTexture[ii])
        {
            glActiveTexture(GL_TEXTURE0+ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    
    // Tear down the various arrays, if we stood them up
    if (usedLocalVertices)
        glDisableVertexAttribArray(vertAttr->index);
    if (!vertArrayObj) {
        for (unsigned int ii=0;ii<progAttrs.size();ii++)
            if (progAttrs[ii])
                glDisableVertexAttribArray(progAttrs[ii]->index);
    }
    
    if (!hasVertexArraySupport)
    {
        // Now tear down all that state
        if (vertAttr)
        {
            glDisableVertexAttribArray(vertAttr->index);
            //            WHIRLYKIT_LOGD("BasicDrawable glDisableVertexAttribArray %d",vertAttr->index);
        }
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            if (progAttrs[ii])
            {
                glDisableVertexAttribArray(progAttrs[ii]->index);
                //                WHIRLYKIT_LOGD("BasicDrawable glDisableVertexAttribArray %d",progAttrs[ii]->index);
            }
        if (boundElements) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            //            WHIRLYKIT_LOGD("BasicDrawable glBindBuffer 0");
        }
        if (sharedBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            //            WHIRLYKIT_LOGD("BasicDrawable glBindBuffer 0");
        }
    }
}

}
