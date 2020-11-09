/*
 *  WhirlyKitLog.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2019 mousebird consulting
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

#ifndef WhirlyKitLog_h
#define WhirlyKitLog_h

typedef enum {Verbose=0,Debug,Info,Warn,Error} WKLogLevel;

// Wrapper around NSLog on iOS.  Other things on other platforms
extern void wkLog(const char *formatStr,...);

// Set, e.g., WK_MIN_LOG_LEVEL=1 to override
#if !defined(WK_MIN_LOG_LEVEL)
# if DEBUG
#  define WK_MIN_LOG_LEVEL WKLogLevel::Verbose
# else
#  define WK_MIN_LOG_LEVEL WKLogLevel::Info
# endif
#endif

// Skip logging calls below the configured level.
// The extra do/while makes it safe to use within if/else conditionals.
// Note that `level` is evaluated twice, watch out for side-effects.
#define wkLogLevel(level, formatStr...) do {if ((level) >= (WK_MIN_LOG_LEVEL)) { wkLogLevel_((level), formatStr); }} while(0)
extern void wkLogLevel_(WKLogLevel level,const char *formatStr,...);

#endif
