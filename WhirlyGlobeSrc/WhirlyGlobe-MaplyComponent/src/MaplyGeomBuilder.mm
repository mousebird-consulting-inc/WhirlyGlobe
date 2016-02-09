/*
 *  MaplyGeomModelBuilder.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 1/20/16
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

#import "MaplyGeomBuilder_private.h"
#import "MaplyMatrix_private.h"
#import "MaplyGeomModel_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyTexture_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyGeomState

- (id)init
{
    self = [super init];
    _color = [UIColor whiteColor];
    
    return self;
}

@end

@implementation MaplyGeomBuilder
{
    std::vector<Point3d> curPts;
}

- (id)initWithViewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    viewC = inViewC;
    
    return self;
}

- (void)addRectangleAroundOrigin:(MaplyCoordinateD)size state:(MaplyGeomState *)state
{
    MaplyCoordinate3dD pts[4];
    pts[0] = {-size.x/2.0,-size.y/2.0,0.0};
    pts[1] = {size.x/2.0,-size.y/2.0,0.0};
    pts[2] = {size.x/2.0,size.y/2.0,0.0};
    pts[3] = {-size.x/2.0,size.y/2.0,0.0};
    MaplyCoordinateD tex[4];
    tex[0] = {0.0,0.0};
    tex[1] = {1.0,0.0};
    tex[2] = {1.0,1.0};
    tex[3] = {0.0,1.0};
    MaplyCoordinate3dD norms[4];
    norms[0] = {0,0,1};
    norms[1] = {0,0,1};
    norms[2] = {0,0,1};
    norms[3] = {0,0,1};
    
    [self addPolygonWithPts:pts tex:tex norms:norms numPts:4 state:state];
}

- (void)addRectangleAroundX:(double)x y:(double)y width:(double)width height:(double)height state:(MaplyGeomState *)state
{
    MaplyCoordinate3dD pts[4];
    pts[0] = {-width/2.0+x,-height/2.0+y,0.0};
    pts[1] = {width/2.0+x,-height/2.0+y,0.0};
    pts[2] = {width/2.0+x,height/2.0+y,0.0};
    pts[3] = {-width/2.0+x,height/2.0+y,0.0};
    MaplyCoordinateD tex[4];
    tex[0] = {0.0,0.0};
    tex[1] = {1.0,0.0};
    tex[2] = {1.0,1.0};
    tex[3] = {0.0,1.0};
    MaplyCoordinate3dD norms[4];
    norms[0] = {0,0,1};
    norms[1] = {0,0,1};
    norms[2] = {0,0,1};
    norms[3] = {0,0,1};
    
    [self addPolygonWithPts:pts tex:tex norms:norms numPts:4 state:state];
}

- (void)addRectangleAroundOriginX:(double)x y:(double)y state:(MaplyGeomState *)state
{
    MaplyCoordinateD size;
    size.x = x;  size.y = y;
    
    [self addRectangleAroundOrigin:size state:state];
}

- (void)addString:(NSString *)str font:(UIFont *)font state:(MaplyGeomState *)state
{
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:str];
    NSInteger strLen = [attrStr length];
    [attrStr addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, strLen)];
    if (state.color)
        [attrStr addAttribute:NSForegroundColorAttributeName value:state.color range:NSMakeRange(0, strLen)];
    if (state.backColor)
        [attrStr addAttribute:NSBackgroundColorAttributeName value:state.backColor range:NSMakeRange(0, strLen)];
    
    [self addAttributedString:attrStr state:state];
}

- (void)addString:(NSString *)str width:(double)width height:(double)height font:(UIFont *)font state:(MaplyGeomState *)state
{
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:str];
    NSInteger strLen = [attrStr length];
    [attrStr addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, strLen)];
    if (state.color)
        [attrStr addAttribute:NSForegroundColorAttributeName value:state.color range:NSMakeRange(0, strLen)];
    if (state.backColor)
        [attrStr addAttribute:NSBackgroundColorAttributeName value:state.backColor range:NSMakeRange(0, strLen)];

    GeomStringWrapper strWrap;
    strWrap.str = attrStr;
    strWrap.size = [attrStr size];

    // width or height might not be set
    if (width == 0.0)
        width = strWrap.size.width / strWrap.size.height * height;
    if (height == 0.0)
        height = strWrap.size.height / strWrap.size.width * width;

    Affine3d scaleAffine(Eigen::Scaling(width / strWrap.size.width, height / strWrap.size.height, 1.0));
    strWrap.mat = scaleAffine.matrix();
    
    strings.push_back(strWrap);
}

- (void)addAttributedString:(NSAttributedString *)str state:(MaplyGeomState *)state
{
    GeomStringWrapper strWrap;
    strWrap.str = str;
    strWrap.size = [str size];
    strWrap.mat = Matrix4d::Identity();
    
    strings.push_back(strWrap);
}

- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts numPts:(int)numPts state:(MaplyGeomState *)state
{
    [self addPolygonWithPts:pts tex:NULL norms:NULL numPts:numPts state:state];
}

- (GeometryRaw *)findMatchingGeom:(MaplyGeomState *)state hasNorms:(bool)hasNorms hasTexCoords:(bool)hasTexCoords hasColors:(bool)hasColors
{
    int texID = -1;
    
    if (state.texture)
    {
        int which = 0;
        for (id thisTex : textures)
        {
            if (thisTex == state.texture)
            {
                texID = which;
                break;
            }
            which++;
        }

        if (texID == -1)
        {
            textures.push_back(state.texture);
            texID = textures.size()-1;
        }

    }
    
    for (auto &geom : rawGeom)
        if (geom.texId == texID && (hasNorms == !geom.norms.empty()) && (hasTexCoords == !geom.texCoords.empty()) && (hasColors == !geom.colors.empty()))
            return &geom;
    
    rawGeom.resize(rawGeom.size()+1);
    GeometryRaw *geom = &rawGeom[rawGeom.size()-1];

    geom->texId = texID;
    
    return geom;
}

- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts tex:(MaplyCoordinateD *)tex norms:(MaplyCoordinate3dD *)norms numPts:(int)numPts state:(MaplyGeomState *)state
{
    GeometryRaw *geom = [self findMatchingGeom:state hasNorms:(norms != NULL) hasTexCoords:(tex != NULL) hasColors:NULL];
    
    // Add the points
    int basePt = geom->pts.size();
    geom->pts.reserve(geom->pts.size()+numPts);
    geom->colors.reserve(geom->colors.size()+numPts);
    if (tex)
        geom->texCoords.reserve(geom->texCoords.size()+numPts);
    if (norms)
        geom->norms.reserve(geom->norms.size()+numPts);
    
    RGBAColor color(0,0,0,255);
    if (state.color)
        color = [state.color asRGBAColor];
    
    for (int ii=0;ii<numPts;ii++)
    {
        MaplyCoordinate3dD &pt = pts[ii];
        geom->pts.push_back(Point3d(pt.x,pt.y,pt.z));
        geom->colors.push_back(color);
        if (tex)
        {
            MaplyCoordinateD &texCoord = tex[ii];
            geom->texCoords.push_back(TexCoord(texCoord.x,texCoord.y));
        }
        if (norms)
        {
            MaplyCoordinate3dD &norm = norms[ii];
            geom->norms.push_back(Point3d(norm.x,norm.y,norm.z));
        }
    }
    
    // Tesselate into triangles
    for (int ii=2;ii<numPts;ii++)
    {
        GeometryRaw::RawTriangle tri;
        tri.verts[0] = 0+basePt;
        tri.verts[1] = ii-1+basePt;
        tri.verts[2] = ii+basePt;
        geom->triangles.push_back(tri);
    }
}

- (void)addCurPointX:(double)x y:(double)y z:(double)z
{
    curPts.push_back(Point3d(x,y,z));
}

- (void)addCurPointX:(double)x y:(double)y
{
    curPts.push_back(Point3d(x,y,0.0));
}

- (void)addCurPoly:(MaplyGeomState *)state
{
    if (curPts.size() > 2)
    {
        GeometryRaw *geom = [self findMatchingGeom:state hasNorms:false hasTexCoords:false hasColors:false];

        int basePt = geom->pts.size();
        geom->pts.reserve(geom->pts.size()+curPts.size());
        if (state.color)
        {
            geom->colors.reserve(geom->colors.size()+curPts.size());
        }
        geom->norms.reserve(geom->norms.size()+curPts.size());
        for (int ii=0;ii<curPts.size();ii++)
        {
            geom->pts.push_back(curPts[ii]);
            if (state.color)
                geom->colors.push_back([state.color asRGBAColor]);
            geom->norms.push_back(Point3d(0,0,1));
        }
        
        // Tesselate into triangles
        for (int ii=2;ii<curPts.size();ii++)
        {
            GeometryRaw::RawTriangle tri;
            tri.verts[0] = 0+basePt;
            tri.verts[1] = ii-1+basePt;
            tri.verts[2] = ii+basePt;
            geom->triangles.push_back(tri);
        }
    }
    
    curPts.clear();
}

- (void)applyTransform:(Matrix4d &)mat
{
    for (auto &geom : rawGeom)
        geom.applyTransform(mat);
    for (auto &string : strings)
        string.mat = mat * string.mat;
}

- (void)scale:(MaplyCoordinate3dD)scale
{
    Affine3d scaleAffine(Eigen::Scaling(scale.x, scale.y, scale.z));
    Matrix4d mat = scaleAffine.matrix();
    [self applyTransform:mat];
}

- (void)scaleX:(double)x y:(double)y z:(double)z
{
    MaplyCoordinate3dD scale;
    scale.x = x;  scale.y = y;  scale.z = z;
    [self scale:scale];
}

- (void)translate:(MaplyCoordinate3dD)trans
{
    Affine3d transAffine(Translation3d(trans.x,trans.y,trans.z));
    Matrix4d mat = transAffine.matrix();
    [self applyTransform:mat];
}

- (void)translateX:(double)x y:(double)y z:(double)z
{
    MaplyCoordinate3dD trans;
    trans.x = x;  trans.y = y;  trans.z = z;
    [self translate:trans];
}

- (void)rotate:(double)angle around:(MaplyCoordinate3dD)axis
{
    Affine3d rotAffine(AngleAxisd(angle,Vector3d(axis.x,axis.y,axis.z)));
    Matrix4d mat = rotAffine.matrix();
    [self applyTransform:mat];
}

- (void)rotate:(double)angle aroundX:(double)x y:(double)y z:(double)z
{
    MaplyCoordinate3dD axis = {x,y,z};
    [self rotate:angle around:axis];
}

- (void)transform:(MaplyMatrix *)matrix
{
    for (auto &geom : rawGeom)
        geom.applyTransform(matrix.mat);
    
    for (auto &string : strings)
        string.mat = string.mat * matrix.mat;
}

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder
{
    [self addGeomFromBuilder:modelBuilder transform:nil];
}

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder transform:(MaplyMatrix *)matrix
{
    // Work through the new geometry
    for (auto &geom : modelBuilder->rawGeom)
    {
        MaplyGeomState *tmpState = [[MaplyGeomState alloc] init];
        if (geom.texId >= 0)
        {
            tmpState.texture = modelBuilder->textures[geom.texId];
        }
        GeometryRaw *dest = [self findMatchingGeom:tmpState hasNorms:!geom.norms.empty() hasTexCoords:!geom.texCoords.empty() hasColors:!geom.colors.empty()];
        
        // Copy over to the destination, with transform
        dest->pts.reserve(dest->pts.size()+geom.pts.size());
        dest->norms.reserve(dest->norms.size()+geom.norms.size());
        dest->texCoords.reserve(dest->texCoords.size()+geom.texCoords.size());
        dest->colors.reserve(dest->colors.size()+geom.colors.size());
        int basePt = dest->pts.size();
        for (int ii=0;ii<geom.pts.size();ii++)
        {
            // Vertex
            Vector3d pt = geom.pts[ii];
            if (matrix)
            {
                Vector4d pt4 = matrix.mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
                pt = Vector3d(pt4.x(),pt4.y(),pt4.z());
            }
            dest->pts.push_back(pt);
            // Normal
            if (!geom.norms.empty())
            {
                Vector3d norm = geom.norms[ii];
                if (matrix)
                {
                    Vector4d pt4 = matrix.mat * Vector4d(norm.x(),norm.y(),norm.z(),0.0);
                    norm = Vector3d(pt4.x(),pt4.y(),pt4.z());
                }
                dest->norms.push_back(norm);
            }
            // Texture coordinates
            if (!geom.texCoords.empty())
            {
                const TexCoord &texCoord = geom.texCoords[ii];
                dest->texCoords.push_back(texCoord);
            }
            // Colors
            if (!geom.colors.empty())
            {
                const RGBAColor &color = geom.colors[ii];
                dest->colors.push_back(color);
            }
        }
        // Now the triangles
        dest->triangles.reserve(dest->triangles.size()+geom.triangles.size());
        for (auto tri : geom.triangles)
        {
            for (auto &idx : tri.verts)
                idx += basePt;
            dest->triangles.push_back(tri);
        }
    }
        
    // And the strings
    for (auto &string : modelBuilder->strings)
    {
        auto theString = string;
        theString.mat = matrix.mat * theString.mat;
        strings.push_back(theString);
    }
}

- (bool)getSizeLL:(MaplyCoordinate3dD *)ll ur:(MaplyCoordinate3dD *)ur
{
    bool isSet = false;
    
    for (auto &geom : rawGeom)
    {
        for (auto &pt : geom.pts)
        {
            if (isSet)
            {
                ll->x = std::min(ll->x,pt.x());  ll->y = std::min(ll->y,pt.y());  ll->z = std::min(ll->z,pt.z());
                ur->x = std::min(ur->x,pt.x());  ur->y = std::min(ur->y,pt.y());  ur->z = std::min(ur->z,pt.z());
            } else {
                ll->x = pt.x();  ll->y = pt.y();  ll->z = pt.z();
                ur->x = pt.x();  ur->y = pt.y();  ur->z = pt.z();
                isSet = true;
            }
        }
    }
    
    for (auto &str : strings)
    {
        Vector4d p[2];
        p[0] = str.mat * Vector4d(0,0,0,1);
        p[1] = str.mat * Vector4d(str.size.width,str.size.height,0,1);
        
        for (auto &pt : p)
        {
            if (isSet)
            {
                ll->x = std::min(ll->x,pt.x());  ll->y = std::min(ll->y,pt.y());  ll->z = std::min(ll->z,pt.z());
                ur->x = std::max(ur->x,pt.x());  ur->y = std::max(ur->y,pt.y());  ur->z = std::max(ur->z,pt.z());
            } else {
                ll->x = pt.x();  ll->y = pt.y();  ll->z = pt.z();
                ur->x = pt.x();  ur->y = pt.y();  ur->z = pt.z();
                isSet = true;
            }
        }
    }
    
    return isSet;
}

- (MaplyCoordinate3dD)getSize
{
    MaplyCoordinate3dD ll,ur;
    [self getSizeLL:&ll ur:&ur];
    
    MaplyCoordinate3dD size;
    size.x = ur.x - ll.x;
    size.y = ur.y - ll.y;
    size.z = ur.z - ll.z;
    
    return size;
}

- (MaplyGeomModel *)makeGeomModel:(MaplyThreadMode)threadMode
{
    MaplyGeomModel *model = [[MaplyGeomModel alloc] init];
    
    std::map<int,SimpleIdentity> texMap;
    
    // Convert the textures
    int which = 0;
    for (id texId : textures)
    {
        MaplyTexture *tex = nil;
        if ([texId isKindOfClass:[MaplyTexture class]])
        {
            tex = texId;
        } else if ([texId isKindOfClass:[UIImage class]])
        {
            // Note: Should allow them to set texture attributes
            tex = [viewC addTexture:texId desc:nil mode:threadMode];
        }
        
        model->maplyTextures.insert(tex);
        texMap[which++] = tex.texID;
    }
    
    // Convert the geometry
    for (const GeometryRaw &geomRaw : rawGeom)
    {
        GeometryRaw thisGeom = geomRaw;
        
        // Remap texture
        if (thisGeom.texId >= 0)
        {
            std::map<int,SimpleIdentity>::iterator it = texMap.find(thisGeom.texId);
            if (it != texMap.end())
                thisGeom.texId = texMap[thisGeom.texId];
        }
        
        model->rawGeom.push_back(thisGeom);
    }
    
    // Convert the strings
    for (const auto &string : strings)
    {
        model->strings.push_back(string);
    }
    
    
    return model;
}

@end
