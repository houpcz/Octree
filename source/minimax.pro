## This is config for debug
#CONFIG +=  console warn_off thread debug qt
## This is config for release (optimized program)
CONFIG	+= console warn_off thread release qt

#this is a config for usage with Intel's ray tracer
#CONFIG	+= console warn_off thread release qt mlrt

TEMPLATE = app

TARGET = minimax


CONFIG(mlrt) {

DEFINES+=USE_MLRT
INCLUDEPATH += support/MultiLevelRayTracing
LIBS += RTScene.lib RTWorld.lib
CONFIG(release) {
LIBPATH += \
support/MultiLevelRayTracing/RTScene/Release \
support/MultiLevelRayTracing/RTWorld/Release
}

CONFIG(debug) {
LIBPATH += \
support/MultiLevelRayTracing/RTScene/Debug \
support/MultiLevelRayTracing/RTWorld/Debug
}

}

win32:INCLUDEPATH += support/Zlib/include
win32:LIBPATH += 

QT += opengl

win32:QMAKE_LFLAGS_CONSOLE=/NODEFAULTLIB:LIBCMT


# Headers must include all headers with Qt objects inside!

HEADERS += RendererWidget.h BasicRenderer.h


# All .cpp sources

SOURCES += main.cpp common.cpp gzstream.cpp \
Environment.cpp AppEnvironment.cpp \
ObjParser.cpp BinParser.cpp Bvh.cpp Scene.cpp \
RendererWidget.cpp BasicRenderer.cpp Triangle.cpp \
GvsPreprocessor.cpp AxisAlignedBox3.cpp Ray.cpp \
LstParser.cpp Trackball.cpp \
BvhRayCaster.cpp

CONFIG(mlrt) {
SOURCES += IntelRayCaster.cpp
}
