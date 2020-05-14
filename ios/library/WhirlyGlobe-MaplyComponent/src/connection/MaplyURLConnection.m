//
//  MaplyURLConnection.m
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 5/12/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//
#import <Security/Security.h>
#import "MaplyURLConnection.h"
#import "MaplyHttpManager+Private.h"
@interface MaplyURLConnection() <NSURLSessionDelegate>

@property (nonatomic,strong) NSObject<OS_dispatch_queue> *  queue;
@property (nonatomic,strong) NSURLSession *  session;
@property (nonatomic,strong) NSCondition *  condition;
@property (nonatomic,strong) NSURLSessionDataTask *  dataTask;
@property (nonatomic,strong) NSMutableData *  dataReceived;
@property (nonatomic,strong) NSURLResponse *  response;

@property (nonatomic,copy) void (^completion)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error);



@end

@implementation MaplyURLConnection
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.queue = dispatch_queue_create("connection_queue_wg", NULL);
        self.condition = [[NSCondition alloc] init];
    }
    return self;
}
- (NSMutableData *)dataReceived{
    if(_dataReceived == nil){
        _dataReceived = [NSMutableData data];
    }
    return _dataReceived;
}
-(NSURLSession*)session{
    if(_session){
        return _session;
    }
    NSURLSessionConfiguration * configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
  
    
    _session = [NSURLSession sessionWithConfiguration:configuration
                                                delegate:self
                                           delegateQueue:[NSOperationQueue mainQueue]];
    
  
    return _session;
}

-(void)resume{
    [self.dataTask resume];
}
-(void)cancel{
    [self.dataTask cancel];
}

   
 - (void)asyncRequest:(NSURLRequest *)request
          completion:(nonnull void (^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
     
     
     self.completion = completion;
     self.dataTask = [self.session dataTaskWithRequest:request completionHandler:completion];
     
 }
  
- (void)syncRequest:(NSURLRequest *)request
         completion:(nonnull void (^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
    
    
    
    self.completion = completion;
    self.dataTask = [self.session dataTaskWithRequest:request completionHandler:completion];    
    [self.condition wait];
}



#pragma mark - NSURLSessionDelegate
- (void)URLSession:(NSURLSession *)session
didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
 completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * __nullable credential))completionHandler
{
    [MaplyHttpManager.sharedInstance handelChanllenge:challenge completionHandler:completionHandler];
}


- (void)URLSession:(NSURLSession *)session dataTask:(MaplyURLConnection *)dataTask didReceiveResponse:(NSURLResponse *)response
completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler {
    
    self.response = response;
    
    completionHandler(NSURLSessionResponseAllow);
    
}

-(void)URLSession:(NSURLSession *)session
         dataTask:(MaplyURLConnection *)dataTask
   didReceiveData:(NSData *)data {
    
    [self.dataReceived appendData:data];
    
    
}


- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    if(error){
        self.completion(nil,self.response,error);
    }else{
        self.completion(self.dataReceived.copy,self.response,nil);
    }
    [self.condition signal];
}

-(void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didSendBodyData:(int64_t)bytesSent totalBytesSent:(int64_t)totalBytesSent totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend

{

   
    
}

@end
