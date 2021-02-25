/*
 *  Shader_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/19.
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

#import "Shader_Android.h"

namespace WhirlyKit
{

Shader_Android::Shader_Android()
: prog(NULL)
{
}

Shader_Android::~Shader_Android()
{
    prog = NULL;
}

void Shader_Android::setupProgram(const std::string &name,const std::string &vertProg,const std::string &fragProg)
{
    if (varyings.empty())
    	prog = ProgramGLESRef(new ProgramGLES(name,vertProg,fragProg));
    else
        prog = ProgramGLESRef(new ProgramGLES(name,vertProg,fragProg,&varyings));
}

void Shader_Android::setupPreBuildProgram(ProgramGLESRef inProg)
{
    prog = inProg;
}

}
