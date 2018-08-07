#import <Foundation/Foundation.h>

// Don't trigger this if we've already seen this typdef
#ifndef __XML_STRING_H__
// We redefine xmlChar to avoid a non-modular include
typedef unsigned char xmlChar;
#endif

NS_ASSUME_NONNULL_BEGIN
@interface NSString (DDXML)

/**
 * xmlChar - A basic replacement for char, a byte in a UTF-8 encoded string.
**/
- (const xmlChar *)xmlChar;

- (NSString *)stringByTrimming;

@end
NS_ASSUME_NONNULL_END

// Used to pull in category selectors
#ifdef __cplusplus
extern "C"
#endif
void DDXMLDummyFunc(void);
