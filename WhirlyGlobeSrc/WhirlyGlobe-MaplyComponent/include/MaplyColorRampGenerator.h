//
//  MaplyColorRampGenerator.h
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 4/20/16.
//
//

#import <UIKit/UIKit.h>

/** The color ramp generator will take a set of color values
    and generate a linear ramp of those colors in an output
    image.  You typically feed the color ramp image into a shader.
  */
@interface MaplyColorRampGenerator : NSObject

// If set we'll stretch the colors out to the whole image
// On by default.
@property (nonatomic,assign) bool stretch;

/// Add a color as a hex value.
- (void)addHexColor:(int)hexColor;

/// This color has an alpha too
- (void)addHexColorWithAlpha:(int)hexColor;

/// Add a color as a UIColor
- (void)addColor:(UIColor *)color;

/// Generate the image with the color ramp in it
- (UIImage *)makeImage:(CGSize)size;

@end
