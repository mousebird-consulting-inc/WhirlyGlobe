/*
 *  ParticleSystemDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
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

#import "ParticleSystemDrawable.h"
#import "GLUtils.h"
#import "BasicDrawable.h"
#import "GlobeScene.h"
#import "SceneRendererES.h"
#import "TextureAtlas.h"
#import "WhirlyKitLog.h"


namespace WhirlyKit
{

ParticleSystemDrawable::ParticleSystemDrawable(const std::string &name,const std::vector<SingleVertexAttributeInfo> &inVertAttrs,int numTotalPoints,int batchSize,bool useRectangles,bool useInstancing)
    : Drawable(name), enable(true), numTotalPoints(numTotalPoints), batchSize(batchSize), vertexSize(0), programId(0), drawPriority(0), pointBuffer(0), rectBuffer(0), requestZBuffer(false), writeZBuffer(false), minVis(0.0), maxVis(10000.0), useRectangles(useRectangles), useInstancing(useInstancing), baseTime(0.0), startb(0), endb(0), chunksDirty(true)
{
    pthread_mutex_init(&batchLock, NULL);
    
    for (auto attr : inVertAttrs)
    {
        vertexSize += attr.size();
        vertAttrs.push_back(attr);
    }
}
    
ParticleSystemDrawable::~ParticleSystemDrawable()
{
    pthread_mutex_destroy(&batchLock);
}
    
bool ParticleSystemDrawable::isOn(RendererFrameInfo *frameInfo) const
{
    if (!enable)
        return false;
    
    return true;
}
    
void ParticleSystemDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    if (pointBuffer != 0)
        return;
    
    int totalBytes = vertexSize*numTotalPoints;
    pointBuffer = memManager->getBufferID(totalBytes,GL_DYNAMIC_DRAW);
    
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
            rectBuffer = memManager->getBufferID(0,GL_STATIC_DRAW);
            
            glBindBuffer(GL_ARRAY_BUFFER, rectBuffer);
            glBufferData(GL_ARRAY_BUFFER, rectSize, (const GLvoid *)&verts[0], GL_STATIC_DRAW);
            CheckGLError("ParticleSystemDrawable::setupGL() glBufferData");
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else {
           // NSLog(@"ParticleSystemDrawable: Can only do instanced rectangles at present.  This system can't handle instancing.");
        }
    }
    
    // Set up the batches
    int numBatches = numTotalPoints / batchSize;
    int batchBufLen = batchSize * vertexSize;
    batches.resize(numBatches);
    unsigned int bufOffset = 0;
    for (unsigned int ii=0;ii<numBatches;ii++)
    {
        Batch &batch = batches[ii];
        batch.active = false;
        batch.batchID = ii;
        batch.offset = bufOffset;
        batch.len = batchBufLen;
        bufOffset += batchBufLen;
    }
    chunks.clear();
    chunksDirty = true;
    
    // Zero it out to avoid warnings
    // Note: Don't actually have to do this
//    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
//    void *glMem = NULL;
//    EAGLContext *context = [EAGLContext currentContext];
//    if (context.API < kEAGLRenderingAPIOpenGLES3)
//        glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//    else
//        glMem = glMapBufferRange(GL_ARRAY_BUFFER, 0, totalBytes, GL_MAP_WRITE_BIT);
//    memset(glMem, 0, totalBytes);
//    if (context.API < kEAGLRenderingAPIOpenGLES3)
//        glUnmapBufferOES(GL_ARRAY_BUFFER);
//    else
//        glUnmapBuffer(GL_ARRAY_BUFFER);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystemDrawable::teardownGL(OpenGLMemManager *memManager)
{
    if (pointBuffer)
        memManager->removeBufferID(pointBuffer);
    pointBuffer = 0;
    if (rectBuffer)
        memManager->removeBufferID(rectBuffer);
    rectBuffer = 0;
    batches.clear();
    chunks.clear();
}
    
void ParticleSystemDrawable::updateRenderer(SceneRendererES *renderer)
{
    renderer->addContinuousRenderRequest(getId());
}
    
void ParticleSystemDrawable::addAttributeData(const std::vector<AttributeData> &attrData,const Batch &batch)
{
    if (attrData.size() != vertAttrs.size())
        return;
    
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    unsigned char *glMem = NULL;

    int glMemOffset = 0;
    unsigned long chunkSize = batchSize * vertexSize;

    if (hasMapBufferSupport)
    {
    //    EAGLContext *context = EAGLContext->currentContext;
    //    if (context.API < kEAGLRenderingAPIOpenGLES3)
    //    {
    //       glMem = (unsigned char *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    //       glMemOffset = batch.offset*vertexSize*batchSize;
    //    } else {
    //       glMem = (unsigned char *)glMapBufferRange(GL_ARRAY_BUFFER, batch.batchID*vertexSize*batchSize, vertexSize*batchSize, GL_MAP_WRITE_BIT);
       //    }
    } else {
        glMem = (unsigned char *)malloc(chunkSize);
    }
    
    // Work through the attribute blocks
    int attrOffset = 0;
    for (unsigned int ai=0;ai<vertAttrs.size();ai++)
    {
        const AttributeData &thisAttrData = attrData[ai];
        SingleVertexAttributeInfo &attrInfo = vertAttrs[ai];
        int attrSize = attrInfo.size();
        unsigned char *rawAttrData = (unsigned char *)thisAttrData.data;
        unsigned char *ptr = glMem + attrOffset + glMemOffset;
        // Copy into each vertex
        for (unsigned int ii=0;ii<batchSize;ii++)
        {
            memcpy(ptr, rawAttrData, attrSize);
            ptr += vertexSize;
            rawAttrData += attrSize;
        }
        
        attrOffset += attrSize;
    }
    
    if (hasMapBufferSupport)
    {
   // if (context.API < kEAGLRenderingAPIOpenGLES3)
   //  glUnmapBufferOES(GL_ARRAY_BUFFER);
    //else
    //    glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, batch.offset, chunkSize, glMem);
        CheckGLError("ParticleSystemDrawable::addAttributeData() glBufferSubData");
        free(glMem);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    pthread_mutex_lock(&batchLock);
    batches[batch.batchID] = batch;
    batches[batch.batchID].active = true;
    pthread_mutex_unlock(&batchLock);
    
    chunksDirty = true;
    
//    WHIRLYKIT_LOGV("Added batch");
}
    
void ParticleSystemDrawable::updateBatches(TimeInterval now)
{
    pthread_mutex_lock(&batchLock);
    // Check the batches to see if any have gone off
    for (int bi=startb;bi<endb;)
    {
        Batch &batch = batches[bi % batches.size()];
        if (batch.active)
        {
            if (batch.startTime + lifetime < now)
            {
                batch.active = false;
                chunksDirty = true;
                startb++;

//                WHIRLYKIT_LOGV("Canceling batch");
            }
        } else
            break;
        
        bi++;
    }
    pthread_mutex_unlock(&batchLock);
    
    updateChunks();
}
    
void ParticleSystemDrawable::updateChunks()
{
    if (!chunksDirty)
        return;
    
    pthread_mutex_lock(&batchLock);
    
    chunksDirty = false;
    chunks.clear();
    if (startb != endb)
    {
        int start = 0;
        do {
            // Skip empty batches at the beginning
            for (;!batches[start].active && start < batches.size();start++);

            int end = start;
            if (start < batches.size())
            {
                for (;batches[end].active && end < batches.size();end++);
                if (start != end)
                {
                    BufferChunk chunk;
                    chunk.bufferStart = (start % batches.size()) * batchSize * vertexSize;
                    chunk.numVertices = (end-start) * batchSize;
                    chunks.push_back(chunk);
                    
//                    WHIRLYKIT_LOGV("Set up chunk: bufferStart = %d; numVertices = %d",chunk.bufferStart,chunk.numVertices);
                }
            }
            
            start = end;
        } while (start < batches.size());
    }
    
    pthread_mutex_unlock(&batchLock);
}
    
bool ParticleSystemDrawable::findEmptyBatch(Batch &retBatch)
{
    bool ret = false;
    
    pthread_mutex_lock(&batchLock);
    if (!batches[endb % batches.size()].active)
    {
        ret = true;
        retBatch = batches[endb % batches.size()];
        endb++;
    }
    pthread_mutex_unlock(&batchLock);
    
    return ret;
}

void ParticleSystemDrawable::draw(RendererFrameInfo *frameInfo,Scene *scene)
{
    updateBatches(frameInfo->currentTime);
    updateChunks();
    
//	TODO REVIEW
// 	EAGLContext *context = [EAGLContext currentContext];
    OpenGLES2Program *prog = frameInfo->program;
    
    // GL Texture IDs
    bool anyTextures = false;
    std::vector<GLuint> glTexIDs;
    for (SimpleIdentity texID : texIDs)
    {
        GLuint glTexID = scene->getGLTexture(texID);
        anyTextures = true;
        glTexIDs.push_back(glTexID);
    }

    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo->mvpMat);
    prog->setUniform("u_mvMatrix", frameInfo->viewAndModelMat);
    prog->setUniform("u_mvNormalMatrix", frameInfo->viewModelNormalMat);
    prog->setUniform("u_mvpNormalMatrix", frameInfo->mvpNormalMat);
    prog->setUniform("u_pMatrix", frameInfo->projMat);
    prog->setUniform("u_scale", Point2f(2.f/(float)frameInfo->sceneRenderer->framebufferWidth,2.f/(float)frameInfo->sceneRenderer->framebufferHeight));
    
    // If this is present, the drawable wants to do something based where the viewer is looking
    prog->setUniform("u_eyeVec", frameInfo->fullEyeVec);
    
    prog->setUniform("u_size", pointSize);
    prog->setUniform("u_time", (float)(frameInfo->currentTime-baseTime));
    prog->setUniform("u_lifetime", (float)lifetime);
    
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

    // Use the rectangle buffer for instancing
    if (rectBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER,rectBuffer);
        const OpenGLESAttribute *thisAttr = prog->findAttribute("a_offset");
        if (thisAttr)
        {
            glVertexAttribPointer(thisAttr->index, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (const GLvoid *)(long)0);
            CheckGLError("ParticleSystemDrawable::draw glVertexAttribPointer");
           // if (context.API < kEAGLRenderingAPIOpenGLES3)
            //    glVertexAttribDivisorEXT(thisAttr->index, 0);
            //else
                glVertexAttribDivisor(thisAttr->index, 0);
            glEnableVertexAttribArray(thisAttr->index);
            CheckGLError("ParticleSystemDrawable::draw glEnableVertexAttribArray");
        }
        thisAttr = prog->findAttribute("a_texCoord");
        if (thisAttr)
        {
            glVertexAttribPointer(thisAttr->index, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (const GLvoid *)(long)(2*sizeof(GLfloat)));
            CheckGLError("ParticleSystemDrawable::draw glVertexAttribPointer");
           // if (context.API < kEAGLRenderingAPIOpenGLES3)
             //   glVertexAttribDivisorEXT(thisAttr->index, 0);
            //else
                glVertexAttribDivisor(thisAttr->index, 0);
            glEnableVertexAttribArray(thisAttr->index);
            CheckGLError("ParticleSystemDrawable::draw glEnableVertexAttribArray");
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER,pointBuffer);
    CheckGLError("BasicDrawable::drawVBO2() glBindTexture");

    // Work through the batches
    for (const BufferChunk &chunk : chunks)
    {
        // Bind the various attributes to their offsets
        int attrOffset = 0;
        for (SingleVertexAttributeInfo &attrInfo : vertAttrs)
        {
            int attrSize = attrInfo.size();
            
            const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.name);
            if (thisAttr)
            {
                glVertexAttribPointer(thisAttr->index, attrInfo.glEntryComponents(), attrInfo.glType(), attrInfo.glNormalize(), vertexSize, (const GLvoid *)(long)(attrOffset));
                CheckGLError("ParticleSystemDrawable::draw() glVertexAttribPointer");
                int divisor = 0;
                
                if (useInstancing)
                    divisor = 1;
                //if (context.API < kEAGLRenderingAPIOpenGLES3)
                  //  glVertexAttribDivisorEXT(thisAttr->index, divisor);
                //else
		        if (divisor != 0)
                    glVertexAttribDivisor(thisAttr->index, divisor);
                CheckGLError("ParticleSystemDrawable::draw() glVertexAttribDivisor");
                glEnableVertexAttribArray(thisAttr->index);
                CheckGLError("ParticleSystemDrawable::draw() glEnableVertexAttribArray");
            }
            
            attrOffset += attrSize;
        }

        if (rectBuffer)
        {
          //  if (context.API < kEAGLRenderingAPIOpenGLES3)
            //    glDrawArraysInstancedEXT(GL_TRIANGLES, 0, 6, chunk.numVertices);
            //else
                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk.numVertices);
            CheckGLError("ParticleSystemDrawable::draw() glDrawArraysInstanced");
        } else {
            glDrawArrays(GL_POINTS, chunk.bufferStart / vertexSize, chunk.numVertices);
            CheckGLError("ParticleSystemDrawable::draw() glDrawArrays");
        }
    }
    
    if (rectBuffer)
    {
        const OpenGLESAttribute *thisAttr = prog->findAttribute("a_offset");
        if (thisAttr)
        {
            glDisableVertexAttribArray(thisAttr->index);
            CheckGLError("ParticleSystemDrawable glDisableVertexAttribArray");
        }
        thisAttr = prog->findAttribute("a_texCoord");
        if (thisAttr)
        {
            glDisableVertexAttribArray(thisAttr->index);
            CheckGLError("ParticleSystemDrawable glDisableVertexAttribArray");
        }
    }
    
    // Tear down the state
    for (SingleVertexAttributeInfo &attrInfo : vertAttrs)
    {
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.name);
        if (thisAttr) {
            glDisableVertexAttribArray(thisAttr->index);
        }
    }

    // Unbind any textures
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
        if (hasTexture[ii])
        {
            frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
    
static const char *vertexShaderTri =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_size;"
"uniform float u_time;"
""
"attribute vec3 a_position;"
"attribute vec4 a_color;"
"attribute vec3 a_dir;"
"attribute float a_startTime;"
""
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_color = a_color;"
"   vec3 thePos = normalize(a_position + (u_time-a_startTime)*a_dir);"
// Convert from model space into display space
"   vec4 pt = u_mvMatrix * vec4(thePos,1.0);"
"   pt /= pt.w;"
// Make sure the object is facing the user
"   vec4 testNorm = u_mvNormalMatrix * vec4(thePos,0.0);"
"   float dot_res = dot(-pt.xyz,testNorm.xyz);"
// Set the point size
    "   gl_PointSize = u_size;"
// Project the point into 3-space
"   gl_Position = (dot_res > 0.0) ? u_mvpMatrix * vec4(thePos,1.0) : vec4(1000.0,1000.0,1000.0,0.0);"
"}"
;

static const char *fragmentShaderTri =
"precision lowp float;"
""
"varying vec4      v_color;"
""
"void main()"
"{"
    "  gl_FragColor = v_color;"
"}"
;
    
OpenGLES2Program *BuildParticleSystemProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kParticleSystemShaderName,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
        WHIRLYKIT_LOGV("Failed to build Particle System Shader");
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}
    
}
