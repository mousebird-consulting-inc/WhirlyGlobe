/*
 *  Platform.h
 *  WhirlyGlobeLib
 *
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/include/Platform.h
 *  Created by Steve Gifford on 12/13/13.
=======
 *  Created by Steve Gifford on 5/11/11.
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/NSString+Stuff.mm
 *  Copyright 2011-2013 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <ctime>
#import <vector>
#import "WhirlyTypes.h"

namespace WhirlyKit
{

<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/include/Platform.h
// Wrapper for platform specific time function
extern TimeInterval TimeGetCurrent();
    
// Retina vs. not on iOS.  Always 1.0 on Android
extern float DeviceScreenScale();
    
=======
// Courtesy: http://stackoverflow.com/questions/3552195/how-to-convert-stdstring-to-nsstring
+(NSString*) stringWithwstring:(const std::wstring&)ws
{
    char* data = (char*)ws.data();
    unsigned size = ws.size() * sizeof(wchar_t);
    
    NSString* result = [[NSString alloc] initWithBytes:data length:size encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)];
    return result;
}

@end

// A function we can call to force the linker to bring in categories
void NSStringDummyFunc()
{
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/NSString+Stuff.mm
}
