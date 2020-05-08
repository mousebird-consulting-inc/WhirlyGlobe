/*
 *  LabelInfoAndroid.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
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

#ifdef __ANDROID__
#import <jni.h>
#endif
#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

/**
 * Android version of the Label Info class.
 * We put Typeface in here for convenience.
 */
class LabelInfoAndroid : public LabelInfo
{
public:
	LabelInfoAndroid(bool screenObject);
	LabelInfoAndroid(const LabelInfoAndroid &that);

	// Clear any global refs we may be holding
	void clearRefs(PlatformInfo_Android *threadInfo);

	// Add the typeface to the label info.  Needed for rendering
	void setTypeface(PlatformInfo_Android *threadInfo,jobject typefaceObj);

	// Compare typefaces
	bool typefaceIsSame(PlatformInfo_Android *threadInfo,const jobject inTypeface) const;

	// Globe reference to typeface object
	jobject typefaceObj;

	// Font size
	float fontSize;

	// Global object pointing to labelInfo on Java side
	jobject labelInfoObj;
};
typedef std::shared_ptr<LabelInfoAndroid> LabelInfoAndroidRef;

}
