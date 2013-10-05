/*
 *  ParticleGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/12/11.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "ParticleGenerator.h"
#import "SceneRendererES.h"

using namespace Eigen;

namespace WhirlyKit
{
    
// Construct a particle system with the default values
ParticleGenerator::ParticleSystem ParticleGenerator::ParticleSystem::makeDefault()
{
    ParticleGenerator::ParticleSystem partSys;
    partSys.loc = Point3f(0,0,0);
    partSys.dirUp = Vector3f(0,0,1);
    partSys.dirE = Vector3f(1,0,0);
    partSys.dirN = Vector3f(0,1,0);
    partSys.minLength = partSys.maxLength = 0.1;
    partSys.numPerSecMin = partSys.numPerSecMax = 100;
    partSys.minLifetime = partSys.maxLifetime = 1.0;
    partSys.minPhi = 0;  partSys.maxPhi = M_PI/2.0;
    partSys.minVis = partSys.maxVis = DrawVisibleInvalid;
    
    return partSys;
}
    
// Given our parameters, generate a randomized particle
ParticleGenerator::Particle ParticleGenerator::ParticleSystem::generateParticle()
{
    Particle newParticle;
    
    newParticle.loc = loc;
    float lifetime = drand48()*(maxLifetime-minLifetime)+minLifetime;
    float len = drand48()*(maxLength-minLength)+minLength;
    float phi = drand48()*(maxPhi-minPhi)+minPhi;
    float theta = drand48()*(2*M_PI);
    // Note: Too expensive
    Vector3f dir = dirE*sinf(theta) + dirN*cosf(theta);
    dir.normalize();
    newParticle.dir = dir*sinf(phi) + dirUp*cosf(phi);
    newParticle.dir.normalize();
    
    newParticle.expiration = lifetime;
    newParticle.velocity = len/lifetime;

    if (colors.empty())
        newParticle.color = RGBAColor(255,255,255,255);
    else {
        int which = random()%colors.size();
        newParticle.color = colors[which];
    }
    
    return newParticle;
}


ParticleGenerator::ParticleGenerator(int maxNumParticles)
    : maxNumParticles(maxNumParticles), lastUpdateTime(0.0)
{
    particles.reserve(maxNumParticles);
    startTime = CFAbsoluteTimeGetCurrent();
}
    
ParticleGenerator::~ParticleGenerator()
{
    for (ParticleSystemSet::iterator it = particleSystems.begin();
         it != particleSystems.end(); ++it)
        delete (*it);
    particles.clear();
}
    
void ParticleGenerator::addParticleSystem(ParticleSystem *particleSystem)
{
    particleSystems.insert(particleSystem);
}
    
void ParticleGenerator::removeParticleSystem(SimpleIdentity theId)
{
    ParticleSystem dumbSys;
    dumbSys.setId(theId);
    
    ParticleSystemSet::iterator it = particleSystems.find(&dumbSys);
    if (it != particleSystems.end())
    {
        particleSystems.erase(it);
        delete *it;
    }
}
    
// Generate the drawables for this frame
void ParticleGenerator::generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo,std::vector<DrawableRef> &drawables,std::vector<DrawableRef> &screenDrawables)
{
    NSTimeInterval currentTime = CFAbsoluteTimeGetCurrent();

    // We won't run on the first frame.
    if (lastUpdateTime == 0.0)
    {
        lastUpdateTime = currentTime;
        return;
    }

    // Update the current particles
    int numFull = 0;
    float deltaT = frameInfo->frameLen;
    for (unsigned int ii=0;ii<particles.size();ii++)
    {
        Particle &particle = particles[ii];
        if (particle.expiration > currentTime)
        {            
            particle.loc += particle.dir * particle.velocity * deltaT;
            numFull++;
        }
    }
   
    // The maximum we can add
    int maxToAdd = maxNumParticles-numFull;
    int addPoint = 0;

    if (maxToAdd > 0)
    {
        // Work through the particle systems, adding new particles
        for (ParticleSystemSet::iterator it = particleSystems.begin();
             it != particleSystems.end(); ++it)
        {
            ParticleSystem *system = *it;
            int numPerSecDiff = system->numPerSecMax-system->numPerSecMin;
            numPerSecDiff = std::max(1,numPerSecDiff);
            int numToAddPerSec = random()%numPerSecDiff+system->numPerSecMin;
            int numToAddNow = numToAddPerSec * deltaT;
            
            // Add as many new particles as we can
            for (unsigned int ii=0;ii<numToAddNow && (maxToAdd > 0);ii++)
            {
                Particle newParticle = system->generateParticle();
                newParticle.expiration += currentTime;
                // Now find a spot for it
                while (addPoint < particles.size() && particles[addPoint].expiration > currentTime)
                    addPoint++;
                if (addPoint < particles.size())
                {
                    particles[addPoint] = newParticle;
                    addPoint++;
                } else
                    particles.push_back(newParticle);
                maxToAdd--;
                numFull++;
            }
        }
    }
    
    // Note: Should shrink the particles vector

    // Build a drawable containing all the active particles
    // Note: Should possibly build more than one
    BasicDrawable *draw = NULL;
    for (unsigned int ii=0;ii<particles.size();ii++)
    {
        Particle &particle = particles[ii];
        if (particle.expiration > currentTime)
        {
            if (!draw || draw->getNumPoints() >= MaxDrawablePoints)
            {
                draw = new BasicDrawable("Particle Generator",(numFull > MaxDrawablePoints ? MaxDrawablePoints : numFull),0);                
                draw->setType(GL_POINTS);
                drawables.push_back(DrawableRef(draw));
            }
            draw->addPoint(particle.loc);
            draw->addColor(particle.color);
        }
    }
    
    lastUpdateTime = currentTime;
}
    
ParticleGeneratorAddSystemRequest::ParticleGeneratorAddSystemRequest(SimpleIdentity generatorID,ParticleGenerator::ParticleSystem *partSystem)
{
    genId = generatorID;
    system = partSystem;
}

ParticleGeneratorAddSystemRequest::~ParticleGeneratorAddSystemRequest()
{
    if (system)
        delete system;
}
    
void ParticleGeneratorAddSystemRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ParticleGenerator *theGen = (ParticleGenerator *)gen;
    theGen->addParticleSystem(system);
    system = NULL;
}
    
ParticleGeneratorRemSystemRequest::ParticleGeneratorRemSystemRequest(SimpleIdentity generatorID,SimpleIdentity systemId)
: systemId(systemId)
{
    genId = generatorID;
}
    
void ParticleGeneratorRemSystemRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen)
{
    ParticleGenerator *theGen = (ParticleGenerator *)gen;
    theGen->removeParticleSystem(systemId);
}

}
