//
//  SecurityUtl.h
//  Pods
//
//  Created by Ryan Pearl on 10/7/15.
//
//

#ifndef SecurityUtl_h
#define SecurityUtl_h

@interface SecurityUtil : NSObject
+ (int)readLength:(uint32_t *)length fromBuffer:(const uint8_t *)buf ofLength:(const uint32_t)bufLen atIndex:(uint32_t *)idx;
+ (CFDataRef) copySerialFromCertificate:(const uint8_t *)derdata certificateLength:(const CFIndex)certlen;
+ (bool)isRevokedCertificate:(SecCertificateRef)certificate;
+ (NSArray *)rootCertificates;
@end
#endif /* SecurityUtl_h */
