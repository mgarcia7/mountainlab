QT += core gui network

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

CONFIG -= app_bundle #Please apple, don't make a bundle today

include(../../mvcommon/mvcommon.pri)
include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mda.pri)
include(../../mlcommon/taskprogress.pri)

QT += widgets
QT+=concurrent

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountaincompare
TEMPLATE = app

INCLUDEPATH += ../../mountainview/src/msv/plugins
VPATH += ../../mountainview/src/msv/plugins
HEADERS += clusterdetailplugin.h clipsviewplugin.h \
    initialize_confusion_matrix.h
SOURCES += clusterdetailplugin.cpp clipsviewplugin.cpp \
    initialize_confusion_matrix.cpp

INCLUDEPATH += ../../mountainview/src/msv/views
VPATH += ../../mountainview/src/msv/views
HEADERS += clusterdetailview.h clusterdetailviewpropertiesdialog.h
SOURCES += clusterdetailview.cpp clusterdetailviewpropertiesdialog.cpp
FORMS += clusterdetailviewpropertiesdialog.ui

HEADERS += \
    mccontext.h \
    views/confusionmatrixview.h \
    views/matrixview.h \
    mcviewfactories.h \
    views/compareclusterview.h

SOURCES += mountaincomparemain.cpp \
    mccontext.cpp \
    views/confusionmatrixview.cpp \
    views/matrixview.cpp \
    mcviewfactories.cpp \
    views/compareclusterview.cpp

INCLUDEPATH += ../../mountainview/src/controlwidgets
VPATH += ../../mountainview/src/controlwidgets
HEADERS += mvopenviewscontrol.h mvtimeseriescontrol.h createtimeseriesdialog.h mvclustervisibilitycontrol.h
SOURCES += mvopenviewscontrol.cpp mvtimeseriescontrol.cpp createtimeseriesdialog.cpp mvclustervisibilitycontrol.cpp
FORMS += createtimeseriesdialog.ui

INCLUDEPATH += ../../mountainview/src/views
VPATH += ../../mountainview/src/views
HEADERS += \
mvclipsview.h mvclipswidget.h mvtimeseriesview2.h mvtimeseriesviewbase.h mvtimeseriesrendermanager.h \
mvclusterview.h mvclusterwidget.h mvclusterlegend.h
SOURCES += \
mvclipsview.cpp mvclipswidget.cpp mvtimeseriesview2.cpp mvtimeseriesviewbase.cpp mvtimeseriesrendermanager.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvclusterlegend.cpp

HEADERS += mvspikesprayview.h mvspikespraypanel.h
SOURCES += mvspikesprayview.cpp mvspikespraypanel.cpp

INCLUDEPATH += ../../mountainsort/src/processors
DEPENDPATH += ../../mountainsort/src/processors
VPATH += ../../mountainsort/src/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp

INCLUDEPATH += ../../mountainview/src
VPATH += ../../mountainview/src
HEADERS += mvcontext.h
SOURCES += mvcontext.cpp


INCLUDEPATH += ../../mountainview/src/multiscaletimeseries
VPATH += ../../mountainview/src/multiscaletimeseries
HEADERS += multiscaletimeseries.h
SOURCES += multiscaletimeseries.cpp

include(../../installbin.pri)
