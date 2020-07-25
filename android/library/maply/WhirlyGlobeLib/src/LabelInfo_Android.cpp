/*
 *  LabelInfoAndroid.cpp
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

#import "LabelInfo_Android.h"

namespace WhirlyKit
{

LabelInfoAndroid::LabelInfoAndroid(bool screenObject)
: LabelInfo(screenObject), typefaceObj(NULL), labelInfoObj(NULL)
{
}

LabelInfoAndroid::LabelInfoAndroid(const LabelInfoAndroid &that)
: LabelInfo(that), typefaceObj(that.typefaceObj), fontSize(that.fontSize), labelInfoObj(that.labelInfoObj)
{
}

void LabelInfoAndroid::clearRefs(PlatformInfo_Android *threadInfo)
{
	if (typefaceObj)
	{
		threadInfo->env->DeleteGlobalRef(typefaceObj);
		typefaceObj = NULL;
	}
}

bool LabelInfoAndroid::typefaceIsSame(PlatformInfo_Android *threadInfo,const jobject inTypeface) const
{
	// Obviously true here
	if (inTypeface == typefaceObj)
		return true;

	// Now for a deeper comparison
	jclass typefaceClass = threadInfo->env->GetObjectClass(inTypeface);
	jmethodID jmethodID = threadInfo->env->GetMethodID(typefaceClass, "equals", "(Ljava/lang/Object;)Z");
	bool res = threadInfo->env->CallBooleanMethod(typefaceObj,jmethodID,inTypeface);
    threadInfo->env->DeleteLocalRef(typefaceClass);

	return res;
}

void LabelInfoAndroid::setTypeface(PlatformInfo_Android *threadInfo,jobject inTypefaceObj)
{
	if (typefaceObj)
		clearRefs(threadInfo);

	if (inTypefaceObj)
		typefaceObj = threadInfo->env->NewGlobalRef(inTypefaceObj);
}

}
