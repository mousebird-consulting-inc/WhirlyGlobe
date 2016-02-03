/*
 *  RendererFrameInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
package com.mousebird.maply;

import java.util.List;
import java.util.Vector;


public class RendererFrameInfo {

    public RendererFrameInfo(){
        initialise();
    }

    public void finalise(){
        dispose();
    }

    static {
        nativeInit();
    }

    private static native void nativeInit();

    native void initialise();
    native void dispose();

    private long nativeHandle;

    public native void setOGLVersion(int version);

    public native int getOGLVersion();

    public native void setSceneRendererES(SceneRendererES sceneRenderer);

    public native SceneRendererES getSceneRendererES();

    public native void setView (View theView);

    public native View getView();

    public native void setModelTrans (Matrix4f modelTrans);

    public native Matrix4f getModelTrans();

    public native void setViewTrans (Matrix4f viewTrans);

    public native Matrix4f getViewTrans();

    public native void setModelTrans4d (Matrix4d modelTrans4d);

    public native Matrix4d getModelTrans4d();

    public native void setViewTrans4d (Matrix4d viewTrans4d);

    public native Matrix4d getViewTrans4d();

    public native void setProjMat (Matrix4f projMat);

    public native Matrix4f getProjMat();

    public native void setProjMat4d (Matrix4d projMat4d);

    public native Matrix4d getProjMat4d();

    public native void setMvpNormalMat (Matrix4f mvpNormalMat);

    public native Matrix4f getMvpNormalMat();

    public native void setViewAndModelMat(Matrix4f viewAndModelMat);

    public native Matrix4f getViewAndModelMat();

    public native void setViewAndModelMat4d (Matrix4d viewAndModelMat4d);

    public native Matrix4d getViewAndModelMat4d();

    public native void setMvpMat (Matrix4f mvpMat);

    public native Matrix4f getMvpMat();

    public native void setViewModelNormalMat (Matrix4f viewModelNormalMat);

    public native Matrix4f getViewModelNormalMat();

    public native void setPvMat4d (Matrix4d pvMat4d);

    public native Matrix4d getPvMat4d();

    public native void setPvMat (Matrix4f pvMat);

    public native Matrix4f getPvMat();

    public native void setOffSetMatrices (List<Matrix4d> offSetMatrices);

    public native List<Matrix4d> getOffSetMatrices();

    public native void setScene (Scene scene);

    public native Scene getScene();

    public native void setFrameLen (float frameLen);

    public native float getFrameLen();

    public native void setCurrentTime (double currentTime);

    public native double getCurrentTime();

    public native void setEyeVec (Point3d eyeVec);

    public native Point3d getEyeVec();

    public native void setFullEyeVec (Point3d fullEyeVec);

    public native Point3d getFullEyeVec();

    public native void setEyePos (Point3d eyePos);

    public native Point3d getEyePos();

    public native void setDispCenter (Point3d dispCenter);

    public native Point3d getDispCenter();

    public native void setHeightAboveSurface (float heightAboveSurface);

    public native float getHeightAboveSurface();

    public native void setScreenSizeInDisplaysCoords (Point2d screenSizeInDisplaysCoords);

    public native Point2d getScreenSizeInDisplaysCoords();

    public native void setOpenGLES2Program (OpenGLES2Program program);

    public native OpenGLES2Program getOpenGLES2Program();

    public native void setStateOptimizer (OpenGLStateOptimizer stateOpt);

    public native OpenGLStateOptimizer getStateOptimizer();

}
