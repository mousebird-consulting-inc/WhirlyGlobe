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
#import "Dictionary_NSDictionary.h"

@interface MaplyVectorStyleSettings()
{
@public
    WhirlyKit::VectorStyleSettingsImplRef impl;
}

@end

// Used to identify Vector Styles with an underlying C++ implementation
@protocol MaplyVectorStyleDelegateSecret <NSObject>

@optional
- (WhirlyKit::VectorStyleDelegateImplRef) getVectorStyleImpl;

@end

namespace WhirlyKit
{

// iOS version wants the view controller for some of the variants
class MapboxVectorStyleSetImpl_iOS : public MapboxVectorStyleSetImpl
{
public:
    MapboxVectorStyleSetImpl_iOS(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings);
    ~MapboxVectorStyleSetImpl_iOS();

    NSObject<MaplyRenderControllerProtocol> *viewC;
    
    /// Local platform implementation for generating a circle and adding it as a texture
    virtual SimpleIdentity makeCircleTexture(PlatformThreadInfo *inst,
                                             double radius,
                                             const RGBAColor &fillColor,
                                             const RGBAColor &strokeColor,
                                             float strokeWidth,Point2f *circleSize) override;
    
    /// Local platform implementation for generating a repeating line texture
    virtual SimpleIdentity makeLineTexture(PlatformThreadInfo *inst,const std::vector<double> &dashComponents) override;
    
    /// Make platform specific label info object (ideally we're caching these)
    virtual LabelInfoRef makeLabelInfo(PlatformThreadInfo *inst,const std::string &fontName,float fontSize) override;

    /// Create a local platform label (fonts are local, and other stuff)
    virtual SingleLabelRef makeSingleLabel(PlatformThreadInfo *inst,const std::string &text) override;
    
    /// Create a platform specific variant of the component object
    ComponentObjectRef makeComponentObject(PlatformThreadInfo *inst) override;

    /// Tie a selection ID to the given vector object
    void addSelectionObject(SimpleIdentity selectID,VectorObjectRef vecObj,ComponentObjectRef compObj) override;
        
    /// Return the width of the given line of text
    double calculateTextWidth(PlatformThreadInfo *threadInfo,LabelInfoRef labelInfo,const std::string &testStr) override;
    
    // Textures we're holding on to (so they don't get released)
    std::vector<MaplyTexture *> textures;
};
typedef std::shared_ptr<MapboxVectorStyleSetImpl_iOS> MapboxVectorStyleSetImpl_iOSRef;

/**
 Wrapper class for older implementations of MaplyVectorStyleDelegate or ones users have made.
 This way we can leave those in place while still doing our low level C++ implementation.
 */
class VectorStyleDelegateWrapper : public VectorStyleDelegateImpl
{
public:
    VectorStyleDelegateWrapper(NSObject<MaplyRenderControllerProtocol> *viewC,NSObject<MaplyVectorStyleDelegate> *delegate);
    
    virtual std::vector<VectorStyleImplRef> stylesForFeature(DictionaryRef attrs,
                                                            const QuadTreeIdentifier &tileID,
                                                            const std::string &layerName);
    
    virtual bool layerShouldDisplay(const std::string &name,
                                    const QuadTreeNew::Node &tileID);

    virtual VectorStyleImplRef styleForUUID(long long uuid);

    virtual std::vector<VectorStyleImplRef> allStyles();
    
    RGBAColorRef backgroundColor(double zoom);
    
protected:
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    NSObject<MaplyVectorStyleDelegate> *delegate;
};
typedef std::shared_ptr<VectorStyleDelegateWrapper> VectorStyleDelegateWrapperRef;

/**
 Wrapper class for existing vector styles implemented in Obj-C.
 This C++ version is presented to the low level code.
 */
class VectorStyleWrapper : public VectorStyleImpl
{
public:
    VectorStyleWrapper(NSObject<MaplyRenderControllerProtocol> *viewC,NSObject<MaplyVectorStyle> *style);
    
    virtual long long getUuid();
    virtual std::string getCategory();
    virtual bool geomAdditive();
    virtual void buildObjects(PlatformThreadInfo *inst,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo);
    
protected:
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    NSObject<MaplyVectorStyle> * __weak style;
};
typedef std::shared_ptr<VectorStyleWrapper> VectorStyleWrapperRef;

}
