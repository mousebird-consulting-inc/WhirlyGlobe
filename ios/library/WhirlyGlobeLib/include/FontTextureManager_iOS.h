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
    
    virtual bool operator < (const FontManager &that) const;
    
    CTFontRef font;
    UIColor *colorUI;
    UIColor *backColorUI;
    UIColor *outlineColorUI;
};

/** FontTextureManager for iOS.
  */
class FontTextureManager_iOS : public FontTextureManager
{
public:
    /// Add the given string.  Caller is responsible for deleting the DrawableString
    WhirlyKit::DrawableString *addString(NSAttributedString *str,ChangeSet &changes);
    
protected:
    NSData *renderGlyph(CGGlyph glyph,FontManager_iOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);
    FontManager_iOS *findFontManagerForFont(UIFont *uiFont,UIColor *colorUI,UIColor *backColorUI,UIColor *outlineColorUI,float outlinesize);
};

}
