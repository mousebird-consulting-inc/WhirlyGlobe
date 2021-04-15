/*  MaplyVectorStyle_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2021 mousebird consulting
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

// This wraps C++ style implementations to be called from ObjC
@interface MaplyVectorStyleReverseWrapper: NSObject<MaplyVectorStyle>

/// The C++ style we're wrapping
- (id __nonnull)initWithCStyle:(WhirlyKit::VectorStyleImplRef)vectorStyle;

/// Unique Identifier for this style
- (long long) uuid;

/// Category used for sorting
- (NSString * _Nullable) getCategory;

/// Set if this geometry is additive (e.g. sticks around) rather than replacement
- (bool) geomAdditive;

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc;

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc
            cancelFn:(bool(^__nullable)(void))cancelFn;

@end

namespace WhirlyKit
{

// iOS version wants the view controller for some of the variants
class MapboxVectorStyleSetImpl_iOS : public MapboxVectorStyleSetImpl
{
public:
    MapboxVectorStyleSetImpl_iOS(Scene *_Nonnull scene,
                                 CoordSystem *_Nonnull coordSys,
                                 const VectorStyleSettingsImplRef &settings);
    ~MapboxVectorStyleSetImpl_iOS();

    NSObject<MaplyRenderControllerProtocol> *_Nullable __weak viewC;
    
    /// Parse the style set
    virtual bool parse(PlatformThreadInfo *_Nullable inst,
                       const DictionaryRef &dict) override;
    
    /// Local platform implementation for generating a circle and adding it as a texture
    virtual SimpleIdentity makeCircleTexture(PlatformThreadInfo *_Nullable inst,
                                             double radius,
                                             const RGBAColor &fillColor,
                                             const RGBAColor &strokeColor,
                                             float strokeWidth,
                                             Point2f *_Nullable circleSize) override;
    
    /// Local platform implementation for generating a repeating line texture
    virtual SimpleIdentity makeLineTexture(PlatformThreadInfo *_Nullable inst,
                                           const std::vector<double> &dashComponents) override;
    
    /// Make platform specific label info object (ideally we're caching these)
    virtual LabelInfoRef makeLabelInfo(PlatformThreadInfo *_Nullable inst,
                                       const std::vector<std::string> &fontName,
                                       float fontSize) override;

    /// Create a local platform label (fonts are local, and other stuff)
    virtual SingleLabelRef makeSingleLabel(PlatformThreadInfo *_Nullable inst,
                                           const std::string &text) override;
    
    /// Create a platform specific variant of the component object
    ComponentObjectRef makeComponentObject(PlatformThreadInfo *_Nullable inst, const Dictionary *_Nullable desc) override;

    /// Tie a selection ID to the given vector object
    void addSelectionObject(SimpleIdentity selectID,const VectorObjectRef &vecObj,const ComponentObjectRef &compObj) override;
        
    /// Return the width of the given line of text
    double calculateTextWidth(PlatformThreadInfo *_Nullable threadInfo,
                              const LabelInfoRef &labelInfo,
                              const std::string &testStr) override;
    
    /// Add a sprite sheet
    void addSprites(MapboxVectorStyleSpritesRef newSprites,MaplyTexture *_Nonnull tex);
    
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
    VectorStyleDelegateWrapper(NSObject<MaplyRenderControllerProtocol> *_Nonnull viewC,
                               NSObject<MaplyVectorStyleDelegate> *_Nullable delegate);

    virtual std::vector<VectorStyleImplRef> stylesForFeature(PlatformThreadInfo *_Nullable inst,
                                                             const Dictionary &attrs,
                                                             const QuadTreeIdentifier &tileID,
                                                             const std::string &layerName) override;

    virtual bool layerShouldDisplay(PlatformThreadInfo *_Nullable inst,
                                    const std::string &name,
                                    const QuadTreeNew::Node &tileID) override;

    virtual VectorStyleImplRef styleForUUID(PlatformThreadInfo *_Nullable inst,long long uuid) override;

    virtual std::vector<VectorStyleImplRef> allStyles(PlatformThreadInfo *_Nullable inst) override;

    virtual VectorStyleImplRef backgroundStyle(PlatformThreadInfo *_Nullable inst) const override;

    virtual RGBAColorRef backgroundColor(PlatformThreadInfo *_Nullable inst,double zoom) override;

protected:
    NSObject<MaplyRenderControllerProtocol> *_Nullable  __weak viewC;
    NSObject<MaplyVectorStyleDelegate> *_Nullable delegate;
};
typedef std::shared_ptr<VectorStyleDelegateWrapper> VectorStyleDelegateWrapperRef;

/**
 Wrapper class for existing vector styles implemented in Obj-C.
 This C++ version is presented to the low level code.
 */
class VectorStyleWrapper : public VectorStyleImpl
{
public:
    VectorStyleWrapper(NSObject<MaplyRenderControllerProtocol> *_Nonnull viewC,
                       NSObject<MaplyVectorStyle> *_Nonnull style);

    virtual long long getUuid(PlatformThreadInfo *_Nullable inst) override;
    virtual std::string getCategory(PlatformThreadInfo *_Nullable inst) override;
    virtual bool geomAdditive(PlatformThreadInfo *_Nullable inst) override;
    virtual void buildObjects(PlatformThreadInfo *_Nullable inst,
                              const std::vector<VectorObjectRef> &vecObjs,
                              const VectorTileDataRef &tileData,
                              const Dictionary *_Nullable desc,
                              const CancelFunction &) override;

protected:
    NSObject<MaplyRenderControllerProtocol> *_Nullable  __weak viewC;
    NSObject<MaplyVectorStyle> *_Nullable  __weak style;
};
typedef std::shared_ptr<VectorStyleWrapper> VectorStyleWrapperRef;

}
