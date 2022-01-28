/*
 *  ParticleSystemDrawableBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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
 *
 */

#import "WhirlyVector.h"
#import "ParticleSystemDrawable.h"

namespace WhirlyKit
{
    
/** Used to construct Particle System Drawables.
  */
class ParticleSystemDrawableBuilder
{
public:
    ParticleSystemDrawableBuilder(const std::string &name,Scene *scene);
    virtual ~ParticleSystemDrawableBuilder();
    
    // Set up our various attributes at once
    virtual void setup(const std::vector<SingleVertexAttributeInfo> &inVertAttrs,
                       const std::vector<SingleVertexAttributeInfo> &inVaryAttrs,
                       const std::vector<SimpleIdentity> &inVaryNames,
                       int numTotalPoints,
                       int batchSize,
                       int vertexSize,
                       bool useRectangles,
                       bool useInstancing);

    // Return the drawable being constructed
    virtual ParticleSystemDrawable *getDrawable() = 0;

protected:
    Scene *scene;
    std::string name;
    ParticleSystemDrawable *draw;
};
    
typedef std::shared_ptr<ParticleSystemDrawableBuilder> ParticleSystemDrawableBuilderRef;
    
}
