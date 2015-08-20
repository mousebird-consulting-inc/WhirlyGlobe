/*
 *  LabelInfoAndroid.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "LabelInfoAndroid.h"

namespace WhirlyKit
{

LabelInfoAndroid::LabelInfoAndroid(const Dictionary &dict)
: LabelInfo(dict), typefaceObj(NULL), env(NULL), labelInfoObj(NULL)
{
}

void LabelInfoAndroid::clearRefs(JNIEnv *env)
{
	if (typefaceObj)
	{
		env->DeleteGlobalRef(typefaceObj);
		typefaceObj = NULL;
	}
}

bool LabelInfoAndroid::typefaceIsSame(const jobject inTypeface) const
{
	// Obviously true here
	if (inTypeface == typefaceObj)
		return true;

	// Now for a deeper comparison
	jclass typefaceClass = env->GetObjectClass(inTypeface);
	jmethodID jmethodID = env->GetMethodID(typefaceClass, "equals", "(Ljava/lang/Object;)Z");
	bool res = env->CallBooleanMethod(typefaceObj,jmethodID,inTypeface);

	return res;
}

void LabelInfoAndroid::setTypeface(JNIEnv *env,jobject inTypefaceObj)
{
	if (typefaceObj)
		clearRefs(env);

	if (inTypefaceObj)
		typefaceObj = env->NewGlobalRef(inTypefaceObj);
}

}
