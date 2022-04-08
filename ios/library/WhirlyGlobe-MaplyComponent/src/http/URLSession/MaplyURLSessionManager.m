//
//  MaplyURLSessionManager.m
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 17/2/2022.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import "MaplyURLSessionManager.h"

static MaplyURLSessionManager * sharedManager = nil;

@interface MaplyURLSessionManager () <NSURLSessionDelegate>

@end

@implementation MaplyURLSessionManager

+ (instancetype)sharedManager
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (sharedManager == nil){
            sharedManager = [[self alloc] init];
        }
    });
    return sharedManager;
}




- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler{
    
    id <MaplyURLSessionManagerDelegate> delegate = self.sessionManagerDelegate;
    
    if ([delegate respondsToSelector:@selector(didReceiveChallenge:completionHandler:)]){
        [delegate didReceiveChallenge:challenge completionHandler:completionHandler];
    }else{
        NSURLCredential *credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, credential);
    }
}
@end
