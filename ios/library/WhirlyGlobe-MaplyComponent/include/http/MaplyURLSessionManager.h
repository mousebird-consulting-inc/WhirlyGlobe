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

typedef NS_ENUM(NSUInteger, MaplyURLSessionPriority) {
    MaplyURLSessionPriorityDefault,
    MaplyURLSessionPriorityLow,
    MaplyURLSessionPriorityHigh,
};

/**
 this header must be a public header
  */
@interface MaplyURLSessionManager : NSObject

@property (nonatomic,weak) id <MaplyURLSessionManagerDelegate> sessionManagerDelegate;

@property (nonatomic) bool allowCellularRequests;
@property (nonatomic) bool allowConstrainedRequests API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (nonatomic) bool allowExpensiveRequests API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (nonatomic, retain) NSDictionary *additionalHeaders;
@property (nonatomic) bool acceptCookies;
@property (nonatomic) int maxConnectionsPerHost;
@property (nonatomic) bool usePipelining;
@property (nonatomic) NSURLRequestCachePolicy defaultCachePolicy;
@property (nonatomic) NSTimeInterval requestTimeout;
@property (nonatomic) NSTimeInterval resourceTimeout;
@property (nonatomic) bool waitForConnectivity;
@property (nonatomic) MaplyURLSessionPriority priority;

@property (nonatomic) bool logRequestMetrics;

- (void)reset;

+ (instancetype)sharedManager;

@end

NS_ASSUME_NONNULL_END
