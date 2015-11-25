/*  Shader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/216/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_Shader.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;
using namespace Eigen;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_nativeInit
  (JNIEnv *env, jclass cls)
{
	OpenGLES2ProgramClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_initialise
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

		OpenGLES2Program *prog = new OpenGLES2Program(name,vertProg,fragProg);
		OpenGLES2ProgramClassInfo::getClassInfo()->setHandle(env,obj,prog);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_valid
  (JNIEnv *env, jobject obj)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		return inst != NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::valid()");
	}
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_Shader_getName
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env,obj);
        return env->NewStringUTF(inst->getName().c_str());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::getName()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shader_addTextureNative
(JNIEnv *env, jobject obj, jobject sceneObj, jstring nameStr, jlong texID)
{
    try
    {
        OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
        OpenGLES2Program *inst = classInfo->getObject(env,obj);
	Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        if (!inst || !scene)
            return;
        
        glUseProgram(inst->getProgram());

        JavaString name(env,nameStr);

	TextureBase *tex = scene->getTexture(texID);
	if (tex && tex->getGLId() != 0)
	{
	  inst->setTexture(name.cStr,tex->getGLId());
	} else {
          __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Missing texture or GL ID in Shader::addTextureNative(), texID = %d, numTextures = %d",(int)texID,scene->textures.size());
	  if (tex)
	    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Missing GL ID in Shader::addTextureNative()");	    
	}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::addTextureNative()");
    }
}


JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2D
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble uni)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram(inst->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		inst->setUniform(name,(float)uni);
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::setUniform()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring nameStr, jint uni)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram(inst->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		inst->setUniform(name,(int)uni);
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::setUniform()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;

        glUseProgram(inst->getProgram());
		
        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		inst->setUniform(name,Vector2f((float)x,(float)y));
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::setUniform()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DDD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y, jdouble z)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram(inst->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		inst->setUniform(name,Vector3f((float)x,(float)y,(float)z));
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::setUniform()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shader_setUniformNative__Ljava_lang_String_2DDDD
  (JNIEnv *env, jobject obj, jstring nameStr, jdouble x, jdouble y, jdouble z, jdouble w)
{
	try
	{
		OpenGLES2ProgramClassInfo *classInfo = OpenGLES2ProgramClassInfo::getClassInfo();
		OpenGLES2Program *inst = classInfo->getObject(env,obj);
		if (!inst)
			return false;
        
        glUseProgram(inst->getProgram());

        const char *cName = env->GetStringUTFChars(nameStr,0);
		std::string name = cName;
		env->ReleaseStringUTFChars(nameStr, cName);

		inst->setUniform(name,Vector4f((float)x,(float)y,(float)z,(float)w));
		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shader::setUniform()");
	}
}
