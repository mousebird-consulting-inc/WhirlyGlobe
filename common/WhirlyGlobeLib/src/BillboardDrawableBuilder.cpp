/*
 *  BillboardDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/13.
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

#import "BillboardDrawableBuilder.h"
#import "Program.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{

void BillboardDrawableBuilder::Init()
{
    BasicDrawableBuilder::Init();
    setupStandardAttributes(0);
    offsetIndex = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_offset"));
    groundMode = false;
}
    
void BillboardDrawableBuilder::setGroundMode(bool newVal)
{
    groundMode = newVal;
}
    
void BillboardDrawableBuilder::addOffset(const Point3f &offset)
{
    addAttributeValue(offsetIndex, offset);
}

void BillboardDrawableBuilder::addOffset(const Point3d &offset)
{
    addAttributeValue(offsetIndex, Point3f(offset.x(),offset.y(),offset.z()));
}

}
