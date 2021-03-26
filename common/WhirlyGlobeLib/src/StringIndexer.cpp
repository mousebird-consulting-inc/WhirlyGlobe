/*  StringIndexer.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/18.
 *  Copyright 2011-2021 Saildrone Inc
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

#import "StringIndexer.h"
#import "SceneRenderer.h"
#import "Identifiable.h"

namespace WhirlyKit {

StringIndexer StringIndexer::instance;

StringIndexer::StringIndexer() : stringToIdent(500)
{
    identToString.reserve(500);
}

StringIdentity StringIndexer::getStringID(const std::string &str)
{
    StringIndexer &index = getInstance();

    // todo: upgradeable shared mutex would be perfect here
    std::lock_guard<std::mutex> lock(index.mutex);

    const auto it = index.stringToIdent.find(str);
    if (it != index.stringToIdent.end())
        return it->second;

    const int strID = index.identToString.size();
    index.identToString.push_back(str);
    index.stringToIdent[str] = strID;
    return strID;
}

std::string StringIndexer::getString(StringIdentity strID)
{
    const StringIndexer &index = getInstance();

    std::lock_guard<std::mutex> lock(index.mutex);

    return (strID < index.identToString.size()) ? index.identToString[strID] : std::string();
}
 
// Note: This is from OpenGL.  Doesn't hold anymore on iOS
#define WhirlyKitMaxTextures 8

// Shared global string IDs speed things up a lot
StringIdentity baseMapNameIDs[WhirlyKitMaxTextures];
StringIdentity hasBaseMapNameIDs[WhirlyKitMaxTextures];
StringIdentity texOffsetNameIDs[WhirlyKitMaxTextures];
StringIdentity texScaleNameIDs[WhirlyKitMaxTextures];
StringIdentity lightViewDependNameIDs[8];
StringIdentity lightDirectionNameIDs[8];
StringIdentity lightHalfplaneNameIDs[8];
StringIdentity lightAmbientNameIDs[8];
StringIdentity lightDiffuseNameIDs[8];
StringIdentity lightSpecularNameIDs[8];
StringIdentity u_numLightsNameID;
StringIdentity materialAmbientNameID;
StringIdentity materialDiffuseNameID;
StringIdentity materialSpecularNameID;
StringIdentity materialSpecularExponentNameID;
StringIdentity mvpMatrixNameID;
StringIdentity mvpInvMatrixNameID;
StringIdentity mvMatrixNameID;
StringIdentity mvNormalMatrixNameID;
StringIdentity mvpNormalMatrixNameID;
StringIdentity u_FadeNameID;
StringIdentity u_pMatrixNameID;
StringIdentity u_ScaleNameID;
StringIdentity u_HasTextureNameID;
StringIdentity a_SingleMatrixNameID;
StringIdentity a_PositionNameID;
StringIdentity u_EyeVecNameID;
StringIdentity u_EyePosNameID;
StringIdentity u_SizeNameID;
StringIdentity u_TimeNameID;
StringIdentity u_lifetimeNameID;
StringIdentity u_pixDispSizeNameID;
StringIdentity u_frameLenID;
StringIdentity a_offsetNameID;
StringIdentity u_uprightNameID;
StringIdentity u_activerotNameID;
StringIdentity a_rotNameID;
StringIdentity a_dirNameID;
StringIdentity a_maskNameID;
StringIdentity a_maskNameIDs[WhirlyKitMaxMasks];
StringIdentity a_texCoordNameID;
StringIdentity u_w2NameID;
StringIdentity u_Realw2NameID;
StringIdentity u_EdgeNameID;
StringIdentity u_texScaleNameID;
StringIdentity u_colorNameID;
StringIdentity u_lengthNameID;
StringIdentity u_interpNameID;
StringIdentity u_screenOriginNameID;
StringIdentity a_colorNameID;
StringIdentity a_normalNameID;
StringIdentity a_modelCenterNameID;
StringIdentity a_useInstanceColorNameID;
StringIdentity a_instanceColorNameID;
StringIdentity a_modelDirNameID;

// Turn the string names into IDs, but just once
static void SetupDrawableStringsOnce()
{
    for (unsigned int ii=0;ii<8;ii++) {
        std::string baseMapName = "s_baseMap" + std::to_string(ii);
        baseMapNameIDs[ii] = StringIndexer::getStringID(baseMapName);
        std::string hasBaseMapName = "u_has_baseMap" + std::to_string(ii);
        hasBaseMapNameIDs[ii] = StringIndexer::getStringID(hasBaseMapName);
        std::string texOffsetName = "u_texOffset" + std::to_string(ii);
        texOffsetNameIDs[ii] = StringIndexer::getStringID(texOffsetName);
        std::string texScaleName = "u_texScale" + std::to_string(ii);
        texScaleNameIDs[ii] = StringIndexer::getStringID(texScaleName);
    }
    char name[200];
    for (unsigned int index=0;index<8;index++) {
        sprintf(name,"light[%d].viewdepend",index);
        lightViewDependNameIDs[index] = StringIndexer::getStringID(name);
        sprintf(name,"light[%d].direction",index);
        lightDirectionNameIDs[index] = StringIndexer::getStringID(name);
        sprintf(name,"light[%d].halfplane",index);
        lightHalfplaneNameIDs[index] = StringIndexer::getStringID(name);
        sprintf(name,"light[%d].ambient",index);
        lightAmbientNameIDs[index] = StringIndexer::getStringID(name);
        sprintf(name,"light[%d].diffuse",index);
        lightDiffuseNameIDs[index] = StringIndexer::getStringID(name);
        sprintf(name,"light[%d].specular",index);
        lightSpecularNameIDs[index] = StringIndexer::getStringID(name);
    }
    u_numLightsNameID = StringIndexer::getStringID("u_numLights");
    materialAmbientNameID = StringIndexer::getStringID("material.ambient");
    materialDiffuseNameID = StringIndexer::getStringID("material.diffuse");
    materialSpecularNameID = StringIndexer::getStringID("material.specular");
    materialSpecularExponentNameID = StringIndexer::getStringID("material.specular_exponent");

    mvpMatrixNameID = StringIndexer::getStringID("u_mvpMatrix");
    mvpInvMatrixNameID = StringIndexer::getStringID("u_mvpInvMatrix");
    mvMatrixNameID = StringIndexer::getStringID("u_mvMatrix");
    mvNormalMatrixNameID = StringIndexer::getStringID("u_mvNormalMatrix");
    mvpNormalMatrixNameID = StringIndexer::getStringID("u_mvpNormalMatrix");
    u_FadeNameID = StringIndexer::getStringID("u_fade");
    u_pMatrixNameID = StringIndexer::getStringID("u_pMatrix");
    u_ScaleNameID = StringIndexer::getStringID("u_scale");
    u_HasTextureNameID = StringIndexer::getStringID("u_hasTexture");
    a_SingleMatrixNameID = StringIndexer::getStringID("a_singleMatrix");
    a_PositionNameID = StringIndexer::getStringID("a_position");
    u_EyeVecNameID = StringIndexer::getStringID("u_eyeVec");
    u_EyePosNameID = StringIndexer::getStringID("u_eyePos");
    u_SizeNameID = StringIndexer::getStringID("u_size");
    u_TimeNameID = StringIndexer::getStringID("u_time");
    u_lifetimeNameID = StringIndexer::getStringID("u_lifetime");
    u_pixDispSizeNameID = StringIndexer::getStringID("u_pixDispSize");
    u_frameLenID = StringIndexer::getStringID("u_frameLen");
    a_offsetNameID = StringIndexer::getStringID("a_offset");
    u_uprightNameID = StringIndexer::getStringID("u_upright");
    u_activerotNameID = StringIndexer::getStringID("u_activerot");
    a_rotNameID = StringIndexer::getStringID("a_rot");
    a_dirNameID = StringIndexer::getStringID("a_dir");
    a_maskNameID = StringIndexer::getStringID("a_maskID");
    for (unsigned int index=0;index<WhirlyKitMaxMasks;index++) {
        sprintf(name,"a_maskID%d",index);
        a_maskNameIDs[index] = StringIndexer::getStringID(name);
    }
    a_texCoordNameID = StringIndexer::getStringID("a_texCoord");
    u_w2NameID = StringIndexer::getStringID("u_w2");
    u_Realw2NameID = StringIndexer::getStringID("u_real_w2");
    u_EdgeNameID = StringIndexer::getStringID("u_edge");
    u_texScaleNameID = StringIndexer::getStringID("u_texScale");
    u_colorNameID = StringIndexer::getStringID("u_color");
    u_lengthNameID = StringIndexer::getStringID("u_length");
    u_interpNameID = StringIndexer::getStringID("u_interp");
    u_screenOriginNameID = StringIndexer::getStringID("u_screenOrigin");
    a_colorNameID = StringIndexer::getStringID("a_color");
    a_normalNameID = StringIndexer::getStringID("a_normal");
    a_modelCenterNameID = StringIndexer::getStringID("a_modelCenter");
    a_useInstanceColorNameID = StringIndexer::getStringID("a_useInstanceColor");
    a_instanceColorNameID = StringIndexer::getStringID("a_instanceColor");
    a_modelDirNameID = StringIndexer::getStringID("a_modelDir");
}
static std::once_flag stringsSetup;
void SetupDrawableStrings()
{
    std::call_once(stringsSetup, SetupDrawableStringsOnce);
}

}
