
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/*  pfmView class definitions.  */

#ifndef PFM_VIEW_H
#define PFM_VIEW_H

#include "pfmViewDef.hpp"
#include "version.hpp"
#include "prefs.hpp"
#include "manageOverlays.hpp"
#include "manageLayers.hpp"
#include "deleteFile.hpp"
#include "deleteQueue.hpp"
#include "changePath.hpp"
#include "findFeature.hpp"
#include "definePolygon.hpp"
#include "displayHeader.hpp"
#include "displayMessage.hpp"
#include "editFeature.hpp"
#include "layers.hpp"
#include "lockValue.hpp"
#include "remisp.hpp"
#include "remispFilter.hpp"
#include "unloadDialog.hpp"
#include "otfDialog.hpp"
#include "dataTypeButtonBox.hpp"


void displayMinMax (nvMap *map, OPTIONS *options, MISC *misc);
void overlayFlag (nvMap *map, OPTIONS *options, MISC *misc, uint8_t suspect, uint8_t selected, uint8_t reference);
void scribe (nvMap *map, OPTIONS *options, MISC *misc, int32_t pfm, float *ar, float highlight);
void loadArrays (MISC *misc, int32_t pfm, BIN_RECORD *bin_record, float *data, float *attr, int32_t attr_num, uint8_t *flags, int32_t highlight,
                 int32_t h_count, float percent);
void compute_total_mbr (MISC *misc);
void adjust_bounds (MISC *misc, int32_t pfm);
int32_t bfd_check_file (MISC *misc, char *path, BFDATA_HEADER *header, int32_t mode);
uint8_t checkFeature (MISC *misc, OPTIONS *options, int32_t ftr, uint8_t *highlight, QString *feature_info);
uint8_t readFeature (QWidget *parent, MISC *misc);


  /*! \mainpage pfmView

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section intro Introduction

       PFM, or Pure File Magic, was conceived on a recording trip to Nashville in early 1996.  
       The design was refined over the next year or two by the usual band of suspects.  The 
       purpose of this little piece of work was to allow hydrographers to geographically view 
       minimum, maximum, and average binned surfaces, of whatever bin size they chose, and 
       then allow them to edit the original depth data.  After editing the depth data, the 
       bins would be recomputed and the binned surface redisplayed.  The idea being that the 
       hydrographer could view the min or max binned surface to find flyers and then just 
       edit those areas that required it.  In addition to the manual viewing and hand editing, 
       the PFM format is helpful for automatic filtering in places where data from different 
       files overlaps.  Also, there is a hook to a GeoTIFF mosaic file that can contain 
       sidescan or photo mosaic data.  pfmView is a very simple binned surface viewer.  After 
       all of the editing is finished, the status information can be loaded back into the 
       original raw input data files.  This program is what most people refer to as the
       Area-Based Editor (ABE) even though it is only the binned surface viewer portion of ABE.
       It is the starting point for all PFM editing in ABE.<br><br>


       <br><br>\section sec1 Searching the Doxygen Documentation

       The easiest way to find documentation for a particular C++ method is to go to the
       <a href="functions_func.html"><b>Data Structures>Data Fields>Functions</b></a> tab.  If, for example,
       you are looking for <b><i>editFeature</i></b> you would then click on the <b><i>e</i></b> tab,
       find <b><i>editFeature</i></b>, and then click on the associated structure (in this case editFeature).
       The entire group of public methods for a particular class are documented via their class documentation
       not via their include files.  You can see documentation for them in the main
       <a href="annotated.html"><b>Data Structures</b></a> tab or by selecting the class in the Data Structures
       section of the .hpp file.  For example, findFeature can be accessed via the overall
       <a href="annotated.html"><b>Data Structures</b></a> tab or from the class defined in the Data Structures
       section of the findFeature.hpp file.


       <br><br>\section nvmap The nvMap Class

       The pfmView program is built around the nvMap class from the nvutility library (libnvutility).  There are
       two main nvMap widgets, the coverage map (cov) and the main map (map).  Due to this there are many calls
       to nvMap methods throughout the program.  Documentation for the nvMap class can be found in the
       <a href="../../NVUTILITY_API_Documentation/html/index.html"><b>nvutility documentation</b></a>.  Specifically,
       look for the nvMap class in the <a href="../../NVUTILITY_API_Documentation/html/annotated.html"><b>Data
       Structures</b></a> tab.<br><br>


       <br><br>\section structures The OPTIONS and MISC Structures

       The pfmView program uses two huge structures; OPTIONS and MISC.  The OPTIONS structure contains variables
       that need to be saved when the program exits and restored when it is started.  This is done using QSettings
       in env_in_out.cpp.  The MISC structure contains a bunch of variables that are used throughout the program
       but don't need to be saved.  Both of these structures are initialized in set_defaults.cpp.  I'm sure that
       it's not all that cool to use giant structures to pass things around but at least I'm passing them by
       reference instead of value so it doesn't slow the program down (try passing a big structure by value in a
       tight loop and you'll see what I mean).  The main reason that I'm using these two structures is that
       I didn't want to have to have function calls with fifty million arguments.  Also, this program was ported
       from C/Motif to C++/Qt and that was how I handled it in C.  If you'd like to rewrite it to make it more
       elegant, be my guest.


       <br><br>\section shared Shared Memory in PFM ABE

       This is a little note about the use of shared memory within the Area-Based Editor (ABE) programs.  If you read
       the Qt documentation (or anyone else's documentation) about the use of shared memory they will say "Dear [insert
       name of omnipotent being of your choice here], whatever you do, always lock shared memory when you use it!".
       The reason they say this is that access to shared memory is not atomic.  That is, reading shared memory and then
       writing to it is not a single operation.  An example of why this might be important - two programs are running,
       the first checks a value in shared memory, sees that it is a zero.  The second program checks the same location
       and sees that it is a zero.  These two programs have different actions they must perform depending on the value
       of that particular location in shared memory.  Now the first program writes a one to that location which was
       supposed to tell the second program to do something but the second program thinks it's a zero.  The second program
       doesn't do what it's supposed to do and it writes a two to that location.  The two will tell the first program 
       to do something.  Obviously this could be a problem.  In real life, this almost never occurs.  Also, if you write
       your program properly you can make sure this doesn't happen.  In ABE we almost never lock shared memory because
       something much worse than two programs getting out of sync can occur.  If we start a program and it locks shared
       memory and then dies (or gets stuck waiting for something), all the other programs will be locked up.  When you
       look through the ABE code you'll see that we very rarely lock shared memory, and then only for very short periods
       of time.  This is by design.<br><br>


  */

