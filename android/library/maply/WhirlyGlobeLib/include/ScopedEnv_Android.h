/*  ScopedEnv_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 3/4/2021
 *  Copyright 2021-2021 mousebird consulting
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

namespace WhirlyKit
{

/**
 * Attach the current thread to the JNI environment for the duration of the object's lifetime
 */
struct ScopedEnv
{
    ScopedEnv(JavaVM *jvm)
        : jvm(jvm), env(nullptr), attached(false)
    {
        std::tie(env, attached) = GetJniEnv(jvm);
    }

    ScopedEnv(const ScopedEnv&) = delete;
    ScopedEnv& operator=(const ScopedEnv&) = delete;

    ScopedEnv(ScopedEnv&& other) :
        jvm(other.jvm), env(other.env), attached(other.attached)
    {
        other.env = nullptr;
        other.attached = false;
    }
    ScopedEnv& operator=(ScopedEnv&& other)
    {
        if (this != &other)
        {
            jvm = other.jvm;
            env = other.env;
            attached = other.attached;
            other.attached = false;
        }
        return *this;
    }

    virtual ~ScopedEnv()
    {
        if (attached)
        {
            attached = false;
            env = nullptr;
            jvm->DetachCurrentThread();
        }
    }

    operator JNIEnv *() const { return env; }
    JNIEnv* operator->() const { return env; }

    static std::pair<JNIEnv*,bool> GetJniEnv(JavaVM *vm)
    {
        // Check if the current thread is attached to the VM
        JNIEnv* env = nullptr;
        const auto result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (result == JNI_OK)
        {
            return std::make_pair(env,false);
        }
        if (result == JNI_EDETACHED &&
            vm->AttachCurrentThread(&env, nullptr) == JNI_OK)
        {
            return std::make_pair(env, true);
        }
        return std::make_pair(nullptr, false);
    }
private:
    bool attached;
    JavaVM *jvm;
    JNIEnv *env;
};

}
