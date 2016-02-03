/*
 *  Drawable.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
package com.mousebird.maply;


import java.util.List;

public class Drawable {

    long id = Identifiable.genID();

    public Drawable(String name){
        initialise(name);
    }

    public Drawable(String name, int numVert, int numTri){
        initialise(name, numVert, numTri);
    }

    public void finalise(){
        dispose();
    }

    public native long getProgram();

    public native void setProgram(int programID);

    public native void setupGL(WhirlyKitGLSetupInfo setupInfo, OpenGLMemManager memManager);

    public native void setupGL(WhirlyKitGLSetupInfo setupInfo, OpenGLMemManager memManager, int sharedBuf, int sharedBufOffset );

    public native void tearDownGL(OpenGLMemManager memManager);

    public native int singleVertexSize();

    public native int setupVAO(OpenGLES2Program prog);

    public native void draw(RendererFrameInfo frameInfo, Scene scene);

    public native int getDrawPriority();

    public native boolean isOn(RendererFrameInfo frameInfo);

    public native void setOnOff(boolean onOff);

    public native void setEnableTimeRange(double inStartEnable, double inEndEnable);

    public native boolean hasAlpha(RendererFrameInfo frameInfo);

    public native boolean setAlpha (boolean onOff);

    public Mbr getLocalMrb(){
        Mbr localMbr = new Mbr(getLocalMbrPoint2dLL(), getLocalMbrPoint2dUr());
        return localMbr;
    }

    private native Point2d getLocalMbrPoint2dLL();

    private native Point2d getLocalMbrPoint2dUr();

    public void setLocalMbr(Mbr localMbr){
        setLocalMbr(localMbr.ll, localMbr.ur);
    }

    private native void setLocalMbr(Point2d ll, Point2d ur);

    public native void setDrawPriority(int newPriority);

    public native void setDrawOffset(float drawOffset);

    public native float getDrawOffset();

    public native void setType(int inType);

    public native int getType();

    public native void setTexId(int which, long inID);

    public native void setTexIds(List<Long> texIds);

    public native float getColorPartR();

    public native float getColorPartG();

    public native float getColorPartB();

    public native float getColorPartA();

    public native void setColor(float r, float g, float b, float a);

    public native void setVisibleRange(float minVis, float maxVis, float minVisBand, float maxVisBand);

    public native float getVisibleRangeMinVis();

    public native float getVisibleRangeMaxVis();

    public native float getVisibleRangeMinVisBand();

    public native float getVisibleRangeMaxVisBand();

    public native void setViewerVisibility(double minViewerDist, double maxViewerDist, Point3d vieweCenter);

    public native double getViewerVisibilityMinDist();

    public native double getViewerVisibilityMaxdist();

    public native Point3d getViewerVisibilityCenter();

    public native void setFade(double inFadeDown, double inFadeUp);

    public native void setLineWidth(float inWidht);

    public native float getLineWidth();

    public native void setRequestZBuffer(boolean val);

    public native boolean getRequestZBuffer();

    public native void setWriteZBuffer(boolean val);

    public native boolean getWriteZBuffer();

    public native int addPoint(Point3f pt);

    public native int addPoint(Point3d pt);

    public native Point3f getPoint(int which);

    public native void addTexCoord(int witch, Point2f coord);

    public native void addColor(float r, float g, float b, float a);

    public native void addNormal(Point3f norm);

    public native void addNormal(Point3d norm);

    public native boolean compareVertexAttributes(List<SingleVertexAttribute> attrs);

    public native void setVertexAttributes(List<SingleVertexAttributeInfo> attrs);

    public native void addVertexAttributes(List<SingleVertexAttribute> attrs);

    public native void addAttributeValue(int attrID, Point2f vec);

    public native void addAttributeValue(int attrID, Point3f vec);

    public native void addAttributeValue(int attrID, Point4f vec);

    public native void addAttributeValue(int attrID, float r, float g, float b, float a);

    public native void addAttributeValue(int attrID, float val);

    public native void addTriangle(Triangle tri);

    public native int getTexID(int which);

    public native List<TexInfo> getTexInfo();

    public native int addAttribute(int dataType, String name);

    public native int getNumPoint();

    public native int getNumTris();

    public native void reserveNumPoint(int numPoints);

    public native void reserveNumTris(int numTris);

    public native void reserveNumTexCoords(int which, int numCoords);

    public native void reserveNumNorms(int numNorms);

    public native void reserveNumColors(int numColors);

    public native void setMatrix(Matrix4d inMat);

    public native Matrix4d getMatrix();

    public native void applySubTexture(int which, SubTexture subTex, int startingAt);

    public native void updateRenderer(SceneRendererES renderer);

    public native RawData asData(boolean dupStart, boolean dupEnd);

    public native void asVextexAndElementData(MutableRawData vertData, MutableRawData elementData, int singleElementSize, Point3d center);

    public native List<VertexAttribute> getVertexAttributes();

    public native void setupTexCoordEntry(int which, int numReserve);

    public native void drawOGL2(RendererFrameInfo frameInfo, Scene scene);

    public native void addPointToBuffer(String basePtr, int which, Point3d center);

    public native void setupAdditionalVAO(OpenGLES2Program prog, int vertArrayObj);

    public native void bindAdditionalRenderObjects(RendererFrameInfo frameInfo, Scene scene);

    public native void postDrawCallBack(RendererFrameInfo frameInfo, Scene scene);

    public native void setupStandardAttributes(int numReserve);

    public native void addTweaker (DrawableTweaker tweaker);

    public native void removeTweaker (DrawableTweaker tweaker);

    public native void runTweakers(RendererFrameInfo frame);

    native void initialise(String name);

    native void initialise(String name, int numVert, int numTri);

    native void dispose();

    static {
        nativeInit();
    }
    private static native void nativeInit();
    private long nativeHandle;


}
