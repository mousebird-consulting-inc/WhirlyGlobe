/*
 *  MaplyParticleSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/26/15.
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

#import <UIKit/UIKit.h>
#import "math/MaplyCoordinate.h"
#import "rendering/MaplyShader.h"
#import "rendering/MaplyRenderTarget.h"

typedef NS_ENUM(NSInteger, MaplyParticleSystemType) {
	MaplyParticleSystemTypePoint,
	MaplyParticleSystemTypeRectangle,
};

/** 
    A particle system is used to spawn large numbers of small moving objects.
    
    The particle system defines what the objects are and how they're controlled.  Actual data is handled through the MaplyParticleBatch.
    
    You set up a particle system and then add MaplyParticleBatches via a view controller.
  */
@interface MaplyParticleSystem : NSObject

/** 
    Name of the particle system.
    
    The particle system name is used for performance debugging.
  */
@property (nonatomic,strong) NSString * __nullable name;

/** 
    The type of the particle system.
    
    At present particle systems are just point geometry.
  */
@property (nonatomic,assign) MaplyParticleSystemType type;

/**
    Position shader for two stage particles.
 
    If there is a position shader then it is run first and particle data is then
    shared between this shader and the regular shader.
  */
@property (nonatomic,strong) MaplyShader * __nullable positionShader;

/**
    Shader to use for rendering particles.
 
    This can either be a single stage shader or it can be part of a two stage shader with
    the positionShader.
  */
@property (nonatomic,strong) MaplyShader * __nullable renderShader;

/** 
    Individual particle lifetime.
    
    The created particles will last only a certain amount of time.
  */
@property (nonatomic,assign) NSTimeInterval lifetime;

/** 
    The base that particle time is measured from.
    
    Individual particles will measure their own lifetime against this base value.
  */
@property (nonatomic,assign) NSTimeInterval baseTime;

/** 
    Total number of particles to be represented at once.
    
    This is the most particles we'll have on the screen at any time.  Space will be allocated for them, so don't overdo it.
  */
@property (nonatomic,assign) int totalParticles;

/** 
    Batch size for MaplyParticleBatch.
    
    Particles need to be created in large batches for efficiency.  This is the size of individual batches.
 
    Only for OpenGL ES.  Metal does particles as one big batch.
  */
@property (nonatomic,assign) int batchSize;

/**
   Metal only.
   Size of a single to be passed in to the calculation and rendering shaders.
  */
@property (nonatomic,assign) int vertexSize;

/** 
    Turn on/off the continuous rendering for particles.
    
    Normally particle systems force the renderer to draw every frame.  That's how the particles move.  You can turn that behavior off by setting this to false.
  */
@property (nonatomic,assign) bool continuousUpdate;

/** 
    Initialize a particle system with a name.
    
    The particle system needs the name for performance and debugging.  The rest of the values can left to their defaults.
  */
- (nonnull instancetype)initWithName:(NSString *__nonnull)name viewC:(NSObject <MaplyRenderControllerProtocol> * __nonnull)viewC;

/** 
    Add an attribute we'll be expecting in each batch.
    
    Adds an attribute name and type which will be present in each batch.
 
    OpenGL ES Only.  Metal handles things as blocks of memory.
  */
- (void)addAttribute:(NSString *__nonnull)attrName type:(MaplyShaderAttrType)type;

/**
    For two stage shaders, these are the varying outputs from one shader to the next.
 
    Two stage shaders run a position shader and then a regular render shader
    from the position output.  Add any varying values you want to share per
    vertex from the former to the latter.

    OpenGL ES Only.  Metal does this more simply.
  */
- (void)addVarying:(NSString *__nonnull)varyAttrName inputName:(NSString *__nonnull)inputName type:(MaplyShaderAttrType)type;

/** 
    Add a texture to the particle system.
    
    All the textures will be handed over to the shader.
  */
- (void)addTexture:(id __nonnull)image;

/**
    Draw these particles to the given render target.
 
    Rather than being drawn to the screen, these particles will be drawn to the offscreen render target.
  */
- (void)setRenderTarget:(MaplyRenderTarget * __nonnull)renderTarget;

@end


/** 
    A particle batch adds a set number of particles to the system.
    
    The particle batch holds the number of particles defined in the MaplyParticleSystem batchSize property.  Each attribute array is added individually via an NSData object.  All attributes must be present or the batch is invalid and won't be passed through the system.
  */
@interface MaplyParticleBatch : NSObject

/** 
    The particle system this batch belongs to.
  */
@property (nonatomic,weak) MaplyParticleSystem * __nullable partSys;

/** 
    The current time.
    
    This will be set by default.  However, you can control what the time basis for a particle batch is.
  */
@property (nonatomic,assign) NSTimeInterval time;

/** 
    Initialize with the particle system.
    
    The batch is initialized with its particle system.  You must then call addAttribute:values: repeatedly with attribute arrays.
  */
- (nonnull instancetype)initWithParticleSystem:(MaplyParticleSystem *__nonnull)partSys;

/**
    OpenGL ES only.
 
    Add an attribute array of the given name.
    
    Each attribute in the MaplyParticleSystem must be filled in here.  The name must correspond and the length of the data must match.
    
    @return Returns true if the attribute array was valid, false otherwise.
  */
- (bool) addAttribute:(NSString *__nonnull)attrName values:(NSData *__nonnull)data;

/**
   Metal only.
 
   We add the batch as a single blob of data.  Format is up to you and your shader.
 
  */
- (void) addData:(NSData * __nonnull)data;

/** 
    Tests if the batch is valid.
    
    This checks if all the attribute arrays are present and valid.
  */
- (bool) isValid;

@end
