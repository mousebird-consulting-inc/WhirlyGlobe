/*  MaplyGeomModel.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 11/26/14.
 *  Copyright 2011-2023 mousebird consulting
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

#import <set>
#import "MaplyGeomModel_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyShape_private.h"
#import "MaplyComponentObject_private.h"
#import "FontTextureManager_iOS.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation MaplyGeomModel
{
    MaplyShape *shape;
    __weak MaplyBaseInteractionLayer *layer;
    WhirlyKit::SimpleIdentity baseModelID;    
}

- (instancetype)initWithObj:(NSString *)fullPath
{
    self = [super init];
    
    const char *str = [fullPath cStringUsingEncoding:NSASCIIStringEncoding];
    FILE *fp = str ? fopen(str, "r") : nullptr;
    if (!fp)
        return nil;
    
    // Parse it out of the file
    GeometryModelOBJ objModel;
    NSString *bundlePath = [[NSBundle mainBundle] resourcePath];
    if (auto cstr = [bundlePath cStringUsingEncoding:NSASCIIStringEncoding])
    {
        objModel.setResourceDir(cstr);
    }
    
    if (!objModel.parse(fp))
        return nil;
    
    objModel.toRawGeometry(textures,rawGeom);
    
    return self;
}

- (instancetype)initWithShape:(MaplyShape *)inShape;
{
    self = [super init];
    
    shape = inShape;
    // Note: Not supporting the linears at the moment
    
    return self;
}

// Return the list of texture file names
- (void)getTextureFileNames:(std::vector<std::string> &)texFileNames
{
    texFileNames = textures;
}

// Convert to raw geometry
- (void)asRawGeometry:(std::vector<WhirlyKit::GeometryRaw> &)outRawGeom
       withTexMapping:(const std::vector<WhirlyKit::SimpleIdentity> &)texFileMap
{
    outRawGeom.reserve(rawGeom.size());
    // Remap the texture IDs to something used by the scene
    for (const auto &geom : rawGeom)
    {
        outRawGeom.push_back(geom);
        for (auto &texID : outRawGeom.back().texIDs)
        {
            if (texID >= 0 && texID < texFileMap.size())
            {
                texID = (int)texFileMap[texID];
            }
        }
    }
}

static const GeometryRaw emptyGeom;

static const int corners[4][2] = {{0,0},{1,0},{1,1},{0,1}};

// Return the ID for or generate a base model in the Geometry Manager
- (WhirlyKit::SimpleIdentity)getBaseModel:(MaplyBaseInteractionLayer *)inLayer
                           fontTexManager:(const WhirlyKit::FontTextureManager_iOSRef &)fontTexManager
                                  compObj:(MaplyComponentObject *)compObj
                                     mode:(MaplyThreadMode)threadMode
{
    @synchronized(self)
    {
        if (layer)
            return baseModelID;
        if (!inLayer)
            return EmptyIdentity;

        ChangeSet changes;
        layer = inLayer;
        
        std::vector<WhirlyKit::GeometryRaw> procGeom;
        
        if (shape)
        {
            if (auto shapeManager = inLayer->scene->getManager<ShapeManager>(kWKShapeManager))
            {
                WhirlyKit::Shape *wkShape = nil;
                if ([shape isKindOfClass:[MaplyShapeCircle class]])
                    wkShape = [(MaplyShapeCircle *)shape asWKShape:nil];
                else if ([shape isKindOfClass:[MaplyShapeSphere class]])
                    wkShape = [(MaplyShapeSphere *)shape asWKShape:nil];
                else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
                    wkShape = [(MaplyShapeCylinder *)shape asWKShape:nil];
                else if ([shape isKindOfClass:[MaplyShapeExtruded class]])
                    wkShape = [(MaplyShapeExtruded *)shape asWKShape:nil];

                if (wkShape)
                {
                    shapeManager->convertShape(*wkShape,procGeom);
                }
            }
        }
        else
        {
            // Add the textures
            std::vector<std::string> texFileNames;
            [self getTextureFileNames:texFileNames];
            std::vector<SimpleIdentity> texIDMap(texFileNames.size());
            const MaplyQuadImageFormat format = MaplyImage4Layer8Bit;
            int whichTex = 0;
            for (const std::string &texFileName : texFileNames)
            {
                if (NSString* name = [NSString stringWithFormat:@"%s", texFileName.c_str()])
                if (UIImage* img = [UIImage imageNamed:name])
                if (MaplyTexture *tex = [inLayer addImage:img imageFormat:format mode:threadMode])
                {
                    maplyTextures.insert(tex);
                    texIDMap[whichTex] = tex.texID;
                }
                whichTex++;
            }
            
            // Convert the geometry and map the texture IDs
            [self asRawGeometry:procGeom withTexMapping:texIDMap];
        }
        
        std::map<SimpleIdentity,WhirlyKit::GeometryRaw> stringGeom;
        
        // Now for the strings
        for (const GeomStringWrapper &strWrap : strings)
        {
            // Convert the string to polygons
            auto drawStr = fontTexManager->addString(nullptr,strWrap.str,changes);
            if (!drawStr)
            {
                continue;
            }

            for (const DrawableString::Rect &rect : drawStr->glyphPolys)
            {
                // Find or insert the appropriate geometry bucket
                const auto result = stringGeom.insert(std::make_pair(rect.subTex.texId, emptyGeom));
                auto &geom = result.first->second;
                if (result.second)
                {
                    // A new item was inserted
                    geom.texIDs.push_back((int)rect.subTex.texId);
                    const int count = 3;
                    geom.pts.reserve(4*count);
                    geom.norms.reserve(4*count);
                    geom.texCoords.reserve(4*count);
                    geom.triangles.reserve(2*count);
                }

                const int basePt = (int)geom.pts.size();
                const Vector4d norm = strWrap.mat * Vector4d(0,0,1,0);

                for (unsigned int ip=0;ip<4;ip++)
                {
                    // Convert and transform the points
                    const auto x = rect.pts[corners[ip][0]].x();
                    const auto y = rect.pts[corners[ip][1]].y();
                    const Vector4d outPt = strWrap.mat * Vector4d(x,y,0.0,1.0);
                    geom.pts.emplace_back(outPt.x(),outPt.y(),outPt.z());

                    // The normal is the same for everything
                    geom.norms.emplace_back(norm.x(),norm.y(),norm.z());

                    // And the texture coordinates
                    geom.texCoords.push_back(rect.subTex.processTexCoord(TexCoord(corners[ip][0],corners[ip][1])));
                }

                // Wire up the two triangles
                geom.triangles.emplace_back(basePt+0,basePt+1,basePt+2);
                geom.triangles.emplace_back(basePt+0,basePt+2,basePt+3);
            }

            compObj->contents->drawStringIDs.insert(drawStr->getId());
        }
        
        // Convert the string geometry
        if (procGeom.empty())
        {
            procGeom.reserve(stringGeom.size());
        }
        for (auto &it : stringGeom)
        {
            procGeom.push_back(it.second);
        }

        if (auto geomManager = inLayer->scene->getManager<GeometryManager>(kWKGeometryManager))
        {
            baseModelID = geomManager->addBaseGeometry(procGeom, GeometryInfo(), changes);
        }

        // Need to flush these changes immediately
        inLayer->scene->addChangeRequests(changes);
        
        return baseModelID;
    }
}

@end

@implementation MaplyGeomModelInstance

@end

@implementation MaplyMovingGeomModelInstance

@end

@implementation MaplyGeomModelGPUInstance

@end