class pfmView:public QMainWindow
{
  Q_OBJECT 


public:

  pfmView (int *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~pfmView ();

  void commandLineFileCheck ();


protected:

  QDialog         *newFeatures;

  QListWidget     *messageBox;

  prefs           *prefs_dialog;

  manageOverlays  *manageOverlays_dialog;

  manageLayers    *manageLayers_dialog;

  deleteFile      *deleteFile_dialog;

  deleteQueue     *deleteQueue_dialog;

  changePath      *changePath_dialog;

  findFeature     *findFeature_dialog;

  definePolygon   *definePolygon_dialog;

  displayHeader   *displayHeader_dialog;

  displayMessage  *displayMessage_dialog;

  editFeature     *editFeature_dialog;

  otfDialog       *otf_dialog;

  dataTypeButtonBox *dataTypeButtons;

  QMenu           *layerMenu, *editMenu, *viewMenu, *toolsMenu, *popupMenu, *covPopupMenu, *recentMenu;

  QTimer          *trackCursor, *programTimer;

  QTabWidget      *statbook;

  QLabel          *layerName[MAX_ABE_PFMS], *lonName, *latLabel, *lonLabel, *attrValue[PRE_ATTR + POST_ATTR], *attrBoxValue[NUM_ATTR],
                  *numLabel, *pfmLabel, *sizeLabel, *miscLabel;

  clickLabel      *attrName[PRE_ATTR + POST_ATTR], *latName, *attrBoxName[NUM_ATTR];

  QCheckBox       *layerCheck[MAX_ABE_PFMS];

  QLineEdit       *displayedArea, *viewChartScale;

  QPainterPath    marker, arrow;

  nvMap           *map, *cov;

  QAction         *fileChecked, *fileUnchecked, *fileVerified, *fileUnverified, *deleteRestore, *deleteFileQueue, *changePathAct, *setChecked,
                  *setUnchecked, *setVerified, *setUnverified, *setMultiChecked, *setMultiValidChecked, *unloadDisplayed, *unloadFile,
                  *fileCloseAction, *fileImportAction, *geotiffOpenAction, *findFeatures, *definePolygons, *outputFeature, *changeFeature,
                  *changeMosaic, *startMosaic, *exportImage, *highlightNone, *highlightAll, *highlightChecked, *highlightMod, *highlightUser01,
                  *highlightUser02, *highlightUser03, *highlightUser04, *highlightUser05, *highlightUser06, *highlightUser07, *highlightUser08,
                  *highlightUser09, *highlightUser10, *highlightInt, *highlightMult, *highlightCount, *highlightIHOS, *highlightIHO1,
                  *highlightIHO2, *highlightPercent, *layer[MAX_ABE_PFMS], *start3D, *recentFileAction[MAX_RECENT], *displayFeature[4], *bHelp,
                  *popup0, *popup1, *popup2, *popup3, *popup4, *popupHelp, *covPopup0, *covPopup1, *covPopup2, *covPopup3, *messageAction;

  QActionGroup    *recentGrp;

  QIcon           highlightIcon[NUM_HIGHLIGHTS], displayFeatureIcon[4];

  QPixmap         *stickpin;

  QFont           prev_font;

  int32_t         menu_cursor_x, menu_cursor_y, cov_menu_cursor_x, cov_menu_cursor_y, pfmEditMod, pfmEditFilt, abe_register_group,
                  prev_icon_size, recent_file_flag, programTimerCount, mv_marker, mv_tracker, mv_arrow, mv_rectangle, pfm3D_polygon, mv_polygon,
                  rb_rectangle, rb_polygon, cov_rb_rectangle, cov_mv_rectangle, redraw_count, editor_mode[4], save_poly_count;

  uint8_t         pfm_edit_active, popup_active, cov_popup_active, pfmEdit_stopped, cov_area_defined, logo_first, double_click, force_3d_edit,
                  threeD_edit, need_redraw, pfmViewMod, command_file_flag, process_running, scale_loop;

  char            cube_name[128], ge_tmp_name[2][1024];

  FILE            *ge_tmp_fp[2];

  double          ss_cell_size_x, ss_cell_size_y, menu_cursor_lon, menu_cursor_lat, cov_menu_cursor_lon, cov_menu_cursor_lat, prev_poly_lat,
                  prev_poly_lon, cov_start_drag_lon, cov_start_drag_lat, save_poly_x[2000], save_poly_y[2000];


  NV_F64_XYMBR    command_line_mbr, cov_drag_bounds;

  NV_F64_COORD2   googleEarthPushpin;

  NVMAP_DEF       mapdef, covdef;

  QString         importTempFile, program, commandFile, zoneName, attrstring, labelColorString[2];

  OPTIONS         options;

  MISC            misc;

  QProcess        *editProc, *cubeProc, *googleEarthProc, *importProc, *mosaicProc, *threeDProc;

  QStatusBar      *covStatus, *mapStatus, *attrStatus;

  QPalette        label_palette[2], blankPalette, colorPalette[2], colorBoxPalette[2], attrPalette[PRE_ATTR + POST_ATTR];

  QColor          labelColor[2];

  QCursor         zoomCursor, editFeatureCursor, deleteFeatureCursor, addFeatureCursor, editRectCursor, editPolyCursor, 
                  filterRectCursor, filterPolyCursor, filterMaskRectCursor, filterMaskPolyCursor, filterMaskPointCursor,
                  remispCursor, drawContourCursor, pencilCursor, highCursor, lowCursor, grabContourCursor, stickpinCursor;

  QToolBar        *toolBar[NUM_TOOLBARS];

  QToolButton     *bOpen, *bZoomOut, *bZoomIn, *bPrefs, *bEditMode, *bFilterDisplay, *bFilterRect, *bFilterPoly,
                  *bFilterRectMask, *bFilterPolyMask, *bClearMasks, *bOpenGeotiff, *bDisplayMinMax, *bSetChecked, *bUnload, *bRedraw,
                  *bAutoRedraw, *bRedrawCoverage, *bDisplaySuspect, *bDisplayFeature, *bDisplayChildren, *bDisplayFeatureInfo,
                  *bDisplayFeaturePoly, *bDisplaySelected, *bDisplayReference, *bRemisp, *bRemispFilter, *bDrawContour, *bGrabContour,
                  *bDrawContourFilter, *bClearFilterContours, *bGoogleEarth, *bGoogleEarthPin, *bLink, *bCube, *bHigh, *bLow, *bStop,
                  *bHighlight, *bStoplight, *bContour, *bGrid, *bCoast, *bCovCoast, *bGeotiff, *bAddFeature, *bDeleteFeature, *bEditFeature,
                  *bVerifyFeatures, *bSurface[NUM_SURFACES], *bSetOtfBin, *bDefaultSize, *bDefaultScale;

  QButtonGroup    *surfaceGrp;

  unloadDialog    *unloadDlg[100];

  int32_t         unload_count[100];


  void killGoogleEarth ();
  uint8_t positionGoogleEarth ();
  void moveMap (int32_t direction);
  void zoomIn (NV_F64_XYMBR bounds, uint8_t pfm3D);
  void zoomOut ();
  void redrawMap (uint8_t clear, uint8_t pfm3D);
  void editFeatureNum (int32_t feature_number);
  void readStandardError (QProcess *proc);

  void runCube (uint8_t displayed_area);
  void setSurfaceValidity (int32_t layer);
  void editCommand (double *mx, double *my, int32_t count);
  void redrawCovBounds ();
  void drawCovFeatures ();
  void setWidgetStates (uint8_t enabled);
  void discardMovableObjects ();
  void initializeMaps ();
  void closeEvent (QCloseEvent *event);
  void setFunctionCursor (int32_t function);
  void clearFilterMasks ();

  void midMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);
  void leftMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);
  void rightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);

  void covMidMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);
  void covLeftMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);
  void covRightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat);

  void setStatusAttributes ();

