/*
 *  MaplyParticleSystem_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/26/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import <set>
#import "MaplyParticleSystem.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{
 
class ParticleSystemAttribute : public Identifiable
{
public:
    NSString *name;
    MaplyShaderAttrType type;
    
    // Size of a single data value
    int dataSize();
};

typedef std::set<ParticleSystemAttribute> ParticleSystemAttributeSet;

class ParticleSystemAttrVals
{
public:
    SimpleIdentity attrID;
    NSData *data;
};
}

@interface MaplyParticleSystem()

@property (nonatomic,assign) WhirlyKit::ParticleSystemAttributeSet &attrs;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity ident;
@property (nonatomic,assign) std::vector<id> &images;

@end

@interface MaplyParticleBatch()

@property (nonatomic,assign) std::vector<WhirlyKit::ParticleSystemAttrVals> &attrVals;

@end
