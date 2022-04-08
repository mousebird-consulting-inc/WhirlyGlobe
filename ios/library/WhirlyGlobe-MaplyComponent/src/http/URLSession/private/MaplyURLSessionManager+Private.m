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
     
    
    NSURLSession * session = [NSURLSession sessionWithConfiguration:NSURLSessionConfiguration.defaultSessionConfiguration
                                                           delegate:(id <NSURLSessionDelegate>)self
                                                      delegateQueue:NSOperationQueue.mainQueue];
    
    
    return session;
}

@end
