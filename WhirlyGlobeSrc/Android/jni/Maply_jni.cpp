#import <jni.h>
#import "Maply_jni.h"

// Have to instantiate the static members somewhere

JavaDoubleClassInfo *JavaDoubleClassInfo::classInfoObj = NULL;
JavaIntegerClassInfo *JavaIntegerClassInfo::classInfoObj = NULL;
JavaHashMapInfo *JavaHashMapInfo::classInfoObj = NULL;
template<> AttrDictClassInfo *AttrDictClassInfo::classInfoObj = NULL;
template<> ChangeSetClassInfo *ChangeSetClassInfo::classInfoObj = NULL;
template<> TextureClassInfo *TextureClassInfo::classInfoObj = NULL;
template<> CoordSystemClassInfo *CoordSystemClassInfo::classInfoObj = NULL;
template<> Point2dClassInfo *Point2dClassInfo::classInfoObj = NULL;
template<> Point3dClassInfo *Point3dClassInfo::classInfoObj = NULL;
template<> Matrix4dClassInfo *Matrix4dClassInfo::classInfoObj = NULL;
template<> CoordSystemDisplayAdapterInfo *CoordSystemDisplayAdapterInfo::classInfoObj = NULL;
template<> MaplySceneRendererInfo *MaplySceneRendererInfo::classInfoObj = NULL;
template<> MapSceneClassInfo *MapSceneClassInfo::classInfoObj = NULL;
template<> MapViewClassInfo *MapViewClassInfo::classInfoObj = NULL;
template<> VectorInfoClassInfo *VectorInfoClassInfo::classInfoObj = NULL;
template<> VectorObjectClassInfo *VectorObjectClassInfo::classInfoObj = NULL;
template<> MarkerInfoClassInfo *MarkerInfoClassInfo::classInfoObj = NULL;
template<> ViewStateClassInfo *ViewStateClassInfo::classInfoObj = NULL;
