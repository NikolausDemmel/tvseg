########################################################################
# FIXES for Mac OS X
########################################################################
macx {
    # the following fixes build on Mavericks (else linking OpenCV fails).
    # need to invesitage the effect on older OS X versions.
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    # Fix default config on Mac, which disables pkg-config
    #QT_CONFIG -= no-pkg-config

    # Dont build app bundle, but normal executable like on linux
    CONFIG -= app_bundle

    # fixme: this is not a very general solution
    BOOST_ROOT=/usr/local/opt/boost-libstdcxx

    # Boost (on Mac this is not automatic)
    include (qinclude/boost.pri)

    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_CXXFLAGS += -stdlib=libstdc++
    QMAKE_LFLAGS += -stdlib=libstdc++

    QMAKE_CXXFLAGS += $$system(PKG_CONFIG_PATH=/usr/local/opt/opencv-libstdcxx/lib/pkgconfig/ pkg-config opencv --cflags)
    LIBS += $$system(PKG_CONFIG_PATH=/usr/local/opt/opencv-libstdcxx/lib/pkgconfig/ pkg-config opencv --libs-only-other)

} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
}
########################################################################



########################################################################
# Main configuration
########################################################################

QT      += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tvseg
TEMPLATE = app

QMAKE_CXXFLAGS += \
    -O2 \
    -pipe \
    -Wall

LIBS += -lboost_system -lboost_regex -lboost_filesystem

INCLUDEPATH += $$PWD/lib $$PWD/src
DEPENDPATH += $$PWD/lib $$PWD/src

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
}

########################################################################



########################################################################
# Includes
########################################################################

include(src/3rdparty/qtpropertybrowser/qtpropertybrowser.pri)

########################################################################



########################################################################
# Files
########################################################################

SOURCES += \
    lib/tvseg/cuda/cvimageshow.cpp \
    lib/tvseg/cvalgorithminterfacebase.cpp \
    lib/tvseg/datastore.cpp \
    lib/tvseg/edgedetectionbase.cpp \
    lib/tvseg/gradientcpu.cpp \
    lib/tvseg/imagedimensions.cpp \
    lib/tvseg/kmeanscpu.cpp \
    lib/tvseg/segmentationimpl.cpp \
    lib/tvseg/settings/backend.cpp \
    lib/tvseg/settings/entrybase.cpp \
    lib/tvseg/settings/serializerbase.cpp \
    lib/tvseg/settings/serializerqt.cpp \
    lib/tvseg/settings/settings.cpp \
    lib/tvseg/tvdatatermbase.cpp \
    lib/tvseg/tvdatatermcolorgpu.cpp \
    lib/tvseg/tvsolverbase.cpp \
    lib/tvseg/tvsolvergpu.cpp \
    lib/tvseg/tvvisualizerbase.cpp \
    lib/tvseg/tvvisualizercpu.cpp \
    lib/tvseg/tvweightbase.cpp \
    lib/tvseg/tvweightedge.cpp \
    lib/tvseg/util/errorhandling.cpp \
    lib/tvseg/util/logging.cpp \
    lib/tvseg/util/qtopencvhelper.cpp \
    src/algorithmwidget.cpp \
    src/consoledockwidget.cpp \
    src/consoletextedit.cpp \
    src/cvimagedisplaywidget.cpp \
    src/feedbackdockwidget.cpp \
    src/feedbackwidget.cpp \
    src/imagewidget.cpp \
    src/main.cpp \
    src/settingseditor.cpp \
    src/tvsegwindow.cpp \
    src/scribbleimagedisplaywidget.cpp \
    lib/tvseg/tvdatatermparzengpu.cpp \
    lib/tvseg/scribbles.cpp \
    lib/tvseg/util/helpers.cpp \
    lib/tvseg/util/opencvhelper.cpp