protected slots:

  void slotCheckMenu (QAction *action);

  void slotPopupMenu0 ();
  void slotPopupMenu1 ();
  void slotPopupMenu2 ();
  void slotPopupMenu3 ();
  void slotPopupMenu4 ();
  void slotPopupHelp ();

  void slotMouseDoubleClick (QMouseEvent *e, double lon, double lat);
  void slotMousePress (QMouseEvent *e, double lon, double lat);
  void slotPreliminaryMousePress (QMouseEvent *e);
  void slotMouseRelease (QMouseEvent *e, double lon, double lat);
  void slotMouseMove (QMouseEvent *e, double lon, double lat);
  void slotResize (QResizeEvent *e);
  void slotClose (QCloseEvent *e);
  void slotPreRedraw (NVMAP_DEF mapdef);

  void slotGoogleEarth (bool checked);
  void slotGoogleEarthError (QProcess::ProcessError error);
  void slotGoogleEarthDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotLink ();
  void slotRegisterABEKeySet (int32_t key);
  void slotTrackCursor ();

  void slotCovPopupMenu0 ();
  void slotCovPopupMenu1 ();
  void slotCovPopupMenu2 ();
  void slotCovPopupHelp ();

  void slotCovMousePress (QMouseEvent *e, double lon, double lat);
  void slotCovMouseRelease (QMouseEvent *e, double lon, double lat);
  void slotCovMouseMove (QMouseEvent *e, double lon, double lat);
  void slotCovPreRedraw (NVMAP_DEF mapdef);
  void slotCovPostRedraw (NVMAP_DEF mapdef);

  void slotQuit ();
  void slotPostRedraw (NVMAP_DEF mapdef);

  void slotOpen ();
  void slotOpenRecent (QAction *action);
  void slotClosePFM ();
  void slotDataTypeButtonsPressed (QMouseEvent *e);
  void slotImport ();
  void slotOpenGeotiff ();
  void slotEditMode (int id);
  void slotZoomIn ();
  void slotZoomOut ();
  void slotDefaultSizeClicked ();
  void slotDisplayedAreaReturnPressed ();
  void slotDefaultScaleClicked ();
  void slotViewChartScaleReturnPressed ();
  void slotFilterDisplay ();
  void slotClearFilterMasks ();
  void slotRedraw ();
  void slotAutoRedraw ();
  void slotRedrawCoverage (); 
  void slotDisplaySuspect ();
  void slotFeatureMenu (QAction *action);
  void slotDisplayChildren ();
  void slotDisplayFeatureInfo ();
  void slotDisplayFeaturePoly ();
  void slotDisplaySelected ();
  void slotDisplayReference ();
  void slotDisplayMinMax ();
  void slotUnloadDialogComplete (int32_t dialog, int32_t error);
  void slotUnloadMenu (QAction *action);
  void slotCube ();

  void slotStop ();

  void slotHighlightMenu (QAction *action);

  void slotLockValueDone (uint8_t accepted);
  void slotMinScalePressed (QMouseEvent *e);
  void slotMaxScalePressed (QMouseEvent *e);

  void slotLayerClicked (int id);
  void slotStoplight ();
  void slotContour ();
  void slotGrid ();
  void slotCoast ();
  void slotCovCoast ();
  void slotGeotiff ();

  void slotSurface (int id);
  void slotSetOtfBin ();
  void slotOtfDataChanged ();
  void slotVerifyFeatures ();
  void slotClearFilterContours ();
  void slotAttrBoxClicked (QMouseEvent *e, int id);
  void slotLatitudeClicked (QMouseEvent *e, int id);
  void slotColorByClicked (QMouseEvent *e, int id);

  void slotEditDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotEditError (QProcess::ProcessError error);
  void slotEditReadyReadStandardError ();
  void slotEditReadyReadStandardOutput ();
  void slotCubeError (QProcess::ProcessError error);
  void slotCubeDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotCubeReadyReadStandardError ();
  void slotCubeReadyReadStandardOutput ();
  void slotImportError (QProcess::ProcessError error);
  void slotImportDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotImportReadyReadStandardError ();

  void slotLayerGrpTriggered (QAction *action);
  void slotLayers ();
  void slotManageLayersDataChanged ();

  void extendedHelp ();
  void slotToolbarHelp ();

  void slotChangeMosaic ();
  void slotStartMosaicViewer ();
  void slotMosaicError (QProcess::ProcessError error);
  void slotMosaicDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slotMosaicReadyReadStandardError ();
  void slotProgramTimer ();

  void slotStart3DViewer ();
  void slot3DError (QProcess::ProcessError error);
  void slot3DDone (int exitCode, QProcess::ExitStatus exitStatus);
  void slot3DReadyReadStandardError ();

  void slotExportImage ();

#ifdef ET_PHONE_HOME
  void slotReplyFinished (QNetworkReply *reply);
#endif

  void about ();
  void slotAcknowledgments ();
  void aboutQt ();

  void slotPrefs ();
  void slotPrefDataChanged (uint8_t feature_search_changed);
  void slotPrefHotKeyChanged (int32_t i);

  void slotOverlays ();
  void slotManageOverlaysDataChanged ();

  void slotDeleteFile ();
  void slotDeleteFileDataChanged ();
  void slotDeleteQueue ();

  void slotChangePath ();
  void slotChangePathDataChanged ();

  void slotFindFeature ();
  void slotDefinePolygonChartScaleChanged ();
  void slotDefinePolygon ();
  void slotOutputFeature ();
  void slotChangeFeature ();
  void slotEditFeatureDataChanged ();
  void slotEditFeatureDefinePolygon ();

  void slotDisplayHeader ();

  void slotReadStandardError (QProcess *proc);
  void slotMessage ();
  void slotMessageDialogClosed ();

  void slotOutputDataPoints ();

  void slotZoomToArea ();

  void slotDefineRectArea ();
  void slotDefinePolyArea ();

  void setStoplight ();


private:

  void keyPressEvent (QKeyEvent *e);

};

#endif
