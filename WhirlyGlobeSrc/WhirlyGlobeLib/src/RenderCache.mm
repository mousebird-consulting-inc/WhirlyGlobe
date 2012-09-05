/*
 *  RenderCache.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
 *
 *  Copyright 2011-2012 mousebird consulting
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

#import "RenderCache.h"

namespace WhirlyKit 
{

// Version of the render cache supported
static const int kRenderCacheVersion=1;

RenderCacheWriter::RenderCacheWriter(NSString *inFileName)
    : fp(NULL), numTextures(0), numDrawables(0), ignoreTextures(false)
{
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *fullName = [NSString stringWithFormat:@"%@/%@.rendercachebin",cacheDir,inFileName];
    fileBase = [inFileName cStringUsingEncoding:NSASCIIStringEncoding];
    
    // Try to open the file, as named
    fp = fopen([fullName cStringUsingEncoding:NSASCIIStringEncoding],"wb");    
    
    if (fp)
    {
        int version = kRenderCacheVersion;
        unsigned int dummy = 0;
        fwrite(&version, sizeof(version), 1, fp);
        fwrite(&dummy, sizeof(dummy), 1, fp);
        fwrite(&dummy, sizeof(dummy), 1, fp);
    }
}
    
RenderCacheWriter::~RenderCacheWriter()
{
    if (fp)
    {
        fseek(fp,sizeof(int),SEEK_SET);
        fwrite(&numTextures,sizeof(numTextures), 1, fp);
        fwrite(&numDrawables,sizeof(numDrawables), 1, fp);
        fclose(fp);
    }
}
    
void RenderCacheWriter::setIgnoreTextures()
{
    ignoreTextures = true;
}


// Write the texture out to its own file
//  and then keep track of that
std::string RenderCacheWriter::addTexture(SimpleIdentity texId,UIImage *theImg)
{
    texIDMap[texId] = numTextures;
    
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];

    // Construct a unique file name
    std::stringstream sstream;
    sstream << numTextures++;
    std::string imageName = fileBase + "_" + sstream.str();
    
    // Write it out as a dumb format
    [UIImagePNGRepresentation(theImg) writeToFile:[NSString stringWithFormat:@"%@/%s.png",cacheDir,imageName.c_str()] atomically:YES];
    
    return imageName;
}

// Write a drawable out, being sure to map the texture IDs
bool RenderCacheWriter::addDrawable(const Drawable *drawable)
{
    if (!fp)
        return false;
    
    if (!drawable->writeToFile(fp,texIDMap,!ignoreTextures))
        return false;
    numDrawables++;
        
    return true;
}

RenderCacheReader::RenderCacheReader(NSString *inFileBase)
    : fp(NULL), numTextures(0), numDrawables(0)
{
    fileBase = [inFileBase cStringUsingEncoding:NSASCIIStringEncoding];

    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];

    // Look for it in the cache dir first, then try the bundle dir
    NSString *fullName = [NSString stringWithFormat:@"%@/%@.rendercachebin",cacheDir,inFileBase];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fullName])
        fullName = [NSString stringWithFormat:@"%@/%@.rendercachebin",bundleDir,inFileBase];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fullName])
        return;

    fp = fopen([fullName cStringUsingEncoding:NSASCIIStringEncoding],"rb");
    
    if (fp)
    {
        int version;
        if (fread(&version,sizeof(version),1,fp) != 1 || (version != kRenderCacheVersion))
        {
            fclose(fp);
            fp = NULL;
        }
        if (fread(&numTextures,sizeof(numTextures),1,fp) !=1)
        {
            fclose(fp);
            fp = NULL;
        }
        if (fread(&numDrawables,sizeof(numDrawables),1,fp) != 1)
        {
            fclose(fp);
            fp = NULL;
        }
    }
    
    // If we successfully read the number of textures, we need to map from the IDs we'll
    //  encounter, to made up texture IDs
    // We'll use these new IDs later
    if (fp)
    {
        for (unsigned int ii=0;ii<numTextures;ii++)
            texIDMap[ii+1] = Identifiable::genId();
    }
}

RenderCacheReader::~RenderCacheReader()
{
    if (fp)
        fclose(fp);
}
    
// Read in the textures and drawables in the cache
// Caller responsible for deletion
bool RenderCacheReader::getDrawablesAndTextures(std::vector<Texture *> &textures,std::vector<Drawable *> &drawables)
{
    if (!fp)
        return false;
    
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
    
    // Load the textures first
    for (unsigned int ii=0;ii<numTextures;ii++)
    {
        // Look for it in the cache dir first, then try the bundle dir
        NSString *pathName = [NSString stringWithFormat:@"%@/%s_%d",cacheDir,fileBase.c_str(),ii];
        if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.png",pathName]])
            pathName = [NSString stringWithFormat:@"%@/%s_%d",bundleDir,fileBase.c_str(),ii];
        if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.png",pathName]])
            return false;
        
        Texture *tex = new Texture(pathName,@"png");
        // We need to use the ID we've previously generated so things match up
        tex->setId(texIDMap[ii+1]);
        textures.push_back(tex);
        
        // Note: Should check for texture validity
    }
    
    // And now the drawables
    for (unsigned int ii=0;ii<numDrawables;ii++)
    {
        // Note: Hack
        BasicDrawable *drawable = new BasicDrawable();
        // Note: Leaking memory
        if (!drawable->readFromFile(fp,texIDMap))
            return false;
        drawables.push_back(drawable);
    }
    
    return true;
}
    
// This version calls back so you can use them as we go
bool RenderCacheReader::getDrawablesAndTexturesAddToScene(Scene *scene,SimpleIDSet &texIDs,SimpleIDSet &drawIDs,float fade)
{
    if (!fp)
        return false;
    
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
    
    // Load the textures first
    for (unsigned int ii=0;ii<numTextures;ii++)
    {
        // Look for it in the cache dir first, then try the bundle dir
        NSString *pathName = [NSString stringWithFormat:@"%@/%s_%d",cacheDir,fileBase.c_str(),ii];
        if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.png",pathName]])
            pathName = [NSString stringWithFormat:@"%@/%s_%d",bundleDir,fileBase.c_str(),ii];
        if (![[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithFormat:@"%@.png",pathName]])
            return false;
        
        Texture *tex = new Texture(pathName,@"png");
        // We need to use the ID we've previously generated so things match up
        tex->setId(texIDMap[ii+1]);
        texIDs.insert(tex->getId());
        scene->addChangeRequest(new AddTextureReq(tex));
        
        // Note: Should check for texture validity
    }
    
    // And now the drawables
    for (unsigned int ii=0;ii<numDrawables;ii++)
    {
        // Note: Hack
        BasicDrawable *drawable = new BasicDrawable();
        // Note: Leaking memory
        if (!drawable->readFromFile(fp,texIDMap))
            return false;
        // If there's a fade in, do that
        if (fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            drawable->setFade(curTime,curTime+fade);
        }
        drawIDs.insert(drawable->getId());
        scene->addChangeRequest(new AddDrawableReq(drawable));
    }
    
    return true;    
}

// Look for the given render cache
// Note: Should check version
bool RenderCacheExists(NSString *baseName)
{
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject];
    NSString *bundleDir = [[NSBundle mainBundle] resourcePath];
    
    // Look for it in the cache dir first, then try the bundle dir
    NSString *fullName = [NSString stringWithFormat:@"%@/%@.rendercachebin",cacheDir,baseName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fullName])
        fullName = [NSString stringWithFormat:@"%@/%@.rendercachebin",bundleDir,baseName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fullName])
        return false;
    
    return true;
}

}
