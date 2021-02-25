/*
 *  VectorStyleSet_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/10/20.
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

#import <jni.h>
#import "WhirlyGlobe.h"
#import <map>

namespace WhirlyKit
{

class VectorStyleSetWrapper_Android;
typedef std::shared_ptr<VectorStyleSetWrapper_Android> VectorStyleSetWrapper_AndroidRef;

// Representation of vector style.  Doesn't actually wrap the Java version.
class VectorStyleImpl_Android : public VectorStyleImpl {
public:
    VectorStyleImpl_Android() { };
    virtual ~VectorStyleImpl_Android() { };

    friend class VectorStyleSetWrapper_Android;

    /// Unique Identifier for this style
    virtual long long getUuid(PlatformThreadInfo *inst) override;

    /// Category used for sorting
    virtual std::string getCategory(PlatformThreadInfo *inst) override;

    // Note: This no longer really holds
    /// Set if this geometry is additive (e.g. sticks around) rather than replacement
    virtual bool geomAdditive(PlatformThreadInfo *inst) override;

    /// Construct objects related to this style based on the input data.
    virtual void buildObjects(PlatformThreadInfo *inst, std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo) override;

protected:
    SimpleIdentity uuid;  // ID of this style on the Java side
    VectorStyleSetWrapper_Android *styleSet;
};
typedef std::shared_ptr<VectorStyleImpl_Android> VectorStyleImpl_AndroidRef;

    // Android version of VectorTileData.  References a Java-side object
class VectorStyleSetWrapper_Android : public VectorStyleDelegateImpl {
public:
    friend class VectorStyleImpl_Android;

    VectorStyleSetWrapper_Android(PlatformThreadInfo *inst,
                                jobject obj,
                                const std::vector<SimpleIdentity> uuids,
                                const std::vector<std::string> categories,
                                const std::vector<bool> &geomAdditive);
    virtual ~VectorStyleSetWrapper_Android() { }

    /// Return the styles that apply to the given feature (attributes).
    virtual std::vector<VectorStyleImplRef> stylesForFeature(PlatformThreadInfo *inst,
                                                             const Dictionary &attrs,
                                                             const QuadTreeIdentifier &tileID,
                                                             const std::string &layerName) override;

    /// Return true if the given layer is meant to display for the given tile (zoom level)
    virtual bool layerShouldDisplay(PlatformThreadInfo *inst,
                                    const std::string &name,
                                    const QuadTreeNew::Node &tileID) override;

    /// Return the style associated with the given UUID.
    virtual VectorStyleImplRef styleForUUID(PlatformThreadInfo *inst,long long uuid) override;

    // Return a list of all the styles in no particular order.  Needed for categories and indexing
    virtual std::vector<VectorStyleImplRef> allStyles(PlatformThreadInfo *inst) override;

    // Return the style for the background, if any
    virtual VectorStyleImplRef backgroundStyle(PlatformThreadInfo *inst) const override;

    /// Return the background color for a given zoom level
    virtual RGBAColorRef backgroundColor(PlatformThreadInfo *inst,double zoom) override;

    /// Called by the dispose on the JNI side
    void shutdown(PlatformThreadInfo *inst);

public:
    void buildObjects(PlatformThreadInfo *inst,SimpleIdentity styleID,std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo);

    jobject wrapperObj;
    jmethodID layerShouldDisplayMethod;
    jmethodID stylesForFeatureMethod;
    jmethodID buildObjectsMethod;

    // Tracking info about the style
    class StyleEntry {
    public:
        std::string category;
        bool geomAdditive;
        VectorStyleImpl_AndroidRef style;
    };

    // Map from UUIDs to styles
    std::map<SimpleIdentity,StyleEntry> styles;
};

}
