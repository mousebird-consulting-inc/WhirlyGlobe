//
//  MaplyHttpManager+Private.h
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 5/12/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//


#import "MaplyHttpManager.h"
#import "MaplyURLConnection.h"
NS_ASSUME_NONNULL_BEGIN

@interface MaplyHttpManager (Private)



-(MaplyURLConnection*)syncRequest:(NSURLRequest*)request
        completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion;

-(MaplyURLConnection*)asyncRequest:(NSURLRequest*)request
        completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion;

@end

NS_ASSUME_NONNULL_END
