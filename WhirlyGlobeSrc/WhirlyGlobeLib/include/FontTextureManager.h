/*
 *  FontTextureManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/15/13.
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
#import <math.h>
#import <set>
#import <map>
#import <CoreText/CoreText.h>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "DynamicTextureAtlas.h"

namespace WhirlyKit
{
    
/// Defines the outline size (if present) of an NSAttributedString
#define kOutlineAttributeSize @"MaplyOutlineAttributeSize"
/// Defines the outline color of an NSAttributedString
#define kOutlineAttributeColor @"MaplyOutlineAttributeColor"


/** Information sufficient to draw a string as 3D geometry.
    All coordinates are in a local space related to the font size.
  */
class DrawableString : public Identifiable
{
public:
    DrawableString() { }
    
    /// A rectangle describing the placement of a single glyph and
    ///  the texture piece used to represent it
    class Rect
    {
    public:
        Point2f pts[2];
        TexCoord texCoords[2];
        SubTexture subTex;
    };
    std::vector<Rect> glyphPolys;

    /// Bounding box of the string in coordinates related to the font size
    Mbr mbr;
};

}

/** Used to manage a dynamic texture set containing glyphs from
    various fonts.
  */
@interface WhirlyKitFontTextureManager : NSObject

/// Initialize with a scene (for the subtextures)
- (id)initWithScene:(WhirlyKit::Scene *)scene;

/// Add the given string.  Caller is responsible for deleting
///  the DrawableString
- (WhirlyKit::DrawableString *)addString:(NSAttributedString *)str changes:(std::vector<WhirlyKit::ChangeRequest *> &)changes;

/// Remove resources associated with the given string
- (void)removeString:(WhirlyKit::SimpleIdentity)drawStringId changes:(std::vector<WhirlyKit::ChangeRequest *> &)changes;

/// Tear down anything we've built
- (void)clear:(std::vector<WhirlyKit::ChangeRequest *> &)changes;

@end
