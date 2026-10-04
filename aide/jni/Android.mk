LOCAL_PATH := $(call my-dir)
ENGINE_SRC := $(LOCAL_PATH)/../../app/src/main/cpp

include $(CLEAR_VARS)
LOCAL_MODULE := apex
LOCAL_SRC_FILES := \
    ../../app/src/main/cpp/main.cpp \
    ../../app/src/main/cpp/physics.cpp \
    ../../app/src/main/cpp/race.cpp \
    ../../app/src/main/cpp/asset_manifest.cpp \
    ../../app/src/main/cpp/asset_catalog.cpp \
    ../../app/src/main/cpp/track_asset_catalog.cpp \
    ../../app/src/main/cpp/glb_loader.cpp \
    ../../app/src/main/cpp/glb_mesh.cpp
LOCAL_C_INCLUDES := $(ENGINE_SRC)
LOCAL_CPPFLAGS := -std=c++17
LOCAL_LDLIBS := -landroid -llog -lEGL -lGLESv3
include $(BUILD_SHARED_LIBRARY)
