/*
 *  MaplyGeomModel.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 11/26/14.
 *  Copyright 2011-2015 mousebird consulting
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

#import <set>
#import "MaplyGeomModel_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyShape_private.h"
#import "MaplyComponentObject_private.h"

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
    FILE *fp = fopen(str, "r");
    if (!fp)
        return nil;
    
    // Parse it out of the file
    GeometryModelOBJ objModel;
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
- (void)asRawGeometry:(std::vector<WhirlyKit::GeometryRaw> &)outRawGeom withTexMapping:(const std::vector<WhirlyKit::SimpleIdentity> &)texFileMap
{
    outRawGeom.reserve(outRawGeom.size()+rawGeom.size());
    // Remap the texture IDs to something used by the scene
    for (auto geom : rawGeom)
    {
        if (geom.texId >= 0 && geom.texId < texFileMap.size())
        {
            geom.texId = texFileMap[geom.texId];
        }
        outRawGeom.push_back(geom);
    }
}

// Return the ID for or generate a base model in the Geometry Manager
- (WhirlyKit::SimpleIdentity)getBaseModel:(MaplyBaseInteractionLayer *)inLayer fontTexManager:(WhirlyKitFontTextureManager *)fontTexManager compObj:(MaplyComponentObject *)compObj mode:(MaplyThreadMode)threadMode
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
            ShapeManager *shapeManager = (ShapeManager *)layer->scene->getManager(kWKShapeManager);

            WhirlyKitShape *wkShape = nil;
            if ([shape isKindOfClass:[MaplyShapeCircle class]])
                wkShape = [(MaplyShapeCircle *)shape asWKShape:nil];
            else if ([shape isKindOfClass:[MaplyShapeSphere class]])
                wkShape = [(MaplyShapeSphere *)shape asWKShape:nil];
            else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
                wkShape = [(MaplyShapeCylinder *)shape asWKShape:nil];
            else if ([shape isKindOfClass:[MaplyShapeExtruded class]])
                wkShape = [(MaplyShapeExtruded *)shape asWKShape:nil];
            
            if (wkShape)
                shapeManager->convertShape(wkShape,rawGeom);
        } else {
            // Add the textures
            std::vector<std::string> texFileNames;
            [self getTextureFileNames:texFileNames];
            std::vector<SimpleIdentity> texIDMap(texFileNames.size());
            int whichTex = 0;
            for (const std::string &texFileName : texFileNames)
            {
                MaplyTexture *tex = [layer addImage:[UIImage imageNamed:[NSString stringWithFormat:@"%s",texFileName.c_str()]] imageFormat:MaplyImage4Layer8Bit mode:threadMode];
                if (tex)
                {
                    maplyTextures.insert(tex);
                    texIDMap[whichTex] = tex.texID;
                } else {
                    texIDMap[whichTex] = EmptyIdentity;
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
            DrawableString *drawStr = [fontTexManager addString:strWrap.str changes:changes];
            for (const DrawableString::Rect &rect : drawStr->glyphPolys)
            {
                // Find the appropriate geometry bucket
                auto it = stringGeom.find(rect.subTex.texId);
                GeometryRaw *geom = NULL;
                if (it == stringGeom.end())
                {
                    stringGeom[rect.subTex.texId] = GeometryRaw();
                    geom = &stringGeom[rect.subTex.texId];
                    geom->texId = rect.subTex.texId;
                } else
                    geom = &stringGeom[rect.subTex.texId];

                // Convert and transform the points
                int basePt = geom->pts.size();
                geom->pts.reserve(geom->pts.size()+4);
                Point2d pts[4];
                pts[0] = Point2d(rect.pts[0].x(),rect.pts[0].y());
                pts[1] = Point2d(rect.pts[1].x(),rect.pts[0].y());
                pts[2] = Point2d(rect.pts[1].x(),rect.pts[1].y());
                pts[3] = Point2d(rect.pts[0].x(),rect.pts[1].y());
                for (unsigned int ip=0;ip<4;ip++)
                {
                    auto &pt = pts[ip];
                    Vector4d outPt = strWrap.mat * Vector4d(pt.x(),pt.y(),0.0,1.0);
                    geom->pts.push_back(Point3d(outPt.x(),outPt.y(),outPt.z()));
                }

                // The normal is the same for everything
                Vector4d norm = strWrap.mat * Vector4d(0,0,1,0);
                geom->norms.reserve(geom->norms.size()+4);
                for (unsigned int ip=0;ip<4;ip++)
                    geom->norms.push_back(Point3d(norm.x(),norm.y(),norm.z()));
                
                // And the texture coordinates
                geom->texCoords.reserve(geom->texCoords.size()+4);
                geom->texCoords.push_back(rect.subTex.processTexCoord(TexCoord(0,0)));
                geom->texCoords.push_back(rect.subTex.processTexCoord(TexCoord(1,0)));
                geom->texCoords.push_back(rect.subTex.processTexCoord(TexCoord(1,1)));
                geom->texCoords.push_back(rect.subTex.processTexCoord(TexCoord(0,1)));
                
                // Wire up the two triangles
                geom->triangles.reserve(geom->triangles.size()+2);
                geom->triangles.push_back(GeometryRaw::RawTriangle(basePt+0,basePt+1,basePt+2));
                geom->triangles.push_back(GeometryRaw::RawTriangle(basePt+0,basePt+2,basePt+3));
            }
            
            compObj.drawStringIDs.insert(drawStr->getId());
            delete drawStr;
        }
        
        // Convert the string geometry
        procGeom.reserve(procGeom.size()+stringGeom.size());
        for (auto &it : stringGeom)
            procGeom.push_back(it.second);
        
        GeometryManager *geomManager = (GeometryManager *)layer->scene->getManager(kWKGeometryManager);
        baseModelID = geomManager->addBaseGeometry(procGeom, changes);

        // Need to flush these changes immediately
        layer->scene->addChangeRequests(changes);
        
        return baseModelID;
    }
}

@end

@implementation MaplyGeomModelInstance

@end

@implementation MaplyMovingGeomModelInstance

@end
