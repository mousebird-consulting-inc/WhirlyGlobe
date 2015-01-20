/*
 *  GeometryOBJReader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/25/14.
 *  Copyright 2012-2014 mousebird consulting
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

#import <stdio.h>
#import "GeometryOBJReader.h"

namespace WhirlyKit
{
    
bool GeometryModelOBJ::parseMaterials(FILE *fp)
{
    bool success = true;
    Material *activeMtl = NULL;
    
    char line[2048];
    int lineNo = 0;
    
    while (fgets(line, 2047, fp))
    {
        lineNo++;
        int lineLen = strlen(line);
        
        // Empty line
        if (lineLen == 0 || line[0] == '\n')
            continue;
        // Comment
        if (line[0] == '#')
            continue;
        // Chop off a \r
        if (line[lineLen-1]  == '\r')
        {
            line[lineLen-1] = 0;
            lineLen--;
        }
        
        std::vector<char *> toks;
        // Parse the various tokens into an array
        char *tok = NULL;
        char *ptr = line;
        char *next = NULL;
        while ( (tok = strtok_r(ptr, " \t\n\r", &next)) )
        {
            toks.push_back(tok);
            ptr = next;
        }
        
        if (toks.empty())
            continue;
        
        char *key = toks[0];
        
        if (!strcmp(key,"newmtl"))
        {
            if (toks.size() < 2)
            {
                success = false;
                break;
            }
            
            char *name = toks[1];
            materials.resize(materials.size()+1);
            Material &mat = materials[materials.size()-1];
            activeMtl = &materials.back();
            mat.name = name;
        } else if (!strcmp(key,"Ka"))
        {
            if (toks.size() < 4 || !activeMtl)
            {
                success = false;
                break;
            }
            
            for (unsigned int ii=0;ii<3;ii++)
                activeMtl->Ka[ii] = atof(toks[ii+1]);
        } else if (!strcmp(key,"Kd"))
        {
            if (toks.size() < 4 || !activeMtl)
            {
                success = false;
                break;
            }
            
            for (unsigned int ii=0;ii<3;ii++)
                activeMtl->Kd[ii] = atof(toks[ii+1]);
            
        } else if (!strcmp(key,"Ks"))
        {
            if (toks.size() < 4 || !activeMtl)
            {
                success = false;
                break;
            }
            
            for (unsigned int ii=0;ii<3;ii++)
                activeMtl->Ks[ii] = atof(toks[ii+1]);
        } else if (!strcmp(key,"d") || !strcmp(key,"Tr"))
        {
            if (toks.size() < 2 || !activeMtl)
            {
                success = false;
                break;
            }
            
            activeMtl->trans = atof(toks[1]);
        } else if (!strcmp(key,"illum"))
        {
            if (toks.size() < 2 || !activeMtl)
            {
                success = false;
                break;
            }
            
            activeMtl->illum = atof(toks[1]);
        } else if (!strcmp(key,"map_Ks"))
        {
            if (toks.size() < 2 || !activeMtl)
            {
                success = false;
                break;
            }
            
            activeMtl->tex_ambient = toks[1];
        } else if (!strcmp(key, "map_Kd"))
        {
            if (toks.size() < 2 || !activeMtl)
            {
                success = false;
                break;
            }
            
            activeMtl->tex_diffuse = toks[1];
        }
        // Note: Ignoring map_d, map_bump, map_Ks, map_Ns or any of the options
    }
    
    return success;
}

bool GeometryModelOBJ::parse(FILE *fp)
{
    bool success = true;
    Group *activeGroup = NULL;
    int activeMtl = -1;
    
    char line[2048],origLine[2048],tmpTok[2048];
    int lineNo = 0;
    
    while (fgets(origLine, 2047, fp))
    {
        lineNo++;
        strcpy(line,origLine);
        int lineLen = strlen(line);

        // Empty line
        if (lineLen == 0 || line[0] == '\n')
            continue;
        // Comment
        if (line[0] == '#')
            continue;
        // Chop off a \r
        if (line[lineLen-1]  == '\r')
        {
            line[lineLen-1] = 0;
            lineLen--;
        }
        
        std::vector<char *> toks;
        // Parse the various tokens into an array
        char *tok = NULL;
        char *ptr = line;
        char *next = NULL;
        while ( (tok = strtok_r(ptr, " \t\n\r", &next)) )
        {
            toks.push_back(tok);
            ptr = next;
        }
        
        if (toks.empty())
            continue;
        
        char *key = toks[0];
        if (!strcmp(key,"mtllib"))
        {
            if (toks.size() < 2)
            {
                success = false;
                break;
            }

            // The full name of the material file might contain spaces
            strcpy(line,origLine);
            strtok_r(line," \t\n\r", &next);
            char *mtlFile = strtok_r(next,"\n\r", &next);
            
            if (!mtlFile)
            {
                success = false;
                break;
            }
            
            // Load the model
            NSString *fullPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:[NSString stringWithFormat:@"%s",mtlFile]];
            FILE *mtlFP = fopen([fullPath cStringUsingEncoding:NSASCIIStringEncoding],"r");
            if (!mtlFP)
            {
                success = false;
                break;
            }
            if (!parseMaterials(mtlFP))
            {
                success = false;
                break;
            }
            fclose(mtlFP);
        } else if (!strcmp(key,"usemtl"))
        {
            // Use a pre-defined material
            if (toks.size() < 1 || !activeGroup)
            {
                success = false;
                break;
            }
            
            // Look for the material
            std::string mtlName = toks[1];
            int whichMtl = -1;
            for (unsigned int ii=0;ii<materials.size();ii++)
            {
                if (mtlName == materials[ii].name)
                {
                    whichMtl = ii;
                    break;
                }
            }

            // Note: Allowing materials we don't recognize
            if (whichMtl < 0)
            {
                success = false;
                break;
            }
            activeMtl = whichMtl;
        } else if (!strcmp(key,"g"))
        {
            groups.resize(groups.size()+1);
            activeGroup = &groups.back();
            
            if (toks.size() > 1)
            {
                activeGroup->name = toks[1];
            } else {
                activeGroup->name = "";
            }
        } else if (!strcmp(key,"f"))
        {
            // Face
            if (toks.size() < 2)
            {
                success = false;
                break;
            }
            activeGroup->faces.resize(activeGroup->faces.size()+1);
            Face &face = activeGroup->faces.back();
            face.mtlID = activeMtl;
            
            // We've either got numbers of collections of numbers separated by /
            for (unsigned int ii=1;ii<toks.size();ii++)
            {
                strcpy(tmpTok, toks[ii]);
                bool emptyTexCoord = false;
                if (strstr(tmpTok,"//"))
                    emptyTexCoord = true;
                
                std::vector<char *> vertToks;
                char *vertTok = NULL;
                char *vertPtr = tmpTok;
                char *vertNext = NULL;
                while ( (vertTok = strtok_r(vertPtr, "/", &vertNext)) )
                {
                    vertToks.push_back(vertTok);
                    vertPtr = vertNext;
                }
                
                face.verts.resize(face.verts.size()+1);
                Vertex &vert = face.verts.back();
                if (vertToks.size() == 0)
                {
                    success = false;
                    break;
                }
                if (vertToks.size() >= 1)
                {
                    vert.vert = atoi(vertToks[0]);
                }
                if (emptyTexCoord)
                {
                    if (vertToks.size() >= 2)
                        vert.norm = atoi(vertToks[1]);
                } else {
                    if (vertToks.size() >= 2)
                    {
                        vert.texCoord = atoi(vertToks[1]);
                    }
                    if (vertToks.size() >= 3)
                    {
                        vert.norm = atoi(vertToks[2]);
                    }
                }
            }
        } else if (!strcmp(key,"v"))
        {
            // Regular vertex
            if (toks.size() < 4)
            {
                success = false;
                break;
            }
            
            verts.resize(verts.size()+1);
            Point3d &vert = verts.back();
            vert.x() = atof(toks[1]);
            vert.y() = atof(toks[2]);
            vert.z() = atof(toks[3]);
        } else if (!strcmp(key,"vn"))
        {
            // Normal
            if (toks.size() < 4)
            {
                success = false;
                break;
            }
            
            norms.resize(norms.size()+1);
            Point3d &norm = norms.back();
            norm.x() = atof(toks[1]);
            norm.y() = atof(toks[2]);
            norm.z() = atof(toks[3]);
        } else if (!strcmp(key,"vt"))
        {
            // Texture coordinate
            if (toks.size() < 3)
            {
                success = false;
                break;
            }
            
            texCoords.resize(texCoords.size()+1);
            Point2d &texCoord = texCoords.back();
            texCoord.x() = atof(toks[1]);
            texCoord.y() = atof(toks[2]);
        }
    }
    
    // Link up the materials
    for (unsigned int ii=0;ii<groups.size();ii++)
    {
        Group &group = groups[ii];
        for (unsigned int jj=0;jj<group.faces.size();jj++)
        {
            Face &face = group.faces[jj];
            if (face.mtlID >= 0 && face.mtlID < materials.size())
                face.mat = &materials[face.mtlID];
        }
    }
    
    return success;
}

// Used to sort faces by material
class FaceBin
{
public:
    FaceBin(GeometryModelOBJ::Face *face) { faces.push_back(face); mtlID = face->mtlID; }

    bool operator < (const FaceBin &that) const
    {
        return mtlID < that.mtlID;
    }
    
    int mtlID;
    
    std::vector<GeometryModelOBJ::Face *> faces;
};
typedef std::set<FaceBin> FaceBinSet;
    
void GeometryModelOBJ::toRawGeometry(std::vector<std::string> &textures,std::vector<GeometryRaw> &rawGeom)
{
    // Unique list of textures
    std::map<std::string,int> textureMapping;
    int texCount = 0;
    for (unsigned int ii=0;ii<materials.size();ii++)
    {
        Material &mat = materials[ii];
        mat.tex_ambientID = -1;  mat.tex_diffuseID = -1;
        if (!mat.tex_ambient.empty())
        {
            auto it = textureMapping.find(mat.tex_ambient);
            if (it != textureMapping.end())
                mat.tex_ambientID = it->second;
            else {
                mat.tex_ambientID = texCount;
                textureMapping[mat.tex_ambient] = texCount++;
            }
        }
        if (!mat.tex_diffuse.empty())
        {
            auto it = textureMapping.find(mat.tex_diffuse);
            if (it != textureMapping.end())
                mat.tex_diffuseID = it->second;
            else {
                mat.tex_diffuseID = texCount;
                textureMapping[mat.tex_diffuse] = texCount++;
            }
        }
    }
    textures.resize(texCount);
    for (auto it: textureMapping)
        textures[it.second] = it.first;
    
    // Sort the faces by material
    FaceBinSet faceBins;
    for (unsigned int ii=0;ii<groups.size();ii++)
    {
        Group *group = &groups[ii];
        for (unsigned int jj=0;jj<group->faces.size();jj++)
        {
            Face *face = &group->faces[jj];
            
            FaceBin faceBin(face);
            const auto &it = faceBins.find(faceBin);
            if (it == faceBins.end())
                faceBins.insert(faceBin);
            else {
                faceBin.faces.insert(faceBin.faces.end(),it->faces.begin(), it->faces.end());
                faceBins.erase(it);
                faceBins.insert(faceBin);
            }
        }
    }
        
    // Convert the face bins to raw geometry
    for (auto it: faceBins)
    {
        rawGeom.resize(rawGeom.size()+1);
        GeometryRaw &geom = rawGeom.back();
        geom.type = WhirlyKitGeometryTriangles;
        
        // Figure out if there's a texture ID
        geom.texId = EmptyIdentity;
        if (it.mtlID > -1)
        {
            Material &mtl = materials[it.mtlID];
            if (mtl.tex_diffuseID > -1)
                geom.texId = mtl.tex_diffuseID;
        }
        
        // Work through the faces
        for (unsigned int jj=0;jj<it.faces.size();jj++)
        {
            const Face *face = it.faces[jj];
            int basePt = geom.pts.size();
            for (unsigned int kk=0;kk<face->verts.size();kk++)
            {
                const Vertex &vert = face->verts[kk];
                // Note: Should be range checking these
                int vertId = vert.vert-1;
                if (vertId < 0 || vertId >= verts.size())
                    break;
                Point3d pt = verts[vertId];
                                    
                Point3d norm(0,0,1);
                int normId = vert.norm-1;
                if (normId >= 0 && normId < norms.size())
                    norm = norms[normId];
                TexCoord texCoord(0,0);
                int texId = vert.texCoord-1;
                if (texId >= 0 && texId < texCoords.size())
                {
                    const Point2d &pt2d = texCoords[texId];
                    texCoord = TexCoord(pt2d.x(),1.0-pt2d.y());
                }
                RGBAColor diffuse(255,255,255,255);
                if (face->mat && face->mat->Kd[0] != -1)
                {
                    diffuse.r = face->mat->Kd[0] * 255;
                    diffuse.g = face->mat->Kd[1] * 255;
                    diffuse.b = face->mat->Kd[2] * 255;
                    diffuse.a = face->mat->trans * 255;
                }
                
                geom.pts.push_back(pt);
                geom.norms.push_back(norm);
                if (face->mat && (face->mat->tex_ambientID >= 0 || face->mat->tex_diffuseID >= 0))
                    geom.texCoords.push_back(texCoord);
                geom.colors.push_back(diffuse);
            }
            
            // Assume these are convex for now
            for (unsigned int kk = 2;kk<face->verts.size();kk++)
            {
                geom.triangles.resize(geom.triangles.size()+1);
                GeometryRaw::RawTriangle &tri = geom.triangles.back();
                tri.verts[0] = basePt;
                tri.verts[1] = basePt+kk-1;
                tri.verts[2] = basePt+kk;
            }

            // Force double-sided ness
            // Assume these are convex for now
            for (unsigned int kk = 2;kk<face->verts.size();kk++)
            {
                geom.triangles.resize(geom.triangles.size()+1);
                GeometryRaw::RawTriangle &tri = geom.triangles.back();
                tri.verts[1] = basePt;
                tri.verts[0] = basePt+kk-1;
                tri.verts[2] = basePt+kk;
            }
        }
    }
}

}
