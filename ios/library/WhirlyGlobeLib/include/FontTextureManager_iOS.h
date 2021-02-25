/*
 *  FontTextureManager_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/4/19.
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

#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>
#import "FontTextureManager.h"

namespace WhirlyKit
{

/** Manages our use of a single font for iOS.
 
    Parent class does most of the work.
  */
class FontManager_iOS : public FontManager
{
public:
    FontManager_iOS(CTFontRef font);
    ~FontManager_iOS();
    
    virtual bool operator < (const FontManager_iOS &that) const;
    
    CTFontRef font;
    UIColor *colorUI;
    UIColor *backColorUI;
    UIColor *outlineColorUI;
};

typedef std::shared_ptr<FontManager_iOS> FontManager_iOSRef;

/** FontTextureManager for iOS.
  */
class FontTextureManager_iOS : public FontTextureManager
{
public:
    FontTextureManager_iOS(SceneRenderer *sceneRender,Scene *scene);
    virtual ~FontTextureManager_iOS();
    
    /// Add the given string.  Caller is responsible for deleting the DrawableString
    WhirlyKit::DrawableString *addString(PlatformThreadInfo *threadInfo,NSAttributedString *str,ChangeSet &changes);
    
protected:
    NSData *renderGlyph(CGGlyph glyph,FontManager_iOSRef fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);
    FontManager_iOSRef findFontManagerForFont(UIFont *uiFont,UIColor *colorUI,UIColor *backColorUI,UIColor *outlineColorUI,float outlinesize);
};
    
typedef std::shared_ptr<FontTextureManager_iOS> FontTextureManager_iOSRef;

}