HEADERS  += \
    lib/boost/enum.hpp \
    lib/boost/enum/base.hpp \
    lib/boost/enum/bitfield.hpp \
    lib/boost/enum/iterator.hpp \
    lib/boost/enum/macros.hpp \
    lib/cuda_samples/common/inc/helper_cuda.h \
    lib/cuda_samples/common/inc/helper_string.h \
    lib/tvseg/3rdparty/easylogging++.h \
    lib/tvseg/cuda/colordataterm.h \
    lib/tvseg/cuda/config.h \
    lib/tvseg/cuda/cvimageshow.h \
    lib/tvseg/cuda/errorhandling.h \
    lib/tvseg/cuda/params.h \
    lib/tvseg/cuda/solver.h \
    lib/tvseg/cuda/utils.h \
    lib/tvseg/cvalgorithminterface.h \
    lib/tvseg/cvalgorithminterfacebase.h \
    lib/tvseg/datastore.h \
    lib/tvseg/defines.h \
    lib/tvseg/edgedetection.h \
    lib/tvseg/edgedetectionbase.h \
    lib/tvseg/feedback.h \
    lib/tvseg/feedbackbase.h \
    lib/tvseg/gradientcpu.h \
    lib/tvseg/imagedimensions.h \
    lib/tvseg/kmeans.h \
    lib/tvseg/kmeanscpu.h \
    lib/tvseg/params.h \
    lib/tvseg/segmentation.h \
    lib/tvseg/segmentationimpl.h \
    lib/tvseg/settings/backend.h \
    lib/tvseg/settings/defineparams.h \
    lib/tvseg/settings/definesettings.h \
    lib/tvseg/settings/entry.h \
    lib/tvseg/settings/entrybase.h \
    lib/tvseg/settings/macroutils.h \
    lib/tvseg/settings/serializer.h \
    lib/tvseg/settings/serializerbase.h \
    lib/tvseg/settings/serializerqt.h \
    lib/tvseg/settings/settings.h \
    lib/tvseg/tvdataterm.h \
    lib/tvseg/tvdatatermbase.h \
    lib/tvseg/tvdatatermcolorgpu.h \
    lib/tvseg/tvsolver.h \
    lib/tvseg/tvsolverbase.h \
    lib/tvseg/tvsolvergpu.h \
    lib/tvseg/tvvisualizer.h \
    lib/tvseg/tvvisualizerbase.h \
    lib/tvseg/tvvisualizercpu.h \
    lib/tvseg/tvweight.h \
    lib/tvseg/tvweightbase.h \
    lib/tvseg/tvweightedge.h \
    lib/tvseg/util/errorhandling.h \
    lib/tvseg/util/includeopencv.h \
    lib/tvseg/util/logging.h \
    lib/tvseg/util/qtopencvhelper.h \
    lib/tvseg/util/sharedptr.h \
    src/algorithmwidget.h \
    src/consoledockwidget.h \
    src/consoletextedit.h \
    src/cvimagedisplaywidget.h \
    src/feedbackdockwidget.h \
    src/feedbackwidget.h \
    src/imagewidget.h \
    src/settingseditor.h \
    src/tvsegwindow.h \
    src/util/valuetransition.h \
    lib/tvseg/util/opencvhelper.h \
    src/scribbleimagedisplaywidget.h \
    lib/tvseg/tvdatatermparzengpu.h \
    lib/tvseg/cuda/parzendataterm.h \
    lib/tvseg/scribbles.h \
    lib/tvseg/util/helpers.h \
    src/uisettings.h \
    lib/tvseg/util/arrayio.h \
    lib/tvseg/settingsdef/outputsettings.h \
    lib/tvseg/settingsdef/weightsettings.h \
    lib/tvseg/settingsdef/inputsettings.h \
    lib/tvseg/settingsdef/parzensettings.h \
    lib/tvseg/settingsdef/segmentationsettings.h \
    lib/tvseg/settingsdef/visualizersettings.h \
    lib/tvseg/settingsdef/solversettings.h

FORMS    += \
    src/tvsegwindow.ui \
    src/consoledockwidget.ui \
    src/feedbackwidget.ui \
    src/feedbackdockwidget.ui



CUDA_SOURCES = \
    lib/tvseg/cuda/colordataterm.cu \
    lib/tvseg/cuda/solver.cu \
    lib/tvseg/cuda/parzendataterm.cu

OTHER_FILES += \
    $$CUDA_SOURCES \
    tvseg.ini \
    lib/tvseg/3rdparty/easylogging++-LICENCE \
    tvseg.el.cfg

########################################################################



########################################################################
#  CUDA
########################################################################

CUDA_ARCH = sm_30

unix {

    # detect CUDA path from nvcc binary
    CUDA_DIR = $$system(which nvcc | sed 's,/bin/nvcc$,,')

    INCLUDEPATH += $$CUDA_DIR/include
    DEPENDPATH += $$CUDA_DIR/include
    CUDA_CXXFLAGS = $$QMAKE_CXXFLAGS
##        -DBOOST_NOINLINE='"\'__attribute__ ((noinline))\'"'
    macx:CUDA_CXXFLAGS += -Wno-unused-function # avoid the many warnings with clang

    linux-g++-64 {
        QMAKE_LIBDIR += $$CUDA_DIR/lib64
    }
    else {
        QMAKE_LIBDIR += $$CUDA_DIR/lib
    }

    LIBS += -lcudart -lnppi

    macx:LIBS += -Wl,-rpath,$$CUDA_DIR/lib

    cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.obj
    cuda.commands = nvcc -c -arch $$CUDA_ARCH -Xcompiler $$join(CUDA_CXXFLAGS,",") $$join(INCLUDEPATH,'" -I "','-I "','"') ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}

    cuda.dependcy_type = TYPE_C
    cuda.depend_command = nvcc -M -arch $$CUDA_ARCH -Xcompiler $$join(CUDA_CXXFLAGS,",") $$join(INCLUDEPATH,'" -I "','-I "','"') ${QMAKE_FILE_NAME}
} else {
    error("CUDA support not implemented in project file.")
}

cuda.input = CUDA_SOURCES
QMAKE_EXTRA_COMPILERS += cuda

########################################################################
