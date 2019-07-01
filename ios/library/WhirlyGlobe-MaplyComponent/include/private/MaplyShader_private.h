/*
 *  MaplyShader_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
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

#import "rendering/MaplyShader.h"
#import <WhirlyGlobe_iOS.h>

@interface MaplyShader()

@property (nonatomic,readonly) WhirlyKit::ProgramRef program;

/// Internal Shader ID used below the Component level
- (WhirlyKit::SimpleIdentity)getShaderID;

/// Initialize directly from an existing program
- (nullable instancetype)initWithProgram:(WhirlyKit::ProgramRef)program viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)baseViewC;

/// Called by the view controller to clear out the shader program
- (void)teardown;

@end
