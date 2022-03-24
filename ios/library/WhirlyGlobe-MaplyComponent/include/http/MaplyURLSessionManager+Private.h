//
//  MaplyURLSessionManager+Private.h
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 18/3/2022.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import "MaplyURLSessionManager.h"

NS_ASSUME_NONNULL_BEGIN

/**
 this header must be a private header
*/

@interface MaplyURLSessionManager (Private)

/** Create an URL session
 
    All url session  must be created by MaplyURLSessionManager using createURLSession method
  */
- (NSURLSession*)createURLSession;

@end

NS_ASSUME_NONNULL_END
