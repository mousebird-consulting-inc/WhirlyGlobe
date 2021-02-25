/*
 *  VectorStyleSet_Android.cpp
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

#include "../include/VectorStyleSet_Android.h"
#import "Maply_jni.h"
#import "Formats_jni.h"
#import "Vectors_jni.h"

namespace WhirlyKit
{

long long VectorStyleImpl_Android::getUuid(PlatformThreadInfo *inst)
{
    return uuid;
}

std::string VectorStyleImpl_Android::getCategory(PlatformThreadInfo *inst)
{
    auto entry = styleSet->styles.find(uuid);
    if (entry == styleSet->styles.end())
        return "";
    return entry->second.category;
}

bool VectorStyleImpl_Android::geomAdditive(PlatformThreadInfo *inst)
{
    auto entry = styleSet->styles.find(uuid);
    if (entry == styleSet->styles.end())
        return false;
    return entry->second.geomAdditive;
}

void VectorStyleImpl_Android::buildObjects(PlatformThreadInfo *inst, std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo)
{
    // Pass through to the parent object, since we're just a wrapper
    styleSet->buildObjects(inst,uuid,vecObjs,tileInfo);
}

VectorStyleSetWrapper_Android::VectorStyleSetWrapper_Android(PlatformThreadInfo *platformInfo,
                                                             jobject obj,
                                                             const std::vector<SimpleIdentity> uuids,
                                                             const std::vector<std::string> categories,
                                                             const std::vector<bool> &geomAdditive
                                                             )
{
    PlatformInfo_Android *info = (PlatformInfo_Android *)platformInfo;
    jclass thisClass = VectorStyleSetWrapperClassInfo::getClassInfo()->getClass();
    wrapperObj = info->env->NewGlobalRef(obj);
    layerShouldDisplayMethod = info->env->GetMethodID(thisClass,"layerShouldDisplay","(Ljava/lang/String;III)Z");
    stylesForFeatureMethod = info->env->GetMethodID(thisClass,"stylesForFeature","(Lcom/mousebird/maply/AttrDictionary;IIILjava/lang/String;)[J");
    buildObjectsMethod = info->env->GetMethodID(thisClass,"buildObjects","(J[Lcom/mousebird/maply/VectorObject;Lcom/mousebird/maply/VectorTileData;)V");

    for (unsigned int ii=0;ii<uuids.size();ii++) {
        StyleEntry entry;
        entry.category = categories[ii];
        entry.geomAdditive = geomAdditive[ii];
        entry.style = VectorStyleImpl_AndroidRef(new VectorStyleImpl_Android());
        entry.style->styleSet = this;
        auto uuid = uuids[ii];
        entry.style->uuid = uuid;
        styles[uuid] = entry;
    }
}

std::vector<VectorStyleImplRef> VectorStyleSetWrapper_Android::stylesForFeature(PlatformThreadInfo *platformInfo,
                                                         const Dictionary &attrs,
                                                         const QuadTreeIdentifier &tileID,
                                                         const std::string &layerName)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)platformInfo;

    // TODO: This is making a copy.  See if we can avoid that
    MutableDictionary_AndroidRef dict = std::make_shared<MutableDictionary_Android>(attrs);

    // Wrap the layer name and attributes and call the method
    jobject jStr = threadInfo->env->NewStringUTF(layerName.c_str());
    jobject attrObj = MakeAttrDictionary(threadInfo->env,dict);
    jlongArray longArray = (jlongArray)threadInfo->env->CallObjectMethod(wrapperObj,stylesForFeatureMethod,attrObj,tileID.x,tileID.y,tileID.level,jStr);
    threadInfo->env->DeleteLocalRef(jStr);
    threadInfo->env->DeleteLocalRef(attrObj);

    // Turn the resulting IDs into a list of styles we can return
    std::vector<VectorStyleImplRef> retStyles;
    if (longArray) {
        std::set<SimpleIdentity> uuids;
        ConvertLongArrayToSet(threadInfo->env,longArray,uuids);
        threadInfo->env->DeleteLocalRef(longArray);

        for (auto uuid: uuids) {
            auto entry = styles.find(uuid);
            if (entry == styles.end()) {
                wkLogLevel(Warn,"Failed to find style for UUID in VectorStyleSet_Android.  Features will dissappear.");
                continue;
            }
            retStyles.push_back(entry->second.style);
        }
    }

    return retStyles;
}

bool VectorStyleSetWrapper_Android::layerShouldDisplay(PlatformThreadInfo *platformInfo,
                                const std::string &name,
                                const QuadTreeNew::Node &tileID)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)platformInfo;

    jobject jStr = threadInfo->env->NewStringUTF(name.c_str());
    bool val = threadInfo->env->CallBooleanMethod(wrapperObj,layerShouldDisplayMethod,jStr,tileID.x,tileID.y,tileID.level);
    threadInfo->env->DeleteLocalRef(jStr);

    return val;
}

VectorStyleImplRef VectorStyleSetWrapper_Android::styleForUUID(PlatformThreadInfo *inst,long long uuid)
{
    auto entry = styles.find(uuid);
    if (entry == styles.end())
        return VectorStyleImplRef();

    return entry->second.style;
}

std::vector<VectorStyleImplRef> VectorStyleSetWrapper_Android::allStyles(PlatformThreadInfo *inst)
{
    std::vector<VectorStyleImplRef> retStyles;
    for (auto style: styles)
        retStyles.push_back(style.second.style);

    return retStyles;
}

VectorStyleImplRef VectorStyleSetWrapper_Android::backgroundStyle(PlatformThreadInfo *inst) const
{
    // TODO
    return VectorStyleImplRef();
}

RGBAColorRef VectorStyleSetWrapper_Android::backgroundColor(PlatformThreadInfo *inst,double zoom)
{
    return RGBAColorRef(new RGBAColor(0,0,0,0));
}

// Break the vectors up into groups of this size before calling back to Java
// This fixes a problem on older Android devices.
static const int VecBatchSize = 500;

void VectorStyleSetWrapper_Android::buildObjects(PlatformThreadInfo *platformInfo,
        SimpleIdentity styleID,
        std::vector<VectorObjectRef> &vecObjs,
        VectorTileDataRef tileInfo)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)platformInfo;

    jobject tileDataObj = MakeVectorTileDataObject(threadInfo->env,tileInfo);

    for (unsigned int ii=0;ii<vecObjs.size();ii+=VecBatchSize) {
        // Make wrapper objects for the vectors and the tile data
        std::vector<jobject> vecObjVec;
        for (unsigned int jj=0;jj<VecBatchSize;jj++) {
            if (jj+ii >= vecObjs.size())
                break;
            auto vecObj = vecObjs[ii+jj];
            jobject newObj = MakeVectorObject(threadInfo->env,vecObj);
            vecObjVec.push_back(newObj);
        }
        jobject vecObjArray = BuildObjectArray(threadInfo->env,VectorObjectClassInfo::getClassInfo()->getClass(),vecObjVec);
        // Tear down our object wrappers
        for (auto vecObj: vecObjVec)
            threadInfo->env->DeleteLocalRef(vecObj);

        threadInfo->env->CallVoidMethod(wrapperObj,buildObjectsMethod,styleID,vecObjArray,tileDataObj);
        threadInfo->env->DeleteLocalRef(vecObjArray);
    }

    threadInfo->env->DeleteLocalRef(tileDataObj);
}

void VectorStyleSetWrapper_Android::shutdown(PlatformThreadInfo *platformInfo)
{
    PlatformInfo_Android *info = (PlatformInfo_Android *)platformInfo;

    if (wrapperObj) {
        info->env->DeleteGlobalRef(wrapperObj);
        wrapperObj = NULL;
    }

    styles.clear();
}

}
