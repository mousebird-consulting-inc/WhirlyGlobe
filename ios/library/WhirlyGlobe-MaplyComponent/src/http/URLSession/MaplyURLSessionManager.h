//
//  MaplyURLSessionManager.h
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 17/2/2022.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MaplyURLSessionManagerDelegate <NSObject>


@optional
/**
 this method will be implemented by the application to do a custom challenge with the http server (very useful in the case of the server with self-signed certificates)
  */
-(void)didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler;


@end

/**
 this header must be a public header
  */
@interface MaplyURLSessionManager : NSObject

@property (nonatomic,weak) id <MaplyURLSessionManagerDelegate> sessionManagerDelegate;



+ (instancetype)sharedManager;



@end

NS_ASSUME_NONNULL_END
