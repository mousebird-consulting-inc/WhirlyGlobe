/*
 *  MapboxVectorStyleSet_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/20.
 *  Copyright 2011-2020 mousebird consulting
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

#import "MapboxVectorStyleSet_Android.h"

namespace WhirlyKit
{

MapboxVectorStyleSetImpl_Android::MapboxVectorStyleSetImpl_Android(Scene *scene,VectorStyleSettingsImplRef settings)
: MapboxVectorStyleSetImpl(scene,settings)
{
}

MapboxVectorStyleSetImpl_Android::~MapboxVectorStyleSetImpl_Android()
{
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeCircleTexture(double radius,const RGBAColor &fillColor,const RGBAColor &strokeColor,float strokeWidth,Point2f *circleSize)
{
    // TODO: Implement
    return EmptyIdentity;
}

SimpleIdentity MapboxVectorStyleSetImpl_Android::makeLineTexture(const std::vector<double> &dashComponents)
{
    // TODO: Implement
    return EmptyIdentity;
}

LabelInfoRef MapboxVectorStyleSetImpl_Android::makeLabelInfo(const std::string &fontName)
{
    // TODO: Implement
    return LabelInfoRef();
}

SingleLabelRef MapboxVectorStyleSetImpl_Android::makeSingleLabel(const std::string &text)
{
    // TODO: Implement
    return SingleLabelRef();
}

ComponentObjectRef MapboxVectorStyleSetImpl_Android::makeComponentObject()
{
    // TODO: Implement
    return ComponentObjectRef();
}

}
