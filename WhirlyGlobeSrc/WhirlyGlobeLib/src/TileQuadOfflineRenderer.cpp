/*
 *  TileQuadOfflineRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "TileQuadOfflineRenderer.h"
#import "FlatMath.h"
#if defined(__ANDROID__)
#import <android/log.h>
#endif
#import "GLUtils.h"

using namespace Eigen;

namespace WhirlyKit
{
    
OfflineTile::OfflineTile()
 : numLoading(0), placeholder(false)
{
};

OfflineTile::OfflineTile(const WhirlyKit::Quadtree::Identifier &ident)
: ident(ident), numLoading(0), placeholder(false)
{
}
    
OfflineTile::OfflineTile(const WhirlyKit::Quadtree::Identifier &ident,int numImages)
    : ident(ident), numLoading(0), placeholder(false)
{
    textures.resize(numImages,0);
}

OfflineTile::~OfflineTile()
{
    for (int ii=0;ii<textures.size();ii++)
    {
        Texture *tex = textures[ii];
        if (tex)
            delete tex;
    }
}
    
void OfflineTile::clearTextures(Scene *scene)
{
    for (int ii=0;ii<textures.size();ii++)
    {
        Texture *tex = textures[ii];
        if (tex)
        {
            tex->destroyInGL(scene->getMemManager());
            delete tex;
        }
        textures[ii] = NULL;
    }
}

void OfflineTile::GetTileSize(int &numX,int &numY)
{
    numX = numY = 0;
    Texture *exampleTexture = NULL;
    for (unsigned int ii=0;ii<textures.size();ii++)
        if (textures[ii])
        {
            exampleTexture = textures[ii];
            break;
        }
    if (exampleTexture)
    {
        numX = exampleTexture->getWidth();
        numY = exampleTexture->getHeight();
    }
}
    
// Return the number of loaded frames
int OfflineTile::getNumLoaded()
{
    int numLoad = 0;
    for (unsigned int ii=0;ii<textures.size();ii++)
        if (textures[ii])
            numLoad++;
    
    return numLoad;
}
    
static const char *vertexShaderImage =
"attribute vec2 a_position;\n"
"attribute vec2 a_texCoord0;\n"
"\n"
"uniform vec2 u_vertOrg;\n"
"uniform vec2 u_vertSize;\n"
"\n"
"varying vec2 v_texCoord;\n"
"\n"
"void main()\n"
"{\n"
"   v_texCoord = a_texCoord0;\n"
"   vec2 vert = 2.0*(u_vertOrg + vec2(u_vertSize.x*a_position.x,u_vertSize.y*a_position.y)) - vec2(1.0,1.0);\n"
"   vert.y *= -1.0;\n"
"\n"
"   gl_Position = vec4(vert,0.0,1.0);\n"
"}\n"
;

static const char *fragmentShaderImage =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"varying vec2      v_texCoord;\n"
"\n"
"void main()\n"
"{\n"
"  vec4 pixel = texture2D(s_baseMap0, v_texCoord);\n"
//    "  gl_FragColor = vec4(pixel.a,pixel.b,pixel.g,pixel.r);\n"
//    "  gl_FragColor = vec4(pixel.r,pixel.g,pixel.b,pixel.a);\n"
    "  gl_FragColor = vec4(pixel.r,pixel.g,pixel.b,1.0);\n"
"}\n"
;


QuadTileOfflineLoader::QuadTileOfflineLoader(const std::string &name,QuadTileImageDataSource *imageSource)
    : name(name), imageSource(imageSource), on(true), numImages(1), sizeX(1024), sizeY(1024), autoRes(true),
    period(10.0), previewLevels(-1), outputDelegate(NULL), numFetches(0), lastRender(0), somethingChanged(true), currentMbr(-1),
    prog(NULL)
{
    theMbr.addPoint(Point2f(0,0));
    theMbr.addPoint(Point2f(1.0*M_PI/180.0, 1.0*M_PI/180.0));

    // Set up a shader
    prog = new OpenGLES2Program("OfflineShader",vertexShaderImage,fragmentShaderImage);
    if (!prog->isValid())
    {
        delete prog;
        prog = NULL;
    }
}
    
QuadTileOfflineLoader::~QuadTileOfflineLoader()
{
    clear(false);
}

void QuadTileOfflineLoader::clear(bool clearTextures)
{
    for (OfflineTileSet::iterator it = tiles.begin();it != tiles.end();++it)
    {
        if (clearTextures)
            (*it)->clearTextures(scene);
        delete *it;
    }
    tiles.clear();
    somethingChanged = true;
}
    
int QuadTileOfflineLoader::numFrames()
{
    // Note: Why are we doing this here?
    {
        std::lock_guard<std::mutex> lock(mut);
        for (unsigned int ii=0;ii<numImages;ii++)
            updatedFrames.insert(ii);
    }
    
    return numImages;
}

void QuadTileOfflineLoader::setMbr(Mbr newMbr)
{
    if (!control)
        return;

    if (newMbr.ll().x() < 0 && newMbr.ur().x() > 0 && (newMbr.ur().x() - newMbr.ll().x() > M_PI))
    {
        float tmp = newMbr.ll().x();
        newMbr.ll().x() = newMbr.ur().x();
        newMbr.ur().x() = tmp;
        newMbr.ur().x() += 2*M_PI;
    }

    {
        std::lock_guard<std::mutex> lock(mut);
        theMbr = newMbr;
        currentMbr++;
        updatedFrames.clear();
        for (unsigned int ii=0;ii<numImages;ii++)
            updatedFrames.insert(ii);
    }

    somethingChanged = true;
}

Point2d QuadTileOfflineLoader::calculateSize()
{
    if (autoRes)
    {
        Point2f imageRes(MAXFLOAT,MAXFLOAT);
        
        Mbr mbr;
        {
            std::lock_guard<std::mutex> lock(mut);
            mbr = theMbr;
        }
        
        // Note: Assuming geographic or spherical mercator
        GeoMbr geoMbr(GeoCoord(mbr.ll().x(), mbr.ll().y()),GeoCoord(mbr.ur().x(),mbr.ur().y()));
        std::vector<Mbr> testMbrs;
        if (geoMbr.ur().x() > M_PI)
        {
            testMbrs.push_back(Mbr(geoMbr.ll(),Point2f((float)M_PI,geoMbr.ur().y())));
            testMbrs.push_back(Mbr(Point2f((float)(M_PI),geoMbr.ll().y()),geoMbr.ur()));
        } else {
            testMbrs.push_back(Mbr(geoMbr.ll(),geoMbr.ur()));
        }

        // Work our way through the tiles looking for overlaps
        for (OfflineTileSet::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            OfflineTile *tile = *it;
            // Scale the extents to the output image
            Mbr tileMbr[2];
            tileMbr[0] = control->getQuadtree()->generateMbrForNode(tile->ident);
            bool overlaps = tileMbr[0].overlaps(testMbrs[0]);
            if (testMbrs.size() > 1 && !overlaps)
            {
                tileMbr[1] = tileMbr[0];
                tileMbr[1].ll().x() += 2*M_PI;
                tileMbr[1].ur().x() += 2*M_PI;
                overlaps = tileMbr[1].overlaps(testMbrs[1]);
            }
            if (!overlaps)
                continue;
            
            for (unsigned int jj=0;jj<testMbrs.size();jj++)
            {
                Mbr &testMbr = testMbrs[jj];
                if (!tileMbr[jj].overlaps(testMbr))
                    continue;
                
                // Figure out how big a pixel is
                int numX,numY;
                tile->GetTileSize(numX,numY);
                if (numX <= 0 || numY <= 0)
                    continue;
                Point2f texSize = tileMbr[0].ur() - tileMbr[0].ll();
                texSize.x() /= numX;  texSize.y() /= numY;
                imageRes.x() = std::min(imageRes.x(),texSize.x());
                imageRes.y() = std::min(imageRes.y(),texSize.y());
            }
        }
        
        Point2f numPix = mbr.ur()-mbr.ll();
        numPix.x() /= imageRes.x();  numPix.y() /= imageRes.y();
        
        numPix.x() = std::min((float)sizeX,numPix.x());  numPix.y() = std::min((float)sizeY,numPix.y());
        numPix.x() = std::max(numPix.x(),16.f);  numPix.y() = std::max(numPix.y(),16.f);
        return Point2d((int)numPix.x(), (int)numPix.y());
    } else
        return Point2d(sizeX, sizeY);
}
    
static const GLfloat imageVerts[] = {
    0.f,0.f,
    1.f,0.f,
    1.f,1.f,
    0.f,0.f,
    1.f,1.f,
    0.f,1.f
};

static const GLfloat imageTexCoords[] =
{
    0.f,1.f,
    1.f,1.f,
    1.f,0.f,
    0.f,1.f,
    1.f,0.f,
    0.f,0.f
};


void QuadTileOfflineLoader::imageRenderToLevel(int deep,ChangeSet &changes)
{
    if (!outputDelegate)
        return;
    
    std::set<int> framesToRender;
    int whichMbr;
    Mbr mbr;
    {
        std::lock_guard<std::mutex> lock(mut);
        framesToRender = updatedFrames;
        updatedFrames.clear();
        mbr = theMbr;
        whichMbr = currentMbr;
    }
    
    if (framesToRender.empty())
        return;
    
    lastRender = TimeGetCurrent();
    
    // Note: Assuming geographic or spherical mercator
    GeoMbr geoMbr(GeoCoord(mbr.ll().x(), mbr.ll().y()),GeoCoord(mbr.ur().x(),mbr.ur().y()));
    std::vector<Mbr> testMbrs;
    if (geoMbr.ur().x() > M_PI)
    {
        testMbrs.push_back(Mbr(geoMbr.ll(),Point2f((float)M_PI,geoMbr.ur().y())));
        testMbrs.push_back(Mbr(Point2f((float)(M_PI),geoMbr.ll().y()),geoMbr.ur()));
    } else {
        testMbrs.push_back(Mbr(geoMbr.ll(),geoMbr.ur()));
    }
    
    Point2d texSize = calculateSize();
    int outSizeX = texSize.x(), outSizeY = texSize.y();
    if (outSizeX == 0 || outSizeY == 0)
        return;
    
    outSizeX = NextPowOf2(outSizeX);
    outSizeY = outSizeX;

    // Set up an OpenGL render buffer to draw to
    GLuint frameBuf;
    glGenFramebuffers(1, &frameBuf);
    CheckGLError("Offline glGenFramebuffers");
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuf);
    CheckGLError("Offline glBindFramebuffer");
    
    // Color renderbuffer and backing store
    GLuint colorBuffer;
    glGenRenderbuffers(1, &colorBuffer);
    CheckGLError("Offline glGenRenderbuffers");
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    CheckGLError("Offline glBindRenderbuffer");
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, outSizeX, outSizeY);
    CheckGLError("Offline glRenderbufferStorage");
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    CheckGLError("Offline glFramebufferRenderbuffer");
    
    glDisable(GL_CULL_FACE);
    
    // We're rendering the same rectangle with a different texture
    glUseProgram(prog->getProgram());
    CheckGLError("Offline glUseProgram");
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
    const OpenGLESAttribute *texAttr = prog->findAttribute("a_texCoord0");
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 2, GL_FLOAT, GL_FALSE, 0, &imageVerts[0]);
        CheckGLError("Offline glVertexAttribPointer");
        glEnableVertexAttribArray(vertAttr->index);
        CheckGLError("Offline glEnableVertexAttribArray");
    }
    if (texAttr)
    {
        glVertexAttribPointer(texAttr->index, 2, GL_FLOAT, GL_FALSE, 0, &imageTexCoords[0]);
        CheckGLError("Offline glVertexAttribPointer");
        glEnableVertexAttribArray(texAttr->index);
        CheckGLError("Offline glEnableVertexAttribArray");
    }
    prog->setUniform("a_baseMap0", 0);
    
    //        NSLog(@"Tex Size = (%f,%f)",texSize.width,texSize.height);
    
    int numRenderedTiles = 0;
    
    // We'll just re-render the frames that were updated
    for (std::set<int>::iterator it = framesToRender.begin(); it != framesToRender.end(); ++it)
    {
        if (whichMbr != currentMbr)
            break;
        
        int whichFrame = *it;
        
        // And a texture to render to
        GLuint renderTex;
        glGenTextures(1, &renderTex);
        CheckGLError("Offline glGenTextures");
        glBindTexture(GL_TEXTURE_2D, renderTex);
        CheckGLError("Offline glBindTexture");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outSizeX, outSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        CheckGLError("Offline glTexImage2D");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
        CheckGLError("Offline glFramebufferTexture2D");
        glViewport(0, 0, outSizeX, outSizeY);
        CheckGLError("Offline glViewport");
        
        glBindTexture(GL_TEXTURE_2D, 0);

        // Clear output texture
        glClearColor(0, 0, 0, 0);
        CheckGLError("Offline glClearColor");
        glClear(GL_COLOR_BUFFER_BIT);
        CheckGLError("Offline glClear");

        // Work through the tiles, drawing as we go
        for (OfflineTileSet::iterator it = tiles.begin(); it != tiles.end(); ++it)
        {
            // If this happens, they've changed the MBR while we were working on this one.  Punt.
            if (whichMbr != currentMbr)
            {
                glDeleteTextures(1, &renderTex);
                
                break;
            }
        
            OfflineTile *tile = *it;
            if (tile->textures[whichFrame] == NULL)
                continue;
            if (deep > 0 && tile->ident.level > deep)
                continue;
            
            // Scale the extents to the output image
            Mbr tileMbr[2];
            tileMbr[0] = control->getQuadtree()->generateMbrForNode(tile->ident);
            bool overlaps = tileMbr[0].overlaps(testMbrs[0]);
            if (testMbrs.size() > 1 && !overlaps)
            {
                tileMbr[1] = tileMbr[0];
                tileMbr[1].ll().x() += 2*M_PI;
                tileMbr[1].ur().x() += 2*M_PI;
                overlaps = tileMbr[1].overlaps(testMbrs[1]);
            }
            if (!overlaps)
                continue;
            
            for (unsigned int jj=0;jj<testMbrs.size();jj++)
            {
                Mbr &testMbr = testMbrs[jj];
                if (!tileMbr[jj].overlaps(testMbr))
                    continue;
                
                Point2f org;
                org.x() = (tileMbr[jj].ll().x() - mbr.ll().x()) / (mbr.ur().x()-mbr.ll().x());
                org.y() = (tileMbr[jj].ll().y() - mbr.ll().y()) / (mbr.ur().y()-mbr.ll().y());
                Point2f span;
                span.x() = (tileMbr[jj].ur().x()-tileMbr[jj].ll().x()) / (mbr.ur().x()-mbr.ll().x());
                span.y() = (tileMbr[jj].ur().y()-tileMbr[jj].ll().y()) / (mbr.ur().y()-mbr.ll().y());
                
                // Find the right input image
                Texture *texToDraw = NULL;
                if (whichFrame < tile->textures.size())
                    texToDraw = tile->textures[whichFrame];
                
                if (texToDraw)
                {
                    prog->setUniform("u_vertOrg", org);
                    prog->setUniform("u_vertSize", span);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texToDraw->getGLId());
                    CheckGLError("Offline glBindTexture");
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    CheckGLError("Offline glDrawArrays");
                    glBindTexture(GL_TEXTURE_2D, 0);
                    CheckGLError("Offline glBindTexture clear");
                }
            }
            numRenderedTiles++;
        }
        
        glFlush();
        
        //            NSLog(@"Offline: Rendered frame %d, Tex Size = (%f,%f)",whichFrame,texSize.width,texSize.height);
        
        // Register the texture we've already created
        TextureWrapper *tex = new TextureWrapper("TileQuadOfflineRenderer",renderTex);
        SimpleIdentity texID = tex->getId();
        scene->addTexture(tex);
        
//            [_quadLayer.layerThread addChangeRequests:changes];
//            [_quadLayer.layerThread flushChangeRequests];
        
        QuadTileOfflineImage offImage;
        offImage.texture = texID;
        offImage.frame = whichFrame;
        offImage.mbr = mbr;
        offImage.texSize = Point2d(outSizeX,outSizeY);
        offImage.centerSize = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x()/2.0, texSize.y()/2.0));
        offImage.cornerSizes[0] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, 0.0));
        offImage.cornerSizes[1] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), 0.0));
        offImage.cornerSizes[2] = pixelSizeForMbr(mbr,texSize,Point2d(texSize.x(), texSize.y()));
        offImage.cornerSizes[3] = pixelSizeForMbr(mbr,texSize,Point2d(0.0, texSize.y()));
        
        if (outputDelegate)
            outputDelegate->offlineRender(this, &offImage);
    }
//        CGContextRelease(theContext);
//        CGColorSpaceRelease(colorSpace);
    
    //        NSLog(@"Rendered %d tiles of %d, depth = %d",numRenderedTiles,(int)tiles.size(),deep);
    
#if defined(__ANDROID__)
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline rendered %d tiles of %d", numRenderedTiles,(int)tiles.size());
#endif
    
    //        NSLog(@"CenterSize = (%f,%f), texSize = (%d,%d)",image.centerSize.width,image.centerSize.height,(int)texSize.width,(int)texSize.height);
    
    // Shut down state we used for rendering, except the texture
    if (vertAttr)
        glDisableVertexAttribArray(vertAttr->index);
    if (texAttr)
        glDisableVertexAttribArray(texAttr->index);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError("Offline glBindFramebuffer clear");
    glBindTexture(GL_TEXTURE_2D, 0);
    CheckGLError("Offline glBindTexture clear");
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CheckGLError("Offline glBindRenderbuffer clear");
    glDeleteRenderbuffers(1, &colorBuffer);
    CheckGLError("Offline glDeleteRenderbuffers");
    glDeleteFramebuffers(1, &frameBuf);
    CheckGLError("Offline glDeleteFramebuffers");
    
    // If we did a quick render, we need to go back again
    if (deep > 0)
        somethingChanged = true;
    else
        somethingChanged = false;
}

// Calculate the real world size of a given pixel
Point2d QuadTileOfflineLoader::pixelSizeForMbr(const Mbr &theMbr,const Point2d &texSize,const Point2d &texel)
{
    Point2f texelSize((theMbr.ur().x()-theMbr.ll().x())/texSize.x(),(theMbr.ur().y()-theMbr.ll().y())/texSize.y());
    
    // Coordinates in the local space
    Point2f l[3];
    l[0] = theMbr.ll() + Point2f(texel.x()*texelSize.x(),texel.y()*texelSize.y());
    l[1] = theMbr.ll() + Point2f((texel.x()+1)*texelSize.x(),texel.y()*texelSize.y());
    l[2] = theMbr.ll() + Point2f(texel.x()*texelSize.x(),(texel.y()+1)*texelSize.y());
    
    // Project the points into display space
    CoordSystemDisplayAdapter *coordAdapter = control->getScene()->getCoordAdapter();
    CoordSystem *localCoordSys = coordAdapter->getCoordSystem();
    CoordSystem *srcCoordSys = control->getCoordSys();
    Point3d d[3];
    for (unsigned int ii=0;ii<3;ii++)
        d[ii] = coordAdapter->localToDisplay(localCoordSys->geocentricToLocal(srcCoordSys->localToGeocentric(Point3d(l[ii].x(),l[ii].y(),0.0))));

    double da = (d[1] - d[0]).norm() * EarthRadius;
    double db = (d[2] - d[0]).norm() * EarthRadius;
    
    return Point2d(da, db);
}

#pragma mark - WhirlyKitQuadLoader

void QuadTileOfflineLoader::shutdownLayer(ChangeSet &changes)
{
    clear(true);
}
    
void QuadTileOfflineLoader::reset(ChangeSet &changes)
{
    clear(true);
}

bool QuadTileOfflineLoader::isReady()
{
    return (numFetches <= imageSource->maxSimultaneousFetches());
}

// Note: Porting
//- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
//{
//    _quadLayer = layer;
//    
//    if (_period > 0.0 && !renderScheduled) {
//        renderScheduled = true;
//        [self performSelector:@selector(imageRenderPeriodic) withObject:nil afterDelay:_period];
//    }
//}

void QuadTileOfflineLoader::loadTile(const Quadtree::NodeInfo &tileInfo,int frame)
{
    OfflineTile *theTile = getTile(tileInfo.ident);
    if (!theTile)
    {
        theTile = new OfflineTile(tileInfo.ident,numImages);
        tiles.insert(theTile);
    }
    theTile->numLoading++;

    numFetches++;
    imageSource->startFetch(this, tileInfo.ident.level, tileInfo.ident.x, tileInfo.ident.y, frame, const_cast<Dictionary *>(&tileInfo.attrs));
    
    OfflineTile *newTile = new OfflineTile(tileInfo.ident);
    
    tiles.insert(newTile);
    somethingChanged = true;
    
#if defined(__ANDROID__)
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline loadTile() %d: (%d,%d) %d", tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y,frame);
#endif
}

OfflineTile *QuadTileOfflineLoader::getTile(const WhirlyKit::Quadtree::Identifier &ident)
{
    OfflineTile dummyTile(ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it == tiles.end())
        return NULL;
    
    return *it;
}

void QuadTileOfflineLoader::unloadTile(const Quadtree::NodeInfo &tileInfo)
{
    OfflineTile dummyTile(tileInfo.ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it != tiles.end())
    {
        OfflineTile *theTile = *it;
        theTile->clearTextures(scene);
        numFetches -= theTile->numLoading;
        delete theTile;
        tiles.erase(it);
    }
    somethingChanged = true;
    
#if defined(__ANDROID__)
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline unloadTile() %d: (%d,%d)", tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y);
#endif
}

bool QuadTileOfflineLoader::canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
{
    return true;
}
    
void QuadTileOfflineLoader::loadedImages(QuadTileImageDataSource *dataSource,const std::vector<LoadedImage *> &loadImages,int level,int col,int row,int frame,ChangeSet &changes)
{
    // Note: Porting
}


void QuadTileOfflineLoader::loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes)
{
    numFetches--;
    Quadtree::Identifier tileIdent(col,row,level);
    OfflineTile *tile = getTile(tileIdent);
    if (!tile)
        return;
    
    if (tile->numLoading > 0)
    {
        numFetches--;
        tile->numLoading--;
    }

    if (!loadImage)
    {
        control->tileDidNotLoad(tileIdent, frame);
        return;
    }

#if defined(__ANDROID__)
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Offline loadedImage() %d: (%d,%d) %d", level,col,row,frame);
#endif

    // Assemble the images
    Texture *loadTex = loadImage->buildTexture(0, loadImage->getWidth(), loadImage->getHeight());
    if (loadTex)
        loadTex->createInGL(scene->getMemManager());

    if (frame == -1)
    {
        tile->textures.resize(1);
        tile->textures[0] = loadTex;
    } else {
        Texture *oldTex = tile->textures[frame];
        if (oldTex)
	  {
            oldTex->destroyInGL(scene->getMemManager());
	    delete oldTex;
	  }
        tile->textures[frame] = loadTex;
    }
    
    //    NSLog(@"Loaded tile %d: (%d,%d), frame = %d",level,col,row,frame);
    control->tileDidLoad(tileIdent, frame);

    // We'll need to update this frame
    {
        std::lock_guard<std::mutex> lock(mut);
        updatedFrames.insert(frame);
        somethingChanged = true;
    }
}

}
