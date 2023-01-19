/*  LayoutManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
#import "Maply_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_LayoutManager.h"
#import "WhirlyGlobeLib.h"
#import <Exceptions_jni.h>

using namespace WhirlyKit;
using namespace Maply;

class LayoutManagerWrapper;
using LayoutManagerWrapperRef = std::shared_ptr<LayoutManagerWrapper>;

// Wrapper that tracks the generator as well
class LayoutManagerWrapper : public ClusterGenerator
{
public:
    
    // Used to keep track of cluster objects for callbacks
    struct ClusterInfo
    {
        explicit ClusterInfo(int clusterID = 0) :
            clusterID(clusterID),
            layoutSize(0, 0)
        {
        }

        // Move only
        ClusterInfo(const ClusterInfo &) = delete;
        ClusterInfo(ClusterInfo &&other) noexcept { operator=(std::move(other)); }
        ~ClusterInfo()
        {
            if (clusterObj)
            {
                wkLogLevel(Warn, "ClusterInfo not cleaned up");
            }
        }

        bool operator <(const ClusterInfo &that) const { return clusterID < that.clusterID; }

        // Move only
        ClusterInfo& operator =(const ClusterInfo &) = delete;
        ClusterInfo& operator =(ClusterInfo &&other) noexcept
        {
            if (this != &other)
            {
                if (clusterObj)
                {
                    // Replacing this one leaks it, we need a JNI env to release it
                    wkLogLevel(Warn, "ClusterInfo not cleaned up");
                }

                clusterID                = other.clusterID;
                layoutSize               = other.layoutSize;
                clusterObj               = other.clusterObj;
                clusterObjClass          = other.clusterObjClass;
                selectable               = other.selectable;
                startClusterGroupJava    = other.startClusterGroupJava;
                makeClusterGroupJNIJava  = other.makeClusterGroupJNIJava;
                endClusterGroupJava      = other.endClusterGroupJava;
                shutdownClusterGroupJava = other.shutdownClusterGroupJava;

                other.clusterObj = nullptr;
                other.clusterObjClass = nullptr;
            }
            return *this;
        }

        bool init(JNIEnv *env,int inClusterID,const Point2d &inLayoutSize,jobject inClusterObj)
        {
            clusterID = inClusterID;
            layoutSize = inLayoutSize;

            logAndClearJVMException(env, "ClusterInfo::init");

            if (clusterObj)
            {
                wkLogLevel(Warn, "ClusterInfo already initialized");
                env->DeleteGlobalRef(clusterObj);
                clusterObj = nullptr;
            }
            if (clusterObjClass)
            {
                env->DeleteGlobalRef(clusterObjClass);
                clusterObjClass = nullptr;
            }

            clusterObj = inClusterObj ? env->NewGlobalRef(inClusterObj) : nullptr;
            if (logAndClearJVMException(env) || !clusterObj)
            {
                wkLogLevel(Error, "Bad cluster object");
                return false;
            }

            // Retain a global ref to the class to keep it from being unloaded.
            // This shouldn't be necessary as the object ref should keep the class loaded,
            // but we keep getting unexplained MethodNotFound exceptions here.
            const jclass theClass = env->GetObjectClass(clusterObj);
            if (logAndClearJVMException(env) || !theClass)
            {
                wkLogLevel(Error, "Bad cluster object class");
                clear(env);
                return false;
            }
            jclass classRef = (jclass)env->NewGlobalRef(theClass);
            if (logAndClearJVMException(env) || !classRef)
            {
                wkLogLevel(Error, "Bad cluster object class ref");
                clear(env);
                return false;
            }

            clusterObjClass = classRef;

            makeClusterGroupJNIJava = env->GetMethodID(theClass, "makeClusterGroupJNI","(I[Ljava/lang/String;)J");
            if (logAndClearJVMException(env) || !makeClusterGroupJNIJava)
            {
                wkLogLevel(Error, "Failed to find ClusterGenerator::makeClusterGroupJNI");
                if (jclass classClass = env->GetObjectClass(theClass))
                {
                    if (jmethodID methodId = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;"))
                    {
                        if (auto className = JavaString(env, (jstring)env->CallObjectMethod(theClass, methodId)))
                        {
                            wkLogLevel(Error, "Failed to find %s::makeClusterGroupJNI", className.getCString());
                        }
                    }
                }

                // Look up the base class instead of using the provided object's class.
                // According to the JNI documentation, this should make no difference.
                classRef = env->FindClass("com/mousebird/maply/ClusterGenerator");
                if (logAndClearJVMException(env) || !classRef) {
                    wkLogLevel(Error, "Failed to find ClusterGenerator class");
                    clear(env);
                    return false;
                }
                // Try again
                makeClusterGroupJNIJava = env->GetMethodID(theClass, "makeClusterGroupJNI","(I[Ljava/lang/String;)J");
                if (logAndClearJVMException(env) || !makeClusterGroupJNIJava)
                {
                    wkLogLevel(Error, "Failed to find ClusterGenerator::makeClusterGroupJNIJava");
                    clear(env);
                    return false;
                }
            }
            startClusterGroupJava = env->GetMethodID(theClass, "startClusterGroup", "()V");
            if (logAndClearJVMException(env) || !startClusterGroupJava)
            {
                wkLogLevel(Error, "Failed to find ClusterGenerator::makeClusterGroupJNI");
                clear(env);
                return false;
            }
            endClusterGroupJava = env->GetMethodID(theClass, "endClusterGroup", "()V");
            if (logAndClearJVMException(env) || !endClusterGroupJava)
            {
                wkLogLevel(Error, "Failed to find ClusterGenerator::endClusterGroup");
                clear(env);
                return false;
            }
            shutdownClusterGroupJava = env->GetMethodID(theClass, "shutdown", "()V");
            if (logAndClearJVMException(env) || !shutdownClusterGroupJava)
            {
                wkLogLevel(Error, "Failed to find ClusterGenerator::shutdown");
                clear(env);
                return false;
            }
            env->DeleteLocalRef(theClass);
            return true;
        }
        
        void clear(JNIEnv *env)
        {
            if (clusterObj)
            {
                if (shutdownClusterGroupJava)
                {
                    env->CallVoidMethod(clusterObj, shutdownClusterGroupJava);
                    logAndClearJVMException(env);
                }
                env->DeleteGlobalRef(clusterObj);
                clusterObj = nullptr;
            }
            if (clusterObjClass)
            {
                env->DeleteGlobalRef(clusterObjClass);
                clusterObjClass = nullptr;
            }
        }

        int clusterID = 0;
        Point2d layoutSize;
        jobject clusterObj = nullptr;
        jclass clusterObjClass = nullptr;
        bool selectable = false;

        // Methods we'll use to call into the Java cluster generator
        jmethodID startClusterGroupJava = nullptr;
        jmethodID makeClusterGroupJNIJava = nullptr;
        jmethodID endClusterGroupJava = nullptr;
        jmethodID shutdownClusterGroupJava = nullptr;

    private:
        friend class LayoutManagerWrapper;
    };
    typedef std::set<ClusterInfo> ClusterInfoSet;

    LayoutManagerWrapper(PlatformThreadInfo *, LayoutManagerRef layoutManager)
        : layoutManager(std::move(layoutManager))
    {
    }

    ~LayoutManagerWrapper() override = default;

    void updateShader()
    {
        if (motionShaderID == EmptyIdentity)
        {
            ProgramGLES *program = (ProgramGLES *)layoutManager->getScene()->findProgramByName(MaplyScreenSpaceDefaultMotionShader);
            if (program)
            {
                motionShaderID = program->getId();
                generatorChanges = true;
            }
        }
    }

    // Add a cluster generator for callback
    bool addClusterGenerator(JNIEnv* env, jobject clusterObj, jint clusterID,bool selectable, double sizeX, double sizeY)
    {
        if (!clusterObj)
        {
            return false;
        }

        ClusterInfo clusterInfo;
        if (!clusterInfo.init(env,clusterID,Point2d(sizeX,sizeY),clusterObj))
        {
            return false;
        }
        clusterInfo.selectable = selectable;

        const auto hit = clusterGens.find(clusterInfo);
        if (hit != clusterGens.end())
        {
            // Already exists, we need to clean up the old one
            const_cast<ClusterInfo&>(*hit).clear(env);
            clusterGens.erase(hit);
        }

        clusterGens.insert(std::move(clusterInfo));
        generatorChanges = true;

        return true;
    }

    const ClusterInfo* getClusterGenerator(JNIEnv*, int clusterID)
    {
        ClusterInfo temp(clusterID);
        const auto hit = clusterGens.find(temp);
        return (hit != clusterGens.end()) ? &*hit : nullptr;
    }

    bool removeClusterGenerator(JNIEnv* env, int clusterID)
    {
        ClusterInfo temp(clusterID);
        const auto hit = clusterGens.find(temp);
        if (hit != clusterGens.end()) {
            const_cast<ClusterInfo&>(*hit).clear(env);
            clusterGens.erase(hit);
            generatorChanges = true;
            return true;
        }
        return false;
    }

    void clearClusterGenerators(JNIEnv* env)
    {
        for (auto &ci : clusterGens)
        {
            // why TF does this produce a const ref?
            // (answer: std::set has `typedef typename __base::const_iterator iterator`, bug?)
            const_cast<ClusterInfo&>(ci).clear(env);
        }
        clusterGens.clear();
        generatorChanges = true;
    }

    /** ClusterGenerator virtual methods.
      */
    // Called right before we start generating layout objects
    void startLayoutObjects(PlatformThreadInfo *threadInfo) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        //oldClusterTex = currentClusterTex;
        currentClusterTex.clear();

        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
        {
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.startClusterGroupJava);
        }
    }

    // Ask the appropriate cluster generator to make a cluster image
    void makeLayoutObject(PlatformThreadInfo *threadInfo,int clusterID,
                          const std::vector<LayoutObjectEntryRef> &layoutObjects,
                          LayoutObject &retObj) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        const auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end() || layoutObjects.empty())
        {
            return;
        }
        
        const ClusterInfo &clusterGenerator = *it;
        
        // Pick a representative screen object
        int maxDrawPriority = -1;
        LayoutObject *maxPriorityObj = nullptr;
        std::vector<std::string> uniqueIDs;
        uniqueIDs.reserve(layoutObjects.size());
        for (const auto &obj : layoutObjects)
        {
            if (!maxPriorityObj || obj->obj.getDrawPriority() > maxDrawPriority)
            {
                maxDrawPriority = obj->obj.getDrawPriority();
                maxPriorityObj = &obj->obj;
            }
            // todo: propagate marker "user object" through layout objects so
            //       the cluster generator can make use of that information.
            uniqueIDs.push_back(obj->obj.uniqueID);
        }

        jobjectArray uniqueIDsObj = BuildStringArray(env, uniqueIDs);
        uniqueIDs.clear();

        // The texture gets created on the Java side, so we'll just use the ID
        const long texID = env->CallLongMethod(clusterGenerator.clusterObj,
                                               clusterGenerator.makeClusterGroupJNIJava,
                                               (jint)layoutObjects.size(),
                                               uniqueIDsObj);
        if (logAndClearJVMException(env,"makeClusterGroup"))
        {
            return;
        }

        env->DeleteLocalRef(uniqueIDsObj);

        if (texID == EmptyIdentity)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "No tex from makeClusterGroup");
        }
        
        const Point2d size = clusterGenerator.layoutSize;
        const SimpleIdentity progID = maxPriorityObj->getTypicalProgramID();

        // Geometry for the new cluster object
        ScreenSpaceConvexGeometry smGeom;
        smGeom.progID = progID;
        smGeom.coords.emplace_back(-size.x()/2.0,-size.y()/2.0); smGeom.texCoords.emplace_back(0,1);
        smGeom.coords.emplace_back( size.x()/2.0,-size.y()/2.0); smGeom.texCoords.emplace_back(1,1);
        smGeom.coords.emplace_back( size.x()/2.0, size.y()/2.0); smGeom.texCoords.emplace_back(1,0);
        smGeom.coords.emplace_back(-size.x()/2.0, size.y()/2.0); smGeom.texCoords.emplace_back(0,0);
        smGeom.color = RGBAColor(255,255,255,255);
        smGeom.texIDs.push_back(texID);

        retObj.layoutPts = smGeom.coords;
        retObj.selectPts = smGeom.coords;

        retObj.setDrawPriority(maxDrawPriority);
        retObj.addGeometry(smGeom);
    }
    
    // Called right after all the layout objects are generated
    void endLayoutObjects(PlatformThreadInfo *threadInfo) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
        {
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.endClusterGroupJava);
        }
    }
    
    void paramsForClusterClass(PlatformThreadInfo *,int clusterID,ClusterClassParams &clusterParams) override
    {
        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end())
            return;
        
        const ClusterInfo &clusterGenerator = *it;
        
        clusterParams.motionShaderID = motionShaderID;
        clusterParams.selectable = clusterGenerator.selectable;
        // Note: Make this selectable
        clusterParams.markerAnimationTime = 0.2;
        clusterParams.clusterSize = clusterGenerator.layoutSize;
    }

    bool hasChanges() override {
        if (generatorChanges) {
            generatorChanges = false;
            return true;
        }
        return false;
    }

