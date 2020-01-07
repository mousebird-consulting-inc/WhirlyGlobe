/*
*  DrawableGLES.h
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 9/30/19.
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

#import "Drawable.h"
#import "SceneRendererGLES.h"

namespace WhirlyKit
{

/**
    Drawable for GLES just does the drawing.
 */
class DrawableGLES : virtual public Drawable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        
    virtual ~DrawableGLES();
    
    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void calculate(RendererFrameInfoGLES *frameInfo,Scene *scene) = 0;

    /// Set up what you need in the way of context and draw.
    virtual void draw(RendererFrameInfoGLES *frameInfo,Scene *scene) = 0;
};
typedef std::shared_ptr<DrawableGLES> DrawableGLESRef;

}
