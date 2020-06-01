//
//  MaplyHttpManager.m
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 5/12/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#import "MaplyHttpManager.h"
#import "MaplyURLConnection.h"
static MaplyHttpManager * sharedInstance;

@interface MaplyHttpManager () <NSURLSessionDelegate>
@property (strong,nonatomic) NSMutableArray <MaplyURLConnection*> * requests;
@property (strong,nonatomic) NSLock * lock;
@property (nonatomic,strong) NSURLSession *  session;
@end
@implementation MaplyHttpManager

+ (instancetype)sharedInstance{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if(sharedInstance == nil){
            sharedInstance = [[MaplyHttpManager alloc] init];
            [sharedInstance configure];
        }
    });
    
    return sharedInstance;
}
-(void)configure{
    sharedInstance.lock = [[NSLock alloc] init];
    self.requests = [NSMutableArray array];
    NSURLSessionConfiguration * configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
     
       
    self.session = [NSURLSession sessionWithConfiguration:configuration
                                                   delegate:self
                                              delegateQueue:[NSOperationQueue mainQueue]];
}

-(void)handelChanllenge:(NSURLAuthenticationChallenge*)challenge
completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition,
                            NSURLCredential * __nullable credential))completionHandler{
    
    if([self.delegate respondsToSelector:@selector(shouldTrustAuthenticationChallenge:completionHandler:)]){
        [self.delegate shouldTrustAuthenticationChallenge:challenge completionHandler:completionHandler];
    }else{

        NSURLCredential *newCredential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        [challenge.sender useCredential:newCredential forAuthenticationChallenge:challenge];
        completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, newCredential);
    }
    
}

-(MaplyURLConnection*)syncRequest:(NSURLRequest*)request
        completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
    
    MaplyURLConnection * connection = [[MaplyURLConnection alloc] init];
    [self.lock lock];
    [self.requests addObject:connection];
    [self.lock unlock];
    [connection syncRequest:request session:self.session completion:^(NSData * data, NSURLResponse * response, NSError * error) {
    
        if(completion){
            completion(data,response,error);
        }
        [self.lock lock];
        [self.requests removeObject:connection];
        [self.lock unlock];
    }];
    
    return connection;
}

-(MaplyURLConnection*)asyncRequest:(NSURLRequest*)request
         completion:(void(^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
 
    MaplyURLConnection * connection = [[MaplyURLConnection alloc] init];
    [self.lock lock];
    [self.requests addObject:connection];
    [self.lock unlock];
    [connection asyncRequest:request session:self.session completion:^(NSData * data, NSURLResponse * response, NSError * error) {
    
        if(completion){
            completion(data,response,error);
        }
        [self.lock lock];
        [self.requests removeObject:connection];
        [self.lock unlock];
    }];
    
    return connection;
}



#pragma mark - NSURLSessionDelegate
- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * __nullable credential))completionHandler
{
    [self handelChanllenge:challenge completionHandler:completionHandler];
}



@end
