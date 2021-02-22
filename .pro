RC_FILE = .rc
RESOURCES = icons.qrc
contains(QT_CONFIG, opengl): QT += opengl
QT += 
INCLUDEPATH += C:/PFM/compile/include
LIBS += -L C:/PFM/compile/lib -lmisp -lgsf -lpfm -lBinaryFeatureData -lCZMIL -lCHARTS -lnvutility -lgdal -lxml2 -lpoppler -lproj -liconv -lwsock32
DEFINES += XML_LIBRARY WIN32 NVWIN3X
DEFINES += 
CONFIG += console
CONFIG += exceptions
CONFIG += static
QMAKE_CXXFLAGS += -fno-strict-aliasing

#
# The following line is included so that the contents of acknowledgments.hpp will be available for translation
#

HEADERS += C:/PFM/compile/include/acknowledgments.hpp

QMAKE_LFLAGS += 
######################################################################
# Automatically generated by qmake (2.01a) Tue Feb 9 09:39:29 2021
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += . release nbproject\private
INCLUDEPATH += .

# Input
HEADERS += attributes.hpp \
           changePath.hpp \
           changePathHelp.hpp \
           dataTypeButtonBox.hpp \
           definePolygon.hpp \
           definePolygonHelp.hpp \
           deleteFile.hpp \
           deleteFileHelp.hpp \
           deleteQueue.hpp \
           deleteQueueHelp.hpp \
           displayHeader.hpp \
           displayHeaderHelp.hpp \
           displayMessage.hpp \
           editFeature.hpp \
           editFeatureHelp.hpp \
           enhancedOpenDisplay.hpp \
           featureTypes.hpp \
           findFeature.hpp \
           findFeatureHelp.hpp \
           gridThread.hpp \
           layers.hpp \
           lockValue.hpp \
           lockValueHelp.hpp \
           manageLayers.hpp \
           manageLayersHelp.hpp \
           manageOverlays.hpp \
           manageOverlaysHelp.hpp \
           otfDialog.hpp \
           otfDialogHelp.hpp \
           pfmView.hpp \
           pfmViewDef.hpp \
           pfmViewHelp.hpp \
           prefs.hpp \
           prefsHelp.hpp \
           release.hpp \
           remisp.hpp \
           remispFilter.hpp \
           screenshot.hpp \
           screenshotHelp.hpp \
           unloadDialog.hpp \
           version.hpp
SOURCES += adjust_bounds.cpp \
           attributes.cpp \
           bfd_check_file.cpp \
           changePath.cpp \
           check_recent.cpp \
           checkFeature.cpp \
           compute_layer_min_max.cpp \
           compute_total_mbr.cpp \
           cov_coasts.cpp \
           dataTypeButtonBox.cpp \
           definePolygon.cpp \
           deleteFile.cpp \
           deleteQueue.cpp \
           displayHeader.cpp \
           displayMessage.cpp \
           displayMinMax.cpp \
           editFeature.cpp \
           enhancedOpenDisplay.cpp \
           env_in_out.cpp \
           export_image.cpp \
           findFeature.cpp \
           geotiff.cpp \
           get_feature_event_time.cpp \
           gridThread.cpp \
           hatchr.cpp \
           highlight_contour.cpp \
           layers.cpp \
           loadArrays.cpp \
           lockValue.cpp \
           main.cpp \
           manageLayers.cpp \
           manageOverlays.cpp \
           mark_feature.cpp \
           otfDialog.cpp \
           outputFeaturePolygons.cpp \
           overlayData.cpp \
           overlayFlag.cpp \
           paint_otf_surface.cpp \
           paint_surface.cpp \
           pfmFilter.cpp \
           pfmView.cpp \
           prefs.cpp \
           readFeature.cpp \
           remisp.cpp \
           remispFilter.cpp \
           screenshot.cpp \
           scribe.cpp \
           set_defaults.cpp \
           setAreaChecked.cpp \
           setHighLow.cpp \
           setScale.cpp \
           unloadDialog.cpp \
           writeContour.cpp \
           nbproject/private/c_standard_headers_indexer.c \
           nbproject/private/cpp_standard_headers_indexer.cpp
RESOURCES += icons.qrc
TRANSLATIONS += pfmView_xx.ts