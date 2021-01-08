/*
 *  MaplyGeomModel_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/1/14.
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

#import "visual_objects/MaplyGeomModel.h"
#import <WhirlyGlobe_iOS.h>
#import "MaplyRenderController_private.h"

namespace WhirlyKit
{
class FontTextureManager_iOS;
    
// Wraps strings with size and translation
class GeomStringWrapper
{
public:
    GeomStringWrapper() : mat(mat.Identity()), str(nil) { }
    
    Eigen::Matrix4d mat;
    CGSize size;
    NSAttributedString *str;
};
}

@class MaplyBaseInteractionLayer;

@interface MaplyGeomModel()
{
@public
    std::vector<std::string> textures;
    std::vector<WhirlyKit::GeometryRaw> rawGeom;
    std::set<MaplyTexture *> maplyTextures;
    std::vector<WhirlyKit::GeomStringWrapper> strings;
}

// Return the list of texture file names
- (void)getTextureFileNames:(std::vector<std::string> &)texFileNames;

// Convert to raw geometry
- (void)asRawGeometry:(std::vector<WhirlyKit::GeometryRaw> &)rawGeom withTexMapping:(const std::vector<WhirlyKit::SimpleIdentity> &)texFileMap;

// Return the ID for or generate a base model in the Geometry Manager
- (WhirlyKit::SimpleIdentity)getBaseModel:(MaplyBaseInteractionLayer *)inLayer
                           fontTexManager:(const WhirlyKit::FontTextureManager_iOSRef &)fontTexManager
                                  compObj:(MaplyComponentObject *)compObj
                                     mode:(MaplyThreadMode)threadMode;

@end


