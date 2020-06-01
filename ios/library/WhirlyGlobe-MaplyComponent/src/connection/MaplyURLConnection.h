//
//  MaplyURLConnection.h
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 5/12/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 this object will used to perform all the http requests
 */

@interface MaplyURLConnection : NSObject


/**
 send the http request synchronously
 */
-(void)syncRequest:(NSURLRequest*)request
           session:(NSURLSession*)session
        completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion;

/**
 send the http request assynchronously
 */
-(void)asyncRequest:(NSURLRequest*)request
            session:(NSURLSession*)session
         completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion;


/**
 resume the current task
 */
-(void)resume;
/**
 cancel the current task
 */
-(void)cancel;
@end

NS_ASSUME_NONNULL_END
