LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BASE_DIR := ../../../
SRC_DIR := $(BASE_DIR)/WhirlyGlobeSrc/WhirlyGlobeLib/src/
INCLUDE_DIR := ../..//WhirlyGlobeSrc/WhirlyGlobeLib/include/
LOCAL_EXPORT_C_INCLUDES = $(BASE_DIR)/WhirlyGlobeSrc/WhirlyGlobeLib/include/

THIRD_PARTY := $(BASE_DIR)third-party/
THIRD_PARTY_INC := ../../third-party/
LOCAL_C_INCLUDES += $(INCLUDE_DIR)
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/eigen/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/boost/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/proj-4/src/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/clipper/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/shapelib/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/libjson/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/glues/include/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/glues/source/
LOCAL_C_INCLUDES += $(THIRD_PARTY_INC)/glues/source/include/
LOCAL_C_INCLUDES += jni/

LOCAL_MODULE    := Maply

PROJ_SRC_FILES := PJ_eck5.c PJ_larr.c PJ_omerc.c PJ_vandg2.c geod_set.c pj_factors.c pj_release.c \
					PJ_eqc.c PJ_lask.c PJ_ortho.c PJ_vandg4.c pj_fwd.c pj_strerrno.c \
					PJ_aea.c PJ_eqdc.c PJ_lcc.c  PJ_poly.c PJ_wag2.c pj_gauss.c pj_transform.c \
					PJ_aeqd.c PJ_fahey.c PJ_lcca.c PJ_putp2.c PJ_wag3.c pj_geocent.c pj_tsfn.c \
					PJ_airy.c PJ_fouc_s.c PJ_loxim.c PJ_putp3.c PJ_wag7.c mk_cheby.c pj_gridinfo.c pj_units.c \
					PJ_aitoff.c PJ_gall.c PJ_lsat.c PJ_putp4p.c PJ_wink1.c pj_gridlist.c pj_utils.c \
					PJ_august.c PJ_geos.c PJ_mbt_fps.c PJ_putp5.c PJ_wink2.c pj_init.c pj_zpoly1.c \
					PJ_bacon.c PJ_gins8.c PJ_mbtfpp.c PJ_putp6.c aasincos.c nad_cvt.c pj_initcache.c \
					PJ_bipc.c PJ_gn_sinu.c PJ_mbtfpq.c PJ_robin.c adjlon.c  nad_init.c pj_inv.c  \
					PJ_boggs.c PJ_gnom.c PJ_merc.c PJ_rpoly.c bch2bps.c nad_intr.c pj_latlong.c \
					PJ_bonne.c PJ_goode.c PJ_mill.c PJ_sconics.c bchgen.c  pj_list.c \
					PJ_cass.c PJ_gstmerc.c PJ_mod_ster.c PJ_somerc.c biveval.c p_series.c proj_etmerc.c \
					PJ_cc.c   PJ_hammer.c PJ_moll.c PJ_stere.c pj_apply_gridshift.c    pj_log.c  proj_mdist.c \
					PJ_cea.c  PJ_hatano.c PJ_natearth.c PJ_sterea.c dmstor.c  pj_apply_vgridshift.c   pj_malloc.c proj_rouss.c \
					PJ_chamb.c PJ_healpix.c PJ_nell.c PJ_sts.c  emess.c   pj_auth.c pj_mlfn.c \
					PJ_collg.c PJ_igh.c  PJ_nell_h.c PJ_tcc.c pj_ctx.c  pj_msfn.c rtodms.c \
					PJ_crast.c PJ_imw_p.c PJ_nocol.c PJ_tcea.c gen_cheb.c pj_datum_set.c          pj_mutex.c vector1.c \
					PJ_denoy.c PJ_isea.c PJ_nsper.c PJ_tmerc.c geocent.c pj_datums.c pj_open_lib.c \
					PJ_eck1.c PJ_krovak.c PJ_nzmg.c PJ_tpeqd.c pj_deriv.c pj_param.c \
					PJ_eck2.c PJ_labrd.c PJ_ob_tran.c PJ_urm5.c geod.c    pj_ell_set.c pj_phi2.c \
					PJ_eck3.c PJ_laea.c PJ_ocea.c PJ_urmfps.c geod_for.c pj_ellps.c pj_pr_list.c \
					PJ_eck4.c PJ_lagrng.c PJ_oea.c  PJ_vandg.c geod_inv.c pj_errno.c pj_qsfn.c
PROJ_SRC_DIR := $(THIRD_PARTY)/proj-4/src/
LOCAL_SRC_FILES += $(PROJ_SRC_FILES:%=$(PROJ_SRC_DIR)/%)

LOCAL_SRC_FILES += $(THIRD_PARTY)/clipper/cpp/clipper.cpp

SHP_SRC_FILES = safileio.c dbfopen.c shpopen.c
SHP_SRC_DIR = $(THIRD_PARTY)/shapelib/
LOCAL_SRC_FILES += $(SHP_SRC_FILES:%=$(SHP_SRC_DIR)/%)

