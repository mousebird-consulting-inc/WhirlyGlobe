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

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyGeomState

@end

@implementation MaplyGeomBuilder

- (void)addRectangleAroundOrigin:(MaplyCoordinateD)size state:(MaplyGeomState *)state
{
    MaplyCoordinate3dD pts[4];
    pts[0] = {-size.x/2.0,-size.y/2.0,0.0};
    pts[1] = {-size.x/2.0,-size.y/2.0,0.0};
    pts[2] = {-size.x/2.0,-size.y/2.0,0.0};
    pts[3] = {-size.x/2.0,-size.y/2.0,0.0};
    MaplyCoordinateD tex[4];
    tex[0] = {0.0,0.0};
    tex[1] = {1.0,0.0};
    tex[2] = {1.0,1.0};
    tex[3] = {0.0,1.0};
    
    [self addPolygonWithPts:pts tex:tex numPts:4 state:state];
}

- (void)addString:(NSString *)str font:(UIFont *)font state:(MaplyGeomState *)state
{
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:str];
    NSInteger strLen = [attrStr length];
    [attrStr addAttribute:NSFontAttributeName value:font range:NSMakeRange(0, strLen)];
    if (state.color)
        [attrStr addAttribute:NSForegroundColorAttributeName value:state.color range:NSMakeRange(0, strLen)];
    
    [self addAttributedString:attrStr state:state];
}

- (void)addAttributedString:(NSAttributedString *)str state:(MaplyGeomState *)state
{
    GeomBuilderStringWrapper strWrap;
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
    }
    
    for (auto &geom : rawGeom)
        if (geom.texId == texID && (hasNorms == !geom.norms.empty()) && (hasTexCoords == !geom.texCoords.empty()) && (hasColors == !geom.colors.empty()))
            return &geom;
    
    if (state.texture)
    {
        textures.push_back(state.texture);
        texID = textures.size()-1;
    }
    
    rawGeom.resize(rawGeom.size()+1);
    GeometryRaw *geom = &rawGeom[rawGeom.size()-1];

    if (texID == -1)
        geom->texId = texID;
    
    return geom;
}

- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts tex:(MaplyCoordinateD *)tex norms:(MaplyCoordinate3dD *)norms numPts:(int)numPts state:(MaplyGeomState *)state
{
    GeometryRaw *geom = [self findMatchingGeom:state hasNorms:(norms != NULL) hasTexCoords:(tex != NULL) hasColors:NULL];
    
    // Add the points
    int basePt = geom->pts.size();
    geom->pts.reserve(geom->pts.size()+numPts);
    if (tex)
        geom->texCoords.reserve(geom->texCoords.size()+numPts);
    if (norms)
        geom->norms.reserve(geom->norms.size()+numPts);
    for (int ii=0;ii<numPts;ii++)
    {
        MaplyCoordinate3dD &pt = pts[ii];
        geom->pts.push_back(Point3d(pt.x,pt.y,pt.z));
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

- (void)translate:(MaplyCoordinate3dD)trans
{
    Affine3d transAffine(Translation3d(trans.x,trans.y,trans.z));
    Matrix4d mat = transAffine.matrix();
    [self applyTransform:mat];
}

- (void)rotate:(double)angle around:(MaplyCoordinate3dD)axis
{
    Affine3d rotAffine(AngleAxisd(angle,Vector3d(axis.x,axis.y,axis.z)));
    Matrix4d mat = rotAffine.matrix();
    [self applyTransform:mat];
}

- (void)transform:(MaplyMatrix *)matrix
{
    for (auto &geom : rawGeom)
        geom.applyTransform(matrix.mat);
}

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder
{
    [self addGeomFromBuilder:modelBuilder transform:nil];
}

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder transform:(MaplyMatrix *)matrix
{
    // Work through the new geometry
//    for (
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
        Vector4d p0 = str.mat * Vector4d(0,0,0,1);
        Vector4d p1 = str.mat * Vector4d(str.size.width,str.size.height,0,1);
        
        if (isSet)
        {
            ll->x = std::min(ll->x,p1.x());  ll->y = std::min(ll->y,p1.y());  ll->z = std::min(ll->z,p1.z());
            ur->x = std::min(ur->x,p1.x());  ur->y = std::min(ur->y,p1.y());  ur->z = std::min(ur->z,p1.z());
        } else {
            ll->x = p0.x();  ll->y = p0.y();  ll->z = p0.z();
            ur->x = p0.x();  ur->y = p0.y();  ur->z = p0.z();
        }
    }
    
    return isSet;
}

@end
