//
//  MaplyURLSessionManager+Private.m
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 18/3/2022.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import "MaplyURLSessionManager+Private.h"

@implementation MaplyURLSessionManager (Private)

-(NSURLSession*)createURLSession{
     
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    
    if (@available(iOS 13, tvOS 13, watchOS 6, macOS 10.15, *)) {
        configuration.TLSMinimumSupportedProtocolVersion = tls_protocol_version_TLSv12;
    } else {
        configuration.TLSMinimumSupportedProtocol = kTLSProtocol12;
    }
    
    NSURLSession * session = [NSURLSession sessionWithConfiguration:configuration
                                                           delegate:(id <NSURLSessionDelegate>)self
                                                      delegateQueue:NSOperationQueue.mainQueue];
    
    return session;
}

@end
