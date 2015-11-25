/*
 *  FontTextureManageriOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/19/14.
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

#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>
#import <WhirlyGlobe.h>

namespace WhirlyKit
{

/// iOS version of font texture manager
class FontTextureManageriOS : public FontTextureManager
{
public:
    ~FontTextureManageriOS();
    
    // Wrapper for FontManager.  Tracks CoreText resources too.
    class FontManageriOS : public FontManager
    {
    public:
        FontManageriOS(CTFontRef theFont);
        FontManageriOS();
        ~FontManageriOS();
        
        CTFontRef font;
    };
    
    /// Add the given string.  Caller is responsible for deleting
    ///  the DrawableString
    DrawableString *addString(NSAttributedString *str,ChangeSet &changes);
    
protected:
    // Find the appropriate font manager
    FontManageriOS *findFontManagerForFont(UIFont *font,UIColor *color,UIColor *outlineColor,float outlineSize);
    
    // Render the glyph with the given font manager
    RawDataRef renderGlyph(WKGlyph glyph,FontManageriOS *fm,Point2f &size,Point2f &glyphSize,Point2f &offset,Point2f &textureOffset);    
};

}