public:
    LayoutManagerRef layoutManager;

    SimpleIDSet currentClusterTex;
    //SimpleIDSet oldClusterTex;
    
    SimpleIdentity motionShaderID = EmptyIdentity;
    ClusterInfoSet clusterGens;

    bool generatorChanges = true;
};

typedef JavaClassInfo<LayoutManagerWrapperRef> LayoutManagerWrapperClassInfo;
template<> LayoutManagerWrapperClassInfo *LayoutManagerWrapperClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_nativeInit(JNIEnv *env, jclass cls)
{
    LayoutManagerWrapperClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_initialise(JNIEnv *env, jobject obj,jobject sceneObj)
{
    try
    {
        PlatformInfo_Android threadInfo(env);
        if (const auto scene = SceneClassInfo::get(env,sceneObj))
        if (const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager))
        if (auto wrap = std::make_shared<LayoutManagerWrapper>(&threadInfo, layoutManager))
        {
            layoutManager->addClusterGenerator(&threadInfo, wrap);
            auto handle = new LayoutManagerWrapperRef(std::move(wrap));
            LayoutManagerWrapperClassInfo::getClassInfo()->setHandle(env, obj, handle);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        auto classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            LayoutManagerWrapperRef *wrap = classInfo->getObject(env, obj);
            classInfo->clearHandle(env, obj);
            delete wrap;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setMaxDisplayObjects(JNIEnv *env, jobject obj, jint maxObjs)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            (*wrap)->layoutManager->setMaxDisplayObjects(maxObjs);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_updateLayout
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeSetObj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            if (auto viewState = ViewStateRefClassInfo::get(env, viewStateObj))
            {
                if (auto changeSet = ChangeSetClassInfo::get(env, changeSetObj))
                {
                    (*wrap)->updateShader();

                    PlatformInfo_Android threadInfo(env);
                    (*wrap)->layoutManager->updateLayout(&threadInfo,*viewState,**changeSet);
                }
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_cancelUpdate
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            if (auto lm = (*wrap)->layoutManager)
            {
                lm->cancelUpdate();
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_hasChanges(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            return (*wrap)->layoutManager->hasChanges();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_addClusterGenerator
    (JNIEnv *env, jobject obj, jobject clusterObj, jint clusterID, jboolean selectable, jdouble sizeX, jdouble sizeY)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            (*wrap)->addClusterGenerator(env, clusterObj, clusterID, selectable, sizeX, sizeY);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_removeClusterGenerator
        (JNIEnv *env, jobject obj, jint clusterID)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            return (*wrap)->removeClusterGenerator(env, clusterID);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_clearClusterGenerators(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            (*wrap)->clearClusterGenerators(env);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setFadeEnabled
        (JNIEnv *env, jobject obj, jboolean enable)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            (*wrap)->layoutManager->setFadeEnabled(enable);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_getFadeEnabled
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            return (*wrap)->layoutManager->getFadeEnabled();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setShowDebugLayoutBoundaries
        (JNIEnv *env, jobject obj, jboolean show)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            (*wrap)->layoutManager->setShowDebugBoundaries(show);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_getShowDebugLayoutBoundaries
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            return (*wrap)->layoutManager->getShowDebugBoundaries();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}
