#import <Foundation/Foundation.h>
#import <libxml/tree.h>


@interface NSString (DDXML)

/**
 * xmlChar - A basic replacement for char, a byte in a UTF-8 encoded string.
**/
- (const xmlChar *)xmlChar;

- (NSString *)stringByTrimming;

@end

// Used to pull in category selectors
#ifdef __cplusplus
extern "C"
#endif
void DDXMLDummyFunc();
