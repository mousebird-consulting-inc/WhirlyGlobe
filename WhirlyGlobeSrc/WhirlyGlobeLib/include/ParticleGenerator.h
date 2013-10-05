/*
 *  ParticleGenerator.h
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "Generator.h"

namespace WhirlyKit
{
    
/** The Particle Generator handles creation and update for particle systems.
    You don't create them here, that's what the ParticleSystemLayer is for.
    This class just manages them and creates their associated Drawables at
    every frame.
  */
class ParticleGenerator : public Generator
{
public:
    /// Construct with the maximum number of particles we'll have at every frame.
    /// We won't exceed this.
    ParticleGenerator(int maxNumParticles);
    virtual ~ParticleGenerator();
    
    /// Generate the list of drawables per frame.  Called by the renderer.
    void generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo,std::vector<DrawableRef> &drawables,std::vector<DrawableRef> &screenDrawables);

    /// This class represents a single particle.
    class Particle
    {
    public:
        /// Location, which is updated every frame
        Point3f loc;
        /// Direction the particle is heading
        Eigen::Vector3f dir;
        /// Particle color
        RGBAColor color;
        /// Particle velocity
        float velocity;
        /// When this particle is done
        NSTimeInterval expiration;
    };

    /// Representation of a particle system.  This will be used
    ///  to generate a certain number of particles per frame.
    class ParticleSystem : public Identifiable
    {
    public:
        ParticleSystem() : Identifiable() { }
        ~ParticleSystem() { }
        
        /// Return a reasonable set of defaults
        static ParticleSystem makeDefault();

        /// Make a new randomized paticle
        Particle generateParticle();
        
        /// Starting location for particles
        Point3f loc;
        /// Axes for the particle system.  Used to orient local math.
        Eigen::Vector3f dirN,dirE,dirUp;
        /// Randomizable particle length
        float minLength,maxLength;
        /// Number of particles to generate per second, randomized
        int numPerSecMin,numPerSecMax;
        /// Randomizable particle lifetime in sections
        float minLifetime,maxLifetime;
        /// Range of the angle from the dirN to -dirN (180 total)
        float minPhi,maxPhi;
        /// Colors, random selection
        std::vector<RGBAColor> colors;
        /// Min and max visibility parameters
        float minVis,maxVis;
    };
    
    /// Add a particle system to the mix
    void addParticleSystem(ParticleSystem *particleSystem);

    /// Remove a particle system by ID
    void removeParticleSystem(SimpleIdentity systemId);
    
protected:
    // All times are offset from here
    NSTimeInterval startTime;
    // When we last updated
    NSTimeInterval lastUpdateTime;

    int maxNumParticles;  // All the particles we can have at once.  Ever.
    std::vector<Particle> particles;
    typedef std::set<ParticleSystem *,IdentifiableSorter> ParticleSystemSet;
    ParticleSystemSet particleSystems;
};
    
/// Add a particle system to the generator.
/// The particle system itself is passed through.
class ParticleGeneratorAddSystemRequest : public GeneratorChangeRequest
{
public:
    ParticleGeneratorAddSystemRequest(SimpleIdentity generatorID,ParticleGenerator::ParticleSystem *partSystem);
    ~ParticleGeneratorAddSystemRequest();

    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
    
protected:
    ParticleGenerator::ParticleSystem *system;
};

/// Remove a particle system from the generator by ID.    
class ParticleGeneratorRemSystemRequest : public GeneratorChangeRequest
{
public:
    ParticleGeneratorRemSystemRequest(SimpleIdentity generatorID,SimpleIdentity systemId);
    ~ParticleGeneratorRemSystemRequest() { }
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
    
protected:
    SimpleIdentity systemId;
};
    
}
