/*
 *  ProgramMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "ProgramMTL.h"

namespace WhirlyKit
{
    
ProgramMTL::ProgramMTL() : lightsLastUpdated(0.0), valid(false)
{
}

ProgramMTL::~ProgramMTL()
{
}

ProgramMTL::ProgramMTL(SimpleIdentity theId) : Program(theId), valid(true), lightsLastUpdated(0.0)
{ }

bool ProgramMTL::isValid()
{
    return valid;
}

bool ProgramMTL::setUniform(StringIdentity nameID,float val)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,float val,int index)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,const Eigen::Vector2f &vec)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,const Eigen::Vector3f &vec)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(StringIdentity nameID,int val)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setUniform(const SingleVertexAttribute &attr)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setTexture(StringIdentity nameID,TextureBase *tex)
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::hasLights()
{
    // TODO: Implement
    
    return false;
}

bool ProgramMTL::setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, Material *mat, Eigen::Matrix4f &modelMat)
{
    // TODO: Implement
    
    return false;
}

const std::string &ProgramMTL::getName()
{ return name; }

id<MTLLibrary> ProgramMTL::getProgram()
{ return program; }

void ProgramMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
    // TODO: Implement
}

}
