/*
 *  MaplyTapMessage.mm
 *  WhirlyGlobeLib
 *
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/MaplyTapMessage.mm
 *  Created by Steve Gifford on 9/19/11.
=======
 *  Created by Steve Gifford on 5/11/11.
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/include/NSString+Stuff.h
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

#import "MaplyTapMessage.h"

@implementation MaplyTapMessage

<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/MaplyTapMessage.mm
=======
/// Build an NSString from a std::wstring
+(NSString*) stringWithwstring:(const std::wstring&)ws;

/// Convert and return a std::string
- (std::string) asStdString;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/include/NSString+Stuff.h

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void NSStringDummyFunc();
