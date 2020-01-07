/*
 *  Shader_Android.h
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

#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

/**
 * Android wrapper around a shader.
 */
class Shader_Android {
public:
    Shader_Android();
    virtual ~Shader_Android();

    // Do the actual program setup
    void setupProgram(const std::string &name,const std::string &vertProg,const std::string &fragProg);

    // Set one up that we've built on the C++ side
    void setupPreBuildProgram(ProgramGLES *prog);

    // Program after set up
    ProgramGLESRef prog;
    // Varyings if they exist.  Need to be passed in during creation.
    std::vector<std::string> varyings;
};

// TODO: Switch to storing Shader_AndroidRef
//       We have some dispose related problems


}
