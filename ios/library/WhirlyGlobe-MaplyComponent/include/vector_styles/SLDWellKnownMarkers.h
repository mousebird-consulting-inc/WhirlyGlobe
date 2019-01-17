//
//  SLDWellKnownMarkers.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-23.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

/** 
    Class for generating images corresponding to WellKnownName elements.
 
    Each static method uses low-level Core Graphics calls to generate an appropriate UIImage object.
 @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
*/
@interface SLDWellKnownMarkers : NSObject

+ (UIImage *)imageWithName:(NSString *)wellKnownName strokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size;

@end
