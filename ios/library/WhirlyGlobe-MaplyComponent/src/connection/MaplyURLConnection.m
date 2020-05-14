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
@interface MaplyURLConnection()

@property (nonatomic,strong) NSCondition *  condition;
@property (nonatomic,strong) NSURLSessionDataTask *  dataTask;
@property (nonatomic,strong) NSURLResponse *  response;



@end

@implementation MaplyURLConnection
- (instancetype)init
{
    self = [super init];
    if (self) {
        self.condition = [[NSCondition alloc] init];
    }
    return self;
}


-(void)resume{
    [self.dataTask resume];
}
-(void)cancel{
    [self.dataTask cancel];
}

   
 - (void)asyncRequest:(NSURLRequest *)request
              session:(NSURLSession*)session
          completion:(nonnull void (^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
     
     
     
     self.dataTask = [session dataTaskWithRequest:request completionHandler:completion];
     
 }
  
- (void)syncRequest:(NSURLRequest *)request
            session:(NSURLSession*)session
         completion:(nonnull void (^)(NSData * _Nullable, NSURLResponse *  _Nullable response, NSError * _Nullable error))completion{
    
    
    
    
    self.dataTask = [session dataTaskWithRequest:request
                                    completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        
        if(completion){
            completion(data,response,error);
        }
        [self.condition signal];
    }];
    [self.condition wait];
}






@end
