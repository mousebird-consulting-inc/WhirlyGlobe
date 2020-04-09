/*
*  MaplyVectorStyle_private.h
*  WhirlyGlobe-MaplyComponent
*
*  Created by Steve Gifford on 1/3/14.
*  Copyright 2011-2017 mousebird consulting
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

#import "MaplyVectorStyle.h"
#import "WhirlyGlobe.h"

@interface MaplyVectorStyleSettings()
{
    WhirlyKit::VectorStyleSettingsImplRef impl;
}

@end

namespace WhirlyKit
{

/**
 Wrapper class for older implementations of MaplyVectorStyleDelegate or ones users have made.
 This way we can leave those in place while still doing our low level C++ implementation.
 */
class VectorStyleDelegateWrapper : VectorStyleDelegateImpl
{
public:
    VectorStyleDelegateWrapper(NSObject<MaplyVectorStyleDelegate> *delegate);
    
    virtual std::vector<VectorStyleImplRef> stylesForFeature(VectorStyleImplRef styleDelegate,
                                                                  const Dictionary &attrs,
                                                                  const QuadTreeNew::Node &tileID,
                                                                  const std::string &layerName);
    
    virtual bool layerShouldDisplay(VectorStyleImplRef styleDelegate,
                                    const std::string &name,
                                    const QuadTreeNew::Node &tileID);

    virtual VectorStyleImplRef styleForUUID(VectorStyleImplRef styleDelegate,long long uuid);

    virtual std::vector<VectorStyleImplRef> allStyles();
};

typedef std::shared_ptr<VectorStyleDelegateWrapper> VectorStyleDelegateWrapperRef;

/**
 Wrapper class for existing vector styles implemented in Obj-C.
 This C++ version is presented to the low level code.
 */
class VectorStyleWrapper : VectorStyleImpl
{
public:
    VectorStyleWrapper(NSObject<MaplyVectorStyle> *style);
    
    virtual long long getUuid() = 0;
    
    virtual const std::string &getCategory() = 0;
    
    virtual bool geomAdditive();

    virtual void buildObject(std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo,MapboxVectorStyleSetImplRef impl);
};

}
