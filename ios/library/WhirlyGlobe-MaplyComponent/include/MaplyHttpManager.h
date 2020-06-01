//
//  MaplyHttpManager.h
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 5/12/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
@protocol MaplyHttpManagerDelegate <NSObject>

/**
    this method will be called every time when a  new http request is started to allow the user to trust or not the server certificate
    after treating the certificate you should call the completionHandler block with the appropriate parameter.
*/

-(void)shouldTrustAuthenticationChallenge:(NSURLAuthenticationChallenge*)challenge
                        completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition,
                                                    NSURLCredential * __nullable credential))completionHandler;

@end

@interface MaplyHttpManager : NSObject

+(instancetype)sharedInstance;

// this the MaplyHttpManager delegate
@property (weak,nonatomic) id <MaplyHttpManagerDelegate> delegate;



@end

NS_ASSUME_NONNULL_END
