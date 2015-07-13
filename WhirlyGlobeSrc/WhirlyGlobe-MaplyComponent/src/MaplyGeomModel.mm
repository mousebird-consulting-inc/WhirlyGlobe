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

using namespace WhirlyKit;
using namespace Eigen;

@implementation MaplyGeomModel
{
    MaplyShape *shape;
    __weak MaplyBaseInteractionLayer *layer;
    WhirlyKit::SimpleIdentity baseModelID;    
}

- (id)initWithObj:(NSString *)fullPath
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

- (id)initWithShape:(MaplyShape *)inShape;
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
    outRawGeom = rawGeom;
    // Remap the texture IDs to something used by the scene
    for (auto &geom : outRawGeom)
    {
        if (geom.texId >= 0 && geom.texId < texFileMap.size())
        {
            geom.texId = texFileMap[geom.texId];
        } else
            geom.texId = EmptyIdentity;
    }
}

// Return the ID for or generate a base model in the Geometry Manager
- (WhirlyKit::SimpleIdentity)getBaseModel:(MaplyBaseInteractionLayer *)inLayer mode:(MaplyThreadMode)threadMode
{
    @synchronized(self)
    {
        if (layer)
            return baseModelID;
        
        if (!inLayer)
            return EmptyIdentity;

        ChangeSet changes;
        layer = inLayer;
        
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
            std::vector<WhirlyKit::GeometryRaw> theRawGeom;
            [self asRawGeometry:theRawGeom withTexMapping:texIDMap];
        }
        
        GeometryManager *geomManager = (GeometryManager *)layer->scene->getManager(kWKGeometryManager);
        baseModelID = geomManager->addBaseGeometry(rawGeom, changes);

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
