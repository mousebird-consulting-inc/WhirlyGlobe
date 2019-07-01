/*
 *  Drawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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
#import "Scene.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
		
DrawableTweaker::~DrawableTweaker()
{
}
    		
Drawable::Drawable(const std::string &name)
    : name(name)
{
}
	
Drawable::~Drawable()
{
}
    
void Drawable::runTweakers(RendererFrameInfo *frame)
{
    for (DrawableTweakerRefSet::iterator it = tweakers.begin();
         it != tweakers.end(); ++it)
        (*it)->tweakForFrame(this,frame);
}
	
void DrawableChangeRequest::execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view)
{
	DrawableRef theDrawable = scene->getDrawable(drawId);
	if (theDrawable)
		execute2(scene,renderer,theDrawable);
}
    
}
