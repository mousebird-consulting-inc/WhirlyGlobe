/*  Shader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/216/15.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <jni.h>
#import "Scene_jni.h"
#import "com_mousebird_maply_Shader.h"

using namespace WhirlyKit;
using namespace Maply;
using namespace Eigen;

template<> ShaderClassInfo *ShaderClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_nativeInit(JNIEnv *env, jclass cls)
{
	ShaderClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_initialise__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring nameStr, jstring vertStr, jstring fragStr)
{
	try
	{
		// Convert the strings
		const char *cNameStr = env->GetStringUTFChars(nameStr,0);
		const char *cVertStr = env->GetStringUTFChars(vertStr,0);
		const char *cFragStr = env->GetStringUTFChars(fragStr,0);
		std::string name = cNameStr, vertProg = cVertStr, fragProg = cFragStr;
		env->ReleaseStringUTFChars(nameStr, cNameStr);
		env->ReleaseStringUTFChars(vertStr, cVertStr);
		env->ReleaseStringUTFChars(fragStr, cFragStr);

		Shader_AndroidRef *shader = new Shader_AndroidRef(new Shader_Android());
		(*shader)->setupProgram(name,vertProg,fragProg);
		ShaderClassInfo::getClassInfo()->setHandle(env,obj,shader);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_initialise__(JNIEnv *env, jobject obj)
{
    try
    {
        Shader_AndroidRef *shader = new Shader_AndroidRef(std::make_shared<Shader_Android>());
        ShaderClassInfo::getClassInfo()->setHandle(env,obj,shader);
    }
	MAPLY_STD_JNI_CATCH()
}

jobject MakeShader(JNIEnv *env,Shader_AndroidRef shader)
{
	ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo(env,"com/mousebird/maply/Shader");
	return classInfo->makeWrapperObject(env,new Shader_AndroidRef(std::move(shader)));
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_delayedSetupNative
  (JNIEnv *env, jobject obj, jstring nameStr, jstring vertStr, jstring fragStr)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *shader = classInfo->getObject(env,obj);
		if (!shader)
            return;

		// Convert the strings
		const char *cNameStr = env->GetStringUTFChars(nameStr,0);
		const char *cVertStr = env->GetStringUTFChars(vertStr,0);
		const char *cFragStr = env->GetStringUTFChars(fragStr,0);
		std::string name = cNameStr, vertProg = cVertStr, fragProg = cFragStr;
		env->ReleaseStringUTFChars(nameStr, cNameStr);
		env->ReleaseStringUTFChars(vertStr, cVertStr);
		env->ReleaseStringUTFChars(fragStr, cFragStr);

		(*shader)->setupProgram(name,vertProg,fragProg);
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
			Shader_AndroidRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_valid(JNIEnv *env, jobject obj)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
            return false;
        return (*inst)->prog->isValid();
	}
	MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_Shader_getError(JNIEnv *env, jobject obj)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		return env->NewStringUTF((*inst)->prog->getName().c_str());
	}
	MAPLY_STD_JNI_CATCH()
	return nullptr;
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_Shader_getName(JNIEnv *env, jobject obj)
{
    try
    {
        ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
        return env->NewStringUTF((*inst)->prog->getName().c_str());
    }
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Shader_getID
  (JNIEnv *env, jobject obj)
{
    try
    {
        ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
		    return EmptyIdentity;
        return (*inst)->prog->getId();
    }
	MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_addTextureNative
  (JNIEnv *env, jobject obj, jobject changeSetObj, jstring nameStr, jlong texID)
{
    try
    {
        ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!inst || !changes)
            return;
        
        JavaString name(env,nameStr);
        // Do this on the rendering thread so we don't get ahead of ourselves
        const auto idx = StringIndexer::getStringID(name.getCString());
		(*changes)->push_back(new ShaderAddTextureReq((*inst)->prog->getId(),idx,texID,-1));
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2F
  (JNIEnv *env, jobject obj, jstring nameStr, jfloat uni)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

		glUseProgram((*inst)->prog->getProgram());

		const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),uni);
		return true;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2D
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble uni)
{
	return Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2F(env, obj, nameStr, (jfloat)uni);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformByIndexNative__Ljava_lang_String_2FI
		(JNIEnv *env, jobject obj, jstring nameStr, jfloat uni, jint index)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

		glUseProgram((*inst)->prog->getProgram());

		const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),uni,index);
		return true;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformByIndexNative__Ljava_lang_String_2DI
		(JNIEnv *env, jobject obj, jstring nameStr, jdouble uni, jint index)
{
	return Java_com_mousebird_maply_Shader_setUniformByIndexNative__Ljava_lang_String_2FI(env, obj, nameStr, (float)uni, index);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring nameStr, jint uni)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram((*inst)->prog->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),(int)uni);
		return true;
	}
	MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2Z
		(JNIEnv *env, jobject obj, jstring nameStr, jboolean uni)
{
	return Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2I(env, obj, nameStr, uni);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

        glUseProgram((*inst)->prog->getProgram());
		
        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),Vector2f((float)x,(float)y));
		return true;
	}
	MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DDD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y, jdouble z)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram((*inst)->prog->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),Vector3f((float)x,(float)y,(float)z));
		return true;
	}
	MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DDDD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y, jdouble z, jdouble w)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram((*inst)->prog->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->prog->setUniform(StringIndexer::getStringID(name),Vector4f((float)x,(float)y,(float)z,(float)w));
		return true;
	}
	MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformColorByIndexNative
		(JNIEnv *env, jobject obj, jstring nameStr, jint colorInt, jint index)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

		glUseProgram((*inst)->prog->getProgram());

		const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		RGBAColor color = RGBAColor::FromInt(colorInt);
		float colors[4];
		color.asUnitFloats(colors);
		Eigen::Vector4f colorVec(colors[0],colors[1],colors[2],colors[3]);
		(*inst)->prog->setUniform(StringIndexer::getStringID(name),colorVec,index);
		return true;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformColorNative
		(JNIEnv *env, jobject obj, jstring nameStr, jint colorInt)
{
	try
	{
		ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

		glUseProgram((*inst)->prog->getProgram());

		const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

        RGBAColor color = RGBAColor::FromInt(colorInt);
		float colors[4];
		color.asUnitFloats(colors);
		Eigen::Vector4f colorVec(colors[0],colors[1],colors[2],colors[3]);
		(*inst)->prog->setUniform(StringIndexer::getStringID(name),colorVec);
		return true;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_addVarying(JNIEnv *env, jobject obj, jstring nameStr)
{
    try
    {
        ShaderClassInfo *classInfo = ShaderClassInfo::getClassInfo();
		Shader_AndroidRef *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;

        const char *cName = env->GetStringUTFChars(nameStr,0);
        std::string name = cName;
        env->ReleaseStringUTFChars(nameStr, cName);

		(*inst)->varyings.push_back(name);
        return;
    }
	MAPLY_STD_JNI_CATCH()
    return;
}