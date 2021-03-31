/*
 *  StringIndexer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/18.
 *  Copyright 2011-2019 Saildrone Inc
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

#import <vector>
#import <unordered_map>
#import <string>
#import <mutex>

namespace WhirlyKit
{

// Unique identifier for strings
typedef unsigned long StringIdentity;

// Set up the global strings IDs the drawables use for their shaders
// Speeds things up on the main thread immensely.
extern void SetupDrawableStrings();

// Number of entries for masks used in the mask target
#define WhirlyKitMaxMasks 2

// Names used for indexing into shaders
extern StringIdentity baseMapNameIDs[];
extern StringIdentity hasBaseMapNameIDs[];
extern StringIdentity texOffsetNameIDs[];
extern StringIdentity texScaleNameIDs[];
extern StringIdentity lightViewDependNameIDs[];
extern StringIdentity lightDirectionNameIDs[];
extern StringIdentity lightHalfplaneNameIDs[];
extern StringIdentity lightAmbientNameIDs[];
extern StringIdentity lightDiffuseNameIDs[];
extern StringIdentity lightSpecularNameIDs[];
extern StringIdentity u_numLightsNameID;
extern StringIdentity materialAmbientNameID;
extern StringIdentity materialDiffuseNameID;
extern StringIdentity materialSpecularNameID;
extern StringIdentity materialSpecularExponentNameID;
extern StringIdentity mvpMatrixNameID;
extern StringIdentity mvpInvMatrixNameID;
extern StringIdentity mvMatrixNameID;
extern StringIdentity mvNormalMatrixNameID;
extern StringIdentity mvpNormalMatrixNameID;
extern StringIdentity u_FadeNameID;
extern StringIdentity u_pMatrixNameID;
extern StringIdentity u_ScaleNameID;
extern StringIdentity u_HasTextureNameID;
extern StringIdentity a_SingleMatrixNameID;
extern StringIdentity a_PositionNameID;
extern StringIdentity u_EyeVecNameID;
extern StringIdentity u_EyePosNameID;
extern StringIdentity u_SizeNameID;
extern StringIdentity u_TimeNameID;
extern StringIdentity u_lifetimeNameID;
extern StringIdentity u_pixDispSizeNameID;
extern StringIdentity u_frameLenID;
extern StringIdentity a_offsetNameID;
extern StringIdentity u_uprightNameID;
extern StringIdentity u_activerotNameID;
extern StringIdentity a_rotNameID;
extern StringIdentity a_dirNameID;
extern StringIdentity a_maskNameID;
extern StringIdentity a_maskNameIDs[];
extern StringIdentity a_texCoordNameID;
extern StringIdentity u_w2NameID;
extern StringIdentity u_Realw2NameID;
extern StringIdentity u_EdgeNameID;
extern StringIdentity u_texScaleNameID;
extern StringIdentity u_colorNameID;
extern StringIdentity u_lengthNameID;
extern StringIdentity u_interpNameID;
extern StringIdentity u_screenOriginNameID;
extern StringIdentity a_colorNameID;
extern StringIdentity a_normalNameID;
extern StringIdentity a_modelCenterNameID;
extern StringIdentity a_useInstanceColorNameID;
extern StringIdentity a_instanceColorNameID;
extern StringIdentity a_modelDirNameID;

/** Globally indexes strings by ID.  This lets us use an ID rather
 than a string in certain high performance unordered maps and such.
 
 Only adds strings.  Never removes them.
 */
class StringIndexer
{
public:
    // Return or make up a string identity
    static StringIdentity getStringID(const std::string &);
    
    // Return the string for a string identity
    static std::string getString(StringIdentity);
    
protected:
    StringIndexer();
    StringIndexer(StringIndexer const&)     = delete;
    void operator=(StringIndexer const&)    = delete;

    static StringIndexer &getInstance() { return instance; }
    
    mutable std::mutex mutex;
    std::unordered_map<std::string,StringIdentity> stringToIdent;
    std::vector<std::string> identToString;

private:
    static StringIndexer instance;
};

}
