/*
 *  Generator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/11/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "Generator.h"
#import "GlobeScene.h"

namespace WhirlyKit
{
    
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/Generator.cpp
void GeneratorChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
=======
void GeneratorChangeRequest::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/Generator.mm
{
    Generator *theGenerator = scene->getGenerator(genId);
	if (theGenerator)
		execute2(scene,renderer,theGenerator);    
}

}
