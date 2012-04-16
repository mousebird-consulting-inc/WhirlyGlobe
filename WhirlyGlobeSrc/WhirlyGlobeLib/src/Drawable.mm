/*
 *  Drawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011 mousebird consulting
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

#import "GLUtils.h"
#import "Drawable.h"
#import "GlobeScene.h"
#import "UIImage+Stuff.h"
#import "SceneRendererES1.h"

using namespace WhirlyGlobe;

namespace WhirlyKit
{
		
Drawable::Drawable()
{
}
	
Drawable::~Drawable()
{
}
	
void DrawableChangeRequest::execute(Scene *scene,WhirlyKitView *view)
{
	Drawable *theDrawable = scene->getDrawable(drawId);
	if (theDrawable)
		execute2(scene,theDrawable);
}
	
BasicDrawable::BasicDrawable()
{
	on = true;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
	type = 0;
	texId = EmptyIdentity;
    minVisible = maxVisible = DrawVisibleInvalid;

    fadeDown = fadeUp = 0.0;
	color.r = color.g = color.b = color.a = 255;
    
    numTris = 0;
    numPoints = 0;
    
    pointBuffer = colorBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
}
	
BasicDrawable::BasicDrawable(unsigned int numVert,unsigned int numTri)
{
	on = true;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
	points.reserve(numVert);
	texCoords.reserve(numVert);
	norms.reserve(numVert);
	tris.reserve(numTri);
    fadeDown = fadeUp = 0.0;
	color.r = color.g = color.b = color.a = 255;
	drawPriority = 0;
	texId = EmptyIdentity;
    minVisible = maxVisible = DrawVisibleInvalid;

    numTris = 0;
    numPoints = 0;
    
    pointBuffer = colorBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
}
	
BasicDrawable::~BasicDrawable()
{
}
    
bool BasicDrawable::isOn(WhirlyKitRendererFrameInfo *frameInfo) const
{
    if (minVisible == DrawVisibleInvalid || !on)
        return on;

    float visVal = [frameInfo.theView heightAboveSurface];
    
    return ((minVisible <= visVal && visVal <= maxVisible) ||
             (maxVisible <= visVal && visVal <= minVisible));
}
    
bool BasicDrawable::hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const
{
    if (isAlpha)
        return true;
    
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo.currentTime < fadeDown)
            return false;
        else
            if (frameInfo.currentTime > fadeUp)
                return false;
            else
                return true;
    } else
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo.currentTime < fadeUp)
                return false;
            else
                if (frameInfo.currentTime > fadeDown)
                    return false;
                else
                    return true;
        }
    
    return false;
}
	
// Widen a line and turn it into a rectangle of the given width
void BasicDrawable::addRect(const Point3f &l0, const Vector3f &nl0, const Point3f &l1, const Vector3f &nl1,float width)
{
	Vector3f dir = l1-l0;
	if (dir.isZero())
		return;
	dir.normalize();

	float width2 = width/2.0;
	Vector3f c0 = dir.cross(nl0);
	c0.normalize();
	
	Point3f pt[3];
	pt[0] = l0 + c0 * width2;
	pt[1] = l1 + c0 * width2;
	pt[2] = l1 - c0 * width2;
	pt[3] = l0 - c0 * width2;

	unsigned short ptIdx[4];
	for (unsigned int ii=0;ii<4;ii++)
	{
		ptIdx[ii] = addPoint(pt[ii]);
		addNormal(nl0);
	}
	
	addTriangle(Triangle(ptIdx[0],ptIdx[1],ptIdx[3]));
	addTriangle(Triangle(ptIdx[3],ptIdx[1],ptIdx[2]));
}


// Define VBOs to make this fast(er)
void BasicDrawable::setupGL(float minZres)
{
	// Offset the geometry upward by minZres units along the normals
	// Only do this once, obviously
	if (drawOffset != 0 && (points.size() == norms.size()))
	{
		// Note: This could be faster
		float scale = minZres*drawOffset;
		for (unsigned int ii=0;ii<points.size();ii++)
		{
			Vector3f pt = points[ii];
			points[ii] = norms[ii] * scale + pt;
		}
	}
	
	pointBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
	if (points.size())
	{
		glGenBuffers(1,&pointBuffer);
        CheckGLError("BasicDrawable::setupGL() glGenBuffers()");
		glBindBuffer(GL_ARRAY_BUFFER,pointBuffer);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
		glBufferData(GL_ARRAY_BUFFER,points.size()*sizeof(Vector3f),&points[0],GL_STATIC_DRAW);
        CheckGLError("BasicDrawable::setupGL() glBufferData()");
		glBindBuffer(GL_ARRAY_BUFFER,0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
	}
    if (colors.size())
    {
		glGenBuffers(1,&colorBuffer);
        CheckGLError("BasicDrawable::setupGL() glGenBuffers()");
		glBindBuffer(GL_ARRAY_BUFFER,colorBuffer);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
		glBufferData(GL_ARRAY_BUFFER,colors.size()*sizeof(RGBAColor),&colors[0],GL_STATIC_DRAW);
        CheckGLError("BasicDrawable::setupGL() glBufferData()");
		glBindBuffer(GL_ARRAY_BUFFER,0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
    }
	if (texCoords.size())
	{
		glGenBuffers(1,&texCoordBuffer);
        CheckGLError("BasicDrawable::setupGL() glGenBuffers()");
		glBindBuffer(GL_ARRAY_BUFFER,texCoordBuffer);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
		glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(Vector2f),&texCoords[0],GL_STATIC_DRAW);
        CheckGLError("BasicDrawable::setupGL() glBufferData()");
		glBindBuffer(GL_ARRAY_BUFFER,0);		
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
	}
	if (norms.size())
	{
		glGenBuffers(1, &normBuffer);
        CheckGLError("BasicDrawable::setupGL() glGenBuffers()");
		glBindBuffer(GL_ARRAY_BUFFER,normBuffer);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
		glBufferData(GL_ARRAY_BUFFER,norms.size()*sizeof(Vector3f),&norms[0],GL_STATIC_DRAW);
        CheckGLError("BasicDrawable::setupGL() glBufferData()");
		glBindBuffer(GL_ARRAY_BUFFER,0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
	}
	if (tris.size())
	{
		glGenBuffers(1, &triBuffer);
        CheckGLError("BasicDrawable::setupGL() glGenBuffers()");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,triBuffer);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,tris.size()*sizeof(Triangle),&tris[0],GL_STATIC_DRAW);
        CheckGLError("BasicDrawable::setupGL() glBufferData()");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer()");
	}
    
    // Clear out the arrays, since we won't need them again
    numPoints = points.size();
    points.clear();
    texCoords.clear();
    norms.clear();
    numTris = tris.size();
    tris.clear();
    colors.clear();
    
    usingBuffers = true;
}
	
// Tear down the VBOs we set up
void BasicDrawable::teardownGL()
{
	if (pointBuffer)
    {
		glDeleteBuffers(1,&pointBuffer);
        CheckGLError("BasicDrawable::teardownGL() glDeleteBuffers()");
    }
    if (colorBuffer)
    {
		glDeleteBuffers(1,&colorBuffer);
        CheckGLError("BasicDrawable::teardownGL() glDeleteBuffers()");        
    }
	if (texCoordBuffer)
    {
		glDeleteBuffers(1,&texCoordBuffer);
        CheckGLError("BasicDrawable::teardownGL() glDeleteBuffers()");
    }
	if (normBuffer)
    {
		glDeleteBuffers(1,&normBuffer);
        CheckGLError("BasicDrawable::teardownGL() glDeleteBuffers()");
    }
	if (triBuffer)
    {
		glDeleteBuffers(1,&triBuffer);
        CheckGLError("BasicDrawable::teardownGL() glDeleteBuffers()");
    }
}
	
void BasicDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) const
{
    if (usingBuffers)
        drawVBO(frameInfo,scene);
    else
        drawReg(frameInfo,scene);
}
    
// Write this drawable to a cache file
bool BasicDrawable::writeToFile(FILE *fp, const TextureIDMap &texIDMap, bool doTextures) const
{
    SimpleIdentity remapTexId = EmptyIdentity;
    if (doTextures)
    {
        if (texId != EmptyIdentity)
        {
            TextureIDMap::const_iterator it = texIDMap.find(texId);
            if (it == texIDMap.end())
                return false;
            remapTexId = it->second + 1;
        }
    }
    
    if (fwrite(&on,sizeof(on),1,fp) != 1 ||
        fwrite(&drawPriority,sizeof(drawPriority),1,fp) != 1 ||
        fwrite(&drawOffset,sizeof(drawOffset),1,fp) != 1 ||
        fwrite(&isAlpha,sizeof(isAlpha),1,fp) != 1)
        return false;
    float ll_x,ll_y,ur_x,ur_y;
    ll_x = geoMbr.ll().x();    ll_y = geoMbr.ll().y();
    ur_x = geoMbr.ur().x();    ur_y = geoMbr.ur().y();
    if (fwrite(&ll_x,sizeof(float),1,fp) != 1 ||
        fwrite(&ll_y,sizeof(float),1,fp) != 1||
        fwrite(&ur_x,sizeof(float),1,fp) != 1||
        fwrite(&ur_y,sizeof(float),1,fp) != 1)
        return false;
    if (fwrite(&type,sizeof(type),1,fp) != 1 ||
        fwrite(&remapTexId,sizeof(remapTexId),1,fp) != 1 ||
        fwrite(&color.r,sizeof(color.r),1,fp) != 1 ||
        fwrite(&color.g,sizeof(color.r),1,fp) != 1 ||
        fwrite(&color.b,sizeof(color.r),1,fp) != 1 ||
        fwrite(&minVisible,sizeof(minVisible),1,fp) != 1 ||        
        fwrite(&maxVisible,sizeof(maxVisible),1,fp) != 1)
        return false;
    if (fwrite(&numPoints,sizeof(numPoints),1,fp) != 1 ||
        fwrite(&numTris,sizeof(numTris),1,fp) != 1)
        return false;
    
    unsigned int tmpNumPoints=points.size();
    if (fwrite(&tmpNumPoints,sizeof(tmpNumPoints),1,fp) != 1)
        return false;
    for (unsigned int ii=0;ii<points.size();ii++)
    {
        const Point3f &pt = points[ii];
        float x = pt.x(), y = pt.y(), z = pt.z();
        if (fwrite(&x,sizeof(float),1,fp) != 1 ||
            fwrite(&y,sizeof(float),1,fp) != 1 ||
            fwrite(&z,sizeof(float),1,fp) != 1)
            return false;
    }
    
    unsigned int tmpNumColors=colors.size();
    if (fwrite(&tmpNumColors,sizeof(tmpNumColors),1,fp) != 1)
        return false;
    for (unsigned int ii=0;ii<colors.size();ii++)
    {
        const RGBAColor &col = colors[ii];
        unsigned char r = col.r, g = col.g, b = col.b;
        if (fwrite(&r,sizeof(unsigned char),1,fp) != 1 ||
            fwrite(&g,sizeof(unsigned char),1,fp) != 1 ||
            fwrite(&b,sizeof(unsigned char),1,fp) != 1)
            return false;
    }

    unsigned int tmpNumTexCoords=texCoords.size();
    if (fwrite(&tmpNumTexCoords,sizeof(tmpNumTexCoords),1,fp) != 1)
        return false;
    for (unsigned int ii=0;ii<texCoords.size();ii++)
    {
        const Vector2f &vec = texCoords[ii];
        float x = vec.x(), y = vec.y();
        if (fwrite(&x,sizeof(float),1,fp) != 1 ||
            fwrite(&y,sizeof(float),1,fp) != 1)
            return false;
    }

    unsigned int tmpNumNorms=norms.size();
    if (fwrite(&tmpNumNorms,sizeof(tmpNumNorms),1,fp) != 1)
        return false;
    for (unsigned int ii=0;ii<norms.size();ii++)
    {
        const Point3f &pt = norms[ii];
        float x = pt.x(), y = pt.y(), z = pt.z();
        if (fwrite(&x,sizeof(float),1,fp) != 1 ||
            fwrite(&y,sizeof(float),1,fp) != 1 ||
            fwrite(&z,sizeof(float),1,fp) != 1)
            return false;        
    }
    
    unsigned int tmpNumTris=tris.size();
    if (fwrite(&tmpNumTris,sizeof(tmpNumTris),1,fp) != 1)
        return false;
    for (unsigned int ii=0;ii<tris.size();ii++)
    {
        const Triangle &tri = tris[ii];
        if (fwrite(&tri.verts,sizeof(unsigned short),3,fp) != 3)
            return false;
    }
    
    return true;
}

// Read this drawable from a cache file
bool BasicDrawable::readFromFile(FILE *fp, const TextureIDMap &texIDMap, bool doTextures)
{
    if (fread(&on,sizeof(on),1,fp) != 1 ||
        fread(&drawPriority,sizeof(drawPriority),1,fp) != 1 ||
        fread(&drawOffset,sizeof(drawOffset),1,fp) != 1 ||
        fread(&isAlpha,sizeof(isAlpha),1,fp) != 1)
        return false;
    float ll_x,ll_y,ur_x,ur_y;
    if (fread(&ll_x,sizeof(float),1,fp) != 1 ||
        fread(&ll_y,sizeof(float),1,fp) != 1||
        fread(&ur_x,sizeof(float),1,fp) != 1||
        fread(&ur_y,sizeof(float),1,fp) != 1)
        return false;
    geoMbr.addGeoCoord(GeoCoord(ll_x,ll_y));
    geoMbr.addGeoCoord(GeoCoord(ur_x,ur_y));

    SimpleIdentity fileTexId;
    if (fread(&type,sizeof(type),1,fp) != 1 ||
        fread(&fileTexId,sizeof(fileTexId),1,fp) != 1 ||
        fread(&color.r,sizeof(color.r),1,fp) != 1 ||
        fread(&color.g,sizeof(color.r),1,fp) != 1 ||
        fread(&color.b,sizeof(color.r),1,fp) != 1 ||
        fread(&minVisible,sizeof(minVisible),1,fp) != 1 ||        
        fread(&maxVisible,sizeof(maxVisible),1,fp) != 1)
        return false;
    // Need to remap from the file tex ID to the preallocated ID
    if (fileTexId != EmptyIdentity && doTextures)
    {
        TextureIDMap::const_iterator it = texIDMap.find(fileTexId);
        if (it == texIDMap.end())
            return false;
        texId = it->second;
    } else
        texId = EmptyIdentity;
    
    if (fread(&numPoints,sizeof(numPoints),1,fp) != 1 ||
        fread(&numTris,sizeof(numTris),1,fp) != 1)
        return false;
    
    unsigned int tmpNumPoints;
    if (fread(&tmpNumPoints,sizeof(tmpNumPoints),1,fp) != 1)
        return false;
    points.resize(tmpNumPoints);
    for (unsigned int ii=0;ii<points.size();ii++)
    {
        float x,y,z;
        if (fread(&x,sizeof(float),1,fp) != 1 ||
            fread(&y,sizeof(float),1,fp) != 1 ||
            fread(&z,sizeof(float),1,fp) != 1)
            return false;
        points[ii] = Point3f(x,y,z);
    }

    unsigned int tmpNumColors;
    if (fread(&tmpNumColors,sizeof(tmpNumColors),1,fp) != 1)
        return false;
    colors.resize(tmpNumColors);
    for (unsigned int ii=0;ii<colors.size();ii++)
    {
        unsigned char r,g,b;
        if (fread(&r,sizeof(float),1,fp) != 1 ||
            fread(&g,sizeof(float),1,fp) != 1 ||
            fread(&b,sizeof(float),1,fp) != 1)
            return false;
        colors[ii] = RGBAColor(r,g,b);
    }
    
    unsigned int tmpNumTexCoords;
    if (fread(&tmpNumTexCoords,sizeof(tmpNumTexCoords),1,fp) != 1)
        return false;
    texCoords.resize(tmpNumTexCoords);
    for (unsigned int ii=0;ii<texCoords.size();ii++)
    {
        float x,y;
        if (fread(&x,sizeof(float),1,fp) != 1 ||
            fread(&y,sizeof(float),1,fp) != 1)
            return false;
        texCoords[ii] = Point2f(x,y);
    }
    
    unsigned int tmpNumNorms;
    if (fread(&tmpNumNorms,sizeof(tmpNumNorms),1,fp) != 1)
        return false;
    norms.resize(tmpNumNorms);
    for (unsigned int ii=0;ii<norms.size();ii++)
    {
        float x,y,z;
        if (fread(&x,sizeof(float),1,fp) != 1 ||
            fread(&y,sizeof(float),1,fp) != 1 ||
            fread(&z,sizeof(float),1,fp) != 1)
            return false;        
        norms[ii] = Point3f(x,y,z);
    }
    
    unsigned int tmpNumTris;
    if (fread(&tmpNumTris,sizeof(tmpNumTris),1,fp) != 1)
        return false;
    tris.resize(tmpNumTris);
    for (unsigned int ii=0;ii<tris.size();ii++)
    {
        Triangle &tri = tris[ii];
        if (fread(&tri.verts,sizeof(unsigned short),3,fp) != 3)
            return false;
    }
    
    return true;
}

// VBO based drawing
void BasicDrawable::drawVBO(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) const
{
	GLuint textureId = scene->getGLTexture(texId);
	
	if (type == GL_TRIANGLES)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
    CheckGLError("BasicDrawable::drawVBO() lighting");
	
    if (!colorBuffer)
    {
        float scale = 1.0;
        if (fadeDown < fadeUp)
        {
            // Heading to 1
            if (frameInfo.currentTime < fadeDown)
                scale = 0.0;
            else
                if (frameInfo.currentTime > fadeUp)
                    scale = 1.0;
                else
                    scale = (frameInfo.currentTime - fadeDown)/(fadeUp - fadeDown);
        } else
            if (fadeUp < fadeDown)
            {
                // Heading to 0
                if (frameInfo.currentTime < fadeUp)
                    scale = 1.0;
                else
                    if (frameInfo.currentTime > fadeDown)
                        scale = 0.0;
                    else
                        scale = 1.0-(frameInfo.currentTime - fadeUp)/(fadeDown - fadeUp);
            }

        RGBAColor newColor = color;
        newColor.r = color.r * scale;
        newColor.g = color.g * scale;
        newColor.b = color.b * scale;
        newColor.a = color.a * scale;
        glColor4ub(newColor.r, newColor.g, newColor.b, newColor.a);
        CheckGLError("BasicDrawable::drawVBO() glColor4ub");
    }

	glEnableClientState(GL_VERTEX_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
	glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	glVertexPointer(3, GL_FLOAT, 0, 0);
    CheckGLError("BasicDrawable::drawVBO() glVertexPointer");

	glEnableClientState(GL_NORMAL_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	glNormalPointer(GL_FLOAT, 0, 0);
    CheckGLError("BasicDrawable::drawVBO() glNormalPointer");
    
    if (colorBuffer)
    {
        glEnable(GL_COLOR_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
        CheckGLError("BasicDrawable::drawVBO() glVertexPointer");        
    }

	if (textureId)
	{
		glEnable(GL_TEXTURE_2D);
        CheckGLError("BasicDrawable::drawVBO() glEnable");
		glBindTexture(GL_TEXTURE_2D, textureId);
        CheckGLError("BasicDrawable::drawVBO() glBindTexture");

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
        CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
        CheckGLError("BasicDrawable::drawVBO() glTexCoordPointer");
	}
    
    if (!textureId && (type == GL_TRIANGLES))
    {
//        NSLog(@"No texture for: %lu",getId());
		glDisable(GL_TEXTURE_2D);
        CheckGLError("BasicDrawable::drawVBO() glDisable");
    }
	
	switch (type)
	{
		case GL_TRIANGLES:
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
            CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
			glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, 0);
            CheckGLError("BasicDrawable::drawVBO() glDrawElements");
		}
			break;
		case GL_POINTS:
		case GL_LINES:
		case GL_LINE_STRIP:
		case GL_LINE_LOOP:
			glDrawArrays(type, 0, numPoints);
            CheckGLError("BasicDrawable::drawVBO() glDrawArrays");
			break;
	}
    
    if (colorBuffer)
    {
        glDisableClientState(GL_COLOR_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
    }
	
	if (textureId)
	{
		glDisable(GL_TEXTURE_2D);
        CheckGLError("BasicDrawable::drawVBO() glDisable");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
	}
	glDisableClientState(GL_VERTEX_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
	glDisableClientState(GL_NORMAL_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glDisableClientState");

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	
	glDisable(GL_LIGHTING);
    CheckGLError("BasicDrawable::drawVBO() glDisable");
}

// Non-VBO based drawing
void BasicDrawable::drawReg(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) const
{
	GLuint textureId = scene->getGLTexture(texId);
	
	if (textureId)
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisable(GL_TEXTURE_2D);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
    if (!norms.empty())
        glEnableClientState(GL_NORMAL_ARRAY);
    if (!colors.empty())
        glEnableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, &points[0]);
    if (!norms.empty())
        glNormalPointer(GL_FLOAT, 0, &norms[0]);
    if (!colors.empty())
    {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
    }
	if (textureId)
	{
		glTexCoordPointer(2, GL_FLOAT, 0, &texCoords[0]);
		glBindTexture(GL_TEXTURE_2D, textureId);
	}
    if (colors.empty())
        glColor4ub(color.r, color.g, color.b, color.a);
	
	switch (type)
	{
		case GL_TRIANGLES:
			glDrawElements(GL_TRIANGLES, tris.size()*3, GL_UNSIGNED_SHORT, (unsigned short *)&tris[0]);
			break;
		case GL_POINTS:
		case GL_LINES:
		case GL_LINE_STRIP:
		case GL_LINE_LOOP:
			glDrawArrays(type, 0, points.size());
			break;
	}
	
	if (textureId)
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
    if (!norms.empty())
        glDisableClientState(GL_NORMAL_ARRAY);
    if (!colors.empty())
        glDisableClientState(GL_COLOR_ARRAY);
}	

ColorChangeRequest::ColorChangeRequest(SimpleIdentity drawId,RGBAColor inColor)
	: DrawableChangeRequest(drawId)
{
	color[0] = inColor.r;
	color[1] = inColor.g;
	color[2] = inColor.b;
	color[3] = inColor.a;
}
	
void ColorChangeRequest::execute2(Scene *scene,Drawable *draw)
{
	BasicDrawable *basicDrawable = dynamic_cast<BasicDrawable *> (draw);
	basicDrawable->setColor(color);
}
	
OnOffChangeRequest::OnOffChangeRequest(SimpleIdentity drawId,bool OnOff)
	: DrawableChangeRequest(drawId), newOnOff(OnOff)
{
	
}
	
void OnOffChangeRequest::execute2(Scene *scene,Drawable *draw)
{
	BasicDrawable *basicDrawable = dynamic_cast<BasicDrawable *> (draw);
	basicDrawable->setOnOff(newOnOff);
}
    
VisibilityChangeRequest::VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis)
    : DrawableChangeRequest(drawId), minVis(minVis), maxVis(maxVis)
{
}
    
void VisibilityChangeRequest::execute2(Scene *scene,Drawable *draw)
{
    BasicDrawable *basicDrawable = dynamic_cast<BasicDrawable *> (draw);
    basicDrawable->setVisibleRange(minVis,maxVis);
}
    
FadeChangeRequest::FadeChangeRequest(SimpleIdentity drawId,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : DrawableChangeRequest(drawId), fadeUp(fadeUp), fadeDown(fadeDown)
{
    
}
    
void FadeChangeRequest::execute2(Scene *scene,Drawable *draw)
{
    // Fade it out, then remove it
    BasicDrawable *basicDraw = (BasicDrawable *)draw;
    basicDraw->setFade(fadeDown, fadeUp);
}

DrawTexChangeRequest::DrawTexChangeRequest(SimpleIdentity drawId,SimpleIdentity newTexId)
: DrawableChangeRequest(drawId), newTexId(newTexId)
{
}

void DrawTexChangeRequest::execute2(Scene *scene,Drawable *draw)
{
    BasicDrawable *basicDraw = (BasicDrawable *)draw;
    basicDraw->setTexId(newTexId);
}    
}