JSON_SRC_FILES = JSONAllocator.cpp JSONDebug.cpp JSONMemory.cpp JSONNode_Mutex.cpp JSONStream.cpp JSONWorker.cpp internalJSONNode.cpp \
					JSONChildren.cpp JSONIterators.cpp JSONNode.cpp JSONPreparse.cpp JSONValidator.cpp JSONWriter.cpp libjson.cpp
JSON_SRC_DIR = $(THIRD_PARTY)/libjson/_internal/Source/
LOCAL_SRC_FILES += $(JSON_SRC_FILES:%=$(JSON_SRC_DIR)/%)

GLU_SRC_FILES = glues_error.c    glues_mipmap.c   glues_project.c  glues_quad.c     glues_registry.c
GLU_SRC_DIR = $(THIRD_PARTY)/glues/source/
LOCAL_SRC_FILES += $(GLU_SRC_FILES:%=$(GLU_SRC_DIR)/%)

TESS_SRC_FILES = dict.c geom.c memalloc.c mesh.c normal.c priorityq.c render.c sweep.c tess.c tessmono.c
TESS_SRC_DIR = $(THIRD_PARTY)/glues/source/libtess
LOCAL_SRC_FILES += $(TESS_SRC_FILES:%=$(TESS_SRC_DIR)/%)

MAPLY_CORE_SRC_FILES := BigDrawable.cpp CoordSystem.cpp Cullable.cpp DefaultShaderPrograms.cpp Dictionary.cpp \
					Drawable.cpp DynamicDrawableAtlas.cpp DynamicTextureAtlas.cpp FlatMath.cpp FontTextureManager.cpp GLUtils.cpp \
					Generator.cpp GlobeMath.cpp GlobeScene.cpp GlobeView.cpp GlobeViewState.cpp GridClipper.cpp Identifiable.cpp \
					LabelManager.cpp LabelRenderer.cpp LayoutManager.cpp LoadedTile.cpp MaplyFlatView.cpp MaplyScene.cpp \
					MaplyView.cpp MaplyViewState.cpp MarkerGenerator.cpp MarkerManager.cpp OpenGLES2Program.cpp \
					PerformanceTimer.cpp QuadDisplayController.cpp Quadtree.cpp RawData.cpp Scene.cpp SceneRendererES.cpp \
					SceneRendererES2.cpp ScreenImportance.cpp ScreenSpaceBuilder.cpp ScreenSpaceDrawable.cpp ScreenSpaceGenerator.cpp \
					SelectionManager.cpp ShapeReader.cpp SphericalMercator.cpp Tesselator.cpp Texture.cpp TextureAtlas.cpp TileQuadLoader.cpp \
					VectorData.cpp VectorManager.cpp VectorObject.cpp ViewState.cpp WhirlyGeometry.cpp WhirlyKitView.cpp WhirlyVector.cpp
		
MAPLY_CORE_SRC_DIR := $(SRC_DIR)
LOCAL_SRC_FILES += $(MAPLY_CORE_SRC_FILES:%=$(MAPLY_CORE_SRC_DIR)/%)

MAPLY_JNI_FILES := Maply_jni.cpp AttrDictionary_jni.cpp ChangeSet_jni.cpp CoordSystemDisplayAdapter_jni.cpp \
					FontTextureManagerAndroid.cpp Identifiable_jni.cpp LabelInfoAndroid.cpp LabelInfo_jni.cpp LabelManager_jni.cpp LayoutManager_jni.cpp \
					MaplyRenderer_jni.cpp MapScene_jni.cpp MapView_jni.cpp \
					Matrix4d_jni.cpp Point2d_jni.cpp Point3d_jni.cpp VectorInfo_jni.cpp VectorIterator_jni.cpp VectorManager_jni.cpp VectorObject_jni.cpp \
					InternalLabel_jni.cpp InternalMarker_jni.cpp MarkerInfo_jni.cpp MarkerManager_jni.cpp Texture_jni.cpp QuadPagingLayer_jni.cpp QuadImageTileLayer_jni.cpp \
					SelectionManager_jni.cpp SingleLabelAndroid.cpp CoordSystem_jni.cpp SphericalMercatorCoordSystem_jni.cpp PlateCarreeCoordSystem_jni.cpp ViewState_jni.cpp \
					Scene_jni.cpp View_jni.cpp MapViewState_jni.cpp GlobeScene_jni.cpp GlobeView_jni.cpp GlobeViewState_jni.cpp
LOCAL_SRC_FILES += $(MAPLY_JNI_FILES)

MAPLY_PLATFORM_FILES := platform.cpp glwrapper.cpp
MAPLY_PLATFORM_SRC_DIR := $(SRC_DIR)/android
LOCAL_SRC_FILES += $(MAPLY_PLATFORM_FILES:%=$(MAPLY_PLATFORM_SRC_DIR)/%)

LOCAL_LDLIBS := -llog -lGLESv2 -lGLESv1_CM -landroid -lEGL -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
