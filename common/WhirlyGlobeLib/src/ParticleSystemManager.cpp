/*  ParticleSystemManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/26/15.
 *  Copyright 2011-2023 mousebird consulting.
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

#import "ParticleSystemManager.h"
#import "ParticleSystemDrawable.h"
#import "BasicDrawableInstanceBuilder.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

ParticleSystemSceneRep::ParticleSystemSceneRep(SimpleIdentity inId) :
    Identifiable(inId)
{
}

void ParticleSystemSceneRep::clearContents(ChangeSet &changes)
{
    for (const ParticleSystemDrawable *it : draws)
    {
        changes.push_back(new RemDrawableReq(it->getId()));
    }
    draws.clear();

    for (const auto id : basicIDs)
    {
        changes.push_back(new RemDrawableReq(id));
    }
    basicIDs.clear();

    for (const auto id : instIDs)
    {
        changes.push_back(new RemDrawableReq(id));
    }
    instIDs.clear();
}

void ParticleSystemSceneRep::enableContents(bool enable,ChangeSet &changes)
{
    for (const ParticleSystemDrawable *it : draws)
    {
        changes.push_back(new OnOffChangeRequest(it->getId(), enable));
    }
    
    for (auto id : basicIDs)
    {
        changes.push_back(new OnOffChangeRequest(id, enable));
    }
    for (auto id : instIDs)
    {
        changes.push_back(new OnOffChangeRequest(id, enable));
    }
}

ParticleSystemManager::~ParticleSystemManager()
{
    try
    {
        std::lock_guard<std::mutex> guardLock(lock);
        for (auto it : sceneReps)
        {
            delete it.second;
        }
        sceneReps.clear();
    }
    WK_STD_DTOR_CATCH()
}

SimpleIdentity ParticleSystemManager::addParticleSystem(const ParticleSystem &newSystem,ChangeSet &changes)
{
    ParticleSystemSceneRep *sceneRep = new ParticleSystemSceneRep(newSystem.getId());

    sceneRep->partSys = newSystem;
    
    SimpleIdentity partSysID = sceneRep->getId();
    
    if (renderer->getType() == SceneRenderer::RenderMetal) {
        // Use a basic drawable for the particle geometry
        BasicDrawableBuilderRef basicBuild = renderer->makeBasicDrawableBuilder(newSystem.name + " Base Calculate");
        basicBuild->setOnOff(false);
        basicBuild->setType(GeometryType::Triangles);

        basicBuild->addPoint(Point3f(-0.5f, -0.5f, 0.0f));  basicBuild->addTexCoord(0, TexCoord(0.0f, 0.0f));
        basicBuild->addPoint(Point3f( 0.5f, -0.5f, 0.0f));  basicBuild->addTexCoord(0, TexCoord(1.0f, 0.0f));
        for (int i = 0; i < newSystem.trianglesPerParticle; ++i)
        {
            // i tri  pt             tri      tex     tex      CW       CCW
            //    0   (-0.5,-0.5,0)  (0,1,2)  (0, 0)  (1, 0)   0--→1    1←--0    0: 0→1→2
            //    0   ( 0.5,-0.5,0)  (1,3,2)  (1, 0)  (0, 0)   ↑ / ↓    ↓ \ ↑    1: 1→3→2
            // 0  0   (-0.5, 0.5,0)  (2,3,4)  (0, 1)  (1, 1)   2←--3    3--→2
            // 1  1   ( 0.5, 0.5,0)  (3,5,4)  (1, 1)  (0, 1)   2--→3    3←--2    2: 2→3→4
            // 2  2   (-0.5, 1.5,0)  (4,5,6)  (0, 2)  (1, 2)   ↑ / ↓    ↓ \ ↑    3: 3→5→4
            // 3  3   ( 0.5, 1.5,0)  (5,7,6)  (1, 2)  (0, 2)   4←--5    5--→4
            // 4  4   (-0.5, 2.5,0)  (6,7,8)  (0, 3)  (1, 3)
            // 5  5   ( 0.5, 2.5,0)  (7,9,8)  (1, 3)  (0, 3)
            const float x = float((i & 1) * 2 - 1) / 2;
            const float y = float(i / 2 * 2 + 1) / 2 - 1;
            const float texX = i & 1;
            const float texY = float((i + 2) / 2) / ((newSystem.trianglesPerParticle + 1) / 2);
            const int tri1 = i + 1 + (i & 1);
            const int tri2 = i + 2 - (i & 1);
            basicBuild->addPoint(Point3f(x, y, 0.f));
            basicBuild->addTexCoord(0, TexCoord(texX, texY));
            basicBuild->addTriangle(BasicDrawable::Triangle(i, tri1, tri2));
        }

        basicBuild->setProgram(Program::NoProgramID);       // Don't actually draw this one
        basicBuild->setCalculationProgram(Program::NoProgramID);
        sceneRep->basicIDs.insert(basicBuild->getDrawableID());
        changes.push_back(new AddDrawableReq(basicBuild->getDrawable()));
        
        // And another basic drawable for the calculation phase
        BasicDrawableBuilderRef calcBuild = renderer->makeBasicDrawableBuilder(newSystem.name + " Calculation");
        calcBuild->setOnOff(newSystem.enable);
        calcBuild->setCalculationProgram(newSystem.calcShaderID);
        sceneRep->basicIDs.insert(calcBuild->getDrawableID());
        calcBuild->setCalculationData(newSystem.totalParticles, newSystem.partData);
        calcBuild->setTexIDs(sceneRep->partSys.texIDs);
        changes.push_back(new AddDrawableReq(calcBuild->getDrawable()));

        // Set up a single instance to manage the particle system
        BasicDrawableInstanceBuilderRef instDrawBuild = renderer->makeBasicDrawableInstanceBuilder(newSystem.name + " Instance");
        instDrawBuild->setOnOff(newSystem.enable);
        instDrawBuild->setProgram(sceneRep->partSys.renderShaderID);
        instDrawBuild->setDrawOrder(sceneRep->partSys.drawOrder);
        instDrawBuild->setDrawPriority(sceneRep->partSys.drawPriority);
        instDrawBuild->setContinuousUpdate(sceneRep->partSys.continuousUpdate);
        instDrawBuild->setRequestZBuffer(sceneRep->partSys.zBufferRead);
        instDrawBuild->setWriteZBuffer(sceneRep->partSys.zBufferWrite);
        instDrawBuild->setRenderTarget(sceneRep->partSys.renderTargetID);
        instDrawBuild->setMasterID(basicBuild->getDrawableID(), BasicDrawableInstance::ReferenceStyle);
        instDrawBuild->setInstID(calcBuild->getDrawableID());
        auto instDraw = instDrawBuild->getDrawable();
        instDraw->setBlendPremultipliedAlpha(newSystem.blendPremultipliedAlpha);
        instDraw->setupForRenderer(renderer->getRenderSetupInfo(),renderer->getScene());
        sceneRep->instIDs.insert(instDrawBuild->getDrawableID());
        changes.push_back(new AddDrawableReq(instDraw));
    } else {
        // Set up a single giant drawable for a particle system
        const bool useRectangles = sceneRep->partSys.type == ParticleSystemRectangle;
        // Note: There are devices where this won't work
        const bool useInstancing = useRectangles;
        const int totalParticles = newSystem.totalParticles;
        ParticleSystemDrawableBuilderRef draw = renderer->makeParticleSystemDrawableBuilder(newSystem.name);
        draw->setup(sceneRep->partSys.vertAttrs,
                    sceneRep->partSys.varyingAttrs,
                    sceneRep->partSys.varyNames,
                    totalParticles,
                    sceneRep->partSys.batchSize,
                    newSystem.vertexSize,
                    useRectangles,
                    useInstancing);
        draw->getDrawable()->setOnOff(newSystem.enable);
        //draw->getDrawable()->setPointSize(sceneRep->partSys.pointSize);
        draw->getDrawable()->setProgram(sceneRep->partSys.renderShaderID);
        draw->getDrawable()->setCalculationProgram(sceneRep->partSys.calcShaderID);
        draw->getDrawable()->setDrawOrder(sceneRep->partSys.drawOrder);
        draw->getDrawable()->setDrawPriority(sceneRep->partSys.drawPriority);
        draw->getDrawable()->setBaseTime(newSystem.baseTime);
        draw->getDrawable()->setLifetime(sceneRep->partSys.lifetime);
        draw->getDrawable()->setTexIDs(sceneRep->partSys.texIDs);
        draw->getDrawable()->setContinuousUpdate(sceneRep->partSys.continuousUpdate);
        draw->getDrawable()->setRequestZBuffer(sceneRep->partSys.zBufferRead);
        draw->getDrawable()->setWriteZbuffer(sceneRep->partSys.zBufferWrite);
        draw->getDrawable()->setRenderTarget(sceneRep->partSys.renderTargetID);
        draw->getDrawable()->setupForRenderer(renderer->getRenderSetupInfo(),renderer->getScene());
        changes.push_back(new AddDrawableReq(draw->getDrawable()));
        sceneRep->draws.insert(draw->getDrawable());
    }
    
    {
        std::lock_guard<std::mutex> guardLock(lock);
        sceneReps[partSysID] = sceneRep;
    }
    
    return partSysID;
}
    
void ParticleSystemManager::enableParticleSystem(SimpleIdentity sysID,bool enable,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    auto it = sceneReps.find(sysID);
    if (it != sceneReps.end())
        it->second->enableContents(enable, changes);
}
    
void ParticleSystemManager::removeParticleSystem(SimpleIdentity sysID,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    auto it = sceneReps.find(sysID);
    if (it != sceneReps.end())
    {
        ParticleSystemSceneRep *ps = it->second;
        ps->clearContents(changes);
        delete ps;
        sceneReps.erase(it);
    }
}
    
void ParticleSystemManager::addParticleBatch(SimpleIdentity sysID,const ParticleBatch &batch,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

//    TimeInterval now = TimeGetCurrent();
    
    ParticleSystemSceneRep *sceneRep = NULL;
    auto it = sceneReps.find(sysID);
    if (it != sceneReps.end())
        sceneRep = it->second;
    
    if (sceneRep) {
        // Should be one drawable in there
        ParticleSystemDrawable *draw = NULL;
        if (sceneRep->draws.size() == 1)
            draw = *(sceneRep->draws.begin());
        
        if (draw) {
            ParticleSystemDrawable::Batch theBatch;
            if (draw->findEmptyBatch(theBatch)) {
                if (renderer->getType() == SceneRenderer::RenderGLES) {
                    // For OpenGL we match everything up
                    std::vector<ParticleSystemDrawable::AttributeData> attrData;
                    for (unsigned int ii=0;ii<batch.attrData.size();ii++) {
                        ParticleSystemDrawable::AttributeData thisAttrData;
                        thisAttrData.data = batch.attrData[ii];
                        attrData.push_back(thisAttrData);
                    }
                    // Note: Should pick this up from the batch
                    theBatch.startTime = scene->getCurrentTime();
                    draw->addAttributeData(renderer->getRenderSetupInfo(),attrData,theBatch);
                } else {
                    // For Metal, we just pass a block of data through
                    draw->addAttributeData(renderer->getRenderSetupInfo(),batch.data,theBatch);
                }
            }
        }
    }
}
    
void ParticleSystemManager::changeRenderTarget(SimpleIdentity sysID,SimpleIdentity targetID,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    ParticleSystemSceneRep *sceneRep = NULL;
    auto it = sceneReps.find(sysID);
    if (it != sceneReps.end())
        sceneRep = it->second;
    
    if (sceneRep) {
        ParticleSystemDrawable *draw = NULL;
        if (sceneRep->draws.size() == 1)
            draw = *(sceneRep->draws.begin());
        
        if (draw) {
            changes.push_back(new RenderTargetChangeRequest(draw->getId(),targetID));
        }
    }
}
    
void ParticleSystemManager::setUniformBlock(const SimpleIDSet &partSysIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);
    
    for (auto sysID : partSysIDs) {
        auto it = sceneReps.find(sysID);
        if (it != sceneReps.end())
            for (auto draw : it->second->draws)
                changes.push_back(new UniformBlockSetRequest(draw->getId(),uniBlock,bufferID));
    }
}

}

