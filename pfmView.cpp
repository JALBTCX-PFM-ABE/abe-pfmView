
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

#ifdef NVLinux
  #include <signal.h>
#endif

//  pfmView class.

/***************************************************************************/
/*!

   - Module :        pfmView

   - Programmer :    Jan C. Depner

   - Date :          01/31/05

   - Purpose :       C++/Qt replacement for C/Motif Area-Based Editor viewer.
                     This program is the PFM surface viewer for the Area-Based
                     Editor.  It is the driver for the actual point editor,
                     pfmEdit3D.  It is also the driver for the 3D PFM surface
                     viewer, pfm3D, and the mosaic viewer, mosaicView.

\***************************************************************************/
    
#include "pfmView.hpp"
#include "pfmViewHelp.hpp"
#include "acknowledgments.hpp"

#include <getopt.h>

#include "release.hpp"


uint8_t envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);
void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow);


pfmView::pfmView (int *argc, char **argv, QWidget *parent):
  QMainWindow (parent, 0)
{
  void set_defaults (MISC *misc, OPTIONS *options, uint8_t restore);


#ifdef ET_PHONE_HOME


  //  For public domain versions we want this program to check for new versions when it starts.

  QNetworkAccessManager *manager = new QNetworkAccessManager (this);
  connect(manager, SIGNAL (finished (QNetworkReply *)), this, SLOT (slotReplyFinished (QNetworkReply *)));

  QNetworkRequest request (QUrl ("http://pfmabe.software/downloads/CURRENT_VERSION.txt"));
  request.setRawHeader ("User-Agent", "Mozilla Firefox");
  manager->get (request);

#endif


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set a few defaults for startup

  strcpy (misc.progname, argv[0]);
  pfm_edit_active = NVFalse;
  threeD_edit = NVFalse;
  cov = NULL;
  googleEarthProc = NULL;
  ge_tmp_fp[0] = ge_tmp_fp[1] = NULL;
  strcpy (ge_tmp_name[0], "");
  strcpy (ge_tmp_name[1], "");
  googleEarthPushpin.x = googleEarthPushpin.y = -1000.0;
  mosaicProc = NULL;
  threeDProc = NULL;
  prefs_dialog = NULL;
  prev_poly_lat = -91.0;
  prev_poly_lon = -181.0;
  popup_active = NVFalse;
  cov_popup_active = NVFalse;
  double_click = NVFalse;
  pfmEditMod = 0;
  pfmEditFilt = 0;
  mv_marker = -1;
  mv_tracker = -1;
  mv_arrow = -1;
  mv_rectangle = -1;
  pfm3D_polygon = -1;
  rb_rectangle = -1;
  rb_polygon = -1;
  cov_rb_rectangle = -1;
  cov_mv_rectangle = -1;
  force_3d_edit = NVFalse;
  need_redraw = NVFalse;
  pfmEdit_stopped = NVFalse;
  pfmViewMod = NVFalse;
  logo_first = NVTrue;
  recent_file_flag = 0;
  command_file_flag = NVFalse;
  process_running = NVFalse;
  redraw_count = 0;
  command_line_mbr.max_x = 999.0;
  for (int32_t i = 0 ; i < 100 ; i++) misc.unload_type[i] = -1;
  misc.unload_window_x = 0;
  misc.unload_window_y = 0;
  misc.unload_window_width = 400;
  misc.unload_window_height = 600;
  displayMessage_dialog = NULL;
  otf_dialog = NULL;
  misc.messages = new QStringList ();
  misc.num_messages = 0;
  scale_loop = NVFalse;


  //  Set all of the possible attributes (according to ABE).

  setTimeAttributes (&options.time_attribute);
  setGSFAttributes (options.gsf_attribute);
  setHOFAttributes (options.hof_attribute);
  setTOFAttributes (options.tof_attribute);
  setCZMILAttributes (options.czmil_attribute, options.czmil_flag_name);
  setLASAttributes (options.las_attribute);
  setBAGAttributes (options.bag_attribute);


  //  Check to see if we have the cube program available.

#ifdef NVWIN3X
  strcpy (cube_name, "cube_pfm.exe");
#else
  strcpy (cube_name, "cube_pfm");
#endif


  misc.cube_available = NVTrue;
  if (find_startup_name (cube_name) == NULL) misc.cube_available = NVFalse;


  for (int32_t pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
    {
      misc.average_type[pfm] = 0;
      misc.cube_attr_available[pfm] = NVFalse;
      misc.last_saved_contour_record[pfm] = 0;
      misc.pfm_alpha[pfm] = 255;
    }


  //  Make the "marker" cursor painter path.

  marker = QPainterPath ();

  marker.moveTo (0, 0);
  marker.lineTo (30, 0);
  marker.lineTo (30, 20);
  marker.lineTo (0, 20);
  marker.lineTo (0, 0);

  marker.moveTo (0, 10);
  marker.lineTo (12, 10);

  marker.moveTo (30, 10);
  marker.lineTo (18, 10);

  marker.moveTo (15, 0);
  marker.lineTo (15, 6);

  marker.moveTo (15, 20);
  marker.lineTo (15, 14);


  //  Make the "arrow" painter path.

  arrow = QPainterPath ();

  arrow.moveTo (0, 0);
  arrow.moveTo (5, 25);
  arrow.lineTo (5, -15);
  arrow.lineTo (10, -15);
  arrow.lineTo (0, -25);
  arrow.lineTo (-10, -15);
  arrow.lineTo (-5, -15);
  arrow.lineTo (-5, 25);
  arrow.lineTo (5, 25);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfm_abe.png"));


  //  Set up the cursors.

  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.png"), 11, 11);
  editFeatureCursor = QCursor (QPixmap (":/icons/edit_feature_cursor.png"), 15, 15);
  deleteFeatureCursor = QCursor (QPixmap (":/icons/delete_feature_cursor.png"), 15, 15);
  addFeatureCursor = QCursor (QPixmap (":/icons/add_feature_cursor.png"), 15, 15);
  editRectCursor = QCursor (QPixmap (":/icons/edit_rect_cursor.png"), 1, 1);
  editPolyCursor = QCursor (QPixmap (":/icons/edit_poly_cursor.png"), 1, 1);
  filterRectCursor = QCursor (QPixmap (":/icons/filter_rect_cursor.png"), 1, 1);
  filterPolyCursor = QCursor (QPixmap (":/icons/filter_poly_cursor.png"), 1, 1);
  filterMaskRectCursor = QCursor (QPixmap (":/icons/filter_mask_rect_cursor.png"), 1, 1);
  filterMaskPolyCursor = QCursor (QPixmap (":/icons/filter_mask_poly_cursor.png"), 1, 1);
  remispCursor = QCursor (QPixmap (":/icons/remisp_cursor.png"), 1, 1);
  drawContourCursor = QCursor (QPixmap (":/icons/draw_contour_cursor.png"), 1, 1);
  pencilCursor = QCursor (QPixmap (":/icons/pencil_cursor.png"), 6, 30);
  highCursor = QCursor (QPixmap (":/icons/high_cursor.png"), 1, 1);
  lowCursor = QCursor (QPixmap (":/icons/low_cursor.png"), 1, 1);
  grabContourCursor = QCursor (QPixmap (":/icons/grab_contour_cursor.png"), 5, 1);
  stickpinCursor = QCursor (QPixmap (":/icons/stickpin_cursor.png"), 9, 31);
  stickpin = new QPixmap (":/icons/stickpin.png");



  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  this->setWindowTitle (misc.program_version);


  //  We have to get the icon size out of sequence (the rest of the options are read in env_in_out.cpp)
  //  so that we'll have the proper sized icons for the toolbars.  Otherwise, they won't be placed correctly.

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + QString (misc.qsettings_app) + ".ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + QString (misc.qsettings_app) + ".ini";
#endif


  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup (misc.qsettings_app);
  options.main_button_icon_size = settings.value (QString ("main button icon size"), options.main_button_icon_size).toInt ();
  settings.endGroup ();


  //  Create the Edit toolbar.  Hopefully, all of this toolbar setup stuff is intuitively obvious to the most
  //  casual observer ;-)

  toolBar[0] = new QToolBar (tr ("Edit tool bar"));
  toolBar[0]->setToolTip (tr ("Edit tool bar"));
  toolBar[0]->setWhatsThis (tr ("The Edit tool bar allows you to make changes to the PFM(s) or edit the underlying point cloud."));
  addToolBar (toolBar[0]);
  toolBar[0]->setObjectName (tr ("Edit tool bar"));

  bOpen = new QToolButton (this);
  bOpen->setIcon (QIcon (":/icons/fileopen.png"));
  bOpen->setToolTip (tr ("Open PFM file"));
  bOpen->setWhatsThis (openText);
  connect (bOpen, SIGNAL (clicked ()), this, SLOT (slotRegularOpenClicked ()));
  toolBar[0]->addWidget (bOpen);
  
  bOpenEnhanced = new QToolButton (this);
  bOpenEnhanced->setIcon (QIcon (":/icons/fileopen-enhanced.png"));
  bOpenEnhanced->setToolTip (tr ("Open PFM file"));
  bOpenEnhanced->setWhatsThis (openText);
  connect (bOpenEnhanced, SIGNAL (clicked ()), this, SLOT (slotEnhancedOpenClicked()));
  toolBar[0]->addWidget (bOpenEnhanced);

  

  //  Get the normal background color

  blankPalette.setColor (QPalette::Window, bOpen->palette ().color (QWidget::backgroundRole ()));
  blankPalette.setColor (QPalette::WindowText, bOpen->palette ().color (QWidget::backgroundRole ()));
  

  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  QButtonGroup *editModeGrp = new QButtonGroup (this);
  connect (editModeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotEditMode (int)));
  editModeGrp->setExclusive (true);


  dataTypeButtons = new dataTypeButtonBox (this);
  dataTypeButtons->setIcon (QIcon (":/icons/inv_ref_11.png"));
  dataTypeButtons->setToolTip (tr ("Select/deselect sending invalid and/or reference data to the point cloud editor")); 
  dataTypeButtons->setWhatsThis (dataTypeButtonsText);
  dataTypeButtons->setCheckable (false);
  connect (dataTypeButtons, SIGNAL (mousePressSignal (QMouseEvent *)), this, SLOT (slotDataTypeButtonsPressed (QMouseEvent *)));
  toolBar[0]->addWidget (dataTypeButtons);


  bEditMode = new QToolButton (this);
  bEditMode->setCheckable (true);
  bEditMode->setWhatsThis (editModeText);
  misc.button[EDIT_MODE_KEY] = bEditMode;
  editModeGrp->addButton (bEditMode, RECT_EDIT_AREA_3D);
  bEditMode->setIcon (QIcon (":/icons/edit_rect.png"));
  toolBar[0]->addWidget (bEditMode);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  bFilterDisplay = new QToolButton (this);
  bFilterDisplay->setIcon (QIcon (":/icons/filter_display.png"));
  bFilterDisplay->setToolTip (tr ("Filter the displayed area"));
  bFilterDisplay->setWhatsThis (filterDisplayText);
  connect (bFilterDisplay, SIGNAL (clicked ()), this, SLOT (slotFilterDisplay ()));
  toolBar[0]->addWidget (bFilterDisplay);

  bFilterRect = new QToolButton (this);
  bFilterRect->setIcon (QIcon (":/icons/filter_rect.png"));
  bFilterRect->setToolTip (tr ("Filter a rectangular area"));
  bFilterRect->setWhatsThis (filterRectText);
  editModeGrp->addButton (bFilterRect, RECT_FILTER_AREA);
  bFilterRect->setCheckable (true);
  toolBar[0]->addWidget (bFilterRect);

  bFilterPoly = new QToolButton (this);
  bFilterPoly->setIcon (QIcon (":/icons/filter_poly.png"));
  bFilterPoly->setToolTip (tr ("Filter a polygonal area"));
  bFilterPoly->setWhatsThis (filterPolyText);
  editModeGrp->addButton (bFilterPoly, POLY_FILTER_AREA);
  bFilterPoly->setCheckable (true);
  toolBar[0]->addWidget (bFilterPoly);

  bFilterRectMask = new QToolButton (this);
  bFilterRectMask->setIcon (QIcon (":/icons/filter_mask_rect.png"));
  bFilterRectMask->setToolTip (tr ("Mask a rectangular area from the filter"));
  bFilterRectMask->setWhatsThis (filterRectMaskText);
  editModeGrp->addButton (bFilterRectMask, RECT_FILTER_MASK);
  bFilterRectMask->setCheckable (true);
  toolBar[0]->addWidget (bFilterRectMask);

  bFilterPolyMask = new QToolButton (this);
  bFilterPolyMask->setIcon (QIcon (":/icons/filter_mask_poly.png"));
  bFilterPolyMask->setToolTip (tr ("Mask a polygonal area from the filter"));
  bFilterPolyMask->setWhatsThis (filterPolyMaskText);
  editModeGrp->addButton (bFilterPolyMask, POLY_FILTER_MASK);
  bFilterPolyMask->setCheckable (true);
  toolBar[0]->addWidget (bFilterPolyMask);


  bClearMasks = new QToolButton (this);
  bClearMasks->setIcon (QIcon (":/icons/clear_filter_masks.png"));
  bClearMasks->setToolTip (tr ("Clear filter masks"));
  bClearMasks->setWhatsThis (clearMasksText);
  connect (bClearMasks, SIGNAL (clicked ()), this, SLOT (slotClearFilterMasks ()));
  toolBar[0]->addWidget (bClearMasks);


  toolBar[0]->addSeparator ();
  toolBar[0]->addSeparator ();


  QMenu *checkMenu = new QMenu (this);

  QActionGroup *checkGrp = new QActionGroup (this);
  connect (checkGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotCheckMenu (QAction *)));

  setChecked = checkMenu->addAction (tr ("Set displayed area checked"));
  setUnchecked = checkMenu->addAction (tr ("Set displayed area unchecked"));
  setVerified = checkMenu->addAction (tr ("Set displayed area verified"));
  setUnverified = checkMenu->addAction (tr ("Set displayed area unverified"));
  setMultiChecked = checkMenu->addAction (tr ("Set multiple coverage checked"));
  setMultiValidChecked = checkMenu->addAction (tr ("Set valid multiple coverage checked"));
  checkGrp->addAction (setChecked);
  checkGrp->addAction (setUnchecked);
  checkGrp->addAction (setVerified);
  checkGrp->addAction (setUnverified);
  checkGrp->addAction (setMultiChecked);
  checkGrp->addAction (setMultiValidChecked);

  bSetChecked = new QToolButton (this);
  bSetChecked->setIcon (QIcon (":/icons/setchecked.png"));
  bSetChecked->setToolTip (tr ("Set displayed area (un)checked/verified"));
  bSetChecked->setWhatsThis (setCheckedText);
  bSetChecked->setPopupMode (QToolButton::InstantPopup);
  bSetChecked->setMenu (checkMenu);
  toolBar[0]->addWidget (bSetChecked);

  QMenu *unloadMenu = new QMenu (this);

  QActionGroup *unloadGrp = new QActionGroup (this);
  connect (unloadGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotUnloadMenu (QAction *)));

  unloadDisplayed = unloadMenu->addAction (tr ("Unload edits for displayed area"));
  unloadFile = unloadMenu->addAction (tr ("Unload edits for entire PFM"));
  unloadGrp->addAction (unloadDisplayed);
  unloadGrp->addAction (unloadFile);

  bUnload = new QToolButton (this);
  bUnload->setIcon (QIcon (":/icons/unload.png"));
  bUnload->setToolTip (tr ("Unload edits for displayed area or entire PFM"));
  bUnload->setWhatsThis (unloadText);
  bUnload->setPopupMode (QToolButton::InstantPopup);
  bUnload->setMenu (unloadMenu);
  toolBar[0]->addWidget (bUnload);


  //  Create the View toolbar

  toolBar[1] = new QToolBar (tr ("View tool bar"));
  toolBar[1]->setToolTip (tr ("View tool bar"));
  toolBar[1]->setWhatsThis (tr ("The View tool bar allows you to change the way the data is viewed (e.g. color, highlighting)."));
  addToolBar (toolBar[1]);
  toolBar[1]->setObjectName (tr ("View tool bar"));

  bStop = new QToolButton (this);
  bStop->setIcon (QIcon (":/icons/stop.png"));
  bStop->setToolTip (tr ("Stop drawing"));
  bStop->setWhatsThis (stopText);
  bStop->setEnabled (false);
  connect (bStop, SIGNAL (clicked ()), this, SLOT (slotStop ()));
  toolBar[1]->addWidget (bStop);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bZoomIn = new QToolButton (this);
  bZoomIn->setIcon (QIcon (":/icons/icon_zoomin.png"));
  bZoomIn->setToolTip (tr ("Zoom in"));
  bZoomIn->setWhatsThis (zoomInText);
  connect (bZoomIn, SIGNAL (clicked ()), this, SLOT (slotZoomIn ()));
  toolBar[1]->addWidget (bZoomIn);

  bZoomOut = new QToolButton (this);
  bZoomOut->setIcon (QIcon (":/icons/icon_zoomout.png"));
  bZoomOut->setToolTip (tr ("Zoom out"));
  bZoomOut->setWhatsThis (zoomOutText);
  connect (bZoomOut, SIGNAL (clicked ()), this, SLOT (slotZoomOut ()));
  toolBar[1]->addWidget (bZoomOut);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bRedraw = new QToolButton (this);
  misc.button[REDRAW_KEY] = bRedraw;
  bRedraw->setIcon (misc.buttonIcon[REDRAW_KEY]);
  bRedraw->setWhatsThis (redrawText);
  connect (bRedraw, SIGNAL (clicked ()), this, SLOT (slotRedraw ()));
  toolBar[1]->addWidget (bRedraw);

  bRedrawCoverage = new QToolButton (this);
  misc.button[REDRAW_COVERAGE_KEY] = bRedrawCoverage;
  bRedrawCoverage->setIcon (misc.buttonIcon[REDRAW_COVERAGE_KEY]);
  bRedrawCoverage->setWhatsThis (redrawCoverageText);
  connect (bRedrawCoverage, SIGNAL (clicked ()), this, SLOT (slotRedrawCoverage ()));
  toolBar[1]->addWidget (bRedrawCoverage);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bContour = new QToolButton (this);
  misc.button[TOGGLE_CONTOUR_KEY] = bContour;
  bContour->setIcon (misc.buttonIcon[TOGGLE_CONTOUR_KEY]);
  bContour->setWhatsThis (contourText);
  bContour->setCheckable (true);
  bContour->setEnabled (false);
  toolBar[1]->addWidget (bContour);

  bGrid = new QToolButton (this);
  misc.button[TOGGLE_GRID_KEY] = bGrid;
  bGrid->setIcon (misc.buttonIcon[TOGGLE_GRID_KEY]);
  bGrid->setWhatsThis (gridText);
  bGrid->setCheckable (true);
  bGrid->setEnabled (false);
  toolBar[1]->addWidget (bGrid);

  bCoast = new QToolButton (this);
  bCoast->setIcon (QIcon (":/icons/coast.png"));
  bCoast->setToolTip (tr ("Toggle main map coastline display"));
  bCoast->setWhatsThis (coastText);
  bCoast->setCheckable (true);
  bCoast->setEnabled (false);
  toolBar[1]->addWidget (bCoast);

  bCovCoast = new QToolButton (this);
  bCovCoast->setIcon (QIcon (":/icons/cov_coast.png"));
  bCovCoast->setToolTip (tr ("Toggle coverage map coastline display"));
  bCovCoast->setWhatsThis (covCoastText);
  bCovCoast->setCheckable (true);
  bCovCoast->setEnabled (false);
  toolBar[1]->addWidget (bCovCoast);

  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bGeotiff = new QToolButton (this);
  misc.button[TOGGLE_GEOTIFF_KEY] = bGeotiff;
  bGeotiff->setIcon (misc.buttonIcon[TOGGLE_GEOTIFF_KEY]);
  bGeotiff->setWhatsThis (geoText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  toolBar[1]->addWidget (bGeotiff);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bAutoRedraw = new QToolButton (this);
  bAutoRedraw->setIcon (QIcon (":/icons/autoredraw.png"));
  bAutoRedraw->setToolTip (tr ("Auto redraw after edit"));
  bAutoRedraw->setWhatsThis (autoRedrawText);
  bAutoRedraw->setCheckable (true);
  toolBar[1]->addWidget (bAutoRedraw);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bDisplayMinMax = new QToolButton (this);
  bDisplayMinMax->setIcon (QIcon (":/icons/display_min_max.png"));
  bDisplayMinMax->setToolTip (tr ("Flag minimum and maximum bins"));
  bDisplayMinMax->setCheckable (true);
  toolBar[1]->addWidget (bDisplayMinMax);


  QMenu *highlightMenu = new QMenu (this);

  QActionGroup *highlightGrp = new QActionGroup (this);
  connect (highlightGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotHighlightMenu (QAction *)));

  for (int32_t i = 0 ; i < NUM_HIGHLIGHTS ; i++)
    {
      switch (i)
        {
        case H_NONE:
          highlightIcon[i] = QIcon (":/icons/highlight_none.png");
          break;

        case H_ALL:
          highlightIcon[i] = QIcon (":/icons/highlight_all.png");
          break;

        case H_CHECKED:
          highlightIcon[i] = QIcon (":/icons/highlight_checked.png");
          break;

        case H_MODIFIED:
          highlightIcon[i] = QIcon (":/icons/highlight_mod.png");
          break;

        case H_01:
          highlightIcon[i] = QIcon (":/icons/highlight01.png");
          break;

        case H_02:
          highlightIcon[i] = QIcon (":/icons/highlight02.png");
          break;

        case H_03:
          highlightIcon[i] = QIcon (":/icons/highlight03.png");
          break;

        case H_04:
          highlightIcon[i] = QIcon (":/icons/highlight04.png");
          break;

        case H_05:
          highlightIcon[i] = QIcon (":/icons/highlight05.png");
          break;

        case H_06:
          highlightIcon[i] = QIcon (":/icons/highlight06.png");
          break;

        case H_07:
          highlightIcon[i] = QIcon (":/icons/highlight07.png");
          break;

        case H_08:
          highlightIcon[i] = QIcon (":/icons/highlight08.png");
          break;

        case H_09:
          highlightIcon[i] = QIcon (":/icons/highlight09.png");
          break;

        case H_10:
          highlightIcon[i] = QIcon (":/icons/highlight10.png");
          break;

        case H_INT:
          highlightIcon[i] = QIcon (":/icons/highlight_int.png");
          break;

        case H_MULT:
          highlightIcon[i] = QIcon (":/icons/highlight_mult.png");
          break;

        case H_COUNT:
          highlightIcon[i] = QIcon (":/icons/highlight_count.png");
          break;

        case H_IHO_S:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_S.png");
          break;

        case H_IHO_1:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_1.png");
          break;

        case H_IHO_2:
          highlightIcon[i] = QIcon (":/icons/highlight_IHO_2.png");
          break;

        case H_PERCENT:
          highlightIcon[i] = QIcon (":/icons/highlight_percent.png");
          break;
        }
    }

  highlightNone = highlightMenu->addAction (tr ("No highlighting"));
  highlightNone->setIcon (highlightIcon[H_NONE]);
  highlightAll = highlightMenu->addAction (tr ("Highlight all"));
  highlightAll->setIcon (highlightIcon[H_ALL]);
  highlightChecked = highlightMenu->addAction (tr ("Highlight checked/verified data"));
  highlightChecked->setIcon (highlightIcon[H_CHECKED]);
  highlightMod = highlightMenu->addAction (tr ("Highlight modified data"));
  highlightMod->setIcon (highlightIcon[H_MODIFIED]);
  highlightUser01 = highlightMenu->addAction (tr ("Highlight PFM_USER_01 data"));
  highlightUser01->setIcon (highlightIcon[H_01]);
  highlightUser02 = highlightMenu->addAction (tr ("Highlight PFM_USER_02 data"));
  highlightUser02->setIcon (highlightIcon[H_02]);
  highlightUser03 = highlightMenu->addAction (tr ("Highlight PFM_USER_03 data"));
  highlightUser03->setIcon (highlightIcon[H_03]);
  highlightUser04 = highlightMenu->addAction (tr ("Highlight PFM_USER_04 data"));
  highlightUser04->setIcon (highlightIcon[H_04]);
  highlightUser05 = highlightMenu->addAction (tr ("Highlight PFM_USER_05 data"));
  highlightUser05->setIcon (highlightIcon[H_05]);
  highlightUser06 = highlightMenu->addAction (tr ("Highlight PFM_USER_06 data"));
  highlightUser06->setIcon (highlightIcon[H_06]);
  highlightUser07 = highlightMenu->addAction (tr ("Highlight PFM_USER_07 data"));
  highlightUser07->setIcon (highlightIcon[H_07]);
  highlightUser08 = highlightMenu->addAction (tr ("Highlight PFM_USER_08 data"));
  highlightUser08->setIcon (highlightIcon[H_08]);
  highlightUser09 = highlightMenu->addAction (tr ("Highlight PFM_USER_09 data"));
  highlightUser09->setIcon (highlightIcon[H_09]);
  highlightUser10 = highlightMenu->addAction (tr ("Highlight PFM_USER_10 data"));
  highlightUser10->setIcon (highlightIcon[H_10]);
  highlightInt = highlightMenu->addAction (tr ("Highlight interpolated (MISP) data"));
  highlightInt->setIcon (highlightIcon[H_INT]);
  highlightMenu->addSeparator ();
  highlightMult = highlightMenu->addAction (tr ("Highlight multiple coverage (>= 200%) areas"));
  highlightMult->setIcon (highlightIcon[H_MULT]);
  highlightCount = highlightMenu->addAction (tr ("Highlight valid bin count exceeding 00"));
  highlightCount->setIcon (highlightIcon[H_COUNT]);
  highlightIHOS = highlightMenu->addAction (tr ("Highlight IHO Special Order"));
  highlightIHOS->setIcon (highlightIcon[H_IHO_S]);
  highlightIHO1 = highlightMenu->addAction (tr ("Highlight IHO Order 1"));
  highlightIHO1->setIcon (highlightIcon[H_IHO_1]);
  highlightIHO2 = highlightMenu->addAction (tr ("Highlight IHO Order 2"));
  highlightIHO2->setIcon (highlightIcon[H_IHO_2]);
  highlightPercent = highlightMenu->addAction (tr ("Highlight 10 percent of depth"));
  highlightPercent->setIcon (highlightIcon[H_PERCENT]);
  highlightGrp->addAction (highlightNone);
  highlightGrp->addAction (highlightAll);
  highlightGrp->addAction (highlightChecked);
  highlightGrp->addAction (highlightMod);
  highlightGrp->addAction (highlightUser01);
  highlightGrp->addAction (highlightUser02);
  highlightGrp->addAction (highlightUser03);
  highlightGrp->addAction (highlightUser04);
  highlightGrp->addAction (highlightUser05);
  highlightGrp->addAction (highlightUser06);
  highlightGrp->addAction (highlightUser07);
  highlightGrp->addAction (highlightUser08);
  highlightGrp->addAction (highlightUser09);
  highlightGrp->addAction (highlightUser10);
  highlightGrp->addAction (highlightInt);
  highlightGrp->addAction (highlightMult);
  highlightGrp->addAction (highlightCount);
  highlightGrp->addAction (highlightIHOS);
  highlightGrp->addAction (highlightIHO1);
  highlightGrp->addAction (highlightIHO2);
  highlightGrp->addAction (highlightPercent);

  bHighlight = new QToolButton (this);
  bHighlight->setIcon (highlightIcon[H_NONE]);
  bHighlight->setToolTip (tr ("Toggle highlight options"));
  bHighlight->setWhatsThis (highlightText);
  bHighlight->setPopupMode (QToolButton::InstantPopup);
  bHighlight->setMenu (highlightMenu);
  toolBar[1]->addWidget (bHighlight);


  toolBar[1]->addSeparator ();
  toolBar[1]->addSeparator ();


  bStoplight = new QToolButton (this);
  bStoplight->setIcon (QIcon (":/icons/stoplight.png"));
  bStoplight->setWhatsThis (stoplightText);
  bStoplight->setCheckable (true);
  bStoplight->setEnabled (false);
  toolBar[1]->addWidget (bStoplight);


  //  Create the Surface toolbar

  toolBar[2] = new QToolBar (tr ("Surface tool bar"));
  toolBar[2]->setToolTip (tr ("Surface tool bar"));
  toolBar[2]->setWhatsThis (tr ("The Surface tool bar allows you to change the surface that you are viewing.  "
                                "For example, the minimum surface or the On-The_Fly average surface."));
  addToolBar (toolBar[2]);
  toolBar[2]->setObjectName (tr ("Surface tool bar"));


  surfaceGrp = new QButtonGroup (this);
  surfaceGrp->setExclusive (true);
  connect (surfaceGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotSurface (int)));


  bSurface[0] = new QToolButton (this);
  misc.button[DISPLAY_AVG_EDITED_KEY] = bSurface[0];
  bSurface[0]->setIcon (misc.buttonIcon[DISPLAY_AVG_EDITED_KEY]);
  if (misc.cube_available)
    {
      bSurface[0]->setWhatsThis (avgFiltCubeText);
    }
  else
    {
      bSurface[0]->setWhatsThis (avgFiltText);
    }
  surfaceGrp->addButton (bSurface[0], AVERAGE_FILTERED_DEPTH);
  bSurface[0]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[0]);

  bSurface[1] = new QToolButton (this);
  misc.button[DISPLAY_MIN_EDITED_KEY] = bSurface[1];
  bSurface[1]->setIcon (misc.buttonIcon[DISPLAY_MIN_EDITED_KEY]);
  bSurface[1]->setWhatsThis (minFiltText);
  surfaceGrp->addButton (bSurface[1], MIN_FILTERED_DEPTH);
  bSurface[1]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[1]);

  bSurface[2] = new QToolButton (this);
  misc.button[DISPLAY_MAX_EDITED_KEY] = bSurface[2];
  bSurface[2]->setIcon (misc.buttonIcon[DISPLAY_MAX_EDITED_KEY]);
  bSurface[2]->setWhatsThis (maxFiltText);
  surfaceGrp->addButton (bSurface[2], MAX_FILTERED_DEPTH);
  bSurface[2]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[2]);

  bSurface[3] = new QToolButton (this);
  misc.button[DISPLAY_AVG_UNEDITED_KEY] = bSurface[3];
  bSurface[3]->setIcon (misc.buttonIcon[DISPLAY_AVG_UNEDITED_KEY]);
  bSurface[3]->setWhatsThis (avgText);
  surfaceGrp->addButton (bSurface[3], AVERAGE_DEPTH);
  bSurface[3]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[3]);

  bSurface[4] = new QToolButton (this);
  misc.button[DISPLAY_MIN_UNEDITED_KEY] = bSurface[4];
  bSurface[4]->setIcon (misc.buttonIcon[DISPLAY_MIN_UNEDITED_KEY]);
  bSurface[4]->setWhatsThis (minText);
  surfaceGrp->addButton (bSurface[4], MIN_DEPTH);
  bSurface[4]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[4]);

  bSurface[5] = new QToolButton (this);
  misc.button[DISPLAY_MAX_UNEDITED_KEY] = bSurface[5];
  bSurface[5]->setIcon (misc.buttonIcon[DISPLAY_MAX_UNEDITED_KEY]);
  bSurface[5]->setWhatsThis (maxText);
  surfaceGrp->addButton (bSurface[5], MAX_DEPTH);
  bSurface[5]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[5]);


  toolBar[2]->addSeparator ();
  toolBar[2]->addSeparator ();


  bSurface[6] = new QToolButton (this);
  bSurface[6]->setIcon (QIcon (":/icons/avgfilt_otf.png"));
  bSurface[6]->setToolTip (tr ("Display the average edited depth/elevation surface in on-the-fly grid"));
  bSurface[6]->setWhatsThis (avgFiltOtfText);
  surfaceGrp->addButton (bSurface[6], AVERAGE_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[6]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[6]);

  bSurface[7] = new QToolButton (this);
  misc.button[DISPLAY_MIN_EDITED_OTF_KEY] = bSurface[7];
  bSurface[7]->setIcon (misc.buttonIcon[DISPLAY_MIN_EDITED_OTF_KEY]);
  bSurface[7]->setWhatsThis (minFiltOtfText);
  surfaceGrp->addButton (bSurface[7], MIN_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[7]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[7]);

  bSurface[8] = new QToolButton (this);
  misc.button[DISPLAY_MAX_EDITED_OTF_KEY] = bSurface[8];
  bSurface[8]->setIcon (misc.buttonIcon[DISPLAY_MAX_EDITED_OTF_KEY]);
  bSurface[8]->setWhatsThis (maxFiltOtfText);
  surfaceGrp->addButton (bSurface[8], MAX_FILTERED_DEPTH + OTF_OFFSET);
  bSurface[8]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[8]);

  bSurface[9] = new QToolButton (this);
  bSurface[9]->setIcon (QIcon (":/icons/avg_otf.png"));
  bSurface[9]->setToolTip (tr ("Display the average unedited depth/elevation surface in on-the-fly grid"));
  bSurface[9]->setWhatsThis (avgOtfText);
  surfaceGrp->addButton (bSurface[9], AVERAGE_DEPTH + OTF_OFFSET);
  bSurface[9]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[9]);

  bSurface[10] = new QToolButton (this);
  bSurface[10]->setIcon (QIcon (":/icons/min_otf.png"));
  bSurface[10]->setToolTip (tr ("Display the minimum unedited depth/elevation surface in on-the-fly grid"));
  bSurface[10]->setWhatsThis (minOtfText);
  surfaceGrp->addButton (bSurface[10], MIN_DEPTH + OTF_OFFSET);
  bSurface[10]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[10]);

  bSurface[11] = new QToolButton (this);
  bSurface[11]->setIcon (QIcon (":/icons/max_otf.png"));
  bSurface[11]->setToolTip (tr ("Display the maximum unedited depth/elevation surface in on-the-fly grid"));
  bSurface[11]->setWhatsThis (maxOtfText);
  surfaceGrp->addButton (bSurface[11], MAX_DEPTH + OTF_OFFSET);
  bSurface[11]->setCheckable (true);
  toolBar[2]->addWidget (bSurface[11]);

  bSetOtfBin = new QToolButton (this);
  bSetOtfBin->setIcon (QIcon (":/icons/set_otf_size.png"));
  bSetOtfBin->setWhatsThis (setOtfBinText);
  connect (bSetOtfBin, SIGNAL (clicked ()), this, SLOT (slotSetOtfBin ()));
  toolBar[2]->addWidget (bSetOtfBin);


  //  Create the Flag Data toolbar

  toolBar[3] = new QToolBar (tr ("Flag Data tool bar"));
  toolBar[3]->setToolTip (tr ("Flag Data tool bar"));
  toolBar[3]->setWhatsThis (tr ("The Flag Data tool bar allows you to flag bins that contain certain types of data (e.g. reference data)."));
  addToolBar (toolBar[3]);
  toolBar[3]->setObjectName (tr ("Flag Data tool bar"));

  bDisplaySuspect = new QToolButton (this);
  bDisplaySuspect->setIcon (QIcon (":/icons/displaysuspect.png"));
  bDisplaySuspect->setToolTip (tr ("Flag bins that contain suspect data"));
  bDisplaySuspect->setWhatsThis (displaySuspectText);
  bDisplaySuspect->setCheckable (true);
  toolBar[3]->addWidget (bDisplaySuspect);

  bDisplaySelected = new QToolButton (this);
  bDisplaySelected->setIcon (QIcon (":/icons/displayselected.png"));
  bDisplaySelected->setToolTip (tr ("Flag bins that contain selected soundings or that contain manually classified CZMIL/LAS data"));
  bDisplaySelected->setWhatsThis (displaySelectedText);
  bDisplaySelected->setCheckable (true);
  toolBar[3]->addWidget (bDisplaySelected);

  bDisplayReference = new QToolButton (this);
  bDisplayReference->setIcon (QIcon (":/icons/displayreference.png"));
  bDisplayReference->setToolTip (tr ("Flag bins that contain reference data"));
  bDisplayReference->setWhatsThis (displayReferenceText);
  bDisplayReference->setCheckable (true);
  toolBar[3]->addWidget (bDisplayReference);


  //  Create the Feature toolbar

  toolBar[4] = new QToolBar (tr ("Feature tool bar"));
  toolBar[4]->setToolTip (tr ("Feature tool bar"));
  toolBar[4]->setWhatsThis (tr ("The Feature tool bar allows you to display and modify user defined features and related information."));
  addToolBar (toolBar[4]);
  toolBar[4]->setObjectName (tr ("Feature tool bar"));

  QMenu *featureMenu = new QMenu (this);

  QActionGroup *featureGrp = new QActionGroup (this);
  connect (featureGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotFeatureMenu (QAction *)));

  displayFeatureIcon[0] = QIcon (":/icons/display_no_feature.png");
  displayFeatureIcon[1] = QIcon (":/icons/display_all_feature.png");
  displayFeatureIcon[2] = QIcon (":/icons/display_unverified_feature.png");
  displayFeatureIcon[3] = QIcon (":/icons/display_verified_feature.png");
  displayFeature[0] = featureMenu->addAction (tr ("Don't display features"));
  displayFeature[1] = featureMenu->addAction (tr ("Display all features"));
  displayFeature[2] = featureMenu->addAction (tr ("Display unverified features"));
  displayFeature[3] = featureMenu->addAction (tr ("Display verified features"));
  for (int32_t i = 0 ; i < 4 ; i++)
    {
      displayFeature[i]->setIcon (displayFeatureIcon[i]);
      featureGrp->addAction (displayFeature[i]);
    }

  bDisplayFeature = new QToolButton (this);
  bDisplayFeature->setToolTip (tr ("Select type of feature data to display"));
  bDisplayFeature->setWhatsThis (displayFeatureText);
  bDisplayFeature->setPopupMode (QToolButton::InstantPopup);
  bDisplayFeature->setMenu (featureMenu);
  toolBar[4]->addWidget (bDisplayFeature);

  bDisplayChildren = new QToolButton (this);
  bDisplayChildren->setIcon (QIcon (":/icons/displayfeaturechildren.png"));
  bDisplayChildren->setToolTip (tr ("Display feature group members"));
  bDisplayChildren->setWhatsThis (displayChildrenText);
  bDisplayChildren->setCheckable (true);
  toolBar[4]->addWidget (bDisplayChildren);

  bDisplayFeatureInfo = new QToolButton (this);
  bDisplayFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.png"));
  bDisplayFeatureInfo->setToolTip (tr ("Display feature information"));
  bDisplayFeatureInfo->setWhatsThis (displayFeatureInfoText);
  bDisplayFeatureInfo->setCheckable (true);
  toolBar[4]->addWidget (bDisplayFeatureInfo);

  bDisplayFeaturePoly = new QToolButton (this);
  bDisplayFeaturePoly->setIcon (QIcon (":/icons/displayfeaturepoly.png"));
  bDisplayFeaturePoly->setToolTip (tr ("Display feature polygonal area"));
  bDisplayFeaturePoly->setWhatsThis (displayFeaturePolyText);
  bDisplayFeaturePoly->setCheckable (true);
  toolBar[4]->addWidget (bDisplayFeaturePoly);

  bAddFeature = new QToolButton (this);
  bAddFeature->setIcon (QIcon (":/icons/addfeature.png"));
  bAddFeature->setToolTip (tr ("Add a feature"));
  bAddFeature->setWhatsThis (addFeatureText);
  editModeGrp->addButton (bAddFeature, ADD_FEATURE);
  bAddFeature->setCheckable (true);
  toolBar[4]->addWidget (bAddFeature);

  bDeleteFeature = new QToolButton (this);
  bDeleteFeature->setIcon (QIcon (":/icons/deletefeature.png"));
  bDeleteFeature->setToolTip (tr ("Delete a feature"));
  bDeleteFeature->setWhatsThis (deleteFeatureText);
  editModeGrp->addButton (bDeleteFeature, DELETE_FEATURE);
  bDeleteFeature->setCheckable (true);
  toolBar[4]->addWidget (bDeleteFeature);

  bEditFeature = new QToolButton (this);
  bEditFeature->setIcon (QIcon (":/icons/editfeature.png"));
  bEditFeature->setToolTip (tr ("Edit a feature"));
  bEditFeature->setWhatsThis (editFeatureText);
  editModeGrp->addButton (bEditFeature, EDIT_FEATURE);
  bEditFeature->setCheckable (true);
  toolBar[4]->addWidget (bEditFeature);

  bVerifyFeatures = new QToolButton (this);
  bVerifyFeatures->setIcon (QIcon (":/icons/verify_features.png"));
  bVerifyFeatures->setToolTip (tr ("Verify all visible valid features"));
  bVerifyFeatures->setWhatsThis (verifyFeaturesText);
  connect (bVerifyFeatures, SIGNAL (clicked ()), this, SLOT (slotVerifyFeatures ()));
  toolBar[4]->addWidget (bVerifyFeatures);


  //  Create the MISP/CUBE toolbar

  if (misc.cube_available)
    {
      toolBar[5] = new QToolBar (tr ("MISP/CUBE tool bar"));
      toolBar[5]->setToolTip (tr ("MISP/CUBE tool bar"));
      toolBar[5]->setWhatsThis (tr ("The MISP/CUBE tool bar allows you to perform operations on the MISP or CUBE surface stored in the PFM Average Filtered Surface layer."));
      addToolBar (toolBar[5]);
      toolBar[5]->setObjectName (tr ("MISP/CUBE tool bar"));
    }
  else
    {
      toolBar[5] = new QToolBar (tr ("MISP tool bar"));
      toolBar[5]->setToolTip (tr ("MISP tool bar"));
      toolBar[5]->setWhatsThis (tr ("The MISP tool bar allows you to perform operations on the MISP surface stored in the PFM Average Filtered Surface layer."));
      addToolBar (toolBar[5]);
      toolBar[5]->setObjectName (tr ("MISP tool bar"));
    }

  bDrawContour = new QToolButton (this);
  bDrawContour->setIcon (QIcon (":/icons/draw_contour.png"));
  bDrawContour->setToolTip (tr ("Draw and insert a contour"));
  bDrawContour->setWhatsThis (drawContourText);
  editModeGrp->addButton (bDrawContour, DRAW_CONTOUR);
  bDrawContour->setCheckable (true);
  bDrawContour->setEnabled (false);
  toolBar[5]->addWidget (bDrawContour);

  bGrabContour = new QToolButton (this);
  bGrabContour->setIcon (QIcon (":/icons/grab_contour.png"));
  bGrabContour->setToolTip (tr ("Capture and insert interpolated contours from a polygon"));
  bGrabContour->setWhatsThis (grabContourText);
  editModeGrp->addButton (bGrabContour, GRAB_CONTOUR);
  bGrabContour->setCheckable (true);
  bGrabContour->setEnabled (false);
  toolBar[5]->addWidget (bGrabContour);

  bRemisp = new QToolButton (this);
  bRemisp->setIcon (QIcon (":/icons/remisp_icon.png"));
  bRemisp->setToolTip (tr ("Regrid a rectangular area"));
  bRemisp->setWhatsThis (remispText);
  editModeGrp->addButton (bRemisp, REMISP_AREA);
  bRemisp->setCheckable (true);
  bRemisp->setEnabled (false);
  toolBar[5]->addWidget (bRemisp);


  toolBar[5]->addSeparator ();
  toolBar[5]->addSeparator ();


  bDrawContourFilter = new QToolButton (this);
  bDrawContourFilter->setIcon (QIcon (":/icons/draw_contour_filter.png"));
  bDrawContourFilter->setToolTip (tr ("Draw and insert a contour for filtering the surface"));
  bDrawContourFilter->setWhatsThis (drawContourFilterText);
  editModeGrp->addButton (bDrawContourFilter, DRAW_CONTOUR_FILTER);
  bDrawContourFilter->setCheckable (true);
  bDrawContourFilter->setEnabled (false);
  toolBar[5]->addWidget (bDrawContourFilter);

  bRemispFilter = new QToolButton (this);
  bRemispFilter->setIcon (QIcon (":/icons/remisp_filter_icon.png"));
  bRemispFilter->setToolTip (tr ("Regrid and filter a polygonal area"));
  bRemispFilter->setWhatsThis (remispFilterText);
  editModeGrp->addButton (bRemispFilter, REMISP_FILTER);
  bRemispFilter->setCheckable (true);
  bRemispFilter->setEnabled (false);
  toolBar[5]->addWidget (bRemispFilter);


  bClearFilterContours = new QToolButton (this);
  bClearFilterContours->setIcon (QIcon (":/icons/clear_filter_contours.png"));
  bClearFilterContours->setToolTip (tr ("Clear filter contours"));
  bClearFilterContours->setWhatsThis (clearFilterContoursText);
  connect (bClearFilterContours, SIGNAL (clicked ()), this, SLOT (slotClearFilterContours ()));
  toolBar[5]->addWidget (bClearFilterContours);


  toolBar[5]->addSeparator ();
  toolBar[5]->addSeparator ();


  bHigh = new QToolButton (this);
  bHigh->setIcon (QIcon (":/icons/high.png"));
  bHigh->setToolTip (tr ("Select the highest point within a rectangle"));
  bHigh->setWhatsThis (highText);
  editModeGrp->addButton (bHigh, SELECT_HIGH_POINT);
  bHigh->setCheckable (true);
  bHigh->setEnabled (false);
  toolBar[5]->addWidget (bHigh);

  bLow = new QToolButton (this);
  bLow->setIcon (QIcon (":/icons/low.png"));
  bLow->setToolTip (tr ("Select the lowest point within a rectangle"));
  bLow->setWhatsThis (lowText);
  editModeGrp->addButton (bLow, SELECT_LOW_POINT);
  bLow->setCheckable (true);
  bLow->setEnabled (false);
  toolBar[5]->addWidget (bLow);

  bCube = new QToolButton (this);
  bCube->setIcon (QIcon (":/icons/cube.png"));
  bCube->setToolTip (tr ("Re-CUBE the displayed area"));
  bCube->setWhatsThis (cubeText);
  connect (bCube, SIGNAL (clicked ()), this, SLOT (slotCube ()));
  bCube->setEnabled (false);
  bCube->setVisible (misc.cube_available);
  toolBar[5]->addWidget (bCube);


  //  Create the Utilities toolbar
 
  toolBar[6] = new QToolBar (tr ("Utilities tool bar"));
  toolBar[6]->setToolTip (tr ("Utilities tool bar"));
  toolBar[6]->setWhatsThis (tr ("The Utilities tool bar allows you to modify preferences, start/stop Google Earth, link the map cursor to other ABE applications, and get context sensitive help."));
  addToolBar (toolBar[6]);
  toolBar[6]->setObjectName (tr ("Utilities tool bar"));

  bGoogleEarth = new QToolButton (this);
  bGoogleEarth->setIcon (QIcon (":/icons/Google_Earth_Icon_small.png"));
  bGoogleEarth->setToolTip (tr ("Start Google Earth to view image of displayed area"));
  bGoogleEarth->setWhatsThis (googleEarthText);
  connect (bGoogleEarth, SIGNAL (clicked (bool)), this, SLOT (slotGoogleEarth (bool)));
  bGoogleEarth->setCheckable (true);
  bGoogleEarth->setEnabled (false);
  toolBar[6]->addWidget (bGoogleEarth);


  bGoogleEarthPin = new QToolButton (this);
  bGoogleEarthPin->setIcon (QIcon (":/icons/Google_Earth_Icon_Pin.png"));
  bGoogleEarthPin->setToolTip (tr ("Select placemark point to be displayed in Google Earth"));
  bGoogleEarthPin->setWhatsThis (googleEarthPinText);
  editModeGrp->addButton (bGoogleEarthPin, GOOGLE_EARTH);
  bGoogleEarthPin->setEnabled (false);
  toolBar[6]->addWidget (bGoogleEarthPin);


  toolBar[6]->addSeparator ();
  toolBar[6]->addSeparator ();


  bLink = new QToolButton (this);
  bLink->setIcon (QIcon (":/icons/unlink.png"));
  bLink->setToolTip (tr ("Connect to other ABE applications"));
  bLink->setWhatsThis (linkText);
  connect (bLink, SIGNAL (clicked ()), this, SLOT (slotLink ()));
  toolBar[6]->addWidget (bLink);


  toolBar[6]->addSeparator ();
  toolBar[6]->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.png"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  toolBar[6]->addWidget (bPrefs);


  toolBar[6]->addSeparator ();
  toolBar[6]->addSeparator ();


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  toolBar[6]->addAction (bHelp);


  //  Create the Chart toolbar

  toolBar[7] = new QToolBar (tr ("Chart tool bar"));
  toolBar[7]->setToolTip (tr ("Chart tool bar"));
  toolBar[7]->setWhatsThis (tr ("The Chart tool bar allows you to modify width, height, or chart scale of the displayed area."));
  addToolBar (toolBar[7]);
  toolBar[7]->setObjectName (tr ("Chart tool bar"));


  bDefaultSize = new QToolButton (this);
  bDefaultSize->setIcon (QIcon (":/icons/default_size.png"));
  bDefaultSize->setWhatsThis (defaultSizeText);
  connect (bDefaultSize, SIGNAL (clicked ()), this, SLOT (slotDefaultSizeClicked ()));
  toolBar[7]->addWidget (bDefaultSize);


  displayedArea = new QLineEdit (this);
  displayedArea->setToolTip (tr ("Width, height of displayed area in meters"));
  displayedArea->setWhatsThis (displayedAreaText);
  connect (displayedArea, SIGNAL (returnPressed ()), this, SLOT (slotDisplayedAreaReturnPressed ()));
  toolBar[7]->addWidget (displayedArea);


  toolBar[7]->addSeparator ();
  toolBar[7]->addSeparator ();


  bDefaultScale = new QToolButton (this);
  bDefaultScale->setIcon (QIcon (":/icons/default_scale.png"));
  bDefaultScale->setWhatsThis (defaultScaleText);
  connect (bDefaultScale, SIGNAL (clicked ()), this, SLOT (slotDefaultScaleClicked ()));
  toolBar[7]->addWidget (bDefaultScale);


  viewChartScale = new QLineEdit (this);
  viewChartScale->setToolTip (tr ("Chart scale of the displayed area"));
  viewChartScale->setWhatsThis (viewChartScaleText);
  connect (viewChartScale, SIGNAL (returnPressed ()), this, SLOT (slotViewChartScaleReturnPressed ()));
  toolBar[7]->addWidget (viewChartScale);


  //  This makes sure that the user can't change the "1:" part of the scale.

  viewChartScale->setInputMask("\\1\\:0000000");


  //  Set the icon sizes so that the tool bars will set up properly.

  prev_icon_size = options.main_button_icon_size;

  QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);

  for (int32_t i = 0 ; i < NUM_TOOLBARS ; i++) toolBar[i]->setIconSize (mainButtonIconSize);


  //  Get the user's defaults if available

  if (!envin (&options, &misc, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  // Set the application font

  prev_font = options.font;
  QApplication::setFont (options.font);


  //  Get the command line arguments.  We might be specifying the HSV min and max color values on the command line
  //  and we want those to override what's in the QSettings (from envin).

  extern char *optarg;
  extern int optind;
  int32_t option_index = 0;

  while (NVTrue) 
    {
      static struct option long_options[] = {{"min_hsv_color", required_argument, 0, 0},
                                             {"max_hsv_color", required_argument, 0, 0},
                                             {"min_hsv_value", required_argument, 0, 0},
                                             {"max_hsv_value", required_argument, 0, 0},
                                             {"area_file", required_argument, 0, 0},
                                             {"nsew", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      char c = (char) getopt_long (*argc, argv, "", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%hd", &options.min_hsv_color[0]);
              break;

            case 1:
              sscanf (optarg, "%hd", &options.max_hsv_color[0]);
              break;

            case 2:
              sscanf (optarg, "%f", &options.min_hsv_value[0]);
              options.min_hsv_locked[0] = NVTrue;
              break;

            case 3:
              sscanf (optarg, "%f", &options.max_hsv_value[0]);
              options.max_hsv_locked[0] = NVTrue;
              break;

            case 4:
              int32_t polygon_count;
              double polygon_x[200], polygon_y[200];
              get_area_mbr (optarg, &polygon_count, polygon_x, polygon_y, &command_line_mbr);
              break;

            case 5:
              sscanf (optarg, "%lf,%lf,%lf,%lf", &command_line_mbr.max_y, &command_line_mbr.min_y, &command_line_mbr.max_x, &command_line_mbr.min_x);


              //  Check the bounds for the correct order (NSEW)

              double tmp_f64;
              if (command_line_mbr.max_y < command_line_mbr.min_y)
                {
                  tmp_f64 = command_line_mbr.min_y;
                  command_line_mbr.min_y = command_line_mbr.max_y;
                  command_line_mbr.max_y = tmp_f64;
                }
              if (command_line_mbr.max_x < command_line_mbr.min_x)
                {
                  tmp_f64 = command_line_mbr.min_x;
                  command_line_mbr.min_x = command_line_mbr.max_x;
                  command_line_mbr.max_x = tmp_f64;
                }
              break;
            }
          break;
        }
    }


  //  Check to see if we have Google Earth.

  misc.googleearth_available = NVTrue;
  if (options.ge_name.isEmpty ())
    {
      misc.googleearth_available = NVFalse;
    }
  else
    {
      if (!QFile (options.ge_name).exists ()) misc.googleearth_available = NVFalse;
    }


  //  Check the min and max colors and flip them if needed.

  if (options.max_hsv_color[0] > 315) options.max_hsv_color[0] = 315;
  if (options.min_hsv_color[0] > 315) options.min_hsv_color[0] = 0;

  if (options.max_hsv_color[0] < options.min_hsv_color[0])
    {
      uint16_t tmpu16 = options.max_hsv_color[0];
      options.max_hsv_color[0] = options.min_hsv_color[0];
      options.min_hsv_color[0] = tmpu16;
    }


  //  Allocate the surface colors.

  float hue_inc = (float) (options.max_hsv_color[0] - options.min_hsv_color[0]) / (float) (NUMHUES + 1);
  for (int32_t m = 0 ; m < 2 ; m++)
    {
      int32_t sat = 255;
      if (m) sat = 25;

      for (int32_t i = 0 ; i < NUMHUES ; i++)
        {
          for (int32_t j = 0 ; j < NUMSHADES ; j++)
            {
              options.color_array[m][i][j].setHsv ((int32_t) (((NUMHUES + 1) - (i - options.min_hsv_color[0])) * hue_inc), sat, j, 255);
            }
        }
    }


  //  Set the sun shading value based on the defaults

  options.sunopts.sun = sun_unv (options.sunopts.azimuth, options.sunopts.elevation);


  //  For stoplight colors we are looking for the nearest color in the color array so we can use the already
  //  defined shades to do sunshading.

  for (int32_t j = 0 ; j < 3 ; j++)
    {
      QColor tmp;

      switch (j)
        {
        case 0:
          tmp = options.stoplight_min_color;
          break;

        case 1:
          tmp = options.stoplight_mid_color;
          break;

        case 2:
          tmp = options.stoplight_max_color;
          break;
        }

      int32_t min_hue_diff = 99999;
      int32_t closest = -1;

      for (int32_t i = 0 ; i < NUMHUES ; i++)
        {
          int32_t hue_diff = abs (tmp.hue () - options.color_array[0][i][255].hue ());

          if (hue_diff < min_hue_diff)
            {
              min_hue_diff = hue_diff;
              closest = i;
            }
        }

      switch (j)
        {
        case 0:
          options.stoplight_min_color = options.color_array[0][closest][255];
          options.stoplight_min_index = closest;
          break;

        case 1:
          options.stoplight_mid_color = options.color_array[0][closest][255];
          options.stoplight_mid_index = closest;
          break;

        case 2:
          options.stoplight_max_color = options.color_array[0][closest][255];
          options.stoplight_max_index = closest;
          break;
        }
    }


  //  Set the map and cov map values from the defaults

  mapdef.projection = CYLINDRICAL_EQUIDISTANT;
  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.overlap_percent = options.overlap_percent;
  mapdef.grid_color = options.overlay_grid_color;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;

  mapdef.coasts = options.coast;
  mapdef.landmask = NVFalse;

  mapdef.border = 5;
  mapdef.coast_color = options.coast_color;
  mapdef.grid_color = options.coast_color;
  mapdef.background_color = options.background_color;


  mapdef.initial_bounds.min_x = -180.0;
  mapdef.initial_bounds.min_y = -90.0;
  mapdef.initial_bounds.max_x = 180.0;
  mapdef.initial_bounds.max_y = 90.0;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the map.

  map = new nvMap (this, &mapdef);


  map->setWhatsThis (mapText);


  misc.map_widget = map;


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, double, double)), this, SLOT (slotMousePress (QMouseEvent *, double, double)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, double, double)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, double, double)));
  connect (map, SIGNAL (preliminaryMousePressSignal (QMouseEvent *)), this, SLOT (slotPreliminaryMousePress (QMouseEvent *)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, double, double)), this, SLOT (slotMouseRelease (QMouseEvent *, double, double)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, double, double)), this, SLOT (slotMouseMove (QMouseEvent *, double, double)));
  connect (map, SIGNAL (closeSignal (QCloseEvent *)), this, SLOT (slotClose (QCloseEvent *)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotPreRedraw (NVMAP_DEF)));
  connect (map, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPostRedraw (NVMAP_DEF)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QGroupBox *leftBox = new QGroupBox ();
  QVBoxLayout *leftBoxLayout = new QVBoxLayout ();
  leftBox->setLayout (leftBoxLayout);


  //  Trying to keep the leftBox from being too big.

  leftBox->setMaximumWidth (misc.cov_width + 20);


  QGroupBox *covBox = new QGroupBox (tr ("Coverage"));
  QVBoxLayout *covBoxLayout = new QVBoxLayout ();
  covBox->setLayout (covBoxLayout);
  covBox->setFlat (true);


  //  Make the coverage map.

  covdef.projection = CYLINDRICAL_EQUIDISTANT;
  covdef.draw_width = misc.cov_width;
  covdef.draw_height = misc.cov_height;

  covdef.landmask = NVFalse;


  //  We're drawing our own coasts for the coverage map because of the way we have to draw it.  So, even though
  //  covdef.coasts is set to NVFalse we will draw the coasts if options.cov_coast is set.

  covdef.coasts = NVFalse;
  covdef.coast_color = options.cov_coast_color;
  covdef.coast_thickness = 1;

  covdef.grid_inc_x = 0.0;
  covdef.grid_inc_y = 0.0;
  covdef.border = 5;
  covdef.background_color = QColor (255, 255, 255);


  covdef.initial_bounds.min_x = -180.0;
  covdef.initial_bounds.min_y = 180.0;
  covdef.initial_bounds.max_x = 90.0;
  covdef.initial_bounds.max_y = -90.0;


  cov = new nvMap (this, &covdef);
  cov->setMinimumSize (misc.cov_width, misc.cov_height);
  cov->setMaximumSize (misc.cov_width, misc.cov_height);


  cov->setWhatsThis (covText + covMenuText);


  //  Connect to the signals from the map class.
    
  connect (cov, SIGNAL (mousePressSignal (QMouseEvent *, double, double)), this, SLOT (slotCovMousePress (QMouseEvent *, double, double)));
  connect (cov, SIGNAL (mouseReleaseSignal (QMouseEvent *, double, double)), this, SLOT (slotCovMouseRelease (QMouseEvent *, double, double)));
  connect (cov, SIGNAL (mouseMoveSignal (QMouseEvent *, double, double)), this, SLOT (slotCovMouseMove (QMouseEvent *, double, double)));
  connect (cov, SIGNAL (closeSignal (QCloseEvent *)), this, SLOT (slotClose (QCloseEvent *)));
  connect (cov, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotCovPreRedraw (NVMAP_DEF)));
  connect (cov, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotCovPostRedraw (NVMAP_DEF)));


  //  Right click popup menu

  covPopupMenu = new QMenu (cov);

  covPopup0 = covPopupMenu->addAction (tr ("Display area"));
  connect (covPopup0, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu0 ()));
  covPopup1 = covPopupMenu->addAction (tr ("Discard rectangle"));
  connect (covPopup1, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu1 ()));
  covPopup2 = covPopupMenu->addAction (tr ("Drag rectangle"));
  connect (covPopup2, SIGNAL (triggered ()), this, SLOT (slotCovPopupMenu2 ()));
  covPopupMenu->addSeparator ();
  covPopup3 = covPopupMenu->addAction (tr ("Help"));
  connect (covPopup3, SIGNAL (triggered ()), this, SLOT (slotCovPopupHelp ()));


  cov->setCursor (Qt::WaitCursor);
  cov_area_defined = NVFalse;

  covBoxLayout->addWidget (cov);

  leftBoxLayout->addWidget (covBox);


  
  QGroupBox *pfmBox = new QGroupBox ();
  QHBoxLayout *pfmBoxLayout = new QHBoxLayout ();
  pfmBox->setLayout (pfmBoxLayout);
  pfmBox->setMaximumWidth (misc.cov_width);


  sizeLabel = new QLabel (this);
  sizeLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  sizeLabel->setToolTip (tr ("Bin size in meters"));
  sizeLabel->setWhatsThis (tr ("Bin size in meters of the current top level PFM"));
  pfmBoxLayout->addWidget (sizeLabel);

  pfmLabel = new QLabel ("PFM name", this);
  pfmLabel->setMaximumWidth (misc.cov_width);
  pfmLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  pfmLabel->setToolTip (tr ("PFM layer name"));
  pfmLabel->setWhatsThis (pfmLabelText);
  pfmBoxLayout->addWidget (pfmLabel, 1);
  leftBoxLayout->addWidget (pfmBox);


  //  Set up the status and PFM layer tabbed notebook

  QGroupBox *noteBox = new QGroupBox ();
  QVBoxLayout *noteBoxLayout = new QVBoxLayout ();
  noteBox->setLayout (noteBoxLayout);
  noteBox->setMaximumWidth (misc.cov_width);


  QGroupBox *statBox = new QGroupBox ();
  QGridLayout *statBoxLayout = new QGridLayout;
  statBox->setLayout (statBoxLayout);
  statBox->setMaximumWidth (misc.cov_width);


  //  The scaleBox widget is built in the utility library.

  for (int32_t i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      misc.scale[i] = new scaleBox (this);
      misc.scale[i]->setMinimumWidth (80);
      misc.scale[i]->setMaximumWidth (80);
      statBoxLayout->addWidget (misc.scale[i], i, 0, Qt::AlignLeft);

      misc.scale[i]->setContents (Qt::white, QString (""), 0);
    }

  connect (misc.scale[0], SIGNAL (mousePressSignal (QMouseEvent *)), this, SLOT (slotMinScalePressed (QMouseEvent *)));
  connect (misc.scale[NUM_SCALE_LEVELS - 1], SIGNAL (mousePressSignal (QMouseEvent *)), this, SLOT (slotMaxScalePressed (QMouseEvent *)));


  misc.widgetForegroundColor = QColor (0, 0, 0);

  labelColorString[0] = "#ffffff";
  labelColorString[1] = "#acacac";
  labelColor[0].setNamedColor (labelColorString[0]);
  labelColor[1].setNamedColor (labelColorString[1]);
  label_palette[0].setColor (QPalette::Window, labelColor[0]);
  label_palette[0].setColor (QPalette::WindowText, misc.widgetForegroundColor);
  label_palette[1].setColor (QPalette::Window, labelColor[1]);
  label_palette[1].setColor (QPalette::WindowText, misc.widgetForegroundColor);

  colorPalette[0] = colorPalette[1] = this->palette ();

  QLinearGradient gradient0 (200, 0, 0, 0);
  gradient0.setColorAt (0, QColor (255, 0, 0));
  gradient0.setColorAt (1, QColor (255, 255, 255));
  QBrush brush0 (gradient0);
  colorPalette[0].setBrush (QPalette::Window, brush0);

  QLinearGradient gradient1 (200, 0, 0, 0);
  gradient1.setColorAt (0, QColor (255, 0, 0));
  gradient1.setColorAt (1, QColor (172, 172, 172));
  QBrush brush1 (gradient1);
  colorPalette[1].setBrush (QPalette::Window, brush1);


  colorBoxPalette[0] = colorBoxPalette[1] = this->palette ();

  QLinearGradient gradient2 (200, 0, 0, 0);
  gradient2.setColorAt (0, QColor (0, 255, 255));
  gradient2.setColorAt (1, QColor (255, 255, 255));
  QBrush brush2 (gradient2);
  colorBoxPalette[0].setBrush (QPalette::Window, brush2);


  QLinearGradient gradient3 (200, 0, 0, 0);
  gradient3.setColorAt (0, QColor (0, 255, 255));
  gradient3.setColorAt (1, QColor (172, 172, 172));
  QBrush brush3 (gradient3);
  colorBoxPalette[1].setBrush (QPalette::Window, brush3);


  int32_t pos = 0;


  //  The clickLabel widget is built in the utility library.

  latName = new clickLabel ("Latitude", this);
  latName->setMinimumSize (latName->sizeHint ());
  latName->setAutoFillBackground (true);
  latName->setPalette (label_palette[pos % 2]);
  connect (latName, SIGNAL (clicked (QMouseEvent *, int)), SLOT (slotLatitudeClicked (QMouseEvent *, int)));
  statBoxLayout->addWidget (latName, pos, 1, Qt::AlignLeft, 1);

  latLabel = new QLabel (" N 90 00 00.00 ", this);
  latLabel->setMinimumSize (latLabel->sizeHint ());
  latLabel->setAutoFillBackground (true);
  latLabel->setPalette (label_palette[pos % 2]);
  statBoxLayout->addWidget (latLabel, pos, 2, Qt::AlignLeft);

  pos++;

  lonName = new QLabel ("Longitude", this);
  lonName->setMinimumSize (lonName->sizeHint ());
  lonName->setAutoFillBackground (true);
  lonName->setPalette (label_palette[pos % 2]);
  statBoxLayout->addWidget (lonName, pos, 1, Qt::AlignLeft, 1);

  lonLabel = new QLabel (" W 180 00 00.00 ", this);
  lonLabel->setMinimumSize (lonLabel->sizeHint ());
  lonLabel->setAutoFillBackground (true);
  lonLabel->setPalette (label_palette[pos % 2]);
  statBoxLayout->addWidget (lonLabel, pos, 2, Qt::AlignLeft);

  pos++;

  QString attrstring = tr ("When the label is clicked it will change its background color to a gradient "
                           "fading from red on the right to the normal background color on the left.  This allows you to "
                           "see, at a glance, what attribute you are coloring the surface by.");

  for (int32_t i = 0 ; i < PRE_ATTR + POST_ATTR ; i++)
    {
      //  The clickLabel widget is built in the utility library.

      switch (i)
        {
        case 0:
          attrName[i] = new clickLabel ("Z value", this, i);
          attrName[i]->setToolTip (tr ("Click to color by Z value"));
          attrName[i]->setWhatsThis (tr ("Click this label to color the data by Z value. %1").arg (attrstring));

          attrValue[i] = new QLabel (" 00000.00 ", this);
          attrValue[i]->setToolTip (tr ("Z value in bin"));
          break;

        case 1:
          attrName[i] = new clickLabel ("Point count", this, i);
          attrName[i]->setToolTip (tr ("Click to color by number of points in bin"));
          attrName[i]->setWhatsThis (tr ("Click this label to color the data by the number of points in the bin. %1").arg (attrstring));

          attrValue[i] = new QLabel ("00000", this);
          attrValue[i]->setToolTip (tr ("Number of points in bin"));
          break;

        case 2:
          attrName[i] = new clickLabel ("StdDev", this, i);
          attrName[i]->setToolTip (tr ("Click to color by standard deviation"));
          attrName[i]->setWhatsThis (tr ("Click this label to color the data by standard deviation. %1").arg (attrstring));

          attrValue[i] = new QLabel (" 00.00 ", this);
          attrValue[i]->setToolTip (tr ("Standard deviation of bin"));
          break;

        case 3:
          attrName[i] = new clickLabel ("Max - Min", this, i);
          attrName[i]->setToolTip (tr ("Click to color by maximum minus minimum"));
          attrName[i]->setWhatsThis
            (tr ("Click this label to color the data by maximum value in the bin minus minimum value in the bin. %1").arg (attrstring));

          attrValue[i] = new QLabel (" 0000.00 ", this);
          attrValue[i]->setToolTip (tr ("Maximum value in bin minus minimum value in bin"));
          break;

        case 4:
          attrName[i] = new clickLabel ("Valid point count", this, i);
          attrName[i]->setToolTip (tr ("Click to color by number of valid points in bin"));
          attrName[i]->setWhatsThis (tr ("Click this label to color the data by the number of valid points in the bin. %1").arg (attrstring));

          attrValue[i] = new QLabel (" 00000 ", this);
          attrValue[i]->setToolTip (tr ("Number of valid points in bin"));
          break;

        default:
          attrName[i] = new clickLabel ("", this, i);
          attrName[i]->setToolTip (tr ("No attribute selected"));
          attrName[i]->setWhatsThis (tr ("No attribute selected"));

          attrValue[i] = new QLabel ("         ", this);
          attrValue[i]->setToolTip (tr ("No attribute selected"));
          break;
        }

      attrPalette[i] = attrName[i]->palette ();

      attrName[i]->setMinimumSize (attrName[i]->sizeHint ());
      attrName[i]->setAutoFillBackground (true);
      attrName[i]->setPalette (label_palette[pos % 2]);
      attrName[i]->setFrameStyle (QFrame::Panel | QFrame::Raised);
      attrName[i]->setLineWidth (1);

      statBoxLayout->addWidget (attrName[i], pos, 1, Qt::AlignLeft, 1);

      attrValue[i]->setMinimumSize (attrValue[i]->sizeHint ());
      attrValue[i]->setAutoFillBackground (true);
      attrValue[i]->setPalette (label_palette[pos % 2]);
      statBoxLayout->addWidget (attrValue[i], pos, 2, Qt::AlignLeft);

      pos++;
    }


  //  Make sure the names fit.

  statBoxLayout->setColumnStretch (1, 1);


  statbook = new QTabWidget ();
  statbook->setTabPosition (QTabWidget::North);


  statbook->addTab (statBox, tr ("Status"));


  //  Set up the Attributes notebook tab

  QGroupBox *attrBox = new QGroupBox ();
  QGridLayout *attrBoxLayout = new QGridLayout;
  attrBox->setLayout (attrBoxLayout);
  attrBox->setMaximumWidth (misc.cov_width);


  pos = 0;


  attrstring = tr ("When the label is clicked it will set the selected attribute in the first <b>Status</b> tab "
                   "attribute slots.  If all of the attribute slots are populated, the last will be removed.");

  for (int32_t i = 0 ; i < NUM_ATTR ; i++)
    {
      attrBoxName[i] = new clickLabel ("", this, i);

      attrBoxValue[i] = new QLabel ("", this);


      attrBoxName[i]->setMaximumHeight (attrName[0]->sizeHint ().height ());
      attrBoxName[i]->setMinimumSize (attrBoxName[i]->sizeHint ());
      attrBoxName[i]->setAutoFillBackground (true);
      attrBoxName[i]->setPalette (label_palette[pos % 2]);
      attrBoxName[i]->setFrameStyle (QFrame::Panel | QFrame::Raised);
      attrBoxName[i]->setLineWidth (1);


      attrBoxValue[i]->setMaximumHeight (attrValue[0]->sizeHint ().height ());
      attrBoxValue[i]->setMinimumSize (attrBoxValue[i]->sizeHint ());
      attrBoxValue[i]->setAutoFillBackground (true);
      attrBoxValue[i]->setPalette (label_palette[pos % 2]);

      attrBoxLayout->addWidget (attrBoxName[i], pos, 0, Qt::AlignLeft, 1);
      attrBoxLayout->addWidget (attrBoxValue[i], pos, 1, Qt::AlignLeft);

      connect (attrBoxName[i], SIGNAL (clicked (QMouseEvent *, int)), SLOT (slotAttrBoxClicked (QMouseEvent *, int)));

      pos++;
    }


  attrBoxLayout->addItem (new QSpacerItem (1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred), pos, 1, 1, 4, Qt::AlignBottom);


  //  Make sure the names fit.

  attrBoxLayout->setColumnStretch (pos, 1);


  statbook->addTab (attrBox, tr ("Attributes"));


  //  Set up the Layers notebook tab

  QGroupBox *layerBox = new QGroupBox ();
  QGridLayout *layerBoxLayout = new QGridLayout;
  layerBox->setLayout (layerBoxLayout);
  layerBox->setMaximumWidth (misc.cov_width);
  layerBoxLayout->setColumnStretch (1, 1);


  QButtonGroup *layerCheckGrp = new QButtonGroup (this);
  layerCheckGrp->setExclusive (false);
  connect (layerCheckGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotLayerClicked (int)));


  for (int32_t i = 0 ; i < MAX_ABE_PFMS ; i++)
    {
      layerCheck[i] = new QCheckBox (this);
      layerCheckGrp->addButton (layerCheck[i], i);
      layerBoxLayout->addWidget (layerCheck[i], i, 0, Qt::AlignLeft);

      layerName[i] = new QLabel (this);
      layerBoxLayout->addWidget (layerName[i], i, 1, Qt::AlignLeft);
      layerCheck[i]->hide ();
      layerName[i]->hide ();
    }
 
  layerBoxLayout->setRowStretch (MAX_ABE_PFMS - 1, 1);


  statbook->addTab (layerBox, tr ("Layers"));


  leftBoxLayout->addWidget (statbook, 1);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (leftBox);
  hBox->addWidget (map, 1);
  vBox->addLayout (hBox);


  QGroupBox *progBox = new QGroupBox ();
  QHBoxLayout *progBoxLayout = new QHBoxLayout;
  progBox->setLayout (progBoxLayout);


  misc.statusProgLabel = new QLabel (this);
  misc.statusProgLabel->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
  misc.statusProgLabel->setToolTip (tr ("Active mode"));
  misc.statusProgLabel->setWhatsThis (miscLabelText);
  misc.statusProgLabel->setAutoFillBackground (true);
  misc.statusProgPalette = misc.statusProgLabel->palette ();
  progBoxLayout->addWidget (misc.statusProgLabel);


  //  Progress status bar

  misc.progStatus = new QStatusBar ();
  misc.progStatus->setSizeGripEnabled (false);
  misc.progStatus->setToolTip (tr ("Progress bar and label"));
  misc.progStatus->setWhatsThis (progStatusText);
  progBoxLayout->addWidget (misc.progStatus);


  misc.statusProg = new QProgressBar (this);
  misc.statusProg->setRange (0, 100);
  misc.statusProg->setValue (0);
  misc.statusProg->setTextVisible (false);
  misc.progStatus->addWidget (misc.statusProg, 10);


  vBox->addWidget (progBox);


  //  Set View toolbar button states based on saved options

  QString hString = tr ("Highlight %L1 percent of depth").arg (options.highlight_percent, 0, 'f', 2);
  highlightPercent->setText (hString);
  highlightCount->setText (tr ("Highlight valid bin count exceeding %1").arg (options.h_count));

  bHighlight->setIcon (highlightIcon[options.highlight]);


  QString tip =
    tr ("Toggle stoplight display [crossovers at %L1 and %L2]").arg (options.stoplight_min_mid, 2, 'f', 1).arg (options.stoplight_max_mid, 2, 'f', 1);
  bStoplight->setToolTip (tip);
  bStoplight->setChecked (options.stoplight);
  connect (bStoplight, SIGNAL (clicked ()), this, SLOT (slotStoplight ()));

  bContour->setChecked (options.contour);
  connect (bContour, SIGNAL (clicked ()), this, SLOT (slotContour ()));

  bGrid->setChecked (options.overlay_grid);
  connect (bGrid, SIGNAL (clicked ()), this, SLOT (slotGrid ()));

  bCoast->setChecked (options.coast);
  connect (bCoast, SIGNAL (clicked ()), this, SLOT (slotCoast ()));

  bCovCoast->setChecked (options.cov_coast);
  connect (bCovCoast, SIGNAL (clicked ()), this, SLOT (slotCovCoast ()));

  bAutoRedraw->setChecked (options.auto_redraw);
  connect (bAutoRedraw, SIGNAL (clicked ()), this, SLOT (slotAutoRedraw ()));


  //  Connect the clickLabel signals.

  for (int32_t i = 0 ; i < PRE_ATTR + POST_ATTR ; i++)
    {
      connect (attrName[i], SIGNAL (clicked (QMouseEvent *, int)), SLOT (slotColorByClicked (QMouseEvent *, int)));
    }


  //  Set Surface toolbar button states based on saved options

  tip = tr ("Set the on-the-fly gridding parameters including attribute (%1) and bin size (%L2)").arg
    (options.attrFilterName).arg (options.otf_bin_size_meters, 3, 'f', 1);
  bSetOtfBin->setToolTip (tip);


  //  Set the correct surface button from the defaults.

  bSurface[options.layer_type - 2]->setChecked (true);

  setSurfaceValidity (options.layer_type);

  misc.prev_surface_val = misc.surface_val;


  //  Set Flag Data toolbar button states based on saved options

  bDisplaySuspect->setChecked (options.display_suspect);
  connect (bDisplaySuspect, SIGNAL (clicked ()), this, SLOT (slotDisplaySuspect ()));

  bDisplaySelected->setChecked (options.display_selected);
  connect (bDisplaySelected, SIGNAL (clicked ()), this, SLOT (slotDisplaySelected ()));

  bDisplayReference->setChecked (options.display_reference);
  connect (bDisplayReference, SIGNAL (clicked ()), this, SLOT (slotDisplayReference ()));

  bDisplayMinMax->setChecked (options.display_minmax);
  connect (bDisplayMinMax, SIGNAL (clicked ()), this, SLOT (slotDisplayMinMax ()));


  //  Set Feature toolbar button states based on saved options

  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);

  bDisplayChildren->setChecked (options.display_children);
  connect (bDisplayChildren, SIGNAL (clicked ()), this, SLOT (slotDisplayChildren ()));

  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  connect (bDisplayFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotDisplayFeatureInfo ()));

  bDisplayFeaturePoly->setChecked (options.display_feature_poly);
  connect (bDisplayFeaturePoly, SIGNAL (clicked ()), this, SLOT (slotDisplayFeaturePoly ()));

  bAddFeature->setEnabled (options.display_feature);

  bDeleteFeature->setEnabled (options.display_feature);

  bEditFeature->setEnabled (options.display_feature);


  //  Set Chart toolbar button states based on saved options

  QFontMetrics fm (options.font);
  QString aString = " 100000.00,100000.00 ";
  int pixelsWide = fm.width (aString);

  displayedArea->setFixedSize (pixelsWide, mainButtonIconSize.height ());

  aString = " 1:1000000 ";
  pixelsWide = fm.width (aString);

  viewChartScale->setFixedSize (pixelsWide, mainButtonIconSize.height ());


  //  Set all of the button hotkey tooltips and shortcuts

  for (int32_t i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  //  Set the edit function from the defaults

  misc.function = options.edit_mode;
  misc.save_function = misc.last_edit_function = misc.function;


  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction (tr ("Close polygon and filter area"));
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction (tr ("Set filter standard deviation (2.4)"));
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popup2 = popupMenu->addAction (tr ("Set deep filter only (Yes)"));
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupMenu2 ()));
  popup3 = popupMenu->addAction (tr ("Discard polygon"));
  connect (popup3, SIGNAL (triggered ()), this, SLOT (slotPopupMenu3 ()));
  popup4 = popupMenu->addAction (tr ("Clear all filter masks"));
  connect (popup4, SIGNAL (triggered ()), this, SLOT (slotPopupMenu4 ()));
  popupMenu->addSeparator ();
  popupHelp = popupMenu->addAction (tr ("Help"));
  connect (popupHelp, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  The following menus are in the menu bar.
     
  //  Setup the file menu.

  QAction *fileOpenAction = new QAction (tr ("Open PFM (Normal)"), this);
  fileOpenAction->setStatusTip (tr ("Open PFM file (Normal)"));
  fileOpenAction->setWhatsThis (openText);
  connect (fileOpenAction, SIGNAL (triggered ()), this, SLOT (slotRegularOpenClicked()));
  
  QAction *fileOpenEnhancedAction = new QAction (tr ("Open PFM (Enhanced)"), this);
  fileOpenEnhancedAction->setStatusTip (tr ("Open PFM file (Enhanced)"));
  fileOpenEnhancedAction->setWhatsThis (openText);
  connect (fileOpenEnhancedAction, SIGNAL (triggered ()), this, SLOT (slotEnhancedOpenClicked ()));


  recentMenu = new QMenu (tr ("Open Recent..."));

  recentGrp = new QActionGroup (this);
  connect (recentGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotOpenRecent (QAction *)));

  for (int32_t i = 0 ; i < MAX_RECENT ; i++)
    {
      recentFileAction[i] = recentMenu->addAction (options.recentFile[i]);
      recentGrp->addAction (recentFileAction[i]);
      if (i < options.recent_file_count)
        {
          recentFileAction[i]->setVisible (true);
        }
      else
        {
          recentFileAction[i]->setVisible (false);
        }
    }


  fileCloseAction = new QAction (tr ("Close PFM file"), this);
  fileCloseAction->setStatusTip (tr ("Close top level/layer PFM file"));
  fileCloseAction->setWhatsThis (closePFMText);
  fileCloseAction->setEnabled (false);
  connect (fileCloseAction, SIGNAL (triggered ()), this, SLOT (slotClosePFM ()));

  fileImportAction = new QAction (tr ("Import DNC Data"), this);
  fileImportAction->setStatusTip (tr ("Import DNC data to feature file"));
  fileImportAction->setWhatsThis (importText);
  fileImportAction->setEnabled (false);
  connect (fileImportAction, SIGNAL (triggered ()), this, SLOT (slotImport ()));


  //  Check to see if we have the import_prog program available.

  char ip_name[256];
  strcpy (ip_name, options.import_prog);
#ifdef NVWIN3X
  strcat (ip_name, ".exe");
#endif
  if (find_startup_name (ip_name) == NULL) fileImportAction->setVisible(false);


  geotiffOpenAction = new QAction (tr ("Open GeoTIFF file"), this);
  geotiffOpenAction->setStatusTip (tr ("Open a GeoTIFF file to be displayed with the PFM data"));
  geotiffOpenAction->setWhatsThis (geotiffText);
  geotiffOpenAction->setEnabled (false);
  connect (geotiffOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpenGeotiff ()));


  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q", "Shortcut key for Quit"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("File"));
  fileMenu->addAction (fileOpenAction);
  fileMenu->addAction (fileOpenEnhancedAction);
  fileMenu->addMenu (recentMenu);
  fileMenu->addAction (fileCloseAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileImportAction);
  fileMenu->addAction (geotiffOpenAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileQuitAction);


  //  Setup the edit menu.

  QActionGroup *editGrp = new QActionGroup (this);
  connect (editGrp, SIGNAL (triggered (QAction*)), this, SLOT (slotCheckMenu (QAction *)));


  fileChecked = new QAction (tr ("Set File Checked"), this);
  fileChecked->setStatusTip (tr ("Set file checked"));
  fileChecked->setWhatsThis (fileCheckedText);
  editGrp->addAction (fileChecked);


  fileUnchecked = new QAction (tr ("Set File Unchecked"), this);
  fileUnchecked->setStatusTip (tr ("Set file unchecked"));
  fileUnchecked->setWhatsThis (fileUncheckedText);
  editGrp->addAction (fileUnchecked);


  fileVerified = new QAction (tr ("Set File Verified"), this);
  fileVerified->setStatusTip (tr ("Set file verified"));
  fileVerified->setWhatsThis (fileVerifiedText);
  editGrp->addAction (fileVerified);


  fileUnverified = new QAction (tr ("Set File Unverified"), this);
  fileUnverified->setStatusTip (tr ("Set file unverified"));
  fileUnverified->setWhatsThis (fileUnverifiedText);
  editGrp->addAction (fileUnverified);


  deleteRestore = new QAction (tr ("Delete/Restore Files"), this);
  deleteRestore->setStatusTip (tr ("Delete/restore input files"));
  deleteRestore->setWhatsThis (restoreDeleteText);
  connect (deleteRestore, SIGNAL (triggered ()), this, SLOT (slotDeleteFile ()));


  deleteFileQueue = new QAction (tr ("Delete File Queue"), this);
  deleteFileQueue->setStatusTip (tr ("Delete input files in the delete file queue"));
  deleteFileQueue->setWhatsThis (deleteQueueText);
  connect (deleteFileQueue, SIGNAL (triggered ()), this, SLOT (slotDeleteQueue ()));


  changePathAct = new QAction (tr ("Change File Paths"), this);
  changePathAct->setStatusTip (tr ("Change input file paths"));
  changePathAct->setWhatsThis (changePathText);
  connect (changePathAct, SIGNAL (triggered ()), this, SLOT (slotChangePath ()));


  QAction *prefsAct = new QAction (tr ("Preferences"), this);
  prefsAct->setStatusTip (tr ("Change program preferences"));
  prefsAct->setWhatsThis (prefsText);
  connect (prefsAct, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  editMenu = menuBar ()->addMenu (tr ("Edit"));
  editMenu->addAction (fileChecked);
  editMenu->addAction (fileUnchecked);
  editMenu->addAction (fileVerified);
  editMenu->addAction (fileUnverified);
  editMenu->addSeparator ();
  editMenu->addAction (deleteRestore);
  editMenu->addAction (deleteFileQueue);
  editMenu->addSeparator ();
  editMenu->addAction (changePathAct);
  editMenu->addSeparator ();
  editMenu->addAction (prefsAct);


  //  Setup the View menu

  QAction *overlay = new QAction (tr ("Select Overlays"), this);
  overlay->setStatusTip (tr ("Select overlays"));
  overlay->setWhatsThis (selectOverlaysText);
  connect (overlay, SIGNAL (triggered ()), this, SLOT (slotOverlays ()));


  QAction *displayPFMHeader = new QAction (tr ("Display PFM Header"), this);
  displayPFMHeader->setStatusTip (tr ("Display the PFM bin file header"));
  displayPFMHeader->setWhatsThis (displayPFMHeaderText);
  connect (displayPFMHeader, SIGNAL (triggered ()), this, SLOT (slotDisplayHeader ()));


  messageAction = new QAction (tr ("Display Messages"), this);
  messageAction->setToolTip (tr ("Display error messages from program and sub-processes"));
  messageAction->setWhatsThis (messageText);
  connect (messageAction, SIGNAL (triggered ()), this, SLOT (slotMessage ()));


  startMosaic = new QAction (tr ("Start mosaic file viewer"), this);
  startMosaic->setShortcut (options.mosaic_hotkey);
  startMosaic->setStatusTip (tr ("Start the mosaic file viewer"));
  startMosaic->setWhatsThis (startMosaicText);
  connect (startMosaic, SIGNAL (triggered ()), this, SLOT (slotStartMosaicViewer ()));


  start3D = new QAction (tr ("Start &3D viewer"), this);
  start3D->setShortcut (tr ("3", "Shortcut key for 3D viewer"));
  start3D->setStatusTip (tr ("Start the 3D viewer"));
  start3D->setWhatsThis (start3DText);
  connect (start3D, SIGNAL (triggered ()), this, SLOT (slotStart3DViewer ()));


  QAction *zoomToArea = new QAction (tr ("Zoom To Area File"), this);
  zoomToArea->setStatusTip (tr ("Zoom in view to selected area file"));
  zoomToArea->setWhatsThis (zoomToAreaText);
  connect (zoomToArea, SIGNAL (triggered ()), this, SLOT (slotZoomToArea ()));


  viewMenu = menuBar ()->addMenu (tr ("View"));
  viewMenu->addAction (startMosaic);
  viewMenu->addAction (start3D);
  viewMenu->addAction (overlay);
  viewMenu->addAction (displayPFMHeader);
  viewMenu->addAction (messageAction);
  viewMenu->addAction (zoomToArea);


  //  Setup the Tools menu.

  findFeatures = new QAction (tr ("Find Feature"), this);
  findFeatures->setStatusTip (tr ("Find a feature"));
  findFeatures->setWhatsThis (findFeaturesText);
  connect (findFeatures, SIGNAL (triggered ()), this, SLOT (slotFindFeature ()));


  definePolygons = new QAction (tr ("Define Feature Polygon"), this);
  definePolygons->setStatusTip (tr ("Define a polygon to associate with a feature"));
  definePolygons->setWhatsThis (definePolygonText);
  definePolygons->setEnabled (false);
  connect (definePolygons, SIGNAL (triggered ()), this, SLOT (slotDefinePolygon ()));


  outputFeature = new QAction (tr ("Output Features to SHAPE"), this);
  outputFeature->setStatusTip (tr ("Output a Shapefile containing the associated feature file polygons"));
  outputFeature->setWhatsThis (outputFeatureText);
  outputFeature->setEnabled (false);
  connect (outputFeature, SIGNAL (triggered ()), this, SLOT (slotOutputFeature ()));


  changeFeature = new QAction (tr ("Change feature file"), this);
  changeFeature->setStatusTip (tr ("Select or change the feature file for this PFM structure"));
  changeFeature->setWhatsThis (changeFeatureText);
  connect (changeFeature, SIGNAL (triggered ()), this, SLOT (slotChangeFeature ()));


  changeMosaic = new QAction (tr ("Change mosaic file"), this);
  changeMosaic->setStatusTip (tr ("Select or change the mosaic file for this PFM structure"));
  changeMosaic->setWhatsThis (changeMosaicText);
  connect (changeMosaic, SIGNAL (triggered ()), this, SLOT (slotChangeMosaic ()));


  exportImage = new QAction (tr ("Export Image"), this);
  exportImage->setStatusTip (tr ("Export an image file of the displayed data"));
  exportImage->setWhatsThis (exportImageText);
  connect (exportImage, SIGNAL (triggered ()), this, SLOT (slotExportImage ()));


  QAction *outputDataPoints = new QAction (tr ("Output Data Points File"), this);
  outputDataPoints->setStatusTip (tr ("Output a data points file"));
  outputDataPoints->setWhatsThis (outputDataPointsText);
  connect (outputDataPoints, SIGNAL (triggered ()), this, SLOT (slotOutputDataPoints ()));


  QAction *defineRectArea = new QAction (tr ("Define Rectangular Area File"), this);
  defineRectArea->setStatusTip (tr ("Define a rectangular area and output to a file"));
  defineRectArea->setWhatsThis (defineRectAreaText);
  connect (defineRectArea, SIGNAL (triggered ()), this, SLOT (slotDefineRectArea ()));


  QAction *definePolyArea = new QAction (tr ("Define Polygonal Area File"), this);
  definePolyArea->setStatusTip (tr ("Define a polygonal area and output to a file"));
  definePolyArea->setWhatsThis (definePolyAreaText);
  connect (definePolyArea, SIGNAL (triggered ()), this, SLOT (slotDefinePolyArea ()));


  toolsMenu = menuBar ()->addMenu (tr ("Tools"));
  toolsMenu->addAction (findFeatures);
  toolsMenu->addAction (definePolygons);
  toolsMenu->addAction (outputFeature);
  toolsMenu->addSeparator ();
  toolsMenu->addAction (changeFeature);
  toolsMenu->addAction (changeMosaic);
  toolsMenu->addSeparator ();
  toolsMenu->addAction (exportImage);
  toolsMenu->addAction (outputDataPoints);
  toolsMenu->addSeparator ();
  toolsMenu->addAction (defineRectArea);
  toolsMenu->addAction (definePolyArea);


  //  Setup the Layers menu.

  QActionGroup *layerGrp = new QActionGroup (this);
  connect (layerGrp, SIGNAL (triggered (QAction*)), this, SLOT (slotLayerGrpTriggered (QAction *)));

  for (int32_t i = 0 ; i < MAX_ABE_PFMS ; i++)
    {
      QString layerString = tr ("Layer %1").arg (i + 1);
      layer[i] = new QAction (layerString, this);
      layerString = tr ("Move layer %1 to the top level").arg (i);
      layer[i]->setStatusTip (layerString);
      if (misc.cube_available)
        {
          layer[i]->setWhatsThis (layerCubeText);
        }
      else
        {
          layer[i]->setWhatsThis (layerText);
        }
      layerGrp->addAction (layer[i]);
      layer[i]->setVisible (false);
    }


  QAction *layerPrefs = new QAction (tr ("Manage Layers"), this);
  layerPrefs->setStatusTip (tr ("Modify the preferences for each layer"));
  layerPrefs->setWhatsThis (layerPrefsText);
  connect (layerPrefs, SIGNAL (triggered ()), this, SLOT (slotLayers ()));


  layerMenu = menuBar ()->addMenu (tr ("Layers"));
  for (int32_t i = 0 ; i < MAX_ABE_PFMS ; i++) layerMenu->addAction (layer[i]);
  layerMenu->addSeparator ();
  layerMenu->addAction (layerPrefs);


  //  Setup the help menu.

  QAction *exHelp = new QAction (tr ("User Guide"), this);
  exHelp->setStatusTip (tr ("Open the Area-Based Editor user guide in a browser"));
  connect (exHelp, SIGNAL (triggered ()), this, SLOT (extendedHelp ()));

  QAction *toolHelp = new QAction (tr ("Tool bars"), this);
  toolHelp->setStatusTip (tr ("Help on tool bars"));
  connect (toolHelp, SIGNAL (triggered ()), this, SLOT (slotToolbarHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);
  whatsThisAct->setIcon (QIcon (":/icons/contextHelp.png"));

  QAction *aboutAct = new QAction (tr ("About"), this);
  aboutAct->setStatusTip (tr ("Information about pfmView"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgments = new QAction (tr ("Acknowledgments"), this);
  acknowledgments->setStatusTip (tr ("Information about supporting libraries and contributors"));
  connect (acknowledgments, SIGNAL (triggered ()), this, SLOT (slotAcknowledgments ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("Help"));
  helpMenu->addAction (exHelp);
  helpMenu->addAction (toolHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgments);
  helpMenu->addAction (aboutQtAct);


  /******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY **************************************** \

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
      memory and then dies, all the other programs will be locked up.  When you look through the ABE code you'll see
      that we very rarely lock shared memory, and then only for very short periods of time.  This is by design.

  \******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY ****************************************/


  //  Get the ABE shared memory area.  If it exists, use it.  The key is the process ID plus "_abe".  

  QString skey;
  skey.sprintf ("%d_abe", misc.process_id);

  misc.abeShare = new QSharedMemory (skey);

  if (!misc.abeShare->create (sizeof (ABE_SHARE), QSharedMemory::ReadWrite)) misc.abeShare->attach (QSharedMemory::ReadWrite);

  misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();


  //  Clear the shared memory area and set the ppid key.

  memset (misc.abe_share, 0, sizeof (ABE_SHARE));
  misc.abe_share->ppid = misc.process_id;

  misc.abe_share->pfm_count = 0;


  //  Set a couple of things that pfmEdit3D will need to know.

  misc.abe_share->settings_changed = NVFalse;
  misc.abe_share->zoom_requested = NVFalse;
  misc.abe_share->position_form = options.position_form;
  misc.abe_share->smoothing_factor = options.smoothing_factor;
  misc.abe_share->z_factor = options.z_factor;
  misc.abe_share->z_offset = options.z_offset;
  misc.abe_share->min_hsv_color = options.min_hsv_color[0];
  misc.abe_share->max_hsv_color = options.max_hsv_color[0];
  misc.abe_share->min_hsv_value = options.min_hsv_value[0];
  misc.abe_share->max_hsv_value = options.max_hsv_value[0];
  misc.abe_share->min_hsv_locked = options.min_hsv_locked[0];
  misc.abe_share->max_hsv_locked = options.max_hsv_locked[0];


  //  Move stuff from options to share.

  misc.abe_share->layer_type = options.layer_type;
  misc.abe_share->cint = options.cint;
  misc.abe_share->num_levels = options.num_levels;
  for (int32_t i = 0 ; i < options.num_levels ; i++) misc.abe_share->contour_levels[i] = options.contour_levels[i];
  strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toLatin1 ());


  //  Set the toolbar buttons and the cursor.

  setWidgetStates (NVFalse);
  setFunctionCursor (misc.function);



  //  If there was a filename on the command line...

  if (*argc > 1)
    {
      commandFile = QFileInfo (QString (argv[optind])).absoluteFilePath ();
      command_file_flag = NVTrue;
    }


  //  Set the tracking timer function.  This is used to track the cursor when it is in other ABE program windows.
  //  Since we may be passing tracking info from child to external apps and vice versa I've halved the normal
  //  time from 50 milliseconds to 25.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (25);


  //  Set the program start timer function.  This is used when starting and stopping ancillary programs (like mosaicView).

  programTimer = new QTimer (this);
  connect (programTimer, SIGNAL (timeout()), this, SLOT (slotProgramTimer ()));
}



pfmView::~pfmView ()
{
}



//!  This handles command line arguments.

void 
pfmView::commandLineFileCheck ()
{
  openFiles (options.defaultFileOpen);


  //  Check to see if we requested a specific area on the command line (--nsew or --area_file).

  if (command_line_mbr.max_x < 360.0)
    {
      //  Adjust the command line arguments to the bounds of the just opened PFM.

      command_line_mbr.max_x = qMin (command_line_mbr.max_x, misc.abe_share->open_args[0].head.mbr.max_x);
      command_line_mbr.max_y = qMin (command_line_mbr.max_y, misc.abe_share->open_args[0].head.mbr.max_y);
      command_line_mbr.min_x = qMax (command_line_mbr.min_x, misc.abe_share->open_args[0].head.mbr.min_x);
      command_line_mbr.min_y = qMax (command_line_mbr.min_y, misc.abe_share->open_args[0].head.mbr.min_y);


      initializeMaps ();


      //  We need to zoom to the area.

      misc.clear = NVTrue;
      cov_area_defined = NVTrue;

      zoomIn (command_line_mbr, NVFalse);

      misc.cov_function = COV_START_DRAW_RECTANGLE;

      redrawCovBounds ();

      setFunctionCursor (misc.function);
    }
}



uint8_t 
pfmView::positionGoogleEarth ()
{
  if ((ge_tmp_fp[1] = fopen (ge_tmp_name[1], "w")) == NULL)
    {
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Unable to open temporary Google Earth KML file!"));
      return (-1);
    }


  fprintf (ge_tmp_fp[1], "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf (ge_tmp_fp[1], "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
  fprintf (ge_tmp_fp[1], "  <Document>\n");

  if (googleEarthPushpin.x >= misc.total_displayed_area.min_x && googleEarthPushpin.x <= misc.total_displayed_area.max_x &&
      googleEarthPushpin.y >= misc.total_displayed_area.min_y && googleEarthPushpin.y <= misc.total_displayed_area.max_y)
    {
      fprintf (ge_tmp_fp[1], "    <Placemark>\n");
      fprintf (ge_tmp_fp[1], "      <name>pfmView</name>\n");
      fprintf (ge_tmp_fp[1], "      <Point>\n");
      fprintf (ge_tmp_fp[1], "        <altitudeMode>relativeToGround</altitudeMode>\n");
      fprintf (ge_tmp_fp[1], "        <coordinates>\n");
      fprintf (ge_tmp_fp[1], "          %.11f,%.11f,0\n", googleEarthPushpin.x, googleEarthPushpin.y);
      fprintf (ge_tmp_fp[1], "        </coordinates>\n");
      fprintf (ge_tmp_fp[1], "      </Point>\n");
      fprintf (ge_tmp_fp[1], "    </Placemark>\n");
    }
  else
    {
      //  If the pushpin is not in the displayed area we want to set it to invalid.

      googleEarthPushpin.x = googleEarthPushpin.y = -1000.0;
    }

  fprintf (ge_tmp_fp[1], "    <Style id=\"Transparent\">\n");
  fprintf (ge_tmp_fp[1], "      <LineStyle>\n");
  fprintf (ge_tmp_fp[1], "        <width>1.5</width>\n");
  fprintf (ge_tmp_fp[1], "      </LineStyle>\n");
  fprintf (ge_tmp_fp[1], "      <PolyStyle>\n");
  fprintf (ge_tmp_fp[1], "        <color>00000000</color>\n");
  fprintf (ge_tmp_fp[1], "      </PolyStyle>\n");
  fprintf (ge_tmp_fp[1], "    </Style>\n");
  fprintf (ge_tmp_fp[1], "    <Placemark>\n");
  fprintf (ge_tmp_fp[1], "      <name>pfmView displayed area</name>\n");
  fprintf (ge_tmp_fp[1], "      <styleUrl>#Transparent</styleUrl>\n");
  fprintf (ge_tmp_fp[1], "      <Polygon>\n");
  fprintf (ge_tmp_fp[1], "        <extrude>1</extrude>\n");
  fprintf (ge_tmp_fp[1], "        <tessellate>1</tessellate>\n");
  fprintf (ge_tmp_fp[1], "        <altitudeMode>relativeToGround</altitudeMode>\n");
  fprintf (ge_tmp_fp[1], "        <outerBoundaryIs>\n");
  fprintf (ge_tmp_fp[1], "          <LinearRing>\n");
  fprintf (ge_tmp_fp[1], "            <coordinates>\n");
  fprintf (ge_tmp_fp[1], "              %.11f,%.11f,10\n", misc.total_displayed_area.min_x, misc.total_displayed_area.min_y);
  fprintf (ge_tmp_fp[1], "              %.11f,%.11f,10\n", misc.total_displayed_area.min_x, misc.total_displayed_area.max_y);
  fprintf (ge_tmp_fp[1], "              %.11f,%.11f,10\n", misc.total_displayed_area.max_x, misc.total_displayed_area.max_y);
  fprintf (ge_tmp_fp[1], "              %.11f,%.11f,10\n", misc.total_displayed_area.max_x, misc.total_displayed_area.min_y);
  fprintf (ge_tmp_fp[1], "              %.11f,%.11f,10\n", misc.total_displayed_area.min_x, misc.total_displayed_area.min_y);
  fprintf (ge_tmp_fp[1], "            </coordinates>\n");
  fprintf (ge_tmp_fp[1], "          </LinearRing>\n");
  fprintf (ge_tmp_fp[1], "        </outerBoundaryIs>\n");
  fprintf (ge_tmp_fp[1], "      </Polygon>\n");
  fprintf (ge_tmp_fp[1], "    </Placemark>\n");
  fprintf (ge_tmp_fp[1], "  </Document>\n");
  fprintf (ge_tmp_fp[1], "</kml>\n");

  fclose (ge_tmp_fp[1]);

  return (0);
}



//!  This is where we launch (or kill) Google Earth.

void 
pfmView::slotGoogleEarth (bool checked)
{
  if (checked)
    {
      QString arg;
      QStringList arguments;

      arguments.clear ();


      sprintf (ge_tmp_name[0], "pfmView_Google_Earth_%d_tmp_link.kml", misc.process_id);
      sprintf (ge_tmp_name[1], "pfmView_Google_Earth_%d_tmp_look.kml", misc.process_id);


      if ((ge_tmp_fp[0] = fopen (ge_tmp_name[0], "w")) == NULL)
        {
          QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Unable to open temporary Google Earth link file!"));
          return;
        }


      //  Get the full path names.

      QString geFile = QFileInfo (QString (ge_tmp_name[0])).absoluteFilePath ();
      QString geFile2 = QFileInfo (QString (ge_tmp_name[1])).absoluteFilePath ();


      //  Put the full names back into the character strings.

      strcpy (ge_tmp_name[0], geFile.toLatin1 ());
      strcpy (ge_tmp_name[1], geFile2.toLatin1 ());


      //  Build the "look at" file.

      if (positionGoogleEarth ())
        {
          fclose (ge_tmp_fp[0]);
          remove (ge_tmp_name[0]);
          return;
        }


      fprintf (ge_tmp_fp[0], "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      fprintf (ge_tmp_fp[0], "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
      fprintf (ge_tmp_fp[0], "  <NetworkLink>\n");
      fprintf (ge_tmp_fp[0], "    <name>NetworkLink</name>\n");
      fprintf (ge_tmp_fp[0], "    <flyToView>1</flyToView>\n");
      fprintf (ge_tmp_fp[0], "    <Link>\n");
      fprintf (ge_tmp_fp[0], "      <href>%s</href>\n", ge_tmp_name[1]);
      fprintf (ge_tmp_fp[0], "      <refreshMode>onInterval</refreshMode>\n");
      fprintf (ge_tmp_fp[0], "      <refreshInterval>4</refreshInterval>\n");
      fprintf (ge_tmp_fp[0], "    </Link>\n");
      fprintf (ge_tmp_fp[0], "  </NetworkLink>\n");
      fprintf (ge_tmp_fp[0], "</kml>\n");

      fclose (ge_tmp_fp[0]);


      arguments << geFile;


      googleEarthProc = new QProcess (this);

      connect (googleEarthProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotGoogleEarthError (QProcess::ProcessError)));
      connect (googleEarthProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotGoogleEarthDone (int, QProcess::ExitStatus)));

      googleEarthProc->start (options.ge_name, arguments);

      redrawMap (NVTrue, NVFalse);
    }
  else
    {
      killGoogleEarth ();
    }
}



//  This kills the Google Earth process and removes the temporary KML file.

void 
pfmView::killGoogleEarth ()
{
#ifdef NVWIN3X

  //  On Windows, Google Earth is a normal application so we can use the normal "kill" to get rid of it.

  if (googleEarthProc->state () == QProcess::Running)
    {
      disconnect (googleEarthProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotGoogleEarthError (QProcess::ProcessError)));
      disconnect (googleEarthProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotGoogleEarthDone (int, QProcess::ExitStatus)));

      googleEarthProc->kill ();
    }


#else

  //  On Linux, Google Earth is run from a script so we can't use the normal "kill" technique because all we'll kill that way will be the script.
  //  That leaves Google Earth still running.  What we need to do on Linux is to find the Google Earth script that belongs to the current user,
  //  find its child, and kill that.

  if (googleEarthProc->state () == QProcess::Running)
    {
      disconnect (googleEarthProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotGoogleEarthError (QProcess::ProcessError)));
      disconnect (googleEarthProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotGoogleEarthDone (int, QProcess::ExitStatus)));

      Q_PID pid = googleEarthProc->pid ();
      QProcess killer;
      QStringList params;
      params << "--ppid";
      params << QString::number(pid);
      params << "-o";
      params << "pid";
      params << "--noheaders";
      killer.start ("/bin/ps", params, QIODevice::ReadOnly);
      if (killer.waitForStarted (-1))
        { 
          if (killer.waitForFinished (-1))
            {
              QByteArray temp = killer.readAllStandardOutput ();
              QString str = QString::fromLocal8Bit (temp);
              QStringList list = str.split ("\n");

              for (int32_t i = 0 ; i < list.size () ; i++)
                {
                  if (!list.at (i).isEmpty ()) kill (list.at (i).toInt (), SIGKILL);
                }
            }
        }


      //  Now kill the script (this probably isn't necessary).

      googleEarthProc->kill ();
    }

#endif

  delete (googleEarthProc);

  googleEarthProc = NULL;

  remove (ge_tmp_name[0]);
  remove (ge_tmp_name[1]);

  googleEarthPushpin.x = googleEarthPushpin.y = -1000.0;


  bGoogleEarth->setChecked (false);


  redrawMap (NVTrue, NVFalse);
}



//!  Error callback from the Google Earth process.

void 
pfmView::slotGoogleEarthError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Unable to start Google Earth!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Google Earth crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Google Earth timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("There was a write error in Google Earth!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("There was a read error in Google Earth!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView Google Earth"), tr ("Google Earth died with an unknown error!"));
      break;
    }


  if (googleEarthProc) killGoogleEarth ();

  redrawMap (NVTrue, NVFalse);
}



//  When Google Earth is closed we need to clear the pushpin and redraw the map but we might as well just use the kill function.

void 
pfmView::slotGoogleEarthDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (googleEarthProc) killGoogleEarth ();

  redrawMap (NVTrue, NVFalse);
}



//!  This is where we link to or unlink from other (non-parent or child) ABE programs.

void 
pfmView::slotLink ()
{
  //  If we're linked, unlink.

  if (misc.linked)
    {
      //  Unregister this application.

      unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id);


      abe_register_group = -1;


      bLink->setToolTip (tr ("Connect to other ABE applications"));
      bLink->setWhatsThis (linkText);
      bLink->setIcon (QIcon (":/icons/unlink.png"));


      //  Unset the link flag.

      misc.linked = NVFalse;


      //  Remove the group id from the title

      QString title;
      title.sprintf ("pfmView : %s", misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);


      setWidgetStates (NVTrue);
    }
  else
    {
      //  Get the ABE_REGISTER shared memory area.

      registerABE *registerABEDialog = new registerABE (this, "pfmView", misc.process_id, misc.abe_share->open_args[0].list_path,
                                                        &misc.abeRegister, &misc.abe_register);


      //  Link to a group (or create a new group).

      connect (registerABEDialog, SIGNAL (keySetSignal (int32_t)), this, SLOT (slotRegisterABEKeySet (int32_t)));
    }
}



//! Callback from the link/unlink slot.

void 
pfmView::slotRegisterABEKeySet (int32_t key)
{
  if (key >= 0)
    {
      abe_register_group = key;


      misc.linked = NVTrue;


      QString title;
      title.sprintf ("pfmView (ABE group %02d) : %s", abe_register_group, misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);


      bLink->setToolTip (tr ("Disconnect from other ABE applications"));
      bLink->setWhatsThis (unlinkText);
      bLink->setIcon (QIcon (":/icons/link.png"));
    }
}



//!  Enable or disable all of the main buttons.

void 
pfmView::setWidgetStates (uint8_t enabled)
{
  //  When we disable the main buttons we want to make sure that mouse presses don't work.

  if (enabled)
    {
      process_running = NVFalse;
    }
  else
    {
      process_running = NVTrue;
    }


  //  If we changed the icon size in the preferences dialog we need to resize all of the buttons.

  QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);

  if (options.main_button_icon_size != prev_icon_size)
    {
      for (int32_t i = 0 ; i < NUM_TOOLBARS ; i++)
        {
          toolBar[i]->setIconSize (mainButtonIconSize);
          toolBar[i]->adjustSize ();
        }

      prev_icon_size = options.main_button_icon_size;
    }


  //  If we changed the font or the button size, set Chart toolbar button/widget size

  if (options.font != prev_font || options.main_button_icon_size != prev_icon_size)
    {
      QFontMetrics fm (options.font);
      QString aString = " 100000.00,100000.00 ";
      int pixelsWide = fm.width (aString);

      displayedArea->setFixedSize (pixelsWide, mainButtonIconSize.height ());

      aString = "1:1000000 ";
      pixelsWide = fm.width (aString);

      viewChartScale->setFixedSize (pixelsWide, mainButtonIconSize.height ());

      prev_font = options.font;
    }

  bDefaultSize->setToolTip (tr ("Set width and height to default displayed area width and height (%L1, %L2)").arg
                            (options.default_width, 0, 'f', 2).arg (options.default_height, 0, 'f', 2));
  bDefaultScale->setToolTip (tr ("Set the chart scale to default displayed area chart scale (1:%1)").arg (options.default_chart_scale));


  //  Set Edit button state based on saved options

  if (options.edit_mode)
    {
      bEditMode->setIcon (QIcon (":/icons/edit_poly.png"));
    }
  else
    {
      bEditMode->setIcon (QIcon (":/icons/edit_rect.png"));
    }


  //  Set the "no load" data types (invalid and/or reference) based on the saved options.

  QString mask_icon = QString (":/icons/inv_ref_%1%2.png").arg (!options.editor_no_load[0]).arg (!options.editor_no_load[1]);
  dataTypeButtons->setContents (QPixmap (mask_icon));
  dataTypeButtons->setEnabled (enabled);


  //  The stop button is only enabled during drawing

  if (cov_area_defined) bStop->setEnabled (!enabled);


  //  Only enable the layers menu if we have more than one layer.

  if (misc.abe_share->pfm_count > 1)
    {
      layerMenu->setEnabled (enabled);
    }
  else
    {
      layerMenu->setEnabled (false);
    }

  if (misc.abe_share->pfm_count)
    {
      editMenu->setEnabled (enabled);
      viewMenu->setEnabled (enabled);
      toolsMenu->setEnabled (enabled);
    }
  else
    {
      editMenu->setEnabled (false);
      viewMenu->setEnabled (false);
      toolsMenu->setEnabled (false);
    }


  bStoplight->setEnabled (enabled);
  bContour->setEnabled (enabled);
  bGrid->setEnabled (enabled);
  bCoast->setEnabled (enabled);
  bCovCoast->setEnabled (enabled);
  fileCloseAction->setEnabled (enabled);
  geotiffOpenAction->setEnabled (enabled);


  //  Only enable the GeoTIFF display button if we have opened a GeoTIFF file

  if (enabled && misc.GeoTIFF_open)
    {
      bGeotiff->setEnabled (enabled);
    }
  else
    {
      bGeotiff->setEnabled (false);
    }


  //  Only enable the zoom out button if we've zoomed in.

  mapdef = map->getMapdef ();
  if (enabled && mapdef.zoom_level > 0)
    {
      bZoomOut->setEnabled (enabled);
    }
  else
    {
      bZoomOut->setEnabled (false);
    }

  bZoomIn->setEnabled (enabled);
  bPrefs->setEnabled (enabled);
  bEditMode->setEnabled (enabled);
  bDisplayMinMax->setEnabled (enabled);
  bRedraw->setEnabled (enabled);
  bAutoRedraw->setEnabled (enabled);
  bRedrawCoverage->setEnabled (enabled);
  bDisplaySuspect->setEnabled (enabled);
  bDisplayFeature->setEnabled (enabled);
  bLink->setEnabled (enabled);


  //  Set the correct displayMinMax What's This help based on the Z orientation.

  if (options.z_orientation > 0.0)
    {
      bDisplayMinMax->setWhatsThis (tr ("<img source=\":/icons/display_min_max.png\"> Click this button to flag the valid minimum, maximum, "
                                        "and maximum standard deviation bins.  Bins will be marked with a circle for the minimum, a "
                                        "square for the maximum, and a diamond for the maximum standard deviation."));
    }
  else
    {
      bDisplayMinMax->setWhatsThis (tr ("<img source=\":/icons/display_min_max.png\"> Click this button to flag the valid minimum, maximum, "
                                        "and maximum standard deviation bins.  Bins will be marked with a circle for the maximum, a "
                                        "square for the minimum, and a diamond for the maximum standard deviation."));
    }


  //  Check for latitude/longitude or northing/easting display

  if (options.lat_or_east)
    {
      latName->setText (tr ("Northing"));
      latName->setToolTip (tr ("Northing of cursor (zone %1), click to switch to latitude and longitude").arg (zoneName));
      latName->setWhatsThis (tr ("This is the UTM northing of the center of the bin that is nearest to the cursor.<br><br>"
                                 "<b>Click this button to switch to displaying latitude and longitude.</b>"));
      latLabel->setToolTip (tr ("Northing of cursor (zone %1)").arg (zoneName));

      lonName->setText (tr ("Easting"));
      lonName->setToolTip (tr ("Easting of cursor (zone %1)").arg (zoneName));
      lonName->setWhatsThis (tr ("This is the UTM easting of the center of the bin that is nearest to the cursor.<br><br>"
                                 "<b>Click this button to switch to displaying latitude and longitude.</b>"));
      lonLabel->setToolTip (tr ("Easting of cursor (zone %1)").arg (zoneName));
    }
  else
    {
      latName->setText (tr ("Latitude"));
      latName->setToolTip (tr ("Latitude of cursor, click to switch to northing and easting"));
      latName->setWhatsThis (tr ("This is the latitude of the center of the bin that is nearest to the cursor.  The format of the latitude "
                                 "may be changed in the Position Format tab of the <b>Preferences</b> dialog "
                                 "<img source=\":/icons/prefs.png\">.<br><br>"
                                 "<b>Click this button to switch to displaying northing and easting.</b>"));
      latLabel->setToolTip (tr ("Latitude of cursor"));

      lonName->setText (tr ("Longitude"));
      lonName->setWhatsThis (tr ("This is the longitude of the center of the bin that is nearest to the cursor.  The format of the longitude "
                                 "may be changed in the Position Format tab of the <b>Preferences</b> dialog "
                                 "<img source=\":/icons/prefs.png\">."));
      lonName->setToolTip (tr ("Longitude of cursor"));
      lonLabel->setToolTip (tr ("Longitude of cursor"));
    }



  //  Don't enable these if we're in read_only mode.

  if (misc.abe_share->read_only)
    {
      bFilterDisplay->setEnabled (false);
      bFilterRect->setEnabled (false);
      bFilterPoly->setEnabled (false);
      bFilterRectMask->setEnabled (false);
      bFilterPolyMask->setEnabled (false);
      bSetChecked->setEnabled (false);
      bUnload->setEnabled (false);
      fileChecked->setEnabled (false);
      fileUnchecked->setEnabled (false);
      fileVerified->setEnabled (false);
      fileUnverified->setEnabled (false);
      deleteRestore->setEnabled (false);
      deleteFileQueue->setEnabled (false);
      changePathAct->setEnabled (false);
    }
  else
    {
      bFilterDisplay->setEnabled (enabled);
      bFilterRect->setEnabled (enabled);
      bFilterPoly->setEnabled (enabled);
      bFilterRectMask->setEnabled (enabled);
      bFilterPolyMask->setEnabled (enabled);
      bSetChecked->setEnabled (enabled);
      bUnload->setEnabled (enabled);
      fileChecked->setEnabled (enabled);
      fileUnchecked->setEnabled (enabled);
      fileVerified->setEnabled (enabled);
      fileUnverified->setEnabled (enabled);
      deleteRestore->setEnabled (enabled);

      if (!misc.abe_share->delete_file_queue_count)
        {
          deleteFileQueue->setEnabled (false);
        }
      else
        {
          deleteFileQueue->setEnabled (enabled);
        }

      changePathAct->setEnabled (enabled);
    }


  //  Only enable the Display Messages option if we have messages to display.

  if (enabled && misc.messages->size ())
    {
      messageAction->setEnabled (enabled);
    }
  else
    {
      messageAction->setEnabled (false);
    }


  //  Only enable the clear filter mask button if we have active filter masks.

  if (!misc.poly_filter_mask_count)
    {
      bClearMasks->setEnabled (false);
    }
  else
    {
      bClearMasks->setEnabled (enabled);
    }


  //  Only enable the clear filter contour button if we have active filter contours.

  if (!misc.filt_contour_count)
    {
      bClearFilterContours->setEnabled (false);
    }
  else
    {
      bClearFilterContours->setEnabled (enabled);
    }


  //  Only enable the feature related buttons if we have an open feature file.

  if (misc.bfd_open)
    {
      findFeatures->setEnabled (enabled);
      definePolygons->setEnabled (enabled);
      outputFeature->setEnabled (enabled);
    }
  else
    {
      findFeatures->setEnabled (false);
      definePolygons->setEnabled (false);
      outputFeature->setEnabled (false);
    }


  for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      //  Turn on the remisp buttons if we have any MISP average surface types.  Note that we break out of the 
      //  loop if this is the case.

      if (enabled && misc.average_type[pfm] > 0 && !misc.abe_share->read_only)
        {
          //  Turn on the clear contours button if we have drawn contours.

          if (misc.filt_contour_count)
            {
              bClearFilterContours->setEnabled (enabled);
            }
          else
            {
              bClearFilterContours->setEnabled (false);
            }

          bDrawContour->setEnabled (enabled);
          bGrabContour->setEnabled (enabled);
          bDrawContourFilter->setEnabled (enabled);
          bRemisp->setEnabled (enabled);
          bRemispFilter->setEnabled (enabled);
          break;
        }
      else
        {
          bRemisp->setEnabled (false);
          bRemispFilter->setEnabled (false);
          bDrawContour->setEnabled (false);
          bGrabContour->setEnabled (false);
          bDrawContourFilter->setEnabled (false);
          bClearFilterContours->setEnabled (false);
        }
    }


  bHigh->setEnabled (enabled);
  bLow->setEnabled (enabled);


  //  If we have the cube executable and the cube attributes and we're only displaying one surface and that surface
  //  is the CUBE surface, we want to offer the recube button and set the average surface button accordingly.

  bCube->setEnabled (false);
  if (misc.abe_share->pfm_count == 1 && misc.cube_available && misc.cube_attr_available[0] &&
      !misc.abe_share->read_only && misc.average_type[0] == -1)
    {
      bCube->setEnabled (enabled);
      bSurface[0]->setIcon (QIcon  (":/icons/cube_surface.png"));
      QString tip = tr ("Display CUBE surface  [%1]").arg (options.buttonAccel[DISPLAY_AVG_EDITED_KEY]);
      misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
    }


  bGoogleEarth->setEnabled (false);
  bGoogleEarthPin->setEnabled (false);
  if (misc.googleearth_available)
    {
      bGoogleEarth->setEnabled (enabled);
      if (googleEarthProc) bGoogleEarthPin->setEnabled (enabled);
    }


  //  No point in having feature info buttons if we're not displaying features

  if (enabled && options.display_feature && !misc.abe_share->read_only)
    {
      bDisplayChildren->setEnabled (enabled);
      bDisplayFeatureInfo->setEnabled (enabled);
      bDisplayFeaturePoly->setEnabled (enabled);
      bAddFeature->setEnabled (enabled);
      bDeleteFeature->setEnabled (enabled);
      bEditFeature->setEnabled (enabled);
      bVerifyFeatures->setEnabled (enabled);
      fileImportAction->setEnabled (enabled);
    }
  else
    {
      bDisplayChildren->setEnabled (false);
      bDisplayFeatureInfo->setEnabled (false);
      bDisplayFeaturePoly->setEnabled (false);
      bAddFeature->setEnabled (false);
      bDeleteFeature->setEnabled (false);
      bEditFeature->setEnabled (false);
      bVerifyFeatures->setEnabled (false);
      fileImportAction->setEnabled (false);
    }


  bDisplaySelected->setEnabled (enabled);
  bDisplayReference->setEnabled (enabled);


  //  Don't allow highlighting if we are doing on-the-fly gridding.

  if (misc.otf_surface)
    {
      bHighlight->setEnabled (false);
    }
  else
    {
      bHighlight->setEnabled (enabled);
    }


  for (int32_t i = 0 ; i < NUM_SURFACES ; i++) bSurface[i]->setEnabled (enabled);
  bSetOtfBin->setEnabled (enabled);


  //  Check whether we are attribute limiting our OTF gridding and set the proper button icon.

  if (misc.otf_attr < 0)
    {
      bSetOtfBin->setIcon (QIcon (":/icons/set_otf_size.png"));
    }
  else
    {
      bSetOtfBin->setIcon (QIcon (":/icons/set_otf_size_attr.png"));
    }


  //  Set the scaleBox ToolTips and WhatsThis (inverted for attributes).

  int32_t k = misc.color_index;
  if (k)
    {
      QString str;

      if (options.max_hsv_locked[k])
        {
          str = tr ("Click here to unlock the maximum value (%L1) and/or change the maximum color").arg (options.max_hsv_value[k], 0, 'f', 2);
          misc.scale[0]->setToolTip (str);
          misc.scale[0]->setWhatsThis (str);
        }
      else
        {
          misc.scale[0]->setToolTip (tr ("Click here to lock the maximum value and/or change the maximum color"));
          misc.scale[0]->setWhatsThis (tr ("Click here to lock the maximum value and/or change the maximum color"));
        }

      if (options.min_hsv_locked[k])
        {
          str = tr ("Click here to unlock the minimum value (%L1) and/or change the maximum color").arg (options.min_hsv_value[k], 0, 'f', 2);
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (str);
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (str);
        }
      else
        {
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (tr ("Click here to lock the minimum value and/or change the minimum color"));
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (tr ("Click here to lock the minimum value and/or change the minimum color"));
        }
    }
  else
    {
      QString str;

      if (options.min_hsv_locked[k])
        {
          str = tr ("Click here to unlock the minimum value (%L1) and/or change the maximum color").arg (options.min_hsv_value[k], 0, 'f', 2);
          misc.scale[0]->setToolTip (str);
          misc.scale[0]->setWhatsThis (str);
        }
      else
        {
          misc.scale[0]->setToolTip (tr ("Click here to lock the minimum value and/or change the minimum color"));
          misc.scale[0]->setWhatsThis (tr ("Click here to lock the minimum value and/or change the minimum color"));
        }

      if (options.max_hsv_locked[k])
        {
          str = tr ("Click here to unlock the maximum value (%L1) and/or change the maximum color").arg (options.max_hsv_value[k], 0, 'f', 2);
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (str);
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (str);
        }
      else
        {
          misc.scale[NUM_SCALE_LEVELS - 1]->setToolTip (tr ("Click here to lock the maximum value and/or change the maximum color"));
          misc.scale[NUM_SCALE_LEVELS - 1]->setWhatsThis (tr ("Click here to lock the maximum value and/or change the maximum color"));
        }
    }


  for (int32_t i = 0 ; i < POST_ATTR ; i++)
    {
      if (misc.attrStatNum[i] > -1)
        {
          attrName[i + PRE_ATTR]->setEnabled (true);
          attrName[i + PRE_ATTR]->setToolTip (options.attrStatName[i] + QString (tr (" - Click to color by this attribute.")));
          attrName[i + PRE_ATTR]->setWhatsThis (options.attrStatName[i] + QString (tr (" - Click to color by this attribute.")));
          attrValue[i + PRE_ATTR]->setToolTip (tr ("Value of %1 at the cursor.").arg (options.attrStatName[i]));
          attrValue[i + PRE_ATTR]->setWhatsThis (tr ("Value of %1 at the cursor.").arg (options.attrStatName[i]));
        }
      else
        {
          attrName[i + PRE_ATTR]->setEnabled (false);
          attrName[i + PRE_ATTR]->setToolTip (tr ("No attribute selected - Select attribute on Attributes page."));
          attrName[i + PRE_ATTR]->setWhatsThis (tr ("No attribute selected - Select attribute on Attributes page."));
          attrValue[i + PRE_ATTR]->setToolTip (tr ("No attribute selected - Select attribute on Attributes page."));
          attrValue[i + PRE_ATTR]->setWhatsThis (tr ("No attribute selected - Select attribute on Attributes page."));
        }
    }


  //  Set the button states based on the active function.

  bEditMode->setChecked (false);
  switch (misc.function)
    {
    case RECT_EDIT_AREA_3D:
      bEditMode->setChecked (true);
      break;

    case POLY_EDIT_AREA_3D:
      bEditMode->setChecked (true);
      break;

    case RECT_FILTER_AREA:
      bFilterRect->setChecked (true);
      break;

    case POLY_FILTER_AREA:
      bFilterPoly->setChecked (true);
      break;

    case RECT_FILTER_MASK:
      bFilterRectMask->setChecked (true);
      break;

    case POLY_FILTER_MASK:
      bFilterPolyMask->setChecked (true);
      break;

    case ADD_FEATURE:
      bAddFeature->setChecked (true);
      break;

    case EDIT_FEATURE:
      bEditFeature->setChecked (true);
      break;

    case DELETE_FEATURE:
      bDeleteFeature->setChecked (true);
      break;

    case DRAW_CONTOUR:
      bDrawContour->setChecked (true);
      break;

    case GRAB_CONTOUR:
      bGrabContour->setChecked (true);
      break;

    case REMISP_AREA:
      bRemisp->setChecked (true);
      break;

    case DRAW_CONTOUR_FILTER:
      bDrawContourFilter->setChecked (true);
      break;

    case REMISP_FILTER:
      bRemispFilter->setChecked (true);
      break;

    case SELECT_HIGH_POINT:
      bHigh->setChecked (true);
      break;

    case SELECT_LOW_POINT:
      bLow->setChecked (true);
      break;

    case GOOGLE_EARTH:
      bGoogleEarth->setChecked (true);
      break;
    }


  //  Set the "color by" labels.

  for (int32_t i = 0 ; i < PRE_ATTR + POST_ATTR; i++)
    {
      attrName[i]->setPalette (label_palette[i % 2]);
      attrValue[i]->setPalette (label_palette[i % 2]);
    }


  int32_t ndx = misc.color_index;

  attrName[ndx]->setPalette (colorPalette[ndx % 2]);


  //  Set the color of the attributes that are duplicated in the Status tab to a blue fade.

  for (int32_t j = 0 ; j < misc.abe_share->open_args[0].head.num_bin_attr ; j++)
    {
      if (misc.attrBoxFlag[j])
        {
          attrBoxName[j]->setPalette (colorBoxPalette[j % 2]);
        }
      else
        {
          attrBoxName[j]->setPalette (label_palette[j % 2]);
        }
    }


  //  If we're coloring by an attribute we need to set the attribute name on the Attributes tab to a red fade.

  if (ndx >= PRE_ATTR)
    {
      int32_t j = misc.attrStatNum[ndx - PRE_ATTR];

      attrBoxName[j]->setPalette (colorPalette[j % 2]);
    }


  //  This just makes sure that the palette is set correctly if we're scaling or offsetting the depth value.

  if (options.z_factor != 1.0 || options.z_offset != 0.0)
    {
      attrPalette[0].setColor (QPalette::WindowText, options.scaled_offset_z_color);
    }
  else
    {
      attrPalette[0].setColor (QPalette::WindowText, misc.widgetForegroundColor);
    }
  attrPalette[0].setColor (QPalette::Window, QColor (255, 255, 255));
  attrValue[0]->setPalette (attrPalette[0]);
}



//!  Clear all movable objects.

void 
pfmView::discardMovableObjects ()
{
  map->closeMovingPath (&mv_arrow);
  map->closeMovingPath (&mv_marker);
  map->closeMovingPath (&mv_tracker);
  map->closeMovingPolygon (&pfm3D_polygon);
  map->closeMovingPolygon (&mv_polygon);
  map->discardRubberbandRectangle (&rb_rectangle);
  map->discardRubberbandPolygon (&rb_polygon);
}



//!  Left mouse press.  Called from map mouse press callback.

void 
pfmView::leftMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)), double lon, double lat)
{
  void get_feature_event_time (int32_t pfm_handle, DEPTH_RECORD depth, time_t *tv_sec, long *tv_nsec);
  void writeContour (MISC *misc, float z_factor, float z_offset, int32_t count, double *cur_x, double *cur_y);
  void filterPolyArea (OPTIONS *options, MISC *misc, double *mx, double *my, int32_t count);
  uint8_t setHighLow (double *mx, double *my, MISC *misc, OPTIONS *options, nvMap *map);
  void filterUndo (MISC *misc);
  void mark_feature (MISC *misc, BFDATA_RECORD *bfd_record);


  BIN_RECORD          bin;
  int32_t             count, *px, *py, ndx;
  double              *mx, *my, mz[4], mm[4], *cur_x, *cur_y;
  NV_F64_XYMBR        bounds;
  QFileDialog         *fd;
  QString             file, string;
  FILE                *fp;
  char                fname[512], ltstring[25], lnstring[25], hem, shape_name[512], prj_file[512];
  double              deg, min, sec;
  uint8_t             hit;
  SHPHandle           shp_hnd;
  SHPObject           *shape;
  DBFHandle           dbf_hnd;  


  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  //  Actions based on the active function.  For the most part, if a rubberband rectangle or polygon is active then
  //  this is the second mouse press and we want to perform the operation.  If one isn't present then it's the
  //  first mouse click and we want to start (anchor) a rectangle, line, or polygon.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:

      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (int32_t i = 0 ; i < 4 ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
              if (my[i] < bounds.min_y) bounds.min_y = my[i];
              if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
              if (my[i] > bounds.max_y) bounds.max_y = my[i];
            }

          misc.clear = NVTrue;

          misc.GeoTIFF_init = NVTrue;

          zoomIn (bounds, NVTrue);


          misc.cov_function = COV_START_DRAW_RECTANGLE;

          redrawCovBounds ();

          map->setToolTip ("");      
          misc.function = misc.save_function;

          setFunctionCursor (misc.function);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine); 
       }
      break;

    case RECT_EDIT_AREA_3D:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          editCommand (mx, my, 4);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);

          if (setHighLow (mx, my, &misc, &options, map)) cov->redrawMap (NVTrue);

          map->discardRubberbandRectangle (&rb_rectangle);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case RECT_FILTER_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);


          misc.filtered_count = 0;


          //  Using a 4 point polygon to avoid writing new filter code.

          filterPolyArea (&options, &misc, mx, my, 4);


          redrawMap (NVTrue, NVTrue);

          if (misc.filtered_count)
            {
              QMessageBox msgBox;
              msgBox.setWindowTitle (tr ("pfmView Filter"));
              msgBox.setText (tr ("%1 points marked invalid").arg (misc.filtered_count));
              msgBox.setInformativeText ("Do you want to save your changes?");
              msgBox.setStandardButtons (QMessageBox::Save | QMessageBox::Discard);
              msgBox.setDefaultButton (QMessageBox::Save);


              //  This is weird.  If I let the message box be modal, nothing else happens but it greys out the window that you
              //  need to look in order to verify that you want to accept the changes.  If I make it non-modal it will leave the 
              //  window alone but you can start clicking buttons and other bad stuff.  So, to make it sort of work, I'm setting
              //  the misc.drawing flag so that everything else will be ignored until you answer the message box.  I also have
              //  to use the "Qt::WindowStaysOnTopHint" to keep it from disappearing into the background if you click on the main
              //  window.

              misc.drawing = NVTrue;
              setWidgetStates (NVFalse);

              msgBox.setWindowFlags (msgBox.windowFlags () | Qt::WindowStaysOnTopHint);
              msgBox.setModal (false);


              //  Move the dialog to the center of the parent if possible.

              QRect tmp = frameGeometry ();
              int32_t window_x = tmp.x ();
              int32_t window_y = tmp.y ();


              tmp = geometry ();
              int32_t width = tmp.width ();
              int32_t height = tmp.height ();


              QSize dlg = msgBox.sizeHint ();
              int32_t dlg_width = dlg.width ();

              msgBox.move (window_x + width / 2 - dlg_width / 2, window_y + height / 2);

              msgBox.show ();

              int ret = msgBox.exec ();

              misc.drawing = NVFalse;
              setWidgetStates (NVTrue);


              if (ret == QMessageBox::Discard)
                {
                  filterUndo (&misc);
                  redrawMap (NVTrue, NVTrue);
                }


              //  Clear the undo memory;

              if (misc.filtered_count)
                {
                  misc.undo.clear ();
                  misc.filtered_count = 0;
                }
            }
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case RECT_FILTER_MASK:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);


          //  Increment the filter mask count and resize the memory.

          try
            {
              misc.poly_filter_mask.resize (misc.poly_filter_mask_count + 1);
            }
          catch (std::bad_alloc&)
            {
              fprintf (stderr, "%s %s %s %d - poly_filter_mask - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
              exit (-1);
            }


          //  Using a 4 point polygon to avoid writing new masking code.

          misc.poly_filter_mask[misc.poly_filter_mask_count].count = 4;

          for (int32_t i = 0 ; i < 4 ; i++)
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = mx[i];
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = my[i];
            }


          map->discardRubberbandRectangle (&rb_rectangle);


          ndx = misc.poly_filter_mask_count;
          if (options.poly_filter_mask_color.alpha () < 255)
            {
              map->fillPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y, 
                                options.poly_filter_mask_color, NVTrue);
            }
          else
            {
              //  We don't have to worry about clipping this because moving the area discards the mask areas.

              map->drawPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y,
                                options.poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
            }

          misc.poly_filter_mask[ndx].displayed = NVTrue;

          misc.poly_filter_mask_count++;
          bClearMasks->setEnabled (true);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;

    case REMISP_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          count = 4;

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (int32_t i = 0 ; i < count ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              bounds.min_x = qMin (bounds.min_x, mx[i]);
              bounds.min_y = qMin (bounds.min_y, my[i]);
              bounds.max_x = qMax (bounds.max_x, mx[i]);
              bounds.max_y = qMax (bounds.max_y, my[i]);
            }

          remisp (&misc, &options, &bounds);

          map->discardRubberbandRectangle (&rb_rectangle);


          misc.function = misc.save_function;
          setFunctionCursor (misc.function);


          //  If filter contours were used, remove them.

          if (misc.filt_contour_count)
            {
              slotClearFilterContours ();
            }
          else
            {
              redrawMap (NVTrue, NVTrue);
            }
       }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:

      //  Second left click

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          int32_t count;
          map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);

          cur_x = (double *) malloc ((count) * sizeof (double));
          cur_y = (double *) malloc ((count) * sizeof (double));
          double *new_x = (double *) malloc ((count) * sizeof (double));
          double *new_y = (double *) malloc ((count) * sizeof (double));

          if (new_y == NULL)
            {
              fprintf (stderr , tr ("Error allocating new_y - %s %s %s %d\n").toLatin1 (), misc.progname, __FILE__,  __FUNCTION__, __LINE__);
              exit (-1);
            }


          for (int32_t i = 0 ; i < count ; i++)
            {
	      if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;
              cur_x[i] = mx[i];
              cur_y[i] = my[i];
            }
          map->discardRubberbandPolygon (&rb_polygon);


          //  We want to plot the line after we have gotten rid of its moveable image.
          //  We also check to find out which PFM layers the line passes through.

          int32_t new_count = 0;
          double prev_x = -181.0, prev_y = -91.0;
          for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++) misc.contour_in_pfm[pfm] = NVFalse;

          for (int32_t i = 0 ; i < count ; i++)
            {
              NV_F64_COORD2 nxy = {cur_x[i], cur_y[i]};


              //  Check against any of the PFM layers.  Don't save points outside of the PFM areas.

              uint8_t hit = NVFalse;
              for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
                {
                  if (bin_inside_ptr (&misc.abe_share->open_args[pfm].head, nxy))
                    {
                      if (prev_x > -181.0) map->drawLine (prev_x, prev_y, cur_x[i], cur_y[i], options.contour_highlight_color,
                                                          LINE_WIDTH, NVTrue, Qt::SolidLine);

                      prev_x = cur_x[i];
                      prev_y = cur_y[i];
                      new_x[new_count] = cur_x[i];
                      new_y[new_count] = cur_y[i];
                      new_count++;
                      misc.contour_in_pfm[pfm] = hit = NVTrue;
                      break;
                    }
                }

              if (!hit) prev_x = -181.0;
            }


          //  Force the map to update.

          map->flush ();


          free (cur_x);
          free (cur_y);


          //  If the contour passed through any of the PFM layers we want to insert the contour values into the PFM file (in the case of DRAW_CONTOUR).

          if (new_count)
            {
              qApp->setOverrideCursor (Qt::WaitCursor);
              qApp->processEvents ();


              if (misc.function == DRAW_CONTOUR)
                {
                  //  Write the points to the PFM file.

                  writeContour (&misc, options.z_factor, options.z_offset, new_count, new_x, new_y);
                }
              else if (misc.function == DRAW_CONTOUR_FILTER)
                {
                  //  Save the data for MISP surface generation and filtering.

                  try
                    {
                      misc.filt_contour.resize (misc.filt_contour_count + new_count + 1);
                    }
                  catch (std::bad_alloc&)
                    {
                      fprintf (stderr, "%s %s %s %d - filt_contour - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                      exit (-1);
                    }

                  for (int32_t i = 0 ; i < new_count ; i++)
                    {
                      misc.filt_contour[misc.filt_contour_count].x = new_x[i];
                      misc.filt_contour[misc.filt_contour_count].y = new_y[i];
                      misc.filt_contour[misc.filt_contour_count].z = (misc.draw_contour_level - options.z_offset) / options.z_factor;
                      misc.filt_contour_count++;
                    }


                  //  Set the end sentinel.

                  misc.filt_contour[misc.filt_contour_count].x = -181.0;
                  misc.filt_contour[misc.filt_contour_count].y = -91.0;
                  misc.filt_contour[misc.filt_contour_count].z = -999999.0;
                  misc.filt_contour_count++;

                  bClearFilterContours->setEnabled (true);
                }


              qApp->restoreOverrideCursor ();
            }
          else
            {
              QMessageBox::warning (this, tr ("pfmView Draw contour"), tr ("No input points defined in cells without valid data"));
            }


          free (new_x);
          free (new_y);

          prev_poly_lon = -181.0;
        }


      //  First left click

      else
        {
          QString msc = tr (" Draw contour - %L1 ").arg (misc.draw_contour_level, 0, 'f', 2);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);

          map->anchorRubberbandPolygon (&rb_polygon, lon, lat, options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
          prev_poly_lon = -181.0;
        }
      break;


    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->closeRubberbandRectangle (rb_rectangle, lon, lat, &px, &py, &mx, &my);
          count = 4;

          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;
          for (int32_t i = 0 ; i < count ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              bounds.min_x = qMin (bounds.min_x, mx[i]);
              bounds.min_y = qMin (bounds.min_y, my[i]);
              bounds.max_x = qMax (bounds.max_x, mx[i]);
              bounds.max_y = qMax (bounds.max_y, my[i]);
            }


          if (!QDir (options.output_area_dir).exists ()) options.output_area_dir = options.input_pfm_dir;


          fd = new QFileDialog (this, tr ("pfmView Output Area File"));
          fd->setViewMode (QFileDialog::List);


          //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
          //  This function is in the nvutility library.

          setSidebarUrls (fd, options.output_area_dir);


          QStringList filters;
          filters << tr ("Generic area file (*.are)")
                  << tr ("Army Corps area file (*.afs)")
                  << tr ("ESRI Shapefile (*.shp)");

          fd->setNameFilters (filters);
          fd->setFileMode (QFileDialog::AnyFile);
          fd->selectNameFilter (options.output_area_name_filter);

          hit = NVFalse;

          QStringList files;
          QString file;
          if (fd->exec () == QDialog::Accepted)
            {
              options.output_area_dir = fd->directory ().absolutePath ();

              files = fd->selectedFiles ();

              file = files.at (0);


              if (!file.isEmpty())
                {
                  //  Add extension to filename if not there.

                  if (fd->selectedNameFilter ().contains ("*.are"))
                    {
                      if (!file.endsWith (".are")) file.append (".are");
                    }
                  else if (fd->selectedNameFilter ().contains ("*.afs"))
                    {
                      if (!file.endsWith (".afs")) file.append (".afs");
                    }
                  else if (fd->selectedNameFilter ().contains ("*.shp"))
                    {
                      if (!file.endsWith (".shp")) file.append (".shp");
                    }


                  strcpy (fname, file.toLatin1 ());


                  //  Write the file.

                  if ((fp = fopen (fname, "w")) != NULL)
                    {
                      if (file.endsWith (".are"))
                        {
                          for (int32_t i = 0 ; i < count ; i++)
                            {
                              strcpy (ltstring, fixpos (my[i], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
                              strcpy (lnstring, fixpos (mx[i], &deg, &min, &sec, &hem, POS_LON, options.position_form));

                              fprintf (fp, "%s, %s\n", ltstring, lnstring);
                            }

                          fclose (fp);
                        }
                      else if (file.endsWith (".afs"))
                        {
                          for (int32_t i = 0 ; i < count ; i++)
                            {
                              //  Make sure we haven't created any duplicate points

                              if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

                              fprintf (fp, "%.9f, %.9f\n", mx[i], my[i]);
                            }

                          fclose (fp);
                        }
                      else
                        {
                          strcpy (shape_name, fname);
                          shape_name[strlen (shape_name) - 4] = 0;
                      
                          if ((shp_hnd = SHPCreate (shape_name, SHPT_POLYGON)) == NULL)
                            {
                              QMessageBox::warning (this, "pfmView", tr ("Error creating shp file."));
                              break;
                            }


                          if ((dbf_hnd = DBFCreate (shape_name)) == NULL)
                            {
                              QMessageBox::warning (this, "pfmView", tr ("Error creating dbf file."));
                              break;
                            }


                          /*  Make attributes*/

                          if (DBFAddField (dbf_hnd, "file", FTString, 15, 0 ) == -1)
                            {
                              QMessageBox::warning (this, "pfmView", tr ("Error adding field to dbf file."));
                              break;
                            }


                          //  Stupid freaking .prj file

                          strcpy (prj_file, shape_name);
                          strcat (prj_file, ".prj");

                          if ((fp = fopen (prj_file, "w")) == NULL)
                            {
                              QMessageBox::warning (this, "pfmView", tr ("Error creating prj file."));
                              break;
                            }

                          fprintf (fp, "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]\n");
                          fclose (fp);


                          for (int32_t i = 0 ; i < count ; i++)
                            {
                              //  Make sure we haven't created any duplicate points

                              if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

                              mz[i] = 0.0;
                              mm[i] = 0.0;
                            }


                          shape = SHPCreateObject (SHPT_POLYGON, -1, 0, NULL, NULL, count, mx, my, mz, mm);
                          SHPWriteObject (shp_hnd, -1, shape);
                          SHPDestroyObject (shape);
                          DBFWriteStringAttribute (dbf_hnd, 0, 0, shape_name);

                          SHPClose (shp_hnd);
                          DBFClose (dbf_hnd);
                        }


                      //  Add the new file to the first available overlay slot.

                      for (int32_t i = 0 ; i < NUM_OVERLAYS ; i++)
                        {
                          if (!misc.overlays[i].file_type)
                            {
                              if (file.endsWith (".are"))
                                {
                                  misc.overlays[i].file_type = GENERIC_AREA;
                                }
                              else if (file.endsWith (".afs"))
                                {
                                  misc.overlays[i].file_type = ACE_AREA;
                                }
                              else
                                {
                                  misc.overlays[i].file_type = SHAPE;
                                }
                              strcpy (misc.overlays[i].filename, file.toLatin1 ());
                              misc.overlays[i].display = NVTrue;
                              misc.overlays[i].color = options.contour_color;

                              hit = NVTrue;

                              break;
                            }
                        }
                    }

                  options.output_area_name_filter = fd->selectedNameFilter ();
                }
            }

          map->discardRubberbandRectangle (&rb_rectangle);

          map->setToolTip ("");      
          misc.function = misc.save_function;

          setFunctionCursor (misc.function);

          if (hit) redrawMap (NVTrue, NVFalse);
        }
      else
        {
          map->anchorRubberbandRectangle (&rb_rectangle, lon, lat, options.contour_color, LINE_WIDTH, Qt::SolidLine);
        }
      break;


    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->vertexRubberbandPolygon (rb_polygon, lon, lat);
        }
      else
        {
          map->anchorRubberbandPolygon (&rb_polygon, lon, lat, options.contour_color, LINE_WIDTH, NVTrue, Qt::SolidLine);
        }
      break;


    case OUTPUT_POINTS:
      try
        {
          misc.output_points.resize (misc.output_point_count + 1);
        }
      catch (std::bad_alloc&)
        {
          fprintf (stderr, "%s %s %s %d - output_points - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }

      misc.output_points[misc.output_point_count].xy.x = lon;
      misc.output_points[misc.output_point_count].xy.y = lat;

      read_bin_record_xy (misc.pfm_handle[0], misc.output_points[misc.output_point_count].xy, &bin);
      misc.output_points[misc.output_point_count].avg = options.z_orientation * bin.avg_filtered_depth * options.z_factor + options.z_offset;
      misc.output_points[misc.output_point_count].min = options.z_orientation * bin.min_filtered_depth * options.z_factor + options.z_offset;
      misc.output_points[misc.output_point_count].max = options.z_orientation * bin.max_filtered_depth * options.z_factor + options.z_offset;

      map->fillCircle (lon, lat, 10, 0.0, 360.0, options.contour_color, NVTrue);

      string = tr (" Point #%1").arg (misc.output_point_count);

      map->drawText (string, lon, lat, options.contour_color, NVTrue);

      misc.output_point_count++;
      break;


    case EDIT_FEATURE:
      if (misc.nearest_feature != -1) editFeatureNum (misc.nearest_feature);
      break;


    case ADD_FEATURE:
      NV_F64_XYMBR mbr;
      double tmplat, tmplon;


      //  Search within 20 meters for a valid point.

      newgp (lat, lon, 0.0, 20.0, &mbr.max_y, &tmplon);
      newgp (lat, lon, 90.0, 20.0, &tmplat, &mbr.max_x);
      newgp (lat, lon, 180.0, 20.0, &mbr.min_y, &tmplon);
      newgp (lat, lon, 270.0, 20.0, &tmplat, &mbr.min_x);


      //  Adjust bounds to nearest grid point

      mbr.min_y = misc.abe_share->open_args[0].head.mbr.min_y + 
        (NINT ((mbr.min_y - misc.abe_share->open_args[0].head.mbr.min_y) / 
               misc.abe_share->open_args[0].head.y_bin_size_degrees)) * misc.abe_share->open_args[0].head.y_bin_size_degrees;

      mbr.max_y = misc.abe_share->open_args[0].head.mbr.min_y + 
        (NINT ((mbr.max_y - misc.abe_share->open_args[0].head.mbr.min_y) /
               misc.abe_share->open_args[0].head.y_bin_size_degrees)) * misc.abe_share->open_args[0].head.y_bin_size_degrees;

      mbr.min_x = misc.abe_share->open_args[0].head.mbr.min_x + 
        (NINT ((mbr.min_x - misc.abe_share->open_args[0].head.mbr.min_x) /
               misc.abe_share->open_args[0].head.x_bin_size_degrees)) * misc.abe_share->open_args[0].head.x_bin_size_degrees;

      mbr.max_x = misc.abe_share->open_args[0].head.mbr.min_x +
        (NINT ((mbr.max_x - misc.abe_share->open_args[0].head.mbr.min_x) /
               misc.abe_share->open_args[0].head.x_bin_size_degrees)) * misc.abe_share->open_args[0].head.x_bin_size_degrees;


      double x, y, min_val;
      NV_F64_COORD2 min_bin, mid;
      BIN_RECORD bin;
      DEPTH_RECORD *dep;
      int32_t numrecs;

      misc.add_feature_index = -1;

      min_bin.x = -999.0;
      min_bin.y = -999.0;
      min_val = 9999999.0;


      //  Search the 0 layer PFM for the minimum depth.

      for (y = mbr.min_y ; y < mbr.max_y ; y += misc.abe_share->open_args[0].head.y_bin_size_degrees)
        {
          mid.y = y + misc.abe_share->open_args[0].head.y_bin_size_degrees / 2.0;

          for (x = mbr.min_x ; x < mbr.max_x ; x += misc.abe_share->open_args[0].head.x_bin_size_degrees)
            {
              mid.x = x + misc.abe_share->open_args[0].head.x_bin_size_degrees / 2.0;

              read_bin_record_xy (misc.pfm_handle[0], mid, &bin);

              if (bin.validity & PFM_DATA)
                {
                  if (bin.min_filtered_depth < min_val)
                    {
                      min_val = (double) bin.min_filtered_depth;
                      min_bin = mid;
                    }
                }
            }
        }


      //  If we found a valid bin value, create the feature and add it to the feature file.

      if (min_bin.x > -999.0)
        {
          read_depth_array_xy (misc.pfm_handle[0], min_bin, &dep, &numrecs);

          for (int32_t i = 0 ; i < numrecs ; i++)
            {
              if (!(dep[i].validity & (PFM_INVAL | PFM_DELETED)) && fabs (dep[i].xyz.z - min_val) < 0.0005)
                {
                  misc.add_feature_coord = dep[i].coord;
                  misc.add_feature_index = i;

                  memset (&misc.new_record, 0, sizeof (BFDATA_RECORD));

                  get_feature_event_time (misc.pfm_handle[0], dep[i], &misc.new_record.event_tv_sec, &misc.new_record.event_tv_nsec);

                  misc.new_record.record_number = misc.bfd_header.number_of_records;
                  misc.new_record.length = 0.0;
                  misc.new_record.width = 0.0;
                  misc.new_record.height = 0.0;
                  misc.new_record.confidence_level = 3;
                  misc.new_record.depth = (float) dep[i].xyz.z;
                  misc.new_record.horizontal_orientation = 0.0;
                  misc.new_record.vertical_orientation = 0.0;
                  strcpy (misc.new_record.description, "");
                  strcpy (misc.new_record.remarks, "");
                  misc.new_record.latitude = dep[i].xyz.y;
                  misc.new_record.longitude = dep[i].xyz.x;
                  strcpy (misc.new_record.analyst_activity, "");
                  misc.new_record.equip_type = 3;
                  misc.new_record.nav_system = 1;
                  misc.new_record.platform_type = 4;
                  misc.new_record.sonar_type = 3;

                  misc.new_record.poly_count = 0;

                  editFeatureNum (-1);

                  break;
                }
            }

          free (dep);
        }


      //  We didn't find a valid depth so we want to add the feature using the cursor position.

      else
        {
          misc.add_feature_coord.x = -1;
          misc.add_feature_coord.y = -1;
          misc.add_feature_index = -1;

          memset (&misc.new_record, 0, sizeof (BFDATA_RECORD));

          misc.new_record.record_number = misc.bfd_header.number_of_records;
          misc.new_record.event_tv_sec = 0;
          misc.new_record.event_tv_nsec = 0;
          misc.new_record.length = 0.0;
          misc.new_record.width = 0.0;
          misc.new_record.height = 0.0;
          misc.new_record.confidence_level = 3;
          misc.new_record.depth = 0.0;
          misc.new_record.horizontal_orientation = 0.0;
          misc.new_record.vertical_orientation = 0.0;
          strcpy (misc.new_record.description, "");
          strcpy (misc.new_record.remarks, "");
          misc.new_record.latitude = lat;
          misc.new_record.longitude = lon;
          strcpy (misc.new_record.analyst_activity, "");
          misc.new_record.equip_type = 3;
          misc.new_record.nav_system = 1;
          misc.new_record.platform_type = 4;
          misc.new_record.sonar_type = 3;

          misc.new_record.poly_count = 0;

          editFeatureNum (-1);
        }

      break;


    case DELETE_FEATURE:
      if (misc.nearest_feature != -1)
        {
          //  Delete feature at nearest_feature

          BFDATA_RECORD bfd_record;
          if (binaryFeatureData_read_record (misc.bfd_handle, misc.nearest_feature, &bfd_record) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfmView Delete Feature"), tr ("Unable to read feature record\nReason: %1").arg (msg));
              break;
            }


          //  Zero out the confidence value

          bfd_record.confidence_level = misc.feature[misc.nearest_feature].confidence_level = 0;


          if (binaryFeatureData_write_record (misc.bfd_handle, misc.nearest_feature, &bfd_record, NULL, NULL) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, tr ("pfmView Delete Feature"), tr ("Unable to update feature record\nReason: %1").arg (msg));
              break;
            }


          //  Find the marked point in the PFM data and try to unset the PFM_SELECTED_FEATURE/PFM_DESIGNATED_SOUNDING flag.

          mark_feature (&misc, &bfd_record);


          redrawMap (NVTrue, NVFalse);


          misc.cov_clear = NVTrue;
          cov->redrawMap (NVTrue);


          //  If the mosaic viewer or 3D viewer was running, tell it to redraw.

          misc.abe_share->key = FEATURE_FILE_MODIFIED;
        }
      break;


    case GOOGLE_EARTH:

      //  Save the cursor position and position Google Earth.

      googleEarthPushpin.x = lon;
      googleEarthPushpin.y = lat;
      positionGoogleEarth ();

      redrawMap (NVTrue, NVFalse);


      //  Reset to the last used edit function.

      slotEditMode (misc.last_edit_function);

      break;
    }
}



//!  Middle mouse button press.  Called from the map mouse press callback.  This is usually a discard operation.

void 
pfmView::midMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)), double lon, double lat)
{
  void highlight_contour (MISC *misc, OPTIONS *options, double lat, double lon, nvMap *map);


  //  Actions based on the active function

  switch (misc.function)
    {
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case ZOOM_IN_AREA:
    case REMISP_AREA:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case DEFINE_RECT_AREA:
      map->discardRubberbandRectangle (&rb_rectangle);
      break;

    case POLY_EDIT_AREA_3D:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      map->discardRubberbandPolygon (&rb_polygon);
      prev_poly_lon = -181.0;
      break;

    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
      map->discardRubberbandPolygon (&rb_polygon);
      prev_poly_lon = -181.0;
      misc.feature_polygon_flag = -1;
      break;

    case OUTPUT_POINTS:
      misc.output_points.clear ();
      misc.output_point_count = 0;
      redrawMap (NVTrue, NVFalse);
      break;


      //  For these we want to load the nearest contour value as the drawn contour value.

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->discardRubberbandPolygon (&rb_polygon);
        }
      else
        {
          //  Grab the nearest cell and highlight the contours.

          highlight_contour (&misc, &options, lat, lon, map);


          QString msc = tr (" Draw contour - %L1 ").arg (misc.draw_contour_level, 0, 'f', 2);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);
        }
      prev_poly_lon = -181.0;
    }

  if (misc.function != ADD_FEATURE && misc.function != EDIT_FEATURE && misc.function != DELETE_FEATURE)
    {
      misc.function = misc.save_function;

      setWidgetStates (NVTrue);
      setFunctionCursor (misc.function);

      map->setToolTip ("");      
    }
}



//!  If we errored out of the pfmEdit3D process...

void 
pfmView::slotEditError (QProcess::ProcessError error)
{
  //  If we manually stopped it we don't want a message.

  if (pfmEdit_stopped)
    {
      pfmEdit_stopped = NVFalse;
      return;
    }


  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("Unable to start the edit process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("There was a write error to the edit process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("There was a read error from the edit process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView edit process"), tr ("The edit process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the edit QProcess (when finished normally)

void 
pfmView::slotEditDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  uint8_t force = NVTrue;


  //  If pfm3D is up we need to tell it to reload it's memory now that we are finished with the 
  //  3D editor.  The other end of this (i.e. setting PFMEDIT3D_OPENED) happens in pfmEdit3D.

  if (threeD_edit)
    {
      //  I put brackets inside the lock/unlock for clarity.

      misc.abeShare->lock ();
      {
        misc.abe_share->key = PFMEDIT3D_CLOSED;
      }
      misc.abeShare->unlock ();

      force = NVFalse;
      threeD_edit = NVFalse;
    }


  //  If we canceled drawing, bail out.

  if (misc.drawing_canceled)
    {
      misc.drawing_canceled = NVFalse;
      return;
    }


  pfm_edit_active = NVFalse;


  uint8_t areaMoved = NVFalse;
  uint8_t areaRegen = NVFalse;


  //  If the return from pfmEdit3D is greater than or equal to 100 we asked for a move in pfmEdit3D.  If it's greater than 200 we
  //  asked for a regen (czmilReprocess causes this to be done).  In either case we will always redraw all of the display and
  //  the coverage map.

  if (pfmEditMod >= 100)
    {
      if (pfmEditMod >= 200)
        {
          pfmEditMod -= 200;
          areaRegen = NVTrue;
        }
      else
        {
          pfmEditMod -= 100;
          areaMoved = NVTrue;
        }
    }


  uint8_t feature_change = NVFalse;


  //  If anything changed, get the features (if they exist).

  if (pfmEditMod)
    {
      //  Get the feature file name in case we created one in the editor.

      get_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);


      if (strcmp (misc.abe_share->open_args[0].target_path, "NONE"))
        {
          if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
          misc.bfd_open = NVFalse;

          if ((misc.bfd_handle = bfd_check_file (&misc, misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
            {
              if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, "pfmView", tr ("Unable to read feature records\nReason: %1").arg (msg));
                  binaryFeatureData_close_file (misc.bfd_handle);
                }
              else
                {
                  misc.bfd_open = NVTrue;
                  feature_change = NVTrue;
                }
            }
        }
    }


  //  If we changed the PFM structure in the edit and the average filtered surface type is a misp surface we need to 
  //  remisp the edited area (surface type checking will be done in remisp).

  NV_F64_XYMBR mbr = misc.abe_share->edit_area;

  if (pfmEditMod == 1) remisp (&misc, &options, &mbr);


  //  If we changed the PFM structure in the edit and the average filtered surface type is a CUBE surface we need to 
  //  re-CUBE the edited area.

  if (pfmEditMod == 1 && misc.abe_share->pfm_count == 1 && misc.cube_available && misc.cube_attr_available[0] &&
      !misc.abe_share->read_only && misc.average_type[0] == -1) runCube (NVFalse);


  //  If we requested a move from within the editor...

  if (areaMoved || areaRegen)
    {
      //  Save the bounds that were passed back from pfmEdit3D since redrawMap or moveMap will reset these.

      NV_F64_XYMBR orig_bounds = misc.abe_share->edit_area;


      //  If the settings changed, grab the new ones.

      if (misc.abe_share->settings_changed)
        {
          misc.abe_share->settings_changed = NVFalse;
          options.smoothing_factor = misc.abe_share->smoothing_factor;
          options.z_factor = misc.abe_share->z_factor;
          options.z_offset = misc.abe_share->z_offset;
          options.position_form = misc.abe_share->position_form;


          options.mosaic_prog = QString (misc.abe_share->mosaic_prog);
          options.mosaic_hotkey = QString (misc.abe_share->mosaic_hotkey);
          options.mosaic_actkey = QString (misc.abe_share->mosaic_actkey);
          startMosaic->setShortcut (options.mosaic_hotkey);
        }


      //  Only redraw if we've moved outside the displayed area or if we actually changed something.

      uint8_t changed = NVFalse;
      if (orig_bounds.min_x < misc.total_displayed_area.min_x)
        {
          moveMap (NVMAP_LEFT);
          changed = NVTrue;
        }
      else if (orig_bounds.max_x > misc.total_displayed_area.max_x)
        {
          moveMap (NVMAP_RIGHT);
          changed = NVTrue;
        }
      else if (orig_bounds.min_y < misc.total_displayed_area.min_y)
        {
          moveMap (NVMAP_DOWN);
          changed = NVTrue;
        }
      else if (orig_bounds.max_y > misc.total_displayed_area.max_y)
        {
          moveMap (NVMAP_UP);
          changed = NVTrue;
        }
      else
        {
          if (options.auto_redraw)
            {
              discardMovableObjects ();

              if (pfmEditMod)
                {
                  redrawMap (NVTrue, NVTrue);
                  changed = NVTrue;
                }
            }
          else
            {
              int32_t count, *px, *py;
              double *mx, *my;

              if (!force_3d_edit && misc.function == RECT_EDIT_AREA_3D)
                {
                  //  If the rubberband rectangle isn't present (ID is -1) we must have done a pfmEdit3D move prior to this
                  //  in which case we are using a moving rectangle instead.

                  if (rb_rectangle >= 0)
                    {
                      map->getRubberbandRectangle (rb_rectangle, &px, &py, &mx, &my);
                      map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                      map->discardRubberbandRectangle (&rb_rectangle);
                    }
                  else
                    {
                      map->getMovingRectangle (mv_rectangle, &px, &py, &mx, &my);
                      map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                      map->closeMovingRectangle (&mv_rectangle);
                    }
                }
              else
                {
                  map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);
                  map->drawPolygon (count, px, py, options.contour_color, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
                  map->discardRubberbandPolygon (&rb_polygon);
                }
            }
        }


      if (changed || areaRegen)
        {
          cov->redrawMap (NVTrue);
          redrawCovBounds ();
        }


      if (areaRegen)
        {
          if (save_poly_count)
            {
              map->setMovingPolygon (&mv_polygon, save_poly_count, save_poly_x, save_poly_y, options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
            }
          else
            {
              map->setMovingPolygon (&mv_polygon, 4, save_poly_x, save_poly_y, options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
            }


          // Set the function and the buttons to rectangle edit mode.

          setWidgetStates (NVFalse);


          //  Kick the editor off again with the same area.

          editCommand (save_poly_x, save_poly_y, save_poly_count);
        }
      else
        {
          map->setMovingRectangle (&mv_rectangle, orig_bounds.min_x, orig_bounds.min_y, orig_bounds.max_x, orig_bounds.max_y,
                                   options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);

          double mx[4], my[4];
          mx[0] = orig_bounds.min_x;
          my[0] = orig_bounds.min_y;
          mx[1] = mx[0];
          my[1] = orig_bounds.max_y;
          mx[2] = orig_bounds.max_x;
          my[2] = my[1];
          mx[3] = mx[2];
          my[3] = my[0];


          // Set the function and the buttons to rectangle edit mode.

          setWidgetStates (NVFalse);


          //  Kick the editor off again with the new area.

          editCommand (mx, my, 4);
        }


      return;
    }
  else
    {
      //  If we filtered the area in the editor we need to "automagically" filter mask the area.

      if (pfmEditFilt)
        {
          try
            {
              misc.poly_filter_mask.resize (misc.poly_filter_mask_count + 1);
            }
          catch (std::bad_alloc&)
            {
              fprintf (stderr, "%s %s %s %d - poly_filter_mask - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
              exit (-1);
            }


          //  Polygon.

          if (misc.abe_share->polygon_count)
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].count = misc.abe_share->polygon_count;

              for (int32_t i = 0 ; i < misc.abe_share->polygon_count ; i++)
                {
                  misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = misc.abe_share->polygon_x[i];
                  misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = misc.abe_share->polygon_y[i];
                }
            }


          //  Rectangle.

          else
            {
              misc.poly_filter_mask[misc.poly_filter_mask_count].count = 4;

              misc.poly_filter_mask[misc.poly_filter_mask_count].x[0] = misc.abe_share->edit_area.min_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[0] = misc.abe_share->edit_area.min_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[1] = misc.abe_share->edit_area.min_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[1] = misc.abe_share->edit_area.max_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[2] = misc.abe_share->edit_area.max_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[2] = misc.abe_share->edit_area.max_y;
              misc.poly_filter_mask[misc.poly_filter_mask_count].x[3] = misc.abe_share->edit_area.max_x;
              misc.poly_filter_mask[misc.poly_filter_mask_count].y[3] = misc.abe_share->edit_area.min_y;
            }
          misc.poly_filter_mask[misc.poly_filter_mask_count].displayed = NVFalse;

          misc.poly_filter_mask_count++;
        }


      //  Auto redraw.

      if (options.auto_redraw)
        {
          //  Just in case we did a pfmEdit3D move prior to this we'll clear the moving rectangle that we used to display the
          //  moved area.

          map->closeMovingRectangle (&mv_rectangle);


          if (!force_3d_edit && misc.function == RECT_EDIT_AREA_3D)
            {
              map->discardRubberbandRectangle (&rb_rectangle);
            }
          else
            {
              map->closeMovingPolygon (&pfm3D_polygon);
              map->discardRubberbandPolygon (&rb_polygon);
            }

          if (pfmEditMod)
            {
              int32_t old_count = misc.bfd_header.number_of_records;


              //  If we didn't have any features and suddenly we have some we need to redraw the entire
              //  map and coverage since the user may have done an add_feature and used an existing
              //  feature file.  We also want to redraw the entire thing if we're displaying contours or a grid overlay since it's
              //  almost impossible to merge the contours or grid back in correctly.

              if ((!old_count && misc.bfd_header.number_of_records) || options.contour || options.overlay_grid)
                {
                  feature_change = NVFalse;

                  redrawMap (NVTrue, NVTrue);

                  misc.cov_clear = NVTrue;
                  cov->redrawMap (NVTrue);
                }
              else
                {
                  //  We want to just redraw the area that was edited.  This is why we don't just 
                  //  call map->redrawMap ().

                  misc.clear = NVFalse;
                  slotPreRedraw (map->getMapdef ());


                  //  Make sure we turn mouse signals back on.  This is normally done in the postRedraw slot.

                  map->enableMouseSignals ();


                  misc.clear = NVTrue;


                  misc.cov_clear = NVFalse;
                  cov->redrawMap (NVFalse);


                  //  Make sure we kick pfm3D if it's up.

                  if (threeDProc)
                    {
                      if (threeDProc->state () == QProcess::Running && force) misc.abe_share->key = PFM3D_FORCE_RELOAD;
                    }
                }
            }
        }


      //  No auto redraw.

      else
        {
          int32_t count, *px, *py;
          double *mx, *my;

          if (!force_3d_edit && misc.function == RECT_EDIT_AREA_3D)
            {
              //  If the rubberband rectangle isn't present (ID is -1) we must have done a pfmEdit3D move prior to this
              //  in which case we are using a moving rectangle instead.

              if (rb_rectangle >= 0)
                {
                  map->getRubberbandRectangle (rb_rectangle, &px, &py, &mx, &my);
                  map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                  map->discardRubberbandRectangle (&rb_rectangle);
                }
              else
                {
                  map->getMovingRectangle (mv_rectangle, &px, &py, &mx, &my);
                  map->drawRectangle (px[0], py[0], px[2], py[2], options.contour_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
                  map->closeMovingRectangle (&mv_rectangle);
                }
            }
          else
            {
              map->getRubberbandPolygon (rb_polygon, &count, &px, &py, &mx, &my);
              map->drawPolygon (count, px, py, options.contour_color, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
              map->discardRubberbandPolygon (&rb_polygon);
            }
        }


      //  If we filtered but aren't in auto redraw mode we must still paint the new filter masked area (in semi transparent mode).

      if (pfmEditFilt && !misc.abe_share->settings_changed)
        {
          int32_t fm = misc.poly_filter_mask_count - 1;
          QColor fmc = options.poly_filter_mask_color;

          if (fmc.alpha () == 255) fmc.setAlpha (96);

          map->fillPolygon (misc.poly_filter_mask[fm].count, misc.poly_filter_mask[fm].x, misc.poly_filter_mask[fm].y, fmc, NVTrue);
          misc.poly_filter_mask[fm].displayed = NVTrue;
        }
    }


  //  Make sure we reset the important things in case we did a partial redraw.

  setFunctionCursor (misc.function);


  //  If the user changed settings in pfmEdit3D let's redraw.

  if (misc.abe_share->settings_changed)
    {
      misc.abe_share->settings_changed = NVFalse;
      options.smoothing_factor = misc.abe_share->smoothing_factor;
      options.z_factor = misc.abe_share->z_factor;
      options.z_offset = misc.abe_share->z_offset;
      options.position_form = misc.abe_share->position_form;
      options.min_hsv_color[0] = misc.abe_share->min_hsv_color;
      options.max_hsv_color[0] = misc.abe_share->max_hsv_color;
      options.min_hsv_value[0] = misc.abe_share->min_hsv_value;
      options.max_hsv_value[0] = misc.abe_share->max_hsv_value;
      options.min_hsv_locked[0] = misc.abe_share->min_hsv_locked;
      options.max_hsv_locked[0] = misc.abe_share->max_hsv_locked;

      options.mosaic_prog = QString (misc.abe_share->mosaic_prog);
      options.mosaic_hotkey = QString (misc.abe_share->mosaic_hotkey);
      options.mosaic_actkey = QString (misc.abe_share->mosaic_actkey);
      startMosaic->setShortcut (options.mosaic_hotkey);


      feature_change = NVFalse;


      //  This gets the Z orientation if it was changed in pfmEdit3D.

#ifdef NVWIN3X
      QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
      QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

      QSettings settings2 (ini_file2, QSettings::IniFormat);
      settings2.beginGroup ("globalABE");


      options.z_orientation = settings2.value (pfmView::tr ("ABE Z orientation factor"), options.z_orientation).toFloat ();


      settings2.endGroup ();


      redrawMap (NVTrue, NVFalse);
    }


  if (options.display_feature && feature_change)
    {
      overlayFlag (map, &options, &misc, NVTrue, NVTrue, NVTrue);
      misc.cov_clear = NVTrue;
      cov->redrawMap (NVTrue);
    }


  setWidgetStates (NVTrue);


  //  No matter what, we need to set the displayed bounds back to the pfmView displayed bounds
  //  when we exit pfmEdit3D.

  misc.abe_share->displayed_area = misc.abe_share->viewer_displayed_area = misc.total_displayed_area;


  misc.drawing = NVFalse;
}



//!  This is the stderr read return from the edit QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotEditReadyReadStandardError ()
{
  readStandardError (editProc);
}



/*!
  This is the stdout read return from the edit QProcess
  We need to look at the value returned on exit to decide if we need to redraw the map.
  pfmEdit3D will print out a 0 for no changes, a 1 for PFM structure changes, or a 2 for feature changes.
  The second number is the filter mask flag.  If it's set we want to filter mask the area.
*/

void 
pfmView::slotEditReadyReadStandardOutput ()
{
  QByteArray response = editProc->readAllStandardOutput ();
  char *res = response.data ();


  //  Only those messages that begin with "Edit return status:" are valid.  The rest may be error messages.

  if (!strncmp (res, "Edit return status:", 19))
    {
      sscanf (res, "Edit return status:%d,%d", &pfmEditMod, &pfmEditFilt);
    }
  else
    {
      //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
      //  killing ancillary programs in the editor.

      if (!strstr (res, "Destroyed while"))
        {
          fprintf (stdout, "%s %s %d %s\n", __FILE__,  __FUNCTION__, __LINE__, res);
          fflush (stdout);
        }
    }
}



//!  Kick off the edit QProcess

void 
pfmView::editCommand (double *mx, double *my, int32_t count)
{
  //  Only do the following if we don't already have an edit window opened

  if (!pfm_edit_active)
    {
      //  Just in case we run czmilReprocess from inside pfmEdit3D we need to save the bounds we started with so that we
      //  can restart the editor with the exact same bounds.

      save_poly_count = count;
      for (int32_t i = 0 ; i < count ; i++)
        {
          save_poly_x[i] = mx[i];
          save_poly_y[i] = my[i];
        }


      pfmEditMod = 0;
      pfmEditFilt = 0;
      pfm_edit_active = NVTrue;


      //  Compute the minimum bounding rectangle for the edit area.

      double min_x = 999.0;
      double max_x = -999.0;
      double min_y = 999.0;
      double max_y = -999.0;
      for (int32_t i = 0 ; i < count ; i++)
        {
          if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

          misc.abe_share->polygon_x[i] = mx[i];
          misc.abe_share->polygon_y[i] = my[i];
          if (mx[i] < min_x) min_x = mx[i];
          if (mx[i] > max_x) max_x = mx[i];
          if (my[i] < min_y) min_y = my[i];
          if (my[i] > max_y) max_y = my[i];
        }


      //  Adjust to displayed area bounds

      if (min_y < misc.total_displayed_area.min_y) min_y = misc.total_displayed_area.min_y;
      if (max_y > misc.total_displayed_area.max_y) max_y = misc.total_displayed_area.max_y;
      if (min_x < misc.total_displayed_area.min_x) min_x = misc.total_displayed_area.min_x;
      if (max_x > misc.total_displayed_area.max_x) max_x = misc.total_displayed_area.max_x;


      misc.abe_share->edit_area.min_x = min_x;
      misc.abe_share->edit_area.max_x = max_x;
      misc.abe_share->edit_area.min_y = min_y;
      misc.abe_share->edit_area.max_y = max_y;

      map->setCursor (Qt::WaitCursor);


      //  For rectangles we pass a count of zero to tell pfmEdit3D that it's a rectangle.  All forced edits from pfm3D are
      //  polygons.

      if (!force_3d_edit && misc.function == RECT_EDIT_AREA_3D)
        {
          misc.abe_share->polygon_count = 0;
        }
      else
        {
          misc.abe_share->polygon_count = count;
        }


      editProc = new QProcess (this);


      QStringList arguments;
      QString arg;


      //  If we are doing a Z value conversion...

      misc.abe_share->z_factor = options.z_factor;


      //  If we are doing a Z value offset...

      misc.abe_share->z_offset = options.z_offset;


      //  Always add the shared memory ID (the process ID).

      arg.sprintf ("--shared_memory_key=%d", misc.abe_share->ppid);
      arguments += arg;


      //  If we want to run in "no invalid" mode we need to add the -n flag.

      if (options.editor_no_load[0])
        {
          arg.sprintf ("-n");
          arguments += arg;
        }


      //  If we want to run in "no reference" mode we need to add the -r flag.

      if (options.editor_no_load[1])
        {
          arg.sprintf ("-r");
          arguments += arg;
        }


      connect (editProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotEditDone (int, QProcess::ExitStatus)));
      connect (editProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotEditReadyReadStandardError ()));
      connect (editProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotEditReadyReadStandardOutput ()));
      connect (editProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotEditError (QProcess::ProcessError)));

      setWidgetStates (NVFalse);
      qApp->processEvents ();


      threeD_edit = NVTrue;

      editProc->start (QString (options.edit_name_3D), arguments);
    }
}



//!  Mouse double click.  Called from the map double click callback.  This is usually a completion operation.

void 
pfmView::slotMouseDoubleClick (QMouseEvent *e, double lon, double lat)
{
  BIN_RECORD          bin;
  int32_t             count, *px, *py, ndx;
  double              *mx, *my;
  std::vector<double> nmx, nmy, nmz, nmm;
  QFileDialog         *fd;
  QString             file, string;
  FILE                *fp;
  char                fname[512], ltstring[25], lnstring[25], hem, shape_name[512], prj_file[512];;
  double              deg, min, sec;
  QStringList         filters;
  QStringList         files;
  static QDir         dir = QDir (".");
  uint8_t             hit = NVFalse;
  SHPHandle           shp_hnd;
  SHPObject           *shape;
  DBFHandle           dbf_hnd;  



  void filterPolyArea (OPTIONS *, MISC *, double *, double *, int32_t);
  void filterUndo (MISC *misc);


  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will flip to NVTrue;

  double_click = !double_click;


  hit = NVFalse;


  //  Actions based on the active function

  switch (misc.function)
    {
    case DEFINE_POLY_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      if (double_click) count--;


      if (!QDir (options.output_area_dir).exists ()) options.output_area_dir = options.input_pfm_dir;


      fd = new QFileDialog (this, tr ("pfmView Output Area File"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.output_area_dir);


      filters << tr ("Generic area file (*.are)")
              << tr ("Army Corps area file (*.afs)")
              << tr ("ESRI Shapefile (*.shp)");

      fd->setNameFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectNameFilter (options.output_area_name_filter);


      if (fd->exec () == QDialog::Accepted)
        {
          files = fd->selectedFiles ();

          file = files.at (0);


          if (!file.isEmpty())
            {

              //  Add extension to filename if not there.
            
              if (fd->selectedNameFilter ().contains ("*.are"))
                {
                  if (!file.endsWith (".are")) file.append (".are");
                }
              else if (fd->selectedNameFilter ().contains ("*.afs"))
                {
                  if (!file.endsWith (".afs")) file.append (".afs");
                }
              else if (fd->selectedNameFilter ().contains ("*.shp"))
                {
                  if (!file.endsWith (".shp")) file.append (".shp");
                }

 
              strcpy (fname, file.toLatin1 ());


              if ((fp = fopen (fname, "w")) != NULL)
                {
                  if (file.endsWith (".are"))
                    {
                      for (int32_t i = 0 ; i < count ; i++)
                        {
                          strcpy (ltstring, fixpos (my[i], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
                          strcpy (lnstring, fixpos (mx[i], &deg, &min, &sec, &hem, POS_LON, options.position_form));

                          fprintf (fp, "%s, %s\n", ltstring, lnstring);
                        }


                      //  Always "close" the polygon by duplicating the first point.

                      strcpy (ltstring, fixpos (my[0], &deg, &min, &sec, &hem, POS_LAT, options.position_form));
                      strcpy (lnstring, fixpos (mx[0], &deg, &min, &sec, &hem, POS_LON, options.position_form));

                      fprintf (fp, "%s, %s\n", ltstring, lnstring);

                      fclose (fp);
                    }
                  else if (file.endsWith (".afs"))
                    {
                      for (int32_t i = 0 ; i < count ; i++)
                        {
                          //  Make sure we haven't created any duplicate points

                          if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;

                          fprintf (fp, "%.9f, %.9f\n", mx[i], my[i]);
                        }


                      //  Always "close" the polygon by duplicating the first point.

                      fprintf (fp, "%.9f, %.9f\n", mx[0], my[0]);

                      fclose (fp);
                    }
                  else
                    {
                      strcpy (shape_name, fname);
                      shape_name[strlen (shape_name) - 4] = 0;
                      
                      if ((shp_hnd = SHPCreate (shape_name, SHPT_POLYGON)) == NULL)
                        {
                          QMessageBox::warning (this, "pfmView", tr ("Error creating shp file."));
                          break;
                        }


                      if ((dbf_hnd = DBFCreate (shape_name)) == NULL)
                        {
                          QMessageBox::warning (this, "pfmView", tr ("Error creating dbf file."));
                          break;
                        }


                      /*  Make attributes*/

                      if (DBFAddField (dbf_hnd, "file", FTString, 15, 0 ) == -1)
                        {
                          QMessageBox::warning (this, "pfmView", tr ("Error adding field to dbf file."));
                          break;
                        }


                      //  Stupid freaking .prj file

                      strcpy (prj_file, shape_name);
                      strcat (prj_file, ".prj");

                      if ((fp = fopen (prj_file, "w")) == NULL)
                        {
                          QMessageBox::warning (this, "pfmView", tr ("Error creating prj file."));
                          break;
                        }

                      fprintf (fp, "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]\n");
                      fclose (fp);


                      int32_t new_count = 0;
                      for (int32_t i = 0 ; i <= count ; i++)
                        {
                          //  Don't check this when we're adding the duplicate first point on the end of the array.

                          if (i < count)
                            {
                              //  Make sure we haven't created any duplicate points

                              if (i && mx[i] == mx[i - 1] && my[i] == my[i -1]) continue;
                            }


                          //  Resize the memory then increment the new count.

                          try
                            {
                              nmx.resize (new_count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - nmx - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              nmy.resize (new_count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - nmy - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              nmz.resize (new_count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - nmz - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              nmm.resize (new_count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - nmm - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                              exit (-1);
                            }


                          //  Always "close" the polygon by duplicating the first point.

                          if (i == count)
                            {
                              nmx[new_count] = mx[0];
                              nmy[new_count] = my[0];
                            }
                          else
                            {
                              nmx[new_count] = mx[i];
                              nmy[new_count] = my[i];
                            }

                          nmz[new_count] = 0.0;
                          nmm[new_count] = 0.0;

                          new_count++;
                        }


                      shape = SHPCreateObject (SHPT_POLYGON, -1, 0, NULL, NULL, new_count, nmx.data (), nmy.data (), nmz.data (), nmm.data ());
                      SHPWriteObject (shp_hnd, -1, shape);
                      SHPDestroyObject (shape);
                      DBFWriteStringAttribute (dbf_hnd, 0, 0, shape_name);

                      SHPClose (shp_hnd);
                      DBFClose (dbf_hnd);


                      //  Clear the vectors.

                      nmx.clear ();
                      nmy.clear ();
                      nmz.clear ();
                      nmm.clear ();
                    }


                  //  Add the new file to the first available overlay slot.

                  for (int32_t i = 0 ; i < NUM_OVERLAYS ; i++)
                    {
                      if (!misc.overlays[i].file_type)
                        {
                          if (file.endsWith (".are"))
                            {
                              misc.overlays[i].file_type = GENERIC_AREA;
                            }
                          else if (file.endsWith (".afs"))
                            {
                              misc.overlays[i].file_type = ACE_AREA;
                            }
                          else
                            {
                              misc.overlays[i].file_type = SHAPE;
                            }
                          strcpy (misc.overlays[i].filename, file.toLatin1 ());
                          misc.overlays[i].display = NVTrue;
                          misc.overlays[i].color = options.contour_color;

                          hit = NVTrue;

                          break;
                        }
                    }
                }

              options.output_area_name_filter = fd->selectedNameFilter ();
            }
          options.output_area_dir = fd->directory ().absolutePath ();
        }

      map->discardRubberbandPolygon (&rb_polygon);

      map->setToolTip ("");      
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      if (hit) redrawMap (NVTrue, NVFalse);
      break;


    case DEFINE_FEATURE_POLY_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      for (int32_t i = 0 ; i < count ; i++)
        {
          misc.polygon_x[i] = mx[i];
          misc.polygon_y[i] = my[i];
        }
      misc.poly_count = count;

      map->discardRubberbandPolygon (&rb_polygon);

      map->setToolTip ("");
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      misc.feature_polygon_flag = 1;
      redrawMap (NVTrue, NVFalse);
      break;


    case POLY_FILTER_AREA:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      misc.filtered_count = 0;

      filterPolyArea (&options, &misc, mx, my, count);

      redrawMap (NVTrue, NVTrue);

      if (misc.filtered_count)
        {
          QMessageBox msgBox;
          msgBox.setWindowTitle (tr ("pfmView Filter"));
          msgBox.setText (tr ("%1 points marked invalid").arg (misc.filtered_count));
          msgBox.setInformativeText ("Do you want to save your changes?");
          msgBox.setStandardButtons (QMessageBox::Save | QMessageBox::Discard);
          msgBox.setDefaultButton (QMessageBox::Save);


          //  This is weird.  If I let the message box be modal, nothing else happens but it greys out the window that you
          //  need to look in order to verify that you want to accept the changes.  If I make it non-modal it will leave the 
          //  window alone but you can start clicking buttons and other bad stuff.  So, to make it sort of work, I'm setting
          //  the misc.drawing flag so that everything else will be ignored until you answer the massage box.  I also have
          //  to use the "Qt::WindowStaysOnTopHint" to keep it from disappearing into the background if you click on the main
          //  window.

          misc.drawing = NVTrue;
          setWidgetStates (NVFalse);

          msgBox.setWindowFlags (msgBox.windowFlags () | Qt::WindowStaysOnTopHint);
          msgBox.setModal (false);

          msgBox.show ();
          QPoint newpos = mapToGlobal (statbook->pos ());
          msgBox.move (newpos);

          int ret = msgBox.exec ();

          misc.drawing = NVFalse;
          setWidgetStates (NVTrue);


          if (ret == QMessageBox::Discard)
            {
              filterUndo (&misc);
              redrawMap (NVTrue, NVTrue);
            }


          //  Clear the undo memory;

          if (misc.filtered_count)
            {
              misc.undo.clear ();
              misc.filtered_count = 0;
            }
        }

      break;


    case POLY_FILTER_MASK:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      try
        {
          misc.poly_filter_mask.resize (misc.poly_filter_mask_count + 1);
        }
      catch (std::bad_alloc&)
        {
          fprintf (stderr, "%s %s %s %d - poly_filter_mask - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }

      misc.poly_filter_mask[misc.poly_filter_mask_count].count = count;

      for (int32_t i = 0 ; i < count ; i++)
        {
          misc.poly_filter_mask[misc.poly_filter_mask_count].x[i] = mx[i];
          misc.poly_filter_mask[misc.poly_filter_mask_count].y[i] = my[i];
        }


      map->discardRubberbandPolygon (&rb_polygon);


      ndx = misc.poly_filter_mask_count;
      if (options.poly_filter_mask_color.alpha () < 255)
        {
          map->fillPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y, 
                            options.poly_filter_mask_color, NVTrue);
        }
      else
        {
          //  We don't have to worry about clipping this because moving the area discards the mask areas.

          map->drawPolygon (misc.poly_filter_mask[ndx].count, misc.poly_filter_mask[ndx].x, misc.poly_filter_mask[ndx].y,
                            options.poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
        }
      misc.poly_filter_mask[ndx].displayed = NVTrue;

      misc.poly_filter_mask_count++;
      bClearMasks->setEnabled (true);
      break;


    case POLY_EDIT_AREA_3D:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);
      editCommand (mx, my, count);
      break;


    case GRAB_CONTOUR:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      for (int32_t i = 0 ; i < count ; i++)
        {
          misc.polygon_x[i] = mx[i];
          misc.polygon_y[i] = my[i];
        }
      misc.poly_count = count;

      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      redrawMap (NVTrue, NVTrue);

      misc.poly_count = 0;
      break;


    case REMISP_FILTER:
      map->closeRubberbandPolygon (rb_polygon, lon, lat, &count, &px, &py, &mx, &my);

      remispFilter (&misc, &options, mx, my, count);

      redrawMap (NVTrue, NVTrue);

      break;


      //  Output manually selected points to a file.

    case OUTPUT_POINTS:
      try
        {
          misc.output_points.resize (misc.output_point_count + 1);
        }
      catch (std::bad_alloc&)
        {
          fprintf (stderr, "%s %s %s %d - output_points - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }

      misc.output_points[misc.output_point_count].xy.x = lon;
      misc.output_points[misc.output_point_count].xy.y = lat;

      read_bin_record_xy (misc.pfm_handle[0], misc.output_points[misc.output_point_count].xy, &bin);
      misc.output_points[misc.output_point_count].avg = options.z_orientation * bin.avg_filtered_depth * options.z_factor + options.z_offset;
      misc.output_points[misc.output_point_count].min = options.z_orientation * bin.min_filtered_depth * options.z_factor + options.z_offset;
      misc.output_points[misc.output_point_count].max = options.z_orientation * bin.max_filtered_depth * options.z_factor + options.z_offset;

      map->fillCircle (lon, lat, 10, 0.0, 360.0, options.contour_color, NVTrue);

      string = tr (" Point #%1").arg (misc.output_point_count);

      map->drawText (string, lon, lat, options.contour_color, NVTrue);

      misc.output_point_count++;


      if (!QDir (options.output_points_dir).exists ()) options.output_points_dir = options.input_pfm_dir;


      fd = new QFileDialog (this, tr ("pfmView Output Points File"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.output_points_dir);


      filters << tr ("Points (*.pts)");

      fd->setNameFilters (filters);
      fd->setFileMode (QFileDialog::AnyFile);
      fd->selectNameFilter (tr ("Points (*.pts)"));

      if (fd->exec () == QDialog::Accepted)
        {
          files = fd->selectedFiles ();

          file = files.at (0);


          if (!file.isEmpty())
            {
              //  Add .pts to filename if not there.

              if (!file.endsWith (".pts")) file.append (".pts");
 
              strcpy (fname, file.toLatin1 ());


              if ((fp = fopen (fname, "w")) != NULL)
                {
                  for (int32_t i = 0 ; i < misc.output_point_count ; i++)
                    fprintf (fp, "%.9f, %.9f, Point #%d %f %f %f\n", misc.output_points[i].xy.y, misc.output_points[i].xy.x, i, misc.output_points[i].avg, 
                             misc.output_points[i].min, misc.output_points[i].max);

                  fclose (fp);


                  //  Add the new file to the first available overlay slot.

                  for (int32_t i = 0 ; i < NUM_OVERLAYS ; i++)
                    {
                      if (!misc.overlays[i].file_type)
                        {
                          misc.overlays[i].file_type = GENERIC_YXZ;
                          strcpy (misc.overlays[i].filename, file.toLatin1 ());
                          misc.overlays[i].display = NVTrue;
                          misc.overlays[i].color = options.contour_color;

                          hit = NVTrue;

                          break;
                        }
                    }
                }
            }
          options.output_points_dir = fd->directory ().absolutePath ();
        }
      if (e != NULL) midMouse (e->x (), e->y (), lon, lat);

      map->setToolTip ("");      
      misc.function = misc.save_function;

      setFunctionCursor (misc.function);

      if (hit) redrawMap (NVTrue, NVFalse);

      break;
    }
}



//!  Right mouse press.  Called from the map mouse press callback.  This pops up the right mouse click menu.

void 
pfmView::rightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat)
{
  QString tmp;

  menu_cursor_lon = lon;
  menu_cursor_lat = lat;
  menu_cursor_x = mouse_x;
  menu_cursor_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);


  popup4->setVisible (false);


  //  Define the menu entrys based on the active function.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      popup0->setText (tr ("Close rectangle and zoom in"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_RECT_AREA:
      popup0->setText (tr ("Close rectangle and save to file"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_EDIT_AREA_3D:
      popup0->setText (tr ("Close rectangle and edit area"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setText (tr ("Switch to polygon mode"));
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SELECT_HIGH_POINT:
      popup0->setText (tr ("Close rectangle and select highest point"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case SELECT_LOW_POINT:
      popup0->setText (tr ("Close rectangle and select lowest point"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case REMISP_AREA:
      popup0->setText (tr ("Close rectangle and regrid area"));
      tmp = tr ("Set MISP weight factor (%1)").arg (options.misp_weight);
      popup1->setText (tmp);
      if (options.misp_force_original)
        {
          tmp = tr ("Set MISP force original input (Yes)");
        }
      else
        {
          tmp = tr ("Set MISP force original input (No)");
        }
      popup2->setText (tmp);
      popup2->setVisible (true);
      popup3->setText (tr ("Discard rectangle"));
      popup3->setVisible (true);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      popup0->setText (tr ("End contour"));
      tmp = tr ("Set depth for contour (%L1)").arg (misc.draw_contour_level, 0, 'f', 2);
      popup1->setText (tmp);
      popup2->setText (tr ("Grab nearest contour depth for contour"));
      popup3->setText (tr ("Discard contour"));

      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          popup0->setVisible (true);
          popup1->setVisible (false);
          popup2->setVisible (false);
          popup3->setVisible (true);
        }
      else
        {
          popup0->setVisible (false);
          popup1->setVisible (true);
          popup2->setVisible (true);
          popup3->setVisible (false);
        }
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_POLY_AREA:
      popup0->setText (tr ("Close polygon and save to file"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      popup0->setText (tr ("Close polygon and save to feature file"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_EDIT_AREA_3D:
      popup0->setText (tr ("Close polygon and edit area"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setText (tr ("Switch to rectangle mode"));
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case GRAB_CONTOUR:
      popup0->setText (tr ("Close polygon and capture contours"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case REMISP_FILTER:
      popup0->setText (tr ("Close polygon, filter, and remisp the area"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_FILTER_AREA:
      popup0->setText (tr ("Close rectangle and filter area"));
      tmp = tr ("Set filter standard deviation (%L1)").arg (options.filterSTD, 0, 'f', 1);
      popup1->setText (tmp);
      if (options.deep_filter_only)
        {
          tmp = tr ("Set deep filter only (Yes)");
        }
      else
        {
          tmp = tr ("Set deep filter only (No)");
        }
      popup2->setText (tmp);
      popup3->setText (tr ("Discard rectangle"));
      popup2->setVisible (true);
      popup3->setVisible (true);
      popup4->setVisible (true);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_FILTER_AREA:
      popup0->setText (tr ("Close polygon and filter area"));
      tmp = tr ("Set filter standard deviation (%L1)").arg (options.filterSTD, 0, 'f', 1);
      popup1->setText (tmp);
      if (options.deep_filter_only)
        {
          tmp = tr ("Set deep filter only (Yes)");
        }
      else
        {
          tmp = tr ("Set deep filter only (No)");
        }
      popup2->setText (tmp);
      popup3->setText (tr ("Discard polygon"));
      popup2->setVisible (true);
      popup3->setVisible (true);
      popup4->setVisible (true);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case RECT_FILTER_MASK:
      popup0->setText (tr ("Close rectangle and save mask"));
      popup1->setText (tr ("Discard rectangle"));
      popup2->setText (tr ("Clear all masks"));
      popup2->setVisible (true);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case POLY_FILTER_MASK:
      popup0->setText (tr ("Close polygon and save mask"));
      popup1->setText (tr ("Discard polygon"));
      popup2->setText (tr ("Clear all masks"));
      popup2->setVisible (true);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case OUTPUT_POINTS:
      popup0->setText (tr ("Save points to file"));
      popup1->setText (tr ("Discard points"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case DELETE_FEATURE:
      popup0->setText (tr ("Delete feature"));
      popup1->setText (tr ("Cancel"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case EDIT_FEATURE:
      popup0->setText (tr ("Edit feature"));
      popup1->setText (tr ("Highlight features (text search)"));
      popup2->setText (tr ("Cancel"));
      popup2->setVisible (true);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ADD_FEATURE:
      popup0->setText (tr ("Add feature"));
      popup1->setText (tr ("Cancel"));
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case GOOGLE_EARTH:
      popup0->setText (tr ("Set placemark in Google Earth"));
      popup1->setText (tr ("Cancel"));
      popup2->setVisible (false);
      popup3->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;
    }
}



//!  Right click popup menu first entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case DEFINE_RECT_AREA:
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case DELETE_FEATURE:
    case ADD_FEATURE:
    case EDIT_FEATURE:
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case REMISP_AREA:
    case GOOGLE_EARTH:
      leftMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;

    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case OUTPUT_POINTS:
    case REMISP_FILTER:
    case GRAB_CONTOUR:


      //  Fake out slotMouseDoubleClick so that it won't decrement the point count.

      double_click = NVTrue;

      slotMouseDoubleClick (NULL, menu_cursor_lon, menu_cursor_lat);
      break;
    }

}



//!  Right click popup menu second entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu1 ()
{
  popup_active = NVFalse;

  bool ok;
  double res;
  int32_t ires;
  QString text;

  switch (misc.function)
    {
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
      double ret;
      ret = QInputDialog::getDouble (this, "pfmView", tr ("Enter contour level:"), misc.draw_contour_level, -20000.0, 20000.0, 1, &ok);

      if (ok)
        {
          if (ret > misc.abe_share->open_args[0].max_depth || ret < -misc.abe_share->open_args[0].offset)
            {
              QMessageBox::warning (this, "pfmView", tr ("Contour value entered is out of range for this PFM structure."));
            }
          else
            {
              misc.draw_contour_level = (float) ret;
            }
        }
      break;

    case ADD_FEATURE:
      break;

    case POLY_FILTER_AREA:
    case RECT_FILTER_AREA:
      res = QInputDialog::getDouble (this, "pfmView", tr ("Enter filter standard deviation (1.0 - 3.0):"), options.filterSTD, 1.0, 4.0, 1, &ok);
      if (ok) options.filterSTD = (float) res;
      break;

    case POLY_FILTER_MASK:
    case RECT_FILTER_MASK:
    case REMISP_FILTER:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;

    case EDIT_FEATURE:
      text = QInputDialog::getText (this, "pfmView", tr ("Enter text to search for:"), QLineEdit::Normal, QString::null, &ok);
      if (ok && !text.isEmpty ())
        {
          options.feature_search_string = text;
        }
      else
        {
          options.feature_search_string = "";
        }
      redrawMap (NVTrue, NVFalse);

      strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toLatin1 ());
      misc.abe_share->feature_search_type = options.feature_search_type;
      misc.abe_share->feature_search_invert = options.feature_search_invert;

      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
      break;

    case REMISP_AREA:
      ires = QInputDialog::getInt (this, "pfmView", tr ("Enter MISP weight:"), options.misp_weight, 1, 3, 1, &ok);

      if (ok) options.misp_weight = ires;
      break;

    case OUTPUT_POINTS:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;

    case GOOGLE_EARTH:
      slotEditMode (misc.last_edit_function);
      break;
    }
}



//!  Right click popup menu third entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu2 ()
{
  QStringList lst, items;
  QString res, item;
  int32_t value, current;
  bool ok;

  popup_active = NVFalse;

  switch (misc.function)
    {
    case RECT_EDIT_AREA_3D:
      misc.function = options.edit_mode = misc.last_edit_function = POLY_EDIT_AREA_3D;
      setFunctionCursor (misc.function);
      setWidgetStates (NVTrue);
      break;

    case POLY_EDIT_AREA_3D:
      misc.function = options.edit_mode = misc.last_edit_function = RECT_EDIT_AREA_3D;
      setFunctionCursor (misc.function);
      setWidgetStates (NVTrue);
      break;

    case RECT_FILTER_AREA:
    case POLY_FILTER_AREA:
      value = 0;
      if (!options.deep_filter_only) value = 1;

      lst << tr ("Yes") << tr ("No");

      res = QInputDialog::getItem (this, "pfmView", tr ("Deep filter only:"), lst, value, false, &ok);

      if (ok)
        {
          if (res.contains (tr ("Yes")))
            {
              options.deep_filter_only = NVTrue;
            }
          else
            {
              options.deep_filter_only = NVFalse;
            }
        }
      break;

    case REMISP_AREA:
      items << tr ("Yes") << tr ("No");

      current = 1;
      if (options.misp_force_original) current = 0;

      item = QInputDialog::getItem (this, "pfmView", tr ("MISP force original input value:"), items, current, false, &ok);
      if (ok && !item.isEmpty ())
        {
          if (item == tr ("Yes"))
            {
              options.misp_force_original = NVTrue;
            }
          else
            {
              options.misp_force_original = NVFalse;
            }
        }
      break;

    case RECT_FILTER_MASK:
    case POLY_FILTER_MASK:
      clearFilterMasks ();

      redrawMap (NVTrue, NVFalse);
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case ADD_FEATURE:
    case EDIT_FEATURE:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//!  Right click popup menu fourth entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu3 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    case POLY_FILTER_AREA:
    case RECT_FILTER_AREA:
    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:
    case REMISP_AREA:
      midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
      break;
    }
}



//!  Right click popup menu fifth entry has been selected.  Perform operations based on the active function.

void 
pfmView::slotPopupMenu4 ()
{
  popup_active = NVFalse;

  if (misc.function == POLY_FILTER_AREA || misc.function == RECT_FILTER_AREA)
    {
      clearFilterMasks ();
      redrawMap (NVTrue, NVFalse);
    }
}



//!  Right click popup menu help entry has been selected.  Display help based on the active function.

void 
pfmView::slotPopupHelp ()
{
  switch (misc.function)
    {
    case ZOOM_IN_AREA:
      QWhatsThis::showText (QCursor::pos ( ), zoomInText, map);
      break;

    case DEFINE_RECT_AREA:
      QWhatsThis::showText (QCursor::pos ( ), defineRectAreaText, map);
      break;

    case RECT_EDIT_AREA_3D:
      QWhatsThis::showText (QCursor::pos ( ), editModeText, map);
      break;

    case SELECT_HIGH_POINT:
      QWhatsThis::showText (QCursor::pos ( ), highText, map);
      break;

    case SELECT_LOW_POINT:
      QWhatsThis::showText (QCursor::pos ( ), lowText, map);
      break;

    case REMISP_AREA:
      QWhatsThis::showText (QCursor::pos ( ), remispText, map);
      break;

    case REMISP_FILTER:
      QWhatsThis::showText (QCursor::pos ( ), remispFilterText, map);
      break;

    case DEFINE_POLY_AREA:
      QWhatsThis::showText (QCursor::pos ( ), definePolyAreaText, map);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      QWhatsThis::showText (QCursor::pos ( ), defineFeaturePolyAreaText, map);
      break;

    case POLY_EDIT_AREA_3D:
      QWhatsThis::showText (QCursor::pos ( ), editModeText, map);
      break;

    case RECT_FILTER_AREA:
      QWhatsThis::showText (QCursor::pos ( ), filterRectText, map);
      break;

    case POLY_FILTER_AREA:
      QWhatsThis::showText (QCursor::pos ( ), filterPolyText, map);
      break;

    case RECT_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterRectMaskText, map);
      break;

    case POLY_FILTER_MASK:
      QWhatsThis::showText (QCursor::pos ( ), filterPolyMaskText, map);
      break;

    case OUTPUT_POINTS:
      QWhatsThis::showText (QCursor::pos ( ), outputDataPointsText, map);
      break;

    case DELETE_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), deleteFeatureText, map);
      break;

    case ADD_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), addFeatureText, map);
      break;

    case EDIT_FEATURE:
      QWhatsThis::showText (QCursor::pos ( ), editFeatureText, map);
      break;

    case DRAW_CONTOUR:
      QWhatsThis::showText (QCursor::pos ( ), drawContourText, map);
      break;

    case GRAB_CONTOUR:
      QWhatsThis::showText (QCursor::pos ( ), grabContourText, map);
      break;

    case DRAW_CONTOUR_FILTER:
      QWhatsThis::showText (QCursor::pos ( ), drawContourFilterText, map);
      break;

    case GOOGLE_EARTH:
      QWhatsThis::showText (QCursor::pos ( ), googleEarthText, map);
      break;
    }


  //  Discard the operation.

  midMouse (menu_cursor_x, menu_cursor_y, menu_cursor_lon, menu_cursor_lat);
}



/*!
  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in associated (linked or child)
  programs not in this window.  This is active whenever the mouse leaves this window.
*/

void
pfmView::slotTrackCursor ()
{
  char               ltstring[25], lnstring[25];
  QString            string, x_string, y_string;
  static double      prev_child_x = -999.0, prev_child_y = -999.0, prev_linked_x = -999.0, prev_linked_y = -999.0;
  static int32_t     prev_linked_command_num = 0;


  //  This is sort of ancillary to the track_cursor function.  What we want to do is monitor the number of error messages in the
  //  error message QStringList and when it changes pop up the messages dialog.  This way was a lot easier than implementing signals
  //  and slots or trying to call parent methods from children.

  if (misc.messages->size () != misc.num_messages)
    {
      misc.num_messages = misc.messages->size ();
      slotMessage ();
      if (misc.num_messages) messageAction->setEnabled (true);
    }


  if (misc.drawing || !cov_area_defined) return;


  //  pfm3D (the 3D bin surface viewer) may issue a request to edit so we're going to implement it here.

  if (misc.abe_share->key == PFMVIEW_FORCE_EDIT || misc.abe_share->key == PFMVIEW_FORCE_EDIT_3D)
    {
      map->setMovingPolygon (&pfm3D_polygon, misc.abe_share->polygon_count, misc.abe_share->polygon_x, misc.abe_share->polygon_y, 
                             options.contour_color, LINE_WIDTH, NVFalse, Qt::SolidLine);

      qApp->processEvents ();


      if (misc.abe_share->key == PFMVIEW_FORCE_EDIT_3D) force_3d_edit = NVTrue;

      editCommand (misc.abe_share->polygon_x, misc.abe_share->polygon_y, misc.abe_share->polygon_count);

      force_3d_edit = NVFalse;

      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED;

      return;
    }


  //  If we received a bunch of resize signals (see slotResize) and we're not currently drawing
  //  the map, we want to make sure we haven't received a resize for about two seconds.  If that's the
  //  case we will redraw.

  if (need_redraw)
    {
      redraw_count++;
      if (redraw_count > 80)
        {
          redrawMap (NVTrue, NVFalse);
          need_redraw = NVFalse;
          redraw_count = 0;
        }
    }


  //  If we have changed the feature file contents in one of our children, we need to reload the features.

  if (misc.abe_share->key == FEATURE_FILE_MODIFIED)
    {
      readFeature (this, &misc);


      //redrawMap (NVTrue, NVFalse);


      //  Wait 2 seconds before resetting key so that all associated programs will see the modified flag.

#ifdef NVWIN3X
      Sleep (2000);
#else
      sleep (2);
#endif

      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 
    }


  //  Checking for pfmEdit3D requesting a zoom in mosaicView

  if (misc.linked && misc.abe_share->zoom_requested)
    { 
      ABE_REGISTER_COMMAND command;
      command.id = ZOOM_TO_MBR_COMMAND;
      command.window_id = misc.process_id;
      command.mbr = misc.abe_share->displayed_area;
      command.number = rand ();

      misc.abe_register->command[abe_register_group] = command;
      
      misc.abe_share->zoom_requested = NVFalse;
    }


  //  Boy, is this weird!  We might be getting position information from one of two sources.  Either from a child process
  //  or from a linked application.  If it's a child and we're linked we need to inform other linked processes about the
  //  cursor location.  If it's a linked app we need to inform the child processes about the location.  In order to figure
  //  out which one it's from we have to check for change in either the abe_register memory or the abe_share memory.


  uint8_t hit = NVFalse;
  double lat = 0.0;
  double lon = 0.0;


  //  This is the case where one of our children has focus.

  if (misc.abe_share->active_window_id != misc.process_id && (prev_child_x != misc.abe_share->cursor_position.x ||
                                                              prev_child_y != misc.abe_share->cursor_position.y))
    {
      lat = misc.abe_share->cursor_position.y;
      lon = misc.abe_share->cursor_position.x;

      prev_child_x = lon;
      prev_child_y = lat;

      if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
          lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x) hit = NVTrue;


      //  Tell other apps what our child is doing.

      if (misc.linked)
        {
          misc.abe_register->data[abe_register_group].active_window_id = misc.abe_share->active_window_id;
          misc.abe_register->data[abe_register_group].cursor_position.x = lon;
          misc.abe_register->data[abe_register_group].cursor_position.y = lat;
        }
    }


  //  This is the case where some other app has focus.

  else if (misc.linked && misc.abe_register->data[abe_register_group].active_window_id != misc.process_id &&
           (prev_linked_x != misc.abe_register->data[abe_register_group].cursor_position.x ||
            prev_linked_y != misc.abe_register->data[abe_register_group].cursor_position.y))
    {

      //  Check for a command, make sure it's a new command (we haven't already done it), and make sure
      //  it's not one that we issued.

      if (misc.abe_register->command[abe_register_group].number != prev_linked_command_num && 
          misc.abe_register->command[abe_register_group].window_id != misc.process_id)
        {
          prev_linked_command_num = misc.abe_register->command[abe_register_group].number;

          switch (misc.abe_register->command[abe_register_group].id)
            {
            case ZOOM_TO_MBR_COMMAND:
              misc.clear = NVTrue;

              misc.GeoTIFF_init = NVTrue;

              zoomIn (misc.abe_register->command[abe_register_group].mbr, NVTrue);


              misc.cov_function = COV_START_DRAW_RECTANGLE;

              redrawCovBounds ();

              map->setToolTip ("");      
              misc.function = misc.save_function;

              setFunctionCursor (misc.function);

              break;
            }
        }


      //  Get the geographic location of the cursor from the other application.

      lat = misc.abe_register->data[abe_register_group].cursor_position.y;
      lon = misc.abe_register->data[abe_register_group].cursor_position.x;

      prev_linked_x = lon;
      prev_linked_y = lat;

      if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
          lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x) hit = NVTrue;


      //  Tell our children what the other apps are doing.

      misc.abe_share->active_window_id = misc.abe_register->data[abe_register_group].active_window_id;
      misc.abe_share->cursor_position.x = lon;
      misc.abe_share->cursor_position.y = lat;
    }


  //  If the geographic position of the cursor in the other application is in our displayed area set the marker and the
  //  status bar information.

  if (hit)
    {
      double deg, min, sec;
      char       hem;


      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));

      if (options.lat_or_east)
        {
          double x = lon * NV_DEG_TO_RAD;
          double y = lat * NV_DEG_TO_RAD;

          pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &x, &y, NULL);

          strcpy (ltstring, QString ("%L1").arg (y, 0, 'f', 2).toLatin1 ());
          strcpy (lnstring, QString ("%L1").arg (x, 0, 'f', 2).toLatin1 ());
        }

      latLabel->setText (ltstring);
      lonLabel->setText (lnstring);


      QBrush b1;
      map->setMovingPath (&mv_tracker, marker, lon, lat, 2, options.contour_color, b1, NVFalse, Qt::SolidLine);
    }
}



//!  Mouse press signal from the map class.

void 
pfmView::slotMousePress (QMouseEvent *e, double lon, double lat)
{
  if (!misc.drawing && !pfm_edit_active && !process_running)
    {
      //  Check for dateline.

      if (misc.dateline && lon < 0.0) lon += 360.0;


      if (e->button () == Qt::LeftButton)
        {
          //  Bring the nearest PFM layer to the top level if Ctrl-left-click.

          if (e->modifiers () == Qt::ControlModifier && misc.nearest_pfm > 0)
            {
              slotLayerGrpTriggered (layer[misc.nearest_pfm]);
            }
          else
            {
              leftMouse (e->x (), e->y (), lon, lat);
            }
        }
      else if (e->button () == Qt::MidButton)
        {
          midMouse (e->x (), e->y (), lon, lat);
        }
      else if (e->button () == Qt::RightButton)
        {
          rightMouse (e->x (), e->y (), lon, lat);
        }
    }
}



/*!
  Mouse press signal prior to signals being enabled from the map class.  This is just used to pop up the help verbiage
  if you haven't opened a PFM file yet.
*/

void 
pfmView::slotPreliminaryMousePress (QMouseEvent *e)
{
  QPoint pos = QPoint (e->x (), e->y ());

  QWhatsThis::showText (pos, mapText, map);
}



//!  Mouse release signal from the map class.

void 
pfmView::slotMouseRelease (QMouseEvent *e, double lon __attribute__ ((unused)), double lat __attribute__ ((unused)))
{
  if (!misc.drawing && !pfm_edit_active)
    {
      if (e->button () == Qt::LeftButton) popup_active = NVFalse;
      if (e->button () == Qt::MidButton) popup_active = NVFalse;
      if (e->button () == Qt::RightButton) {};
    }
}



//!  Mouse move signal from the map class.

void
pfmView::slotMouseMove (QMouseEvent *e, double lon, double lat)
{
  char                 ltstring[25], lnstring[25], hem;
  QString              string, x_string, y_string, num_string, stddev_string, min_string, avg_string, max_string, mmm_string;
  double               deg, min, sec;
  BIN_RECORD           bin;
  NV_F64_COORD2        xy;
  NV_I32_COORD2        hot = {-1, -1}, ixy, prev_xy = {-1, -1};
  static NV_I32_COORD2 prev_hot;


  //  Don't worry about the mouse if we're drawing

  if (misc.drawing) return;


  //  Check for dateline.

  if (misc.dateline && lon < 0.0) lon += 360.0;


  //  Get rid of the tracking cursor from slotTrackCursor ()since, if we got a move signal, we're obviously in
  //  this window.  We only want to kill it if it already exists, otherwise we will be creating a new one (which
  //  we don't want to do).

  if (mv_tracker >= 0) map->closeMovingPath (&mv_tracker);


  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = misc.process_id;


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;


  //  If we're linked to other apps (non-child) we need to let them know as well.

  if (misc.linked)
    {
      misc.abe_register->data[abe_register_group].active_window_id = misc.process_id;
      misc.abe_register->data[abe_register_group].cursor_position.y = lat;
      misc.abe_register->data[abe_register_group].cursor_position.x = lon;
    }


  //  Save the cursor posiyion in screen space (pixels).

  ixy.x = e->x ();
  ixy.y = e->y ();

  misc.nearest_feature = -1;


  //  If the cursor position has changed we want to set the status display and markers (also other goodies).

  if (lat >= misc.total_displayed_area.min_y && lat <= misc.total_displayed_area.max_y && 
      lon >= misc.total_displayed_area.min_x && lon <= misc.total_displayed_area.max_x)
    {
      strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
      strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));


      //  Try to find the highest layer with a valid value to display in the status bar.

      xy.y = lat;
      xy.x = lon;
      int32_t hit = -1;

      for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
        {
          if (misc.abe_share->display_pfm[pfm] && bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
            {
              read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);


              //  Check the validity.

              if ((bin.validity & PFM_DATA) || ((bin.validity & PFM_INTERPOLATED) && misc.abe_share->layer_type == AVERAGE_FILTERED_DEPTH))
                {
                  hit = pfm;
                  break;
                }
            }
        }


      //  We only change the status bar PFM values if we were actually over a PFM data area

      if (hit >= 0)
        {
          misc.nearest_pfm = hit;


          //  If misc.abe_share->open_args.head.x_bin_size_degrees = misc.abe_share->open_args.head.y_bin_size_degrees
          //  we're using lat/lon bin size in minutes.

          QString tmp;
          if (misc.otf_surface)
            {
              tmp += QString ("%L1").arg (options.otf_bin_size_meters, 0, 'f', 2);
              sizeLabel->setToolTip (tr ("Bin size in meters"));

              switch (options.layer_type)
                {
                case MIN_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Min OTF grid"));
                  break;

                case MAX_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Max OTF grid"));
                  break;

                case AVERAGE_FILTERED_DEPTH:
                  pfmLabel->setText (tr ("Average OTF grid"));
                  break;
                }
            }
          else if (misc.abe_share->open_args[hit].head.x_bin_size_degrees != misc.abe_share->open_args[hit].head.y_bin_size_degrees)
            {
              tmp += QString ("%L1").arg (misc.abe_share->open_args[hit].head.bin_size_xy, 0, 'f', 2);
              sizeLabel->setToolTip (tr ("Bin size in meters"));

              pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));
              pfmLabel->setToolTip (misc.abe_share->open_args[hit].list_path);
            }
          else
            {
              tmp += QString ("%L1").arg (misc.abe_share->open_args[hit].head.y_bin_size_degrees * 60.0, 0, 'f', 2);
              sizeLabel->setToolTip (tr ("Bin size in minutes"));

              pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));
              pfmLabel->setToolTip (misc.abe_share->open_args[hit].list_path);
            }

          sizeLabel->setText (tmp);


          if (misc.surface_val)
            {
              min_string = QString ("%L1").arg (options.z_orientation * bin.min_filtered_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              avg_string = QString ("%L1").arg (options.z_orientation * bin.avg_filtered_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              max_string = QString ("%L1").arg (options.z_orientation * bin.max_filtered_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              mmm_string = QString ("%L1").arg ((options.z_orientation * bin.max_filtered_depth * options.z_factor + options.z_offset) - 
                                                (options.z_orientation * bin.min_filtered_depth * options.z_factor + options.z_offset), 0, 'f', 2);
            }
          else
            {
              min_string = QString ("%L1").arg (options.z_orientation * bin.min_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              avg_string = QString ("%L1").arg (options.z_orientation * bin.avg_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              max_string = QString ("%L1").arg (options.z_orientation * bin.max_depth * options.z_factor + options.z_offset, 0, 'f', 2);
              mmm_string = QString ("%L1").arg ((options.z_orientation * bin.max_depth * options.z_factor + options.z_offset) - 
                                                (options.z_orientation * bin.min_depth * options.z_factor + options.z_offset), 0, 'f', 2);
            }


          //  Set the Z value text.

          if (misc.otf_surface)
            {
              switch (options.layer_type)
                {
                case MIN_FILTERED_DEPTH:
                  attrValue[0]->setText (min_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset minimum valid on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset minimum on-the-fly bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Minimum valid on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Minimum on-the-fly bin value"));
                        }
                    }
                  break;

                case MAX_FILTERED_DEPTH:
                  attrValue[0]->setText (max_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset maximum valid on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset maximum on-the-fly bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Maximum valid on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Maximum on-the-fly bin value"));
                        }
                    }
                  break;

                case AVERAGE_FILTERED_DEPTH:
                  attrValue[0]->setText (avg_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset valid average on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset average on-the-fly bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Average valid on-the-fly bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Average on-the-fly bin value"));
                        }
                    }
                  break;
                }
            }
          else
            {
              switch (options.layer_type)
                {
                case MIN_FILTERED_DEPTH:
                  attrValue[0]->setText (min_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset minimum valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset minimum bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Minimum valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Minimum bin value"));
                        }
                    }
                  break;

                case MAX_FILTERED_DEPTH:
                  attrValue[0]->setText (max_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset maximum valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset maximum bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Maximum valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Maximum bin value"));
                        }
                    }
                  break;

                case AVERAGE_FILTERED_DEPTH:
                  attrValue[0]->setText (avg_string);
                  if (options.z_factor != 1.0 || options.z_offset != 0.0)
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset average valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Scaled/offset average bin value"));
                        }
                    }
                  else
                    {
                      if (misc.surface_val)
                        {
                          attrName[0]->setToolTip (tr ("Average valid bin value"));
                        }
                      else
                        {
                          attrName[0]->setToolTip (tr ("Average bin value"));
                        }
                    }
                  break;
                }
            }

          num_string.sprintf ("%d", bin.num_soundings);
          attrValue[1]->setText (num_string);
          stddev_string = QString ("%L1").arg (bin.standard_dev, 0, 'f', 4);
          attrValue[2]->setText (stddev_string);
          attrValue[3]->setText (mmm_string);
          num_string.sprintf ("%d", bin.num_valid);
          attrValue[4]->setText (num_string);

          for (int32_t i = 0 ; i < misc.abe_share->open_args[0].head.num_bin_attr ; i++)
            {
              if (misc.abe_share->open_args[0].head.bin_attr_scale[i] == 1.0)
                {
                  num_string.sprintf ("%d", NINT (bin.attr[i]));
                }
              else
                {
                  num_string.sprintf ("%.4f", bin.attr[i]);
                }

              attrBoxValue[i]->setText (num_string);
            }

          for (int32_t i = 0 ; i < POST_ATTR ; i++)
            {
              if (misc.attrStatNum[i] > -1)
                {
                  int32_t j = misc.attrStatNum[i];

                  attrName[i + PRE_ATTR]->setText (attrBoxName[j]->text ());

                  if (misc.abe_share->open_args[0].head.bin_attr_scale[j] == 1.0)
                    {
                      num_string.sprintf ("%d", NINT (bin.attr[j]));
                    }
                  else
                    {
                      num_string.sprintf ("%.4f", bin.attr[j]);
                    }

                  attrValue[i + PRE_ATTR]->setText (num_string);
                }
            }
        }


      if (options.lat_or_east)
        {
          double x = lon * NV_DEG_TO_RAD;
          double y = lat * NV_DEG_TO_RAD;

          pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &x, &y, NULL);

          strcpy (ltstring, QString ("%L1").arg (y, 0, 'f', 2).toLatin1 ());
          strcpy (lnstring, QString ("%L1").arg (x, 0, 'f', 2).toLatin1 ());
        }

      latLabel->setText (ltstring);
      lonLabel->setText (lnstring);
    }


  double anc_lat, anc_lon, az, dist, min_dist, dz;
  int32_t xyz_x, xyz_y, xyz_z;
  QBrush b1;
  QColor c1;


  //  Actions based on the active function.  Here we're usually draggin a rubberband rectangle, line, or polygon line based
  //  on the active function.

  switch (misc.function)
    {
    case ZOOM_IN_AREA:
    case RECT_EDIT_AREA_3D:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case REMISP_AREA:
    case DEFINE_RECT_AREA:
      if (map->rubberbandRectangleIsActive (rb_rectangle))
        {
          map->dragRubberbandRectangle (rb_rectangle, lon, lat);

          map->getRubberbandRectangleAnchor (rb_rectangle, &anc_lon, &anc_lat);

          double rect_width, rect_height;

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, anc_lat, lon, &rect_width, &az);
          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, anc_lon, &rect_height, &az);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);

          string = QString (" W: %L1m, H: %L2m, Diag: %L3m ").arg (rect_width).arg (rect_height).arg (dist);
          misc.statusProgLabel->setText (string);
        }
      break;

    case POLY_EDIT_AREA_3D:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DEFINE_POLY_AREA:
    case DEFINE_FEATURE_POLY_AREA:
    case REMISP_FILTER:
    case GRAB_CONTOUR:
      if (map->rubberbandPolygonIsActive (rb_polygon))
        {
          map->dragRubberbandPolygon (rb_polygon, lon, lat);

          map->getRubberbandPolygonCurrentAnchor (rb_polygon, &anc_lon, &anc_lat);

          double rect_width, rect_height;

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, anc_lat, lon, &rect_width, &az);
          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, anc_lon, &rect_height, &az);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);


          string = QString (" W: %L1m, H: %L2m, Diag: %L3m ").arg (rect_width).arg (rect_height).arg (dist);
          misc.statusProgLabel->setText (string);
        }
      break;

    case OUTPUT_POINTS:
      string.sprintf (" Point #: %d ", misc.output_point_count);
      misc.statusProgLabel->setText (string);
      break;

    case EDIT_FEATURE:
    case DELETE_FEATURE:
      min_dist = 999999999.0;

      for (uint32_t i = 0 ; i < misc.bfd_header.number_of_records ; i++)
        {
          if (misc.feature[i].confidence_level || !misc.surface_val)
            {
              if (misc.feature[i].latitude >= misc.total_displayed_area.min_y &&
                  misc.feature[i].latitude <= misc.total_displayed_area.max_y && 
                  misc.feature[i].longitude >= misc.total_displayed_area.min_x &&
                  misc.feature[i].longitude <= misc.total_displayed_area.max_x)
                {
                  if (options.display_children || !misc.feature[i].parent_record)
                    {
                      map->map_to_screen (1, &misc.feature[i].longitude, &misc.feature[i].latitude, &dz, &xyz_x, &xyz_y, &xyz_z);

                      dist = sqrt ((double) ((ixy.y - xyz_y) * (ixy.y - xyz_y)) + (double) ((ixy.x - xyz_x) * (ixy.x - xyz_x)));

                      if (dist < min_dist)
                        {
                          misc.nearest_feature = i;
                          hot.x = xyz_x;
                          hot.y = xyz_y;
                          min_dist = dist;
                        }
                    }
                }
            }
        }

      map->setMovingPath (&mv_marker, marker, hot.x, hot.y, LINE_WIDTH, options.contour_color, b1, NVFalse, Qt::SolidLine);

      prev_hot = hot;

      break;

    case ADD_FEATURE:
      break;

    case DRAW_CONTOUR:
    case DRAW_CONTOUR_FILTER:

      //  Only process if the pointer position has changed pixels.

      if ((ixy.x != prev_xy.x || ixy.y != prev_xy.y) && map->rubberbandPolygonIsActive (rb_polygon))
        {
          xy.y = lat;
          xy.x = lon;


          if (prev_poly_lon > -180.5) map->vertexRubberbandPolygon (rb_polygon, prev_poly_lon, prev_poly_lat);
          map->dragRubberbandPolygon (rb_polygon, lon, lat);
          prev_poly_lat = lat;
          prev_poly_lon = lon;

          QString msc = tr (" Draw contour - %L1 ").arg (misc.draw_contour_level, 0, 'f', 2);
          misc.statusProgLabel->setText (msc);
          misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
          misc.statusProgLabel->setPalette (misc.statusProgPalette);
        }
      break;
    }


  //  Set the previous cursor.
             
  prev_xy = ixy;
}



//!  Resize signal from the map class.

void
pfmView::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  //  The first time we get a resize (or if we have closed all of the open PFM files) we want to display the 
  //  fabulously elegant PFM splash screen ;-)

  if (logo_first)
    {
      QPixmap *logo = new QPixmap (":/icons/PFMLogo.png");

      int32_t w = map->width () - 3 * mapdef.border;
      int32_t h = map->height () - 3 * mapdef.border;

      QPixmap *logo_scaled = new QPixmap (logo->scaled (w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

      delete logo;

      map->drawPixmap (mapdef.border, mapdef.border, logo_scaled, 0, 0, w, h, NVTrue);

      delete logo_scaled;

      map->setCursor (Qt::WhatsThisCursor);
    }
  else
    {
      //  If we're actually drawing the data at this time we don't want to tell it to redraw after we're done.  Also, if we haven't
      //  even picked an area to draw (in the coverage map) there's no point in telling it to redraw nothing 2 seconds later ;-)

      if (misc.drawing || (!cov_area_defined)) return;


      //  If the windowing system is doing "Display content in resizing windows" we'll get about a million
      //  resize callbacks all stacked up.  What we want to do is use the trackCursor function to wait for
      //  2 seconds of inactivity (i.e. no resizes) and then redraw the map.  We use the redraw_count to 
      //  time it.  I thought this was easier than starting and killing a one-shot timer every time we
      //  came through this section of code.

      need_redraw = NVTrue;
      redraw_count = 0;
      misc.GeoTIFF_init = NVTrue;

      map->setCursor (Qt::WaitCursor);
    }
}



//!  Prior to drawing coastline signal from the map class.  This is where we do our heavy lifting (actually in paint_surface).

void 
pfmView::slotPreRedraw (NVMAP_DEF l_mapdef)
{
  void paint_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, uint8_t cov_area_defined);
  void paint_otf_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, uint8_t cov_area_defined);


  //  Make sure we know whether we're using a valid of invalid surface.

  setSurfaceValidity (misc.abe_share->layer_type);


  //  Set the drawing flag and turn the map mouse signals off until we are through painting.

  misc.drawing = NVTrue;
  map->disableMouseSignals ();


  setWidgetStates (NVFalse);

  map->setCursor (Qt::WaitCursor);
  cov->setCursor (Qt::WaitCursor);

  qApp->processEvents ();

  mapdef = l_mapdef;


  if (misc.otf_surface)
    {
      paint_otf_surface (&misc, &options, map, &mapdef, cov_area_defined);
    }
  else
    {
      paint_surface (&misc, &options, map, &mapdef, cov_area_defined);
    }
}



/*!
  Move the display left, right, up, or down.  This is called from the keyPressEvent or when a move is requested in pfmEdit3D.
*/

void 
pfmView::moveMap (int32_t direction)
{
  discardMovableObjects ();


  clearFilterMasks ();


  map->moveMap (direction);


  //  We want to redraw the pfm3D view if it's up.

  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
    }
}



//!  Zoom in to the specified bounds.

void
pfmView::zoomIn (NV_F64_XYMBR bounds, uint8_t pfm3D)
{
  discardMovableObjects ();

  clearFilterMasks ();

  map->zoomIn (bounds);

  bZoomOut->setEnabled (true);


  //  We want to redraw the pfm3D view if it's up.

  if (pfm3D)
    {
      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }
}



//!  Zoom out one level.

void
pfmView::zoomOut ()
{
  discardMovableObjects ();


  clearFilterMasks ();


  map->zoomOut ();


  //  We want to redraw the pfm3D view if it's up.

  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
    }
}



//!  Redraw the entire map.

void
pfmView::redrawMap (uint8_t clear, uint8_t pfm3D)
{
  misc.clear = NVTrue;

  discardMovableObjects ();


  //  Clear the filter mask displayed flags.

  for (int32_t i = 0 ; i < misc.poly_filter_mask_count ; i++) misc.poly_filter_mask[i].displayed = NVFalse;

  map->redrawMap (clear);


  //  If we changed the surface we want to redraw the pfm3D view if it's up.

  if (pfm3D)
    {
      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }
}



//!  Slot to call the redrawMap function.

void
pfmView::slotRedraw ()
{
  redrawMap (NVTrue, NVFalse);
}



//!  Slot to toggle auto redraw after edit.

void
pfmView::slotAutoRedraw ()
{
  if (bAutoRedraw->isChecked ())
    {
      options.auto_redraw = NVTrue;
    }
  else
    {
      options.auto_redraw = NVFalse;
    }
}



//!  Redraw the coverage map.

void
pfmView::slotRedrawCoverage ()
{
  misc.cov_clear = NVTrue;

  cov->redrawMap (NVTrue);
}



//!  Using the keys to move around.  We also may be starting the GeoTIFF viewer (mosaicView) or pfm3D.

void
pfmView::keyPressEvent (QKeyEvent *e)
{
  //  Discard key presses if we're drawing or if one of the editors is running.

  if (!misc.drawing && !pfm_edit_active)
    {
      QString qkey = e->text ();


      if (qkey == options.mosaic_hotkey)
        {
          //  Mosaic (GeoTIFF) viewer.

          slotStartMosaicViewer ();
        }
      else if (qkey == options.threeD_hotkey)
        {
          //  3D surface viewer.

          slotStart3DViewer ();
        }
      else if (qkey == options.buttonAccel[EDIT_MODE_KEY])
        {
          //  Switch back to the last used edit mode from whatever mode (like EDIT_FEATURE) we were in.

          slotEditMode (misc.last_edit_function);
        }

      //  Check for one of three mosaic viewer action keys.

      else if (qkey == options.mosaic_actkey.section (',', 0, 0) ||
               qkey == options.mosaic_actkey.section (',', 1, 1) ||
               qkey == options.mosaic_actkey.section (',', 2, 2))
        {
          misc.abe_share->modcode = 0;
          char key[20];
          strcpy (key, e->text ().toLatin1 ());
          misc.abe_share->key = (uint32_t) key[0];


          //  Tell other registered ABE applications to zoom to MBR (the first mosaicView action key is zoom to MBR).

          if (misc.linked && qkey == options.mosaic_actkey.section (',', 0, 0))
            {
              ABE_REGISTER_COMMAND command;
              command.id = ZOOM_TO_MBR_COMMAND;
              command.window_id = misc.process_id;
              command.mbr = misc.total_displayed_area;
              command.number = rand ();

              misc.abe_register->command[abe_register_group] = command;
            }
        }


      //  Move the viewing area up, down, left, right, up/right, down/right, up/left, and down/left based on the arrow (or keypad) key pressed.

      else
        {
          misc.clear = NVTrue;
          switch (e->key ())
            {
            case Qt::Key_Left:
              moveMap (NVMAP_LEFT);
              break;

            case Qt::Key_Up:
              moveMap (NVMAP_UP);
              break;

            case Qt::Key_Right:
              moveMap (NVMAP_RIGHT);
              break;

            case Qt::Key_Down:
              moveMap (NVMAP_DOWN);
              break;

            case Qt::Key_PageUp:
              moveMap (NVMAP_UP_RIGHT);
              break;

            case Qt::Key_PageDown:
              moveMap (NVMAP_DOWN_RIGHT);
              break;

            case Qt::Key_Home:
              moveMap (NVMAP_UP_LEFT);
              break;

            case Qt::Key_End:
              moveMap (NVMAP_DOWN_LEFT);
              break;

            default:
              return;
            }

          redrawCovBounds ();
        }
    }
}



//!  Somebody pressed the little X in the window decoration.  DOH!

void 
pfmView::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
pfmView::slotClose (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



//!  Slot called on exit.

void 
pfmView::slotQuit ()
{
  //  Check for running unload processes.

  uint8_t unload_running = NVFalse;
  for (int32_t i = 0 ; i < 100 ; i++)
    {
      if (misc.unload_type[i] >= 0)
        {
          unload_running = NVTrue;
          break;
        }
    }

  if (unload_running)
    {
      int32_t ret = QMessageBox::Yes;
      ret = QMessageBox::information (this, "pfmView", tr ("There are active unloads in progress.  Do you really want to exit?"),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (ret == QMessageBox::No) return;
    }


  //  Check to see if Google Earth is running.  This will also get rid of the temporary KML files.

  if (googleEarthProc) killGoogleEarth ();


  //  Check for files in the delete file queue.  If there are any there offer the user the chance to get rid of them.

  if (misc.abe_share->delete_file_queue_count)
    {
      int32_t ret = QMessageBox::Yes;
      ret = QMessageBox::information (this, "pfmView", tr ("There are files in the delete file queue.  Do you really want to exit?"),
                                      QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

      if (ret == QMessageBox::No) return;
    }


  //  No matter what program started them (i.e. this or pfmEdit3D), if we're leaving the viewer we want to nicely terminate
  //  all child processes (or at least try).

  misc.abe_share->key = CHILD_PROCESS_FORCE_EXIT;


  //  Make sure we don't save GeoTIFF or an attribute as the default layer to display.

  if (misc.abe_share->layer_type >= ATTRIBUTE0) misc.abe_share->layer_type = MIN_FILTERED_DEPTH;


  //  Move stuff from share to options before we save the options.

  options.layer_type = misc.abe_share->layer_type;
  options.cint = misc.abe_share->cint;
  options.num_levels = misc.abe_share->num_levels;
  for (int32_t i = 0 ; i < misc.abe_share->num_levels ; i++) options.contour_levels[i] = misc.abe_share->contour_levels[i];


  options.edit_mode = misc.last_edit_function;


  //  Save the user's defaults

  envout (&options, &misc, this);


  //  Close the feature file if it's open

  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);


  //  Get rid of the shared memory.

  misc.abeShare->detach ();


  //  If we're linked to another program(s), unregister the link.

  if (misc.linked) unregisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id);


  exit (0);
}



//!  Post grid drawing signal from map class.  Mostly just cleanup work.

void 
pfmView::slotPostRedraw (NVMAP_DEF mapdef)
{
  //  Turn the map mouse signals back on.

  map->enableMouseSignals ();


  logo_first = NVFalse;


  if (cov_area_defined)
    {
      //  Redraw the coverage map if needed.

      if (misc.prev_total_displayed_area.min_x != 999.0 &&
          (misc.prev_total_displayed_area.min_x != misc.total_displayed_area.min_x || 
           misc.prev_total_displayed_area.min_y != misc.total_displayed_area.min_y ||
           misc.prev_total_displayed_area.max_x != misc.total_displayed_area.max_x || 
           misc.prev_total_displayed_area.max_y != misc.total_displayed_area.max_y))
        {
          misc.cov_clear = NVTrue;
          cov->redrawMap (NVFalse);


          //  If Google Earth is running we want to refocus it on the new area by changing the networkLink file being used by GoogleEarth.

          if (googleEarthProc && googleEarthProc->state () == QProcess::Running) positionGoogleEarth ();
        }

      setWidgetStates (NVTrue);


      //  We don't want to set these if we just set these......

      if (!scale_loop)
        {
          //  Get the width and height of the area in meters (at center for longitude).

          double az, rect_width, rect_height;

          invgp (NV_A0, NV_B0, mapdef.center_y, mapdef.bounds[mapdef.zoom_level].min_x, mapdef.center_y, mapdef.bounds[mapdef.zoom_level].max_x,
                 &rect_width, &az);
          invgp (NV_A0, NV_B0, mapdef.bounds[mapdef.zoom_level].min_y, mapdef.bounds[mapdef.zoom_level].min_x, mapdef.bounds[mapdef.zoom_level].max_y,
                 mapdef.bounds[mapdef.zoom_level].min_x, &rect_height, &az);


          QString string = QString (" %L1, %L2").arg (rect_width, 0, 'f', 2).arg (rect_height, 0, 'f', 2);
          displayedArea->setText (string);


          //  We're making the assumption that a pixel is square (you know what they say about assuming ;-)

          int32_t dpi_x = qApp->desktop ()->physicalDpiX ();
          //int32_t dpi_y = qApp->desktop ()->physicalDpiY ();
          float dpm_x = dpi_x * 39.37;
          //float dpm_y = dpi_y * 39.37;


          int32_t scale = NINT (rect_width * (1.0 / ((float) mapdef.width / dpm_x)));

          string = QString ("1:%1").arg (scale);
          viewChartScale->setText (string);
        }

      scale_loop = NVFalse;
    }

  misc.prev_total_displayed_area = misc.total_displayed_area;
  misc.drawing_canceled = NVFalse;


  //  If Google Earth has been launched and is running, we might have a stickpin in the displayed area.

  if (googleEarthProc && googleEarthProc->state () == QProcess::Running && googleEarthPushpin.x > -999.0)
    {
      double dz = 0.0;
      int32_t xyz_x, xyz_y, xyz_z;
      map->map_to_screen (1, &googleEarthPushpin.x, &googleEarthPushpin.y, &dz, &xyz_x, &xyz_y, &xyz_z);
      map->drawPixmap (xyz_x - 11, xyz_y - 26, stickpin, 0, 0, 32, 32, NVTrue);
    }


  setFunctionCursor (misc.function);

  cov->setCursor (editRectCursor);

  misc.drawing = NVFalse;
}



void pfmView::openEnhanced(){

     QStringList files;
     enhancedOpenDisplay *enhancedOpen = new enhancedOpenDisplay(&options,&misc);
     
   
     if (enhancedOpen->showMe() == QDialog::Accepted) {
	 this->inputFilesList =  enhancedOpen->selectedFiles ();
     } else  {
         this->inputFilesList.clear();  
     }
     
     // set the default open mode
     options.defaultFileOpen = enhancedOpen->defaultOpenMode();
     options.lastScannedDirectory = enhancedOpen->getLastScannedDirectory();
     options.lastFileFilter = enhancedOpen->getCurrentFilter();
}

void pfmView::openRegular() {
     uint8_t check_recent (QWidget *, OPTIONS *options, MISC *misc, QString file, QActionGroup *recentGrp, QMenu *recentMenu, QAction **recentFileAction);
     void setSidebarUrls (QFileDialog *fd, QString dir);
     QStringList files,filters;
     
    if (!QDir (options.input_pfm_dir).exists ()) options.input_pfm_dir = ".";

      QFileDialog *fd = new QFileDialog (this, tr ("pfmView Open PFM Structure"));
      fd->setViewMode (QFileDialog::List);
      fd->setMinimumHeight (800);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, options.input_pfm_dir);


      filters << tr ("PFM (*.pfm)");
      
      fd->setNameFilters (filters);
      fd->setFileMode (QFileDialog::ExistingFiles);
      fd->selectNameFilter (tr ("PFM (*.pfm)"));


      this->inputFilesList.clear();
      QStringList alreadyOpened;
      if (fd->exec () == QDialog::Accepted) {
	// make sure none of these files are already open
	  QStringList tmp = fd->selectedFiles();
	  boolean found;
	  for (int i = 0; i < tmp.count(); i++) {
	      found = false;
	      for (int j = 0; j < misc.abe_share->pfm_count;j++)
	      {
		  if (tmp.at(i) == QString(misc.abe_share->open_args[j].list_path)) {
		      found = true;
		      alreadyOpened << tmp.at(i);
		  }
	      }
	      if (!found) this->inputFilesList << tmp.at(i);
	  }
	  
	 if (alreadyOpened.size() > 0 ) { 
	 QString msg = "These files were already opened : \n" + alreadyOpened.join(",\n");    
	 QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), 
		 msg);
	 }

    //    this->inputFilesList =  fd->selectedFiles ();
      }
}


void 
pfmView::slotRegularOpenClicked() {
    
    openFiles("regular");
   
}

void
pfmView::slotEnhancedOpenClicked() {
    openFiles("enhanced");
}
//!  Open PFM file.  Called when someone uses "open" in the menu or presses the "open" button.

void 
pfmView::openFiles(QString openMode)
{
  QStringList filters;
  QString file, dir;
  PFM_OPEN_ARGS open_args;
  int32_t pfm_handle = -1;

  uint8_t check_recent (QWidget *, OPTIONS *options, MISC *misc, QString file, QActionGroup *recentGrp, QMenu *recentMenu, QAction **recentFileAction);
 // void setSidebarUrls (QFileDialog *fd, QString dir);


  uint8_t accepted = NVFalse;

  if (recent_file_flag)
    {
      this->inputFilesList << options.recentFile[recent_file_flag - 1];
      dir = QFileInfo (options.recentFile[recent_file_flag - 1]).dir ().absolutePath ();

      recent_file_flag = 0;

      accepted = NVTrue;
    }
  else if (command_file_flag)
    {
      this->inputFilesList << commandFile;
      dir = QFileInfo (commandFile).dir ().absolutePath ();

      command_file_flag = NVFalse;

      accepted = NVTrue;
    }
  else
    {
      if (openMode == "enhanced") {
	  openEnhanced();
       } else {
	  openRegular();
      }
      
 
      if (this->inputFilesList.size() > 0 ) accepted = NVTrue;

      
//      if (!QDir (options.input_pfm_dir).exists ()) options.input_pfm_dir = ".";
//
//      QFileDialog *fd = new QFileDialog (this, tr ("pfmView Open PFM Structure"));
//      fd->setViewMode (QFileDialog::List);
//      fd->setMinimumHeight (800);
//
//
//      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
//      //  This function is in the nvutility library.
//
//      setSidebarUrls (fd, options.input_pfm_dir);
//
//
//      filters << tr ("PFM (*.pfm)");
//      
//      fd->setNameFilters (filters);
//      fd->setFileMode (QFileDialog::ExistingFiles);
//      fd->selectNameFilter (tr ("PFM (*.pfm)"));
//
//
//      if (fd->exec () == QDialog::Accepted) accepted = NVTrue;
//
//      files = fd->selectedFiles ();
//      dir = fd->directory ().absolutePath ();
//      qDebug() << "dir = " << dir;
//      
//      for (int i = 0; i < files.size(); i++)
//      {
//	  qDebug() << files.at(i);
//      }
    }


  if (accepted)
    {
      
      if ((misc.abe_share->pfm_count + this->inputFilesList.size ()) >= MAX_ABE_PFMS)
        {
          QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), tr ("Too many open PFM structures.\nPlease close some before trying to open new ones."));
          return;
        }


      for (int32_t i = 0 ; i < this->inputFilesList.size () ; i++)
        {
          file = this->inputFilesList.at (i);
	  QFileInfo info(file);
	  dir = QDir(info.absoluteDir()).absolutePath();
           
	 
 
          if (!file.isEmpty())
            {
              //  Open the file and make sure it is a valid PFM file.

              strcpy (open_args.list_path, file.toLatin1 ());

              open_args.checkpoint = 0;
              pfm_handle = open_existing_pfm_file (&open_args);

              if (pfm_handle < 0)
                {
                  QMessageBox::warning (this, tr ("pfmView Open PFM Structure"),
                                        tr ("The file \n%1\n is not a PFM structure or there was an error reading the file.  The error message returned was: %2").arg
                                        (QDir::toNativeSeparators (QString (open_args.list_path))).arg (QString (pfm_error_str (pfm_error))));

                  if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
                    {
                      fprintf (stderr, "%s %s %s %d - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, pfm_error_str (pfm_error));
                      slotQuit ();
                    }

                  return;
                }


              //  We need to initialize the UTM/Geodetic conversion software (from PROJ.4).  Get the zone from the center of this PFM file.

              double cen_x = open_args.head.mbr.min_x + (open_args.head.mbr.max_x - open_args.head.mbr.min_x) / 2.0;
              double cen_y = open_args.head.mbr.min_y + (open_args.head.mbr.max_y - open_args.head.mbr.min_y) / 2.0;

              int32_t zone = (int32_t) (31.0 + cen_x / 6.0);
              if (zone >= 61) zone = 60;
              if (zone <= 0) zone = 1;

              char blk[20] = {'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'};
              int32_t blknum = int32_t ((cen_y + 80.0) / 8.0);

              zoneName.sprintf ("%d%1c", zone, blk[blknum]);

              char str[128];
              if (cen_y < 0.0)
                {
                  sprintf (str, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84 +south", zone, cen_x);
                }
              else
                {
                  sprintf (str, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84", zone, cen_x);
                }

              if (!(misc.pj_utm = pj_init_plus (str)))
                {
                  QMessageBox::critical (this, "pfmView", tr ("Error initializing UTM projection\n"));
                }

              if (!(misc.pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
                {
                  QMessageBox::critical (this, "pfmView", tr ("Error initializing latlon projection\n"));
                }


              //  Check the file to see if we already have it opened.  If we do then we'll just move it to the top level.

              uint8_t opened = NVFalse;
              for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
                {
                  if (!strcmp (misc.abe_share->open_args[pfm].list_path, open_args.list_path))
                    {
                      //  No point in moving the top level layer to the top level.

                      if (pfm) slotLayerGrpTriggered (layer[pfm]);

                      opened = NVTrue;
                    }
                }


              if (!opened)
                {
                  //  We don't do projected ;-)

                  if (open_args.head.proj_data.projection)
                    {
                      QMessageBox::warning (this, tr ("pfmView Open PFM Structure"), tr ("Sorry, pfmView only handles geographic PFM structures."));
                      close_pfm_file (pfm_handle);
                      return;
                    }
                  else
                    {
                      //  Check to see if this is one of the recently opened files.  If not, add it to the list.

                      if (!check_recent (this, &options, &misc, file, recentGrp, recentMenu, recentFileAction)) return;


                      for (int32_t pfm = misc.abe_share->pfm_count ; pfm > 0 ; pfm--)
                        {
                          misc.abe_share->open_args[pfm] = misc.abe_share->open_args[pfm - 1];
                          misc.ss_cell_size_x[pfm] = misc.ss_cell_size_x[pfm - 1];
                          misc.ss_cell_size_y[pfm] = misc.ss_cell_size_y[pfm - 1];
                          for (int32_t i = 0 ; i < misc.abe_share->open_args[pfm - 1].head.num_bin_attr ; i++)
                            strcpy (misc.attr_format[pfm][i], misc.attr_format[pfm - 1][i]);
                          misc.pfm_handle[pfm] = misc.pfm_handle[pfm - 1];
                          misc.average_type[pfm] = misc.average_type[pfm - 1];
                          misc.cube_attr_available[pfm] = misc.cube_attr_available[pfm - 1];
                          misc.abe_share->display_pfm[pfm] = misc.abe_share->display_pfm[pfm - 1];
                          misc.pfm_alpha[pfm] = misc.pfm_alpha[pfm - 1];
                          misc.last_saved_contour_record[pfm] = misc.last_saved_contour_record[pfm - 1];


                          QString layerString;
                          layerString.sprintf ("%d - ", pfm + 1);
                          layerString += QFileInfo (misc.abe_share->open_args[pfm].list_path).fileName ();
                          layer[pfm]->setText (layerString);
                          layer[pfm]->setVisible (true);
                        }

                      misc.pfm_handle[0] = pfm_handle;
                      misc.abe_share->open_args[0] = open_args;

                      misc.GeoTIFF_open = NVFalse;
                      misc.display_GeoTIFF = NVFalse;
                      bGeotiff->setEnabled (false);
                      misc.GeoTIFF_init = NVTrue;


                      QString tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[0]));
                      highlightUser01->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser01->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[1]));
                      highlightUser02->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser02->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[2]));
                      highlightUser03->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser03->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[3]));
                      highlightUser04->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser04->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[4]));
                      highlightUser05->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser05 ->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[5]));
                      highlightUser06->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser06 ->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[6]));
                      highlightUser07->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser07 ->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[7]));
                      highlightUser08->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser08 ->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[8]));
                      highlightUser09->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser09 ->setVisible (false);
                      tmp = tr ("Highlight %1 data").arg (QString (misc.abe_share->open_args[0].head.user_flag_name[9]));
                      highlightUser10->setText (tmp);
                      if (tmp.contains ("PFM_USER_")) highlightUser10 ->setVisible (false);


                      //  Default average surface button setup.  We'll check for CUBE right after this.

                      bCube->setEnabled (false);

                      if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MISP"))
                        {
                          bSurface[0]->setIcon (QIcon  (":/icons/misp_surface.png"));
                          QString tip = tr ("Display MISP surface  [%1]").arg (options.buttonAccel[DISPLAY_AVG_EDITED_KEY]);
                          misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
                        }
                      else
                        {
                          bSurface[0]->setIcon (misc.buttonIcon[DISPLAY_AVG_EDITED_KEY]);
                          QString tip = misc.buttonText[DISPLAY_AVG_EDITED_KEY] + "  [" +
                            options.buttonAccel[DISPLAY_AVG_EDITED_KEY] + "]";
                          misc.button[DISPLAY_AVG_EDITED_KEY]->setToolTip (tip);
                        }


                      //  Determine the attribute formats, if any

                      misc.cube_attr_available[0] = NVFalse;
                      if (misc.abe_share->open_args[0].head.num_bin_attr)
                        {
                          for (int32_t i = 0 ; i < NUM_ATTR ; i++)
                            {
                              if (i < misc.abe_share->open_args[0].head.num_bin_attr)
                                {
                                  int32_t decimals = (int32_t) (log10 (misc.abe_share->open_args[0].head.bin_attr_scale[i]) + 0.05);
                                  sprintf (misc.attr_format[0][i], "%%.%df", decimals);
                                }
                            }
                        }


                      //  Check for the color_index flag.

                      if (misc.abe_share->open_args[0].head.num_bin_attr < misc.color_index) misc.color_index = 0;


                      //  Compute cell sizes for sunshading.

                      misc.ss_cell_size_x[0] = misc.abe_share->open_args[0].head.bin_size_xy;
                      misc.ss_cell_size_y[0] = misc.abe_share->open_args[0].head.bin_size_xy;


                      QString layerString = "1 - " + QFileInfo (misc.abe_share->open_args[0].list_path).fileName ();
                      layer[0]->setText (layerString);
                      layer[0]->setVisible (true);
                    }
                }


              //  Set the attributes for the new "top" PFM.

              setStatusAttributes ();


              //  Figure out which attributes that were in the Status tab attributes we still have.

              int32_t atcnt = 0;

              for (int32_t i = 0 ; i < POST_ATTR ; i++)
                {
                  for (int32_t j = 0 ; j < misc.abe_share->open_args[0].head.num_bin_attr ; j++)
                    {
                      QString tmp = QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[j]));

                      if (options.attrStatName[i] == tmp)
                        {
                          misc.attrStatNum[atcnt] = j;
                          options.attrStatName[atcnt] = tmp;
                          attrName[atcnt + PRE_ATTR]->setText (tmp);
                          atcnt++;
                          break;
                        }
                    }
                }

              for (int32_t i = atcnt ; i < POST_ATTR ; i++)
                {
                  misc.attrStatNum[i] = -1;
                  options.attrStatName[i] = "";
                  attrName[i + PRE_ATTR]->setText ("");
                }


              //  If the mosaic viewer and/or 3D viewer was running, tell them to redraw.

              if (strcmp (misc.abe_share->open_args[0].target_path, "NONE")) misc.abe_share->key = FEATURE_FILE_MODIFIED;


              misc.abe_share->display_pfm[0] = NVTrue;
              misc.pfm_alpha[0] = 255;

              misc.average_type[0] = 0;


              //  Check to see if we're using something other than the standard AVERAGE EDITED DEPTH (MISP interpolated surface).

              if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MINIMUM MISP"))
                {
                  misc.average_type[0] = 1;
                }
              else if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "AVERAGE MISP"))
                {
                  misc.average_type[0] = 2;
                }
              else if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "MAXIMUM MISP"))
                {
                  misc.average_type[0] = 3;
                }
              else if (strstr (misc.abe_share->open_args[0].head.average_filt_name, "CUBE Surface"))
                {
                  misc.average_type[0] = -1;
                }

              misc.abe_share->display_pfm[0] = NVTrue;


              misc.abe_share->pfm_count++;


              //  If we are editing a new PFM, switch the working directories to be the PFM directory.  Otherwise, we
              //  leave them where they were.

              if (options.input_pfm_dir != dir)
                {
                  options.overlay_dir = dir;
                  options.output_area_dir = dir;
                  options.output_points_dir = dir;
                  options.geotiff_dir = dir;
                  options.area_dir = dir;
                  options.dnc_dir = dir;
                }
              options.input_pfm_dir = dir;
            }
        }


      //  This is pretty confusing.  What we're doing is finding out which attribute in the current PFM matches which possible attribute
      //  so that we can retrieve the attribute filter string associated with that attribute (not the attribute number of that attribute in this
      //  particular PFM).  That way, when we open another PFM, if the attributes are in a different order we can assign the correct 
      //  filter ranges to the correct attribute.  In pfmEdit3D this code is in envin but, since we don't open the PFM(s) prior to calling
      //  envin in pfmView we had to move it to this location.

      for (int32_t i = 0 ; i < NUM_ATTR ; i++)
        {
          uint8_t done = NVFalse;

          if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.time_attribute.name)
            {
              options.attrFilterText[i] = options.time_attr_filt;
              if (options.attrFilterName == options.time_attribute.name) misc.otf_attr = i;
              done = NVTrue;
            }

          if (!done)
            {
              for (int32_t j = 0 ; j < GSF_ATTRIBUTES ; j++)
                {
                  if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.gsf_attribute[j].name)
                    {
                      options.attrFilterText[i] = options.gsf_attr_filt[j];
                      if (options.attrFilterName == options.gsf_attribute[j].name) misc.otf_attr = i;
                      done = NVTrue;
                      break;
                    }
                }

              if (!done)
                {
                  for (int32_t j = 0 ; j < HOF_ATTRIBUTES ; j++)
                    {
                      if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.hof_attribute[j].name)
                        {
                          options.attrFilterText[i] = options.hof_attr_filt[j];
                          if (options.attrFilterName == options.hof_attribute[j].name) misc.otf_attr = i;
                          done = NVTrue;
                          break;
                        }
                    }

                  if (!done)
                    {
                      for (int32_t j = 0 ; j < TOF_ATTRIBUTES ; j++)
                        {
                          if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.tof_attribute[j].name)
                            {
                              options.attrFilterText[i] = options.tof_attr_filt[j];
                              if (options.attrFilterName == options.tof_attribute[j].name) misc.otf_attr = i;
                              done = NVTrue;
                              break;
                            }
                        }

                      if (!done)
                        {
                          for (int32_t j = 0 ; j < CZMIL_ATTRIBUTES ; j++)
                            {
                              if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.czmil_attribute[j].name)
                                {
                                  options.attrFilterText[i] = options.czmil_attr_filt[j];
                                  if (options.attrFilterName == options.czmil_attribute[j].name) misc.otf_attr = i;
                                  done = NVTrue;
                                  break;
                                }
                            }

                          if (!done)
                            {
                              for (int32_t j = 0 ; j < LAS_ATTRIBUTES ; j++)
                                {
                                  if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.las_attribute[j].name)
                                    {
                                      options.attrFilterText[i] = options.las_attr_filt[j];
                                      if (options.attrFilterName == options.las_attribute[j].name) misc.otf_attr = i;
                                      done = NVTrue;
                                      break;
                                    }
                                }

                              if (!done)
                                {
                                  for (int32_t j = 0 ; j < BAG_ATTRIBUTES ; j++)
                                    {
                                      if (QString (misc.abe_share->open_args[0].head.ndx_attr_name[i]) == options.bag_attribute[j].name)
                                        {
                                          options.attrFilterText[i] = options.bag_attr_filt[j];
                                          if (options.attrFilterName == options.bag_attribute[j].name) misc.otf_attr = i;
                                          break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


      //  Initialize the coverage and main maps

      initializeMaps ();


      //  Check to see if we have a GeoTIFF file associated with this PFM.

      if (!strcmp (misc.abe_share->open_args[0].image_path, "NONE"))
        {
          //  If the mosaic viewer was running, kill it.

          if (mosaicProc)
            {
              if (mosaicProc->state () == QProcess::Running)
                {
                  disconnect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
                  disconnect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));
                  disconnect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));

                  mosaicProc->kill ();
                }

              delete mosaicProc;
              mosaicProc = NULL;
            }

          startMosaic->setEnabled (false);
        }
      else
        {
          //  If the mosaic viewer was running, tell it to switch files.

          misc.abe_share->key = MOSAICVIEW_FORCE_RELOAD;

          startMosaic->setEnabled (true);
        }
    }
}



//!  Opens a recently used file based on the qaction that was assigned to that file.

void 
pfmView::slotOpenRecent (QAction *action)
{
  for (int32_t i = 0 ; i < MAX_RECENT ; i++)
    {
      if (recentFileAction[i] == action)
        {
          recent_file_flag = i + 1;
          break;
        }
    }
 openFiles (options.defaultFileOpen);
}



//!  Close one of the PFM layers.

void 
pfmView::slotClosePFM ()
{
  if (QMessageBox::information (this, tr ("pfmView Close PFM confirmation"), 
                                tr ("Do you really want to close\n%1").arg (QDir::toNativeSeparators (QString (misc.abe_share->open_args[0].list_path))),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;


  remove_layer (&misc, 0);

  if (!misc.abe_share->pfm_count) cov_area_defined = NVFalse;

  slotManageLayersDataChanged ();
}



void 
pfmView::slotDataTypeButtonsPressed (QMouseEvent *e)
{
  float yloc = (float) e->y () / (float) dataTypeButtons->height ();

  if (yloc <= 0.50)
    {
      options.editor_no_load[0] ^= 1;
    }
  else
    {
      options.editor_no_load[1] ^= 1;
    }

  setWidgetStates (NVTrue);
}



//!  This is where we define the bounds and other things associated with the open PFM(s).

void 
pfmView::initializeMaps ()
{
  //  Set up the PFM layers notebook tab filenames

  int32_t num_displayed = 0;
  statbook->setTabEnabled (2, false);
  for (int32_t pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
    {
      if (pfm < misc.abe_share->pfm_count)
        {
          QString name = QFileInfo (QString (misc.abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

          layerName[pfm]->setText (name);

          if (pfm) statbook->setTabEnabled (2, true);

          if (misc.abe_share->display_pfm[pfm])
            {
              num_displayed++;
              layerCheck[pfm]->setChecked (true);
            }
          else
            {
              layerCheck[pfm]->setChecked (false);
            }
          layerName[pfm]->show ();
          layerCheck[pfm]->show ();
        }
      else
        {
          layerName[pfm]->hide ();
          layerCheck[pfm]->hide ();
        }
    }


  //  Check to see if we have a MISP surface in the top PFM layer.

  if (misc.average_type[0] > 0)
    {
      bRemisp->setEnabled (true);
      bRemispFilter->setEnabled (true);
      bDrawContour->setEnabled (true);
      bDrawContourFilter->setEnabled (true);
      bClearFilterContours->setEnabled (true);
    }
  else
    {
      bRemisp->setEnabled (false);
      bRemispFilter->setEnabled (false);
      bDrawContour->setEnabled (false);
      bDrawContourFilter->setEnabled (false);
      bClearFilterContours->setEnabled (false);
    }


  if (misc.linked)
    {
      changeFileRegisterABE (misc.abeRegister, misc.abe_register, abe_register_group, misc.process_id,
                             misc.abe_share->open_args[0].list_path);
      QString title;
      title.sprintf ("pfmView (ABE group %02d) : %s", abe_register_group, misc.abe_share->open_args[0].list_path);
      this->setWindowTitle (title);
    }


  //  Compute the MBR of all of the PFM areas.

  compute_total_mbr (&misc);


  misc.cov_clear = NVTrue;


  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  NV_F64_XYMBR mbr = mapdef.bounds[mapdef.zoom_level];


  //  Disable the map signals, reset the bounds and then enable the map signals.

  map->disableSignals ();

  clearFilterMasks ();

  map->resetBounds (misc.total_mbr);
  map->enableSignals ();


  //  Don't want to draw coasts or grids when opening the file, but we need to open it to get a size for the coverage map.

  map->setCoasts (NVMAP_NO_COAST);
  map->setLandmask (NVFalse);
  map->setGridSpacing (0.0, 0.0);

  if (cov_area_defined)
    {
      zoomIn (mbr, NVFalse);
    }
  else
    {
      redrawMap (NVTrue, NVFalse);
    }

  map->setCoasts (options.coast);


  //  Set the overlay grid values

  slotGrid ();


  //  Get the size and other goodies

  mapdef = map->getMapdef ();


  //  Redefine the coverage map bounds.

  covdef.initial_bounds.min_x = misc.total_mbr.min_x;
  covdef.initial_bounds.min_y = misc.total_mbr.min_y;
  covdef.initial_bounds.max_x = misc.total_mbr.max_x;
  covdef.initial_bounds.max_y = misc.total_mbr.max_y;


  cov->resetBounds (covdef.initial_bounds);

  cov->enableSignals ();

  covdef = cov->getMapdef ();

  slotRedrawCoverage ();
}



//!  Open a GeoTIFF (image) file.

void 
pfmView::slotOpenGeotiff ()
{
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Open GeoTIFF"));
  fd->setViewMode (QFileDialog::List);

  if (!QDir (options.geotiff_dir).exists ()) options.geotiff_dir = options.input_pfm_dir;


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.geotiff_dir);


  filters << tr ("GeoTIFF (*.tif)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("GeoTIFF (*.tif)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          //  Check the file to make sure it is in the area

          GDALDataset        *poDataset;
          double             adfGeoTransform[6];
          double             GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, 
                             GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
          int32_t            width, height;


          GDALAllRegister ();


          char path[512];
          strcpy (path, file.toLatin1 ());

          poDataset = (GDALDataset *) GDALOpen (path, GA_ReadOnly);
          if (poDataset != NULL)
            {
              if (poDataset->GetProjectionRef ()  != NULL)
                {
                  QString projRef = QString (poDataset->GetProjectionRef ());

                  if (projRef.contains ("GEOGCS"))
                    {
                      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                        {
                          GeoTIFF_lon_step = adfGeoTransform[1];
                          GeoTIFF_lat_step = -adfGeoTransform[5];


                          width = poDataset->GetRasterXSize ();
                          height = poDataset->GetRasterYSize ();


                          GeoTIFF_wlon = adfGeoTransform[0];
                          GeoTIFF_nlat = adfGeoTransform[3];


                          GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
                          GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


                          if (GeoTIFF_nlat < misc.total_displayed_area.min_y || 
                              GeoTIFF_slat > misc.total_displayed_area.max_y ||
                              GeoTIFF_elon < misc.total_displayed_area.min_x || 
                              GeoTIFF_wlon > misc.total_displayed_area.max_x)
                            {
                              QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                                    tr ("No part of this GeoTIFF file falls within the displayed area."));
                              return;
                            }
                        }
                      else
                        {
                          QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                                tr ("This program only handles unprojected GeoTIFF files."));
                          return;
                        }
                    }
                  else
                    {
                      QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                            tr ("This program only handles geographic GeoTIFF files."));
                      return;
                    }
                }
              else
                {
                  QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                        tr ("This file has no datum/projection information."));
                  return;
                }
            }
          else
            {
              QMessageBox::warning (this, tr ("pfmView Open GeoTIFF"), 
                                    tr ("Unable to open file."));
              return;
            }

          delete poDataset;

          misc.GeoTIFF_open = NVTrue;
          bGeotiff->setEnabled (true);

          strcpy (misc.GeoTIFF_name, file.toLatin1 ());
          misc.GeoTIFF_init = NVTrue;

          bGeotiff->setEnabled (true);
          bGeotiff->setChecked (true);
          misc.display_GeoTIFF = NVTrue;
          redrawMap (NVTrue, NVFalse);

          options.geotiff_dir = fd->directory ().absolutePath ();
        }
    }
}



//!  Kick off the preferences dialog.

void 
pfmView::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();
  prefs_dialog = new prefs (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  prefs_dialog->move (x () + width () / 2 - prefs_dialog->width () / 2, y () + height () / 2 - prefs_dialog->height () / 2);

  connect (prefs_dialog, SIGNAL (dataChangedSignal (uint8_t)), this, SLOT (slotPrefDataChanged (uint8_t)));
  connect (prefs_dialog, SIGNAL (hotKeyChangedSignal (int32_t)), this, SLOT (slotPrefHotKeyChanged (int32_t)));
}



//!  Changed some of the preferences in the preferences dialog.

void 
pfmView::slotPrefDataChanged (uint8_t feature_search_changed)
{
  misc.GeoTIFF_init = NVTrue;


  bContour->setChecked (options.contour);
  bGrid->setChecked (options.overlay_grid);


  bAutoRedraw->setChecked (options.auto_redraw);
  bDisplaySuspect->setChecked (options.display_suspect);
  bDisplaySelected->setChecked (options.display_selected);
  bDisplayReference->setChecked (options.display_reference);
  bDisplayMinMax->setChecked (options.display_minmax);
  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);
  bDisplayChildren->setChecked (options.display_children);
  bDisplayFeatureInfo->setChecked (options.display_feature_info);
  bDisplayFeaturePoly->setChecked (options.display_feature_poly);


  for (int32_t i = 0 ; i < NUM_SURFACES ; i++) bSurface[i]->setChecked (false);

  bSurface[misc.abe_share->layer_type - 2]->setChecked (true);

 
 //  Set the tooltip for the bin size button just in case it changed.

  QString tip = tr ("Set the on-the-fly gridding parameters including attribute (%1) and bin size (%L2)").arg
    (options.attrFilterName).arg (options.otf_bin_size_meters, 3, 'f', 1);
  bSetOtfBin->setToolTip (tip);


  QString percentString = tr ("Highlight %L1 percent of depth").arg (options.highlight_percent, 0, 'f', 2);
  highlightPercent->setText (percentString);
  highlightCount->setText (tr ("Highlight valid bin count exceeding %1").arg (options.h_count));


  map->setCoastColor (options.coast_color);
  map->setBackgroundColor (options.background_color);
  map->setOverlapPercent (options.overlap_percent);


  slotRedrawCoverage ();


  //  Calling slotGrid will set all the possible overlay grid parameters as well as redrawing the map.  If you want to remove slotGrid just put
  //  the redrawMap call back in.

  if (!misc.drawing) slotGrid ();    //  redrawMap (NVTrue, NVFalse);


  //  Set a couple of things that pfmEdit3D will need to know.

  misc.abe_share->position_form = options.position_form;
  misc.abe_share->smoothing_factor = options.smoothing_factor;


  //  Check to see if we changed z_factor or z_offset.

  if (misc.abe_share->z_factor != options.z_factor || misc.abe_share->z_offset != options.z_offset) setStoplight ();


  misc.abe_share->z_factor = options.z_factor;
  misc.abe_share->z_offset = options.z_offset;


  for (int32_t i = 0 ; i < HOTKEYS ; i++) slotPrefHotKeyChanged (i);


  // Set the application font

  QApplication::setFont (options.font);


  //  If the feature search string changed we need to tell pfm3D (if it's running).

  if (feature_search_changed)
    {
      strcpy (misc.abe_share->feature_search_string, options.feature_search_string.toLatin1 ());
      misc.abe_share->feature_search_type = options.feature_search_type;
      misc.abe_share->feature_search_invert = options.feature_search_invert;

      if (threeDProc)
        {
          if (threeDProc->state () == QProcess::Running) misc.abe_share->key = PFM3D_FORCE_RELOAD;
        }
    }


  //  Check to see if we have Google Earth now.

  misc.googleearth_available = NVTrue;
  if (options.ge_name.isEmpty ())
    {
      misc.googleearth_available = NVFalse;
    }
  else
    {
      if (!QFile (options.ge_name).exists ()) misc.googleearth_available = NVFalse;
    }


  //  Save the user's defaults

  envout (&options, &misc, this);


  setWidgetStates (NVTrue);
}



//!  Changed some of the hot keys in the preferences dialog.

void 
pfmView::slotPrefHotKeyChanged (int32_t i)
{
  QString tip = misc.buttonText[i] + "  [" + options.buttonAccel[i] + "]";
  misc.button[i]->setToolTip (tip);


  //  We don't want to actually set a shortcut for the edit mode.  We're just using it to swicth back to edit mode from some other mode.
  //  Granted, this is a kludge but it works and Icommented it ;-)

  if (i != EDIT_MODE_KEY) misc.button[i]->setShortcut (QKeySequence (options.buttonAccel[i]));
}



//! Slot called when the user pressed one of the "highlight" buttons.

void
pfmView::slotHighlightMenu (QAction *action)
{
  setWidgetStates (NVFalse);


  if (action == highlightNone) options.highlight = H_NONE;
  if (action == highlightAll) options.highlight = H_ALL;
  if (action == highlightChecked) options.highlight = H_CHECKED;
  if (action == highlightMod) options.highlight = H_MODIFIED;
  if (action == highlightUser01) options.highlight = H_01;
  if (action == highlightUser02) options.highlight = H_02;
  if (action == highlightUser03) options.highlight = H_03;
  if (action == highlightUser04) options.highlight = H_04;
  if (action == highlightUser05) options.highlight = H_05;
  if (action == highlightUser06) options.highlight = H_06;
  if (action == highlightUser07) options.highlight = H_07;
  if (action == highlightUser08) options.highlight = H_08;
  if (action == highlightUser09) options.highlight = H_09;
  if (action == highlightUser10) options.highlight = H_10;
  if (action == highlightInt) options.highlight = H_INT;
  if (action == highlightMult) options.highlight = H_MULT;
  if (action == highlightCount) options.highlight = H_COUNT;
  if (action == highlightIHOS) options.highlight = H_IHO_S;
  if (action == highlightIHO1) options.highlight = H_IHO_1;
  if (action == highlightIHO2) options.highlight = H_IHO_2;
  if (action == highlightPercent) options.highlight = H_PERCENT;

  bHighlight->setIcon (highlightIcon[options.highlight]);


  redrawMap (NVTrue, NVFalse);
}



//!  Display suspect toggle function.

void
pfmView::slotDisplaySuspect ()
{
  if (bDisplaySuspect->isChecked ())
    {
      options.display_suspect = NVTrue;
    }
  else
    {
      options.display_suspect = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display features toggle function.

void
pfmView::slotFeatureMenu (QAction *action)
{
  for (int32_t i = 0 ; i < 4 ; i++)
    {
      if (action == displayFeature[i])
        {
          options.display_feature = i;
          break;
        }
    }

  bDisplayFeature->setIcon (displayFeatureIcon[options.display_feature]);

  if (options.display_feature)
    {
      bDisplayChildren->setEnabled (true);
      bDisplayFeatureInfo->setEnabled (true);
      bDisplayFeaturePoly->setEnabled (true);
      bAddFeature->setEnabled (true);
      bDeleteFeature->setEnabled (true);
      bEditFeature->setEnabled (true);
      bVerifyFeatures->setEnabled (true);
    }
  else
    {
      bDisplayChildren->setEnabled (false);
      bDisplayFeatureInfo->setEnabled (false);
      bDisplayFeaturePoly->setEnabled (false);
      bAddFeature->setEnabled (false);
      bDeleteFeature->setEnabled (false);
      bEditFeature->setEnabled (false);
      bVerifyFeatures->setEnabled (false);
    }

  redrawMap (NVTrue, NVFalse);
  misc.cov_clear = NVTrue;
  cov->redrawMap (NVTrue);
  redrawCovBounds ();
}



//!  Display feature children toggle function.

void
pfmView::slotDisplayChildren ()
{
  if (bDisplayChildren->isChecked ())
    {
      options.display_children = NVTrue;
    }
  else
    {
      options.display_children = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display feature text information toggle function.

void
pfmView::slotDisplayFeatureInfo ()
{
  if (bDisplayFeatureInfo->isChecked ())
    {
      options.display_feature_info = NVTrue;
    }
  else
    {
      options.display_feature_info = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display feature polygon toggle function.

void
pfmView::slotDisplayFeaturePoly ()
{
  if (bDisplayFeaturePoly->isChecked ())
    {
      options.display_feature_poly = NVTrue;
    }
  else
    {
      options.display_feature_poly = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display selected soundings toggle function.

void
pfmView::slotDisplaySelected ()
{
  if (bDisplaySelected->isChecked ())
    {
      options.display_selected = NVTrue;
    }
  else
    {
      options.display_selected = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display reference points toggle function.

void
pfmView::slotDisplayReference ()
{
  if (bDisplayReference->isChecked ())
    {
      options.display_reference = NVTrue;
    }
  else
    {
      options.display_reference = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Display minimum, maximum, and maximum standard deviation toggle function.

void
pfmView::slotDisplayMinMax ()
{
  if (bDisplayMinMax->isChecked ())
    {
      options.display_minmax = NVTrue;
    }
  else
    {
      options.display_minmax = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



/*!
  This is the completion from the unload dialog.
*/

void 
pfmView::slotUnloadDialogComplete (int32_t dialog, int32_t error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("Unable to start the unload process!"));
      delete unloadDlg[dialog];
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process crashed!"));
      delete unloadDlg[dialog];
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process timed out!"));
      delete unloadDlg[dialog];
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("There was a write error to the unload process!"));
      delete unloadDlg[dialog];
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("There was a read error from the unload process!"));
      delete unloadDlg[dialog];
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("The unload process died with an unknown error!"));
      delete unloadDlg[dialog];
      break;

    case -1:
      QMessageBox::critical (this, tr ("pfmView unload process"), tr ("An error occurred during unload, some or all of your edits were not saved!\n"
                                                                      "Please run pfm_unload separatedly to see what the error condition was."));
      delete unloadDlg[dialog];
      break;

    case -99:

      //  If we're auto closing on normal comletion, go ahead and get rid of the dialog.

      if (options.auto_close_unload)
        {
          delete unloadDlg[dialog];
          misc.unload_type[dialog] = -1;
        }


      //  Increment the count so we get the next PFM layer.

      unload_count[dialog]++;


      //  Check to see if we're done with layers.  If not, use the same dialog slot to kick off the next layer.

      if (unload_count[dialog] < misc.abe_share->pfm_count)
        {
          unloadDlg[dialog] = new unloadDialog (this, &options, &misc, dialog, unload_count[dialog]);
          connect (unloadDlg[dialog], SIGNAL (unloadCompleteSignal (int32_t, int32_t)), this, SLOT (slotUnloadDialogComplete (int32_t, int32_t)));
        }
      break;
    }
}



//!  Kick off an unload Dialog

void
pfmView::slotUnloadMenu (QAction *action)
{
  //  Find the first available unload dialog slot.

  int32_t unload_dlg = -1;
  for (int32_t i = 0 ; i < 100 ; i++)
    {
      if (misc.unload_type[i] < 0)
        {
          unload_dlg = i;
          break;
        }
    }


  //  Check to see if we have too many unloads running

  if (unload_dlg < 0)
    {
      QMessageBox::warning (this, tr ("pfmView Unload"), tr ("There are too many unload processes running at this time.\n"
                                                             "Please close some of the unload dialogs and retry."));
      return;
    }


  if (action == unloadDisplayed)
    {
      //  Confirm the unload for the displayed area

      if (QMessageBox::information (this, tr ("pfmView Unload data confirmation"), 
                                    tr ("Do you really want to unload all edits for the displayed area?"),
                                    QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

      misc.unload_type[unload_dlg] = 1;
      misc.unload_area[unload_dlg] = misc.total_displayed_area;
    }
  else
    {
      //  Confirm the unload for the PFM area

      if (QMessageBox::information (this, tr ("pfmView Unload data confirmation"), 
                                    tr ("Do you really want to unload all edits for the entire PFM?"),
                                    QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

      misc.unload_type[unload_dlg] = 0;
    }


  //  Redraw the coverage map prior to grabbing a pixmap of it.

  slotRedrawCoverage ();
  misc.unload_cov_map = QPixmap (cov->size ());
  cov->render (&misc.unload_cov_map);


  unload_count[unload_dlg] = 0;


  unloadDlg[unload_dlg] = new unloadDialog (this, &options, &misc, unload_dlg, unload_count[unload_dlg]);
  unloadDlg[unload_dlg]->setModal (false);
  connect (unloadDlg[unload_dlg], SIGNAL (unloadCompleteSignal (int32_t, int32_t)), this, SLOT (slotUnloadDialogComplete (int32_t, int32_t)));
}



//!  This is the stderr read return from the CUBE QProcess.

void 
pfmView::slotCubeReadyReadStandardError ()
{
  readStandardError (cubeProc);
}



//!  This is the stdout read return from the CUBE QProcess.

void 
pfmView::slotCubeReadyReadStandardOutput ()
{
  static QString resp_string = "";


  QByteArray response = cubeProc->readAllStandardOutput ();


  //  Parse the return response for carriage returns and line feeds

  for (int32_t i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n' || response.at (i) == '\r')
        {
          if (resp_string.contains ("%"))
            {
              char resp[512];
              strcpy (resp, resp_string.toLatin1 ());

              int32_t value;
              sscanf (resp, "%d", &value);
              misc.statusProg->setValue (value);
            }

	  if (resp_string.contains ("Processing Entire PFM") || resp_string.contains ("Processing PFM sub-area"))
            {
              misc.statusProgLabel->setText (tr (" PFM CUBE processing (pass 1 of 2) "));
              misc.statusProg->reset ();
            }

          if (resp_string.contains ("Extract Best Hypothesis"))
            {
              misc.statusProgLabel->setText (tr (" Extracting best CUBE hypotheses (pass 2 of 2) "));
              misc.statusProg->reset ();
            }

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



//!  Error return from the CUBE process.

void 
pfmView::slotCubeError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("Unable to start the CUBE process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("There was a write error from the CUBE process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("There was a read error from the CUBE process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("The CUBE process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the CUBE QProcess (when finished normally)

void 
pfmView::slotCubeDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  misc.statusProg->reset ();
  misc.statusProgLabel->setVisible (false);
  misc.statusProg->setTextVisible (false);
  qApp->processEvents();

  if (cubeProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView CUBE"), tr ("An error occurred during CUBE processing!"
                                                            "\nPlease run pfmCube separatedly to see what the error condition was."));
    }
  if (misc.color_index) redrawMap (NVTrue, NVFalse);
}



//!  Kick off the CUBE QProcess from the CUBE button

void
pfmView::slotCube ()
{
  runCube (NVTrue);
}



//!  Kick off the CUBE QProcess

void 
pfmView::runCube (uint8_t displayed_area)
{
  if (displayed_area)
    {
      //  Confirm the cube

      if (QMessageBox::information (this, tr ("pfmView CUBE data confirmation"), 
                                    tr ("Do you really want to CUBE process the displayed area?"),
                                    QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;
    }


  //  Only start it if we have the (SAIC proprietary) CUBE program available and we have CUBE attributes in the PFM

  if (misc.cube_available && misc.cube_attr_available[0])
    {
      cubeProc = new QProcess (this);

      QStringList arguments;
      QString arg;


      arg.sprintf ("""%s""", misc.abe_share->open_args[0].list_path);
      arguments += arg;


      //  Run for entire displayed area if called from slotCube.

      if (displayed_area)
        {
          arg.sprintf ("%.9f", misc.total_displayed_area.min_x);
          arguments += arg;

          arg.sprintf ("%.9f", misc.total_displayed_area.min_y);
          arguments += arg;

          arg.sprintf ("%.9f", misc.total_displayed_area.max_x);
          arguments += arg;

          arg.sprintf ("%.9f", misc.total_displayed_area.max_y);
          arguments += arg;
        }


      //  Run for only the edited area.

      else
        {
          arg.sprintf ("%.9f", misc.abe_share->edit_area.min_x);
          arguments += arg;

          arg.sprintf ("%.9f", misc.abe_share->edit_area.min_y);
          arguments += arg;

          arg.sprintf ("%.9f", misc.abe_share->edit_area.max_x);
          arguments += arg;

          arg.sprintf ("%.9f", misc.abe_share->edit_area.max_y);
          arguments += arg;
        }

      arguments += arg.setNum (options.iho);
      arguments += arg.setNum (options.capture);
      arguments += arg.setNum (options.queue);
      arguments += arg.setNum (options.horiz);
      arguments += arg.setNum (options.distance);
      arguments += arg.setNum (options.min_context);
      arguments += arg.setNum (options.max_context);
      arguments += arg.setNum (options.std2conf + 1);
      arguments += arg.setNum (options.disambiguation);


      misc.statusProg->setRange (0, 100);
      misc.statusProgLabel->setText (tr (" CUBE processing "));
      misc.statusProgLabel->setVisible (true);
      misc.statusProg->setTextVisible (true);
      qApp->processEvents ();


      connect (cubeProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotCubeDone (int, QProcess::ExitStatus)));
      connect (cubeProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotCubeReadyReadStandardError ()));
      connect (cubeProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotCubeReadyReadStandardOutput ()));
      connect (cubeProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotCubeError (QProcess::ProcessError)));

      cubeProc->start (QString (cube_name), arguments);
    }
}



//!  This function determines if we are viewing a valid or invalid surface.

void 
pfmView::setSurfaceValidity (int32_t layer)
{
  if (cov_area_defined)
    {
      //  Determine if the surface layer that we are displaying is an edited or unedited surface.

      if (layer == AVERAGE_FILTERED_DEPTH || layer == MIN_FILTERED_DEPTH || layer == MAX_FILTERED_DEPTH || layer == NUM_LAYERS) 
        {
          misc.surface_val = NVTrue;
        }
      else
        {
          misc.surface_val = NVFalse;
        }


      if (misc.surface_val != misc.prev_surface_val)
        {
          misc.prev_surface_val = misc.surface_val;
          if (cov) slotRedrawCoverage ();
        }
    }
}



//!  Slot called when we have finished setting a lock value for the min or max color scale.

void 
pfmView::slotLockValueDone (uint8_t accepted)
{
  if (accepted)
    {
      //  We have to reset the colors here because we might have changed the min or max color.

      int32_t k = misc.color_index;

      float hue_inc = (float) (options.max_hsv_color[k] - options.min_hsv_color[k]) / (float) (NUMHUES + 1);
      for (int32_t m = 0 ; m < 2 ; m++)
        {
          int32_t sat = 255;
          if (m) sat = 25;

          for (int32_t i = 0 ; i < NUMHUES ; i++)
            {
              for (int32_t j = 0 ; j < NUMSHADES ; j++)
                {
                  options.color_array[m][i][j].setHsv ((int32_t) (((NUMHUES + 1) - (i - options.min_hsv_color[k])) * hue_inc), sat, j, 255);
                }
            }
        }


      //  If we changed the depth colors, locks, or values (index 0) then we need to set shared memory to inform the editors when they start up.

      if (!k)
        {
          misc.abe_share->min_hsv_color = options.min_hsv_color[0];
          misc.abe_share->max_hsv_color = options.max_hsv_color[0];
          misc.abe_share->min_hsv_value = options.min_hsv_value[0];
          misc.abe_share->max_hsv_value = options.max_hsv_value[0];
          misc.abe_share->min_hsv_locked = options.min_hsv_locked[0];
          misc.abe_share->max_hsv_locked = options.max_hsv_locked[0];
        }


      redrawMap (NVTrue, NVTrue);
      slotRedrawCoverage ();
    }
}



//!  Slot called if we pressed the minimum color scale "button".

void 
pfmView::slotMinScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.drawing) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVTrue, misc.color_index);
  connect (lck, SIGNAL (lockValueDone (uint8_t)), this, SLOT (slotLockValueDone (uint8_t)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



//!  Slot called if we pressed the maximum color scale "button".

void 
pfmView::slotMaxScalePressed (QMouseEvent *e __attribute__ ((unused)))
{
  if (misc.drawing) return;

  lockValue *lck = new lockValue (map, &options, &misc, NVFalse, misc.color_index);
  connect (lck, SIGNAL (lockValueDone (uint8_t)), this, SLOT (slotLockValueDone (uint8_t)));


  //  Put the dialog in the middle of the screen.

  lck->move (x () + width () / 2 - lck->width () / 2, y () + height () / 2 - lck->height () / 2);
}



//!  Slot called to toggle display of a PFM layer.

void 
pfmView::slotLayerClicked (int id)
{
  if (layerCheck[id]->checkState () == Qt::Checked)
    {
      misc.abe_share->display_pfm[id] = NVTrue;
    }
  else
    {
      misc.abe_share->display_pfm[id] = NVFalse;
    }

  redrawMap (NVTrue, NVTrue);
  slotRedrawCoverage ();
}



//!  Toggle display of stoplight colors.

void
pfmView::slotStoplight ()
{
  if (bStoplight->isChecked ())
    {
      options.stoplight = NVTrue;


      //  See if we need to reset the stoplight crossover colors.

      if (options.stoplight_min_mid == options.stoplight_max_mid) setStoplight ();
    }
  else
    {
      options.stoplight = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of contours.

void
pfmView::slotContour ()
{
  if (bContour->isChecked ())
    {
      options.contour = NVTrue;
    }
  else
    {
      options.contour = NVFalse;
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of overlay grid.

void
pfmView::slotGrid ()
{
  double spacing_x = 0.0, spacing_y = 0.0;

  if (bGrid->isChecked ())
    {
      options.overlay_grid = NVTrue;


      //  Grid spacing in decimal minutes

      if (options.overlay_grid_type)
        {
          spacing_x = spacing_y = options.overlay_grid_minutes;
        }


      //  Grid spacing in meters (nominal)

      else
        {
          double new_x, new_y;


          //  Compute the grid spacing in decimal minutes at the center of the displayed area (based on grid spacing in meters).

          mapdef = map->getMapdef ();
          newgp (mapdef.center_y, mapdef.center_x, 90.0, options.overlay_grid_spacing, &new_y, &new_x);
          spacing_x = (new_x - mapdef.center_x) * 60.0;
          newgp (mapdef.center_y, mapdef.center_x, 0.0, options.overlay_grid_spacing, &new_y, &new_x);
          spacing_y = (new_y - mapdef.center_y) * 60.0;
        }
    }
  else
    {
      options.overlay_grid = NVFalse;
    }


  //  Set the nvMap options

  map->setGridColor (options.overlay_grid_color);
  map->setGridSpacing (spacing_x, spacing_y);
  map->setGridThickness (options.overlay_grid_width);


  if (cov_area_defined) redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of coastlines in the main map.

void
pfmView::slotCoast ()
{
  if (bCoast->isChecked ())
    {
      options.coast = NVTrue;
      map->setCoasts (NVMAP_AUTO_COAST);
    }
  else
    {
      options.coast = NVFalse;
      map->setCoasts (NVMAP_NO_COAST);
    }

  redrawMap (NVTrue, NVFalse);
}



//!  Toggle display of coastlines in the coverage map.

void
pfmView::slotCovCoast ()
{
  if (bCovCoast->isChecked ())
    {
      options.cov_coast = NVTrue;
    }
  else
    {
      options.cov_coast = NVFalse;
    }

  slotRedrawCoverage ();
}



//!  Toggle display of GeoTIFF.

void
pfmView::slotGeotiff ()
{
  if (!misc.display_GeoTIFF)
    {
      misc.display_GeoTIFF = 1;
      bGeotiff->setIcon (QIcon (":/icons/geo.png"));
    }
  else if (misc.display_GeoTIFF > 0)
    {
      misc.display_GeoTIFF = -1;
      bGeotiff->setIcon (QIcon (":/icons/geo_under.png"));
    }
  else
    {
      misc.display_GeoTIFF = 0;
      bGeotiff->setIcon (QIcon (":/icons/geo_off.png"));
    }
  qApp->processEvents ();

  misc.clear = NVTrue;
  redrawMap (NVTrue, NVFalse);
}



//!  Stop drawing and kill pfmEdit3D if it is running.

void
pfmView::slotStop ()
{
  misc.clear = NVTrue;
  misc.drawing_canceled = NVTrue;

  if (pfm_edit_active) 
    {
      //  Set the PFMEDIT_KILL flag.

      misc.abe_share->key = PFMEDIT_KILL;


      //  Wait 2 seconds so that pfmEdit3D will see the flag.

#ifdef NVWIN3X
      Sleep (2000);
#else
      sleep (2);
#endif


      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED; 


      pfmEdit_stopped = NVTrue;


      //  Make sure it's really dead.

      editProc->kill ();

      setWidgetStates (NVTrue);

      pfm_edit_active = NVFalse;


      //  Make sure we reset the important things in case we did a partial redraw.

      setFunctionCursor (misc.function);
      if (misc.function == RECT_EDIT_AREA_3D)
        {
          map->discardRubberbandRectangle (&rb_rectangle);
        }
      else
        {
          map->discardRubberbandPolygon (&rb_polygon);
        }

      setWidgetStates (NVTrue);
      misc.drawing = NVFalse;
    }
}



//!  We changed a feature in the feature file (BFD).  This is usally done in pfmEdit3D.

void 
pfmView::slotEditFeatureDataChanged ()
{
  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  misc.add_feature_index = -1;


  redrawMap (NVTrue, NVTrue);


  misc.cov_clear = NVTrue;
  cov->redrawMap (NVTrue);


  //  If the mosaic viewer or 3D viewer was running, tell it to redraw.

  misc.abe_share->key = FEATURE_FILE_MODIFIED;
}



//!  Slot called when the user wants to define a feature polygon from the feature edit dialog.

void 
pfmView::slotEditFeatureDefinePolygon ()
{
  map->setToolTip (tr ("Left click to define polygon vertices, double click to define last vertex, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = DEFINE_FEATURE_POLY_AREA;
  setFunctionCursor (misc.function);
}



//!  Bring up the feature edit dialog for feature record number "feature_number".

void 
pfmView::editFeatureNum (int32_t feature_number)
{
  if (options.chart_scale)
    {
      misc.feature_poly_radius = ((options.millimeters / 1000.0) * options.chart_scale) / 2.0;
    }
  else
    {
      //  Default to 1:5000

      misc.feature_poly_radius = ((options.millimeters / 1000.0) * 5000.0) / 2.0;
    }


  redrawMap (NVTrue, NVTrue);
  qApp->processEvents ();


  editFeature_dialog = new editFeature (this, &options, &misc, feature_number);
  connect (editFeature_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotEditFeatureDataChanged ()));
  connect (editFeature_dialog, SIGNAL (definePolygonSignal ()), this, SLOT (slotEditFeatureDefinePolygon ()));
  connect (editFeature_dialog, SIGNAL (chartScaleChangedSignal ()), this, SLOT (slotDefinePolygonChartScaleChanged ()));
  connect (editFeature_dialog, SIGNAL (readStandardErrorSignal (QProcess *)), this, SLOT (slotReadStandardError (QProcess *)));

  editFeature_dialog->setModal (true);

  editFeature_dialog->show ();
}



//!  We pushed one of the buttons to display one of the surfaces (min, max, avg, OTF min, ...).

void
pfmView::slotSurface (int id)
{
  if (id != NUM_LAYERS)
    {
      misc.clear = NVTrue;


      //  I added OTF_OFFSET to the MIN_FILTERED_DEPTH, MAX_FILTERED_DEPTH, AVERAGE_FILTERED_DEPTH, MIN_DEPTH, MAX_DEPTH, and
      //  AVERAGE_DEPTH values to indicate that I want to do an on-the-fly binned surface.

      if (id > OTF_OFFSET)
        {
          misc.otf_surface = NVTrue;
          id -= OTF_OFFSET;
        }
      else
        {
          misc.otf_surface = NVFalse;

          if (misc.otf_grid != NULL)
            {
              misc.otfShare->detach ();
              misc.otf_grid = NULL;
              misc.abe_share->otf_width = misc.abe_share->otf_height = 0;
            }
        }


      misc.abe_share->layer_type = options.layer_type = id;
    }

  redrawMap (NVTrue, NVTrue);
}



//!  Slot called when the user wants to redefine the OTF bin size and/or the attribute and attribute limits (they pushed the OTF bin size button).

void 
pfmView::slotSetOtfBin ()
{
  if (otf_dialog) otf_dialog->close ();
  otf_dialog = new otfDialog (this, &options, &misc);


  connect (otf_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotOtfDataChanged ()));
}



void 
pfmView::slotOtfDataChanged ()
{
  if (misc.otf_surface) redrawMap (NVTrue, NVTrue);

  setWidgetStates (NVTrue);
}



//!  Mark all displayed features as valid (button slot).

void
pfmView::slotVerifyFeatures ()
{
  int32_t ret = QMessageBox::Yes;
  ret = QMessageBox::information (this, "pfmView", tr ("Are you sure you want to verify all displayed features?"),
                                  QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (ret == QMessageBox::No) return;


  setWidgetStates (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  misc.statusProg->setRange (0, misc.bfd_header.number_of_records);
  misc.statusProgLabel->setText (tr (" Verifying features "));
  misc.statusProgLabel->setVisible (true);
  misc.statusProg->setTextVisible (true);
  qApp->processEvents();


  BFDATA_RECORD bfd_record;

  for (uint32_t i = 0 ; i < misc.bfd_header.number_of_records ; i++)
    {
      misc.statusProg->setValue (i);
      if (misc.feature[i].confidence_level)
	{
	  if (misc.feature[i].latitude >= misc.total_displayed_area.min_y && misc.feature[i].latitude <= misc.total_displayed_area.max_y &&
              misc.feature[i].longitude >= misc.total_displayed_area.min_x && misc.feature[i].longitude <= misc.total_displayed_area.max_x)
	    {
	      binaryFeatureData_read_record (misc.bfd_handle, i, &bfd_record);

	      bfd_record.confidence_level = 5;

	      binaryFeatureData_write_record (misc.bfd_handle, i, &bfd_record, NULL, NULL);
	    }
	}
    }


  if (misc.bfd_open) binaryFeatureData_close_file (misc.bfd_handle);
  misc.bfd_open = NVFalse;

  if ((misc.bfd_handle = binaryFeatureData_open_file (misc.abe_share->open_args[0].target_path, &misc.bfd_header, BFDATA_UPDATE)) >= 0)
    {
      if (binaryFeatureData_read_all_short_features (misc.bfd_handle, &misc.feature) < 0)
	{
	  QString msg = QString (binaryFeatureData_strerror ());
	  QMessageBox::warning (this, "pfmView", tr ("Unable to read feature records\nReason: %1").arg (msg));
	  binaryFeatureData_close_file (misc.bfd_handle);
	}
      else
	{
	  misc.bfd_open = NVTrue;
	}
    }


  misc.statusProg->reset ();
  misc.statusProgLabel->setVisible (false);
  misc.statusProg->setTextVisible (false);

  qApp->restoreOverrideCursor ();
  qApp->processEvents ();

  redrawMap (NVTrue, NVTrue);

  if (options.display_feature)
    {
      misc.cov_clear = NVTrue;
      cov->redrawMap (NVTrue);
    }
}



/*!
  Clear all contours drawn for filtering purposes.  This is one of the PFM "grid" functions so it should be
  deleted eventually.
*/

void
pfmView::slotClearFilterContours ()
{
  misc.filt_contour.clear ();
  misc.filt_contour_count = 0;

  redrawMap (NVTrue, NVTrue);
}



//!  Latitude button was pushed to change to/from northing/easting.

void 
pfmView::slotLatitudeClicked (QMouseEvent *e __attribute__ ((unused)), int id __attribute__ ((unused)))
{
  options.lat_or_east ^= 1;
  setWidgetStates (NVTrue);
}



//!  An Attribute tab attribute button was clicked.

void 
pfmView::slotAttrBoxClicked (QMouseEvent *e __attribute__ ((unused)), int id)
{
  QString name = QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[id]));


  //  If it was on, turn it off...

  if (misc.attrBoxFlag[id])
    {
      //  It's not polite to turn off the color-by attribute...

      if (misc.color_index >= PRE_ATTR && misc.attrStatNum[misc.color_index - PRE_ATTR] == id)
        {
          QMessageBox::warning (this, tr ("pfmView"), tr ("You cannot unselect the attribute that is being used for the color scale."));
          return;
        }


      for (int32_t i = 0 ; i < POST_ATTR ; i++)
        {
          int32_t j = misc.attrStatNum[i];

          if (name == pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[j]))
            {
              misc.attrStatNum[i] = -1;
              options.attrStatName[i] = "";
              attrName[i + PRE_ATTR]->setText (tr ("No attribute selected"));
              attrValue[i + PRE_ATTR]->setText ("");
              misc.attrBoxFlag[id] = 0;
            }
        }
    }
  else
    {
      //  Check to see if we have any open slots...

      int32_t hitcnt = 0;
      for (int32_t i = 0 ; i < NUM_ATTR ; i++) hitcnt += misc.attrBoxFlag[i];

      if (hitcnt == POST_ATTR)
        {
          QMessageBox::warning (this, tr ("pfmView"), tr ("All attributes on the Status tab are already assigned.\n"
                                                          "You must unselect one or more of the selected attributes."));
          return;
        }


      //  Find the first open slot on the Status tab...

      for (int32_t i = 0 ; i < POST_ATTR ; i++)
        {
          if (misc.attrStatNum[i] == -1)
            {
              misc.attrStatNum[i] = id;
              options.attrStatName[i] = name;

              attrName[i + PRE_ATTR]->setText (options.attrStatName[i]);
              attrName[i + PRE_ATTR]->setToolTip (options.attrStatName[i] +
                                                  QString (tr (" - To change, select attribute on Attributes tab.")));
              attrName[i + PRE_ATTR]->setWhatsThis (options.attrStatName[i] +
                                                    QString (tr (" - To change, select attribute on Attributes tab.")));
              attrValue[i + PRE_ATTR]->setToolTip (tr ("This is the value of %1 at the cursor").arg (options.attrStatName[i]));
              break;
            }
        }

      misc.attrBoxFlag[id] = 1;
    }


  setWidgetStates (NVTrue);
}



//!  Attribute button was pushed to change "color by" option.

void 
pfmView::slotColorByClicked (QMouseEvent *e __attribute__ ((unused)), int id)
{
  misc.color_index = id;


  //  We have to reset the colors here in case the "color by" attribute has a different color range than the last 
  //  option.

  float hue_inc = (float) (options.max_hsv_color[id] - options.min_hsv_color[id]) / (float) (NUMHUES + 1);
  for (int32_t m = 0 ; m < 2 ; m++)
    {
      int32_t sat = 255;
      if (m) sat = 25;

      for (int32_t i = 0 ; i < NUMHUES ; i++)
        {
          for (int32_t j = 0 ; j < NUMSHADES ; j++)
            {
              options.color_array[m][i][j].setHsv ((int32_t) (((NUMHUES + 1) - (i - options.min_hsv_color[id])) * hue_inc), sat, j, 255);
            }
        }
    }


  setStoplight ();

  redrawMap (NVTrue, NVFalse);
}



//!  Slot called when zoom out button is pressed.

void
pfmView::slotZoomOut ()
{
  mapdef = map->getMapdef ();
  if (mapdef.zoom_level)
    {
      misc.clear = NVTrue;
      misc.GeoTIFF_init = NVTrue;

      zoomOut ();

      redrawCovBounds ();

      mapdef = map->getMapdef ();
      if (mapdef.zoom_level == 0) bZoomOut->setEnabled (false);

      misc.cov_function = COV_START_DRAW_RECTANGLE;
    }
}



//!  Slot called when zoom in button is pressed.  Puts us into ZOOM_IN_AREA mode.

void
pfmView::slotZoomIn ()
{
  map->setToolTip (tr ("Left click to start rectangle, middle click to abort, right click for menu"));
  misc.function = ZOOM_IN_AREA;
  setFunctionCursor (misc.function);
}



//!  Chart toolbar defaultSize clicked.

void 
pfmView::slotDefaultSizeClicked ()
{
  if (misc.drawing) return;

  QString string = QString (" %L1, %L2").arg (options.default_width, 0, 'f', 2).arg (options.default_height, 0, 'f', 2);

  displayedArea->setText (string);

  slotDisplayedAreaReturnPressed ();
}



//!  Return pressed in the Chart toolbar displayedArea QLineEdit.

void 
pfmView::slotDisplayedAreaReturnPressed ()
{
  if (misc.drawing) return;

  QString text = displayedArea->text ();


  double rect_width = 0.0, rect_height = 0.0;


  uint8_t err = NVFalse;

  for (int32_t i = 0 ; i < text.length () ; i++)
    {
      if (!(text.at (i).isDigit ()))
        {
          if (text.at (i) != ',' && text.at (i) != '.' && text.at (i) != ' ')
            {
              err = NVTrue;
            }
        }
    }

  if (!err)
    {
      if (!text.contains (','))
        {
          err = NVTrue;
        }
      else
        {
          QString rectWidth = text.section (',', 0, 0);
          QString rectHeight = text.section (',', 1, 1);

          if (rectWidth.isEmpty () || rectHeight.isEmpty ())
            {
              err = NVTrue;
            }
          else
            {
              rect_width = rectWidth.toDouble ();
              rect_height = rectHeight.toDouble ();

              if (rect_width < (2.0 * misc.abe_share->open_args[0].head.bin_size_xy) || rect_height < (2.0 * misc.abe_share->open_args[0].head.bin_size_xy))
                {
                  err = NVTrue;
                }
            }
        }
    }

  if (err)
    {
      QMessageBox::warning (this, tr ("Chart tool bar"), tr ("The data entered in the Chart tool bar width/height field is invalid!\n"
                                                             "Please read the help documentation for this field."));

      redrawMap (NVTrue, NVFalse);
    }
  else
    {
      //  Make sure we don't reset the scale when redrawing (slotPostRedraw).

      scale_loop = NVTrue;


      //  Compute and MBR based on the center of the displayed area and the user entered width and height.

      NV_F64_XYMBR mbr;
      double x, y;


      //  Divide by two

      rect_width /= 2.0;
      rect_height /= 2.0;


      mapdef = map->getMapdef ();
      newgp (mapdef.center_y, mapdef.center_x, 270.0, rect_width, &y, &mbr.min_x);
      newgp (mapdef.center_y, mapdef.center_x, 90.0, rect_width, &y, &mbr.max_x);
      newgp (mapdef.center_y, mapdef.center_x, 180.0, rect_height, &mbr.min_y, &x);
      newgp (mapdef.center_y, mapdef.center_x, 0.0, rect_height, &mbr.max_y, &x);


      //  Make sure we don't exceed our PFM bounds.

      if (mbr.min_x < misc.total_mbr.min_x) mbr.min_x = misc.total_mbr.min_x;
      if (mbr.max_x > misc.total_mbr.max_x) mbr.max_x = misc.total_mbr.max_x;
      if (mbr.min_y < misc.total_mbr.min_y) mbr.min_y = misc.total_mbr.min_y;
      if (mbr.max_y > misc.total_mbr.max_y) mbr.max_y = misc.total_mbr.max_y;


      //  Now zoom to the MBR

      zoomIn (mbr, NVTrue);


      misc.cov_function = COV_START_DRAW_RECTANGLE;


      redrawCovBounds ();
    }


  //  Reset the focus back to the map otherwise it gets left in the width/height QLineEdit.

  map->setFocus (Qt::OtherFocusReason);
}



//!  Chart toolbar defaultScale clicked.

void 
pfmView::slotDefaultScaleClicked ()
{
  if (misc.drawing) return;

  QString string = QString ("1:%1").arg (options.default_chart_scale);

  viewChartScale->setText (string);

  slotViewChartScaleReturnPressed ();
}



//!  Return pressed in the Chart toolbar viewChartScale QLineEdit.

void 
pfmView::slotViewChartScaleReturnPressed ()
{
  if (misc.drawing) return;

  QString text = viewChartScale->text ();


  double scale_value = 0.0;


  uint8_t err = NVFalse;

  for (int32_t i = 0 ; i < text.length () ; i++)
    {
      if (!(text.at (i).isDigit ()))
        {
          if (text.at (i) != ':' && text.at (i) != ' ')
            {
              err = NVTrue;
            }
        }
    }

  if (!err)
    {
      if (!text.contains (':'))
        {
          err = NVTrue;
        }
      else
        {
          QString scale = text.section (':', 1, 1);

          if (scale.isEmpty ())
            {
              err = NVTrue;
            }
          else
            {
              scale_value = scale.toDouble ();

              if (scale_value < 100.0)
                {
                  err = NVTrue;
                }
            }
        }
    }

  if (err)
    {
      QMessageBox::warning (this, tr ("Chart tool bar"), tr ("The data entered in the Chart tool bar chart scale field is invalid!\n"
                                                             "Please read the help documentation for this field."));

      redrawMap (NVTrue, NVFalse);
    }
  else
    {
      //  Make sure we don't reset the scale when redrawing (slotPostRedraw).

      scale_loop = NVTrue;


      //  Compute an MBR based on the center of the displayed area and the user entered chart scale.
      //  We're making the assumption that a pixel is square (you know what they say about assuming ;-)

      int32_t dpi_x = qApp->desktop ()->physicalDpiX ();
      //int32_t dpi_y = qApp->desktop ()->physicalDpiY ();
      float dpm_x = dpi_x * 39.37;
      //float dpm_y = dpi_y * 39.37;


      mapdef = map->getMapdef ();


      //  Set the height and width equal to each other in distance.

      double rect_width = (double) scale_value / (1.0 / ((double) mapdef.width / dpm_x));
      double rect_height = rect_width;


      //  Use the newly computed width and height to set the width and height field.

      QString string = QString (" %L1, %L2").arg (rect_width, 0, 'f', 2).arg (rect_height, 0, 'f', 2);

      displayedArea->setText (string);

      slotDisplayedAreaReturnPressed ();
    }


  //  Reset the focus back to the map otherwise it gets left in the width/height QLineEdit.

  map->setFocus (Qt::OtherFocusReason);
}



//!  Filter the displayed area using the selected filter constraints from the preferences.

void
pfmView::slotFilterDisplay ()
{
  double              mx[4], my[4];

  void filterPolyArea (OPTIONS *options, MISC *misc, double *mx, double *my, int32_t count);
  void filterUndo (MISC *misc);


  //  Don't run the filter if the user is displaying more bins than pixels regardless of the tposiafps state.

  if (misc.displayed_area_width[0] > mapdef.draw_width || misc.displayed_area_height[0] > mapdef.draw_height)
    {
      QMessageBox::critical (this, tr ("pfmView Filter"), tr ("Displayed area filter not allowed when displaying more bins than pixels!\n"
                                                              "Please reduce the size of the viewed area in order to filter it."));
      return;
    }


  mx[0] = misc.total_displayed_area.min_x;
  my[0] = misc.total_displayed_area.min_y;
  mx[1] = misc.total_displayed_area.min_x;
  my[1] = misc.total_displayed_area.max_y;
  mx[2] = misc.total_displayed_area.max_x;
  my[2] = misc.total_displayed_area.max_y;
  mx[3] = misc.total_displayed_area.max_x;
  my[3] = misc.total_displayed_area.min_y;

  misc.filtered_count = 0;


  //  Using a 4 point polygon to avoid writing new filter code.

  filterPolyArea (&options, &misc, mx, my, 4);

  redrawMap (NVTrue, NVTrue);

  if (misc.filtered_count)
    {
      QMessageBox msgBox;
      msgBox.setWindowTitle (tr ("pfmView Filter"));
      msgBox.setText (tr ("%1 points marked invalid").arg (misc.filtered_count));
      msgBox.setInformativeText ("Do you want to save your changes?");
      msgBox.setStandardButtons (QMessageBox::Save | QMessageBox::Discard);
      msgBox.setDefaultButton (QMessageBox::Save);


      //  This is weird.  If I let the message box be modal, nothing else happens but it greys out the window that you
      //  need to look in order to verify that you want to accept the changes.  If I make it non-modal it will leave the 
      //  window alone but you can start clicking buttons and other bad stuff.  So, to make it sort of work, I'm setting
      //  the misc.drawing flag so that everything else will be ignored until you answer the massage box.  I also have
      //  to use the "Qt::WindowStaysOnTopHint" to keep it from disappearing into the background if you click on the main
      //  window.

      misc.drawing = NVTrue;
      setWidgetStates (NVFalse);

      msgBox.setWindowFlags (msgBox.windowFlags () | Qt::WindowStaysOnTopHint);
      msgBox.setModal (false);


      //  Move the dialog to the center of the parent if possible.

      QRect tmp = frameGeometry ();
      int32_t window_x = tmp.x ();
      int32_t window_y = tmp.y ();


      tmp = geometry ();
      int32_t width = tmp.width ();
      int32_t height = tmp.height ();


      QSize dlg = msgBox.sizeHint ();
      int32_t dlg_width = dlg.width ();

      msgBox.move (window_x + width / 2 - dlg_width / 2, window_y + height / 2);

      msgBox.show ();

      int ret = msgBox.exec ();

      misc.drawing = NVFalse;
      setWidgetStates (NVTrue);


      if (ret == QMessageBox::Discard)
        {
          filterUndo (&misc);
          redrawMap (NVTrue, NVTrue);
        }


      //  Clear the undo memory;

      if (misc.filtered_count)
        {
          misc.undo.clear ();
          misc.filtered_count = 0;
        }
    }
}



//!  Clear all filter masks.

void
pfmView::slotClearFilterMasks ()
{
  clearFilterMasks ();
  redrawMap (NVTrue, NVTrue);
}



//!  This determines the active function.  The id is set by the editMode button group or sent from the editor mode menu above.

void
pfmView::slotEditMode (int id)
{
  QString msc;


  switch (id)
    {
    case RECT_EDIT_AREA_3D:
    case POLY_EDIT_AREA_3D:

      //  This will uncheck whatever mode was last used (like EDIT_FEATURE);

      bEditMode->setChecked (true);

      misc.function = misc.last_edit_function;
      misc.save_function = misc.function;
      break;

    case GRAB_CONTOUR:
      misc.function = misc.last_edit_function = id;
      misc.save_function = misc.function;
      break;

    case RECT_FILTER_AREA:
    case RECT_FILTER_MASK:
    case SELECT_HIGH_POINT:
    case SELECT_LOW_POINT:
    case POLY_FILTER_AREA:
    case POLY_FILTER_MASK:
    case DELETE_FEATURE:
    case EDIT_FEATURE:
    case REMISP_AREA:
    case REMISP_FILTER:
    case GOOGLE_EARTH:
      misc.function = id;
      misc.save_function = misc.function;
      break;

    case DRAW_CONTOUR:
      misc.function = id;
      misc.save_function = misc.function;
      break;

    case DRAW_CONTOUR_FILTER:
      misc.function = id;
      misc.save_function = misc.function;


      //  If contours already existed, remove them.

      if (misc.filt_contour_count) slotClearFilterContours ();
      break;

    case ADD_FEATURE:
      misc.save_function = misc.function;
      misc.function = id;
      break;
    }

  setFunctionCursor (misc.function);
}



/*!
  Setting the displayed area or the entire file to checked, unchecked, verified, or unverified.  The displayed area
  set comes from the toolbar button.  The entire file set comes from the "Edit" pulldown menu.
*/

void 
pfmView::slotCheckMenu (QAction *action)
{
  void setAreaChecked (MISC *misc, int32_t type, uint8_t area);


  setWidgetStates (NVFalse);


  int32_t type = 0;


  //  Determine what action to take and whether we need to perform it for an area or the entire file.

  uint8_t area = NVTrue;
  if (action == setChecked || action == fileChecked) type = 0;
  if (action == setUnchecked || action == fileUnchecked) type = 1;
  if (action == setVerified || action == fileVerified) type = 2;
  if (action == setUnverified || action == fileUnverified) type = 3;
  if (action == setMultiChecked) type = 4;
  if (action == setMultiValidChecked) type = 5;

  if (action == fileChecked || action == fileUnchecked || action == fileVerified || action == fileUnverified) area = NVFalse;


  pfmViewMod = NVTrue;


  setWidgetStates (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  setAreaChecked (&misc, type, area);


  //  Why are we closing and opening the file?  This should force a cache flush on SMB or NFS file systems.
  //  We have seen a problem when two people are accessing the same file - when one of them sets an area
  //  to checked/unchecked/verified/unverified the other won't see it when they redraw their coverage map.
  //  This is caused by the remote client using it's SMB or NFS cache instead of actually reading the file.
  //  Close and open is supposed to inform remote clients that they need to flush their cache and reread.
  //  I wouldn't bet on it though ;-)

  close_pfm_file (misc.pfm_handle[0]);
  misc.abe_share->open_args[0].checkpoint = 0;
  misc.pfm_handle[0] = open_existing_pfm_file (&misc.abe_share->open_args[0]);


  qApp->restoreOverrideCursor ();
  qApp->processEvents ();


  redrawMap (NVTrue, NVFalse);

  cov->redrawMap (NVFalse);
}



//!  Move the selected layer to the top of the PFM layers.
  
void 
pfmView::slotLayerGrpTriggered (QAction *action)
{
  int32_t hit = -1;


  //  Find out which layer we selected

  for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if (action == layer[pfm])
        {
          hit = pfm;
          break;
        }
    }


  //  If we selected a layer other than 0, move the layer to the top.

  if (hit > 0)
    {
      move_layer_to_top (&misc, hit);

      slotManageLayersDataChanged ();
    }
}



//!  Kick off the layer management dialog.

void 
pfmView::slotLayers ()
{
  manageLayers_dialog = new manageLayers (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  manageLayers_dialog->move (x () + width () / 2 - manageLayers_dialog->width () / 2, y () + height () / 2 - manageLayers_dialog->height () / 2);

  connect (manageLayers_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotManageLayersDataChanged ()));
}



//!  We've changed something about the layers (multiple PFMs) in the manage layer dialog so we have to do something.

void 
pfmView::slotManageLayersDataChanged ()
{
  if (!misc.drawing)
    {
      //  Set up the layers stuff

      int32_t num_displayed = 0;
      statbook->setTabEnabled (2, false);

      for (int32_t pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
        {
          if (pfm < misc.abe_share->pfm_count)
            {
              QString layerString;
              layerString.sprintf ("%d - ", pfm + 1);
              layerString += QFileInfo (misc.abe_share->open_args[pfm].list_path).fileName ();
              layer[pfm]->setText (layerString);
              layer[pfm]->setVisible (true);


              //  This is for the PFM layers notebook tab.

              QString name = QFileInfo (QString (misc.abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

              layerName[pfm]->setText (name);

              if (pfm) statbook->setTabEnabled (2, true);

              if (misc.abe_share->display_pfm[pfm])
                {
                  num_displayed++;
                  layerCheck[pfm]->setChecked (true);
                }
              else
                {
                  layerCheck[pfm]->setChecked (false);
                }
              layerName[pfm]->show ();
              layerCheck[pfm]->show ();
            }
          else
            {
              layer[pfm]->setVisible (false);

              layerName[pfm]->hide ();
              layerCheck[pfm]->hide ();
            }
        }
    }


  //  Try to open the feature file and read the features into memory.

  readFeature (this, &misc);


  if (!misc.abe_share->pfm_count)
    {
      discardMovableObjects ();

      cov_area_defined = NVFalse;

      misc.clear = NVTrue;
      misc.cov_clear = NVTrue;

      clearFilterMasks ();

      map->setZoomLevel (0);

      slotRedrawCoverage ();

      logo_first = NVTrue;
      slotResize (NULL);
    }
  else
    {
      misc.cov_clear = NVTrue;
      misc.clear = NVTrue;


      //  Compute the MBR of all of the PFM areas.

      compute_total_mbr (&misc);


      map->disableSignals ();

      clearFilterMasks ();

      map->resetBounds (misc.total_mbr);

      map->enableSignals ();


      covdef.initial_bounds.min_x = misc.total_mbr.min_x;
      covdef.initial_bounds.min_y = misc.total_mbr.min_y;
      covdef.initial_bounds.max_x = misc.total_mbr.max_x;
      covdef.initial_bounds.max_y = misc.total_mbr.max_y;

      cov->resetBounds (covdef.initial_bounds);

      slotRedrawCoverage ();


      if (cov_area_defined) zoomIn (misc.total_displayed_area, NVTrue);
    }


  //  Set the attributes for the new "top" PFM.

  setStatusAttributes ();


  //  Let other programs know that we have changed PFM layers

  misc.abe_share->key = PFM_LAYERS_CHANGED;


  //  Wait 2 seconds so that all associated programs will see the flag.

#ifdef NVWIN3X
  Sleep (2000);
#else
  sleep (2);
#endif


  misc.abe_share->key = 0;
  misc.abe_share->modcode = NO_ACTION_REQUIRED; 
}



//!  Kick off the browser and point it to the help HTML file.

void
pfmView::extendedHelp ()
{
  if (getenv ("PFM_ABE") == NULL)
    {
      QMessageBox::warning (this, tr ("pfmView User Guide"), 
                            tr ("The PFM_ABE environment variable is not set.\n"
                                "This must point to the folder that contains the doc folder containing the PFM_ABE documentation."));
      return;
    }



  QString html_help_file;
  QString dir (getenv ("PFM_ABE"));
  html_help_file = dir + SEPARATOR + "doc" + SEPARATOR + "PFM_ABE_User_Guide.html";


  QFileInfo hf (html_help_file);

  if (!hf.exists () || !hf.isReadable ())
    {
      QMessageBox::warning (this, tr ("pfmView User Guide"), 
                            tr ("Can't find the Area Based Editor documentation file: %1").arg (html_help_file));
      return;
    }


  QDesktopServices::openUrl (QUrl (html_help_file, QUrl::TolerantMode));
}



//!  Help for the toolbars so that the user can figure out how to set them up.

void
pfmView::slotToolbarHelp ()
{
  if (misc.cube_available)
    {
      QWhatsThis::showText (QCursor::pos ( ), toolbarCubeText, map);
    }
  else
    {
      QWhatsThis::showText (QCursor::pos ( ), toolbarText, map);
    }
}



#ifdef ET_PHONE_HOME

void 
pfmView::slotReplyFinished (QNetworkReply *reply)
{
  QString ver = "", changes = "", install = "", license = "", win_install = "", mingw_install = "", source = "", foss = "", resp_string = "";
  uint8_t new_version = NVFalse;


  QString current = QString (PFM_ABE_RELEASE);

  int32_t cur_major = current.section ('.', 0, 0).toInt ();
  int32_t cur_minor = current.section ('.', 1, 1).toInt ();
  int32_t cur_rel = current.section ('.', 2, 2).toInt ();
  int32_t cur_sub = current.section ('.', 3, 3).toInt ();
  int32_t cur_big_num = cur_sub + cur_rel * 1000 + cur_minor * 10000 + cur_major * 100000;


  QUrl url = reply->url ();

  if (reply->error ())
    {
      //  Just silently go away if the request failed.
      //  fprintf (stderr, "Download of %s failed: %s\n", url.toEncoded ().constData (), qPrintable (reply->errorString ()));
    }
  else
    {
      QByteArray vrsn = reply->readAll ();

      for (int32_t i = 0 ; i < vrsn.size () ; i++)
        {
          if (vrsn.at (i) == '\n')
            {
              if (resp_string.contains ("[VERSION]"))
                {
                  ver = resp_string.remove ("[VERSION]");
                  int32_t new_major = ver.section ('.', 0, 0).toInt ();
                  int32_t new_minor = ver.section ('.', 1, 1).toInt ();
                  int32_t new_rel = ver.section ('.', 2, 2).toInt ();
                  int32_t new_sub = ver.section ('.', 3, 3).toInt ();
                  int32_t new_big_num = new_sub + new_rel * 1000 + new_minor * 10000 + new_major * 100000;

                  if (cur_big_num >= new_big_num) break;

                  new_version = NVTrue;
                }
              else if (resp_string.contains ("[CHANGES]"))
                {
                  changes = resp_string.remove ("[CHANGES]");
                }
              else if (resp_string.contains ("[INSTALL]"))
                {
                  install = resp_string.remove ("[INSTALL]");
                }
              else if (resp_string.contains ("[LICENSE]"))
                {
                  license = resp_string.remove ("[LICENSE]");
                }
              else if (resp_string.contains ("[MINGW_INSTALL]"))
                {
                  mingw_install = resp_string.remove ("[MINGW_INSTALL]");
                }
              else if (resp_string.contains ("[WIN_INSTALL]"))
                {
                  win_install = resp_string.remove ("[WIN_INSTALL]");
                }
              else if (resp_string.contains ("[SOURCE]"))
                {
                  source = resp_string.remove ("[SOURCE]");
                }
              else if (resp_string.contains ("[FOSS]"))
                {
                  foss = resp_string.remove ("[FOSS]");
                }

              resp_string = "";
            }
          else
            {
              resp_string += vrsn.at (i);
            }
        }

      if (new_version)
        {
          QString msg = tr ("<center><br><h3>There is a new version of PFMABE available</h3><br></center>"
                            "Your current version is %1<br>"
                            "The new version is %2<br><br>"
                            "You may download updated files using the links below. "
                            "If a file is not listed below it has not been modified since the last release and does not need to be updated.<br><br><ul>"
                            "<li><a href=%3>List of Changes</a></li>").arg (current).arg (ver).arg (changes);
          if (!install.isEmpty ()) msg += QString ("<li><a href=%1>INSTALL file</a></li>").arg (install);
          if (!license.isEmpty ()) msg += QString ("<li><a href=%1>LICENSE file</a></li>").arg (license);

#ifdef NVWIN3X
          if (!mingw_install.isEmpty ())
            {
              msg += QString ("<li><a href=%1>MinGW Installer</a></li>").arg (mingw_install);
              msg += QString ("<li><a href=%1.md5sum>MinGW Installer md5sum</a></li>").arg (mingw_install);
            }
          if (!win_install.isEmpty ())
            {
              msg += QString ("<li><a href=%1>PFMABE Windows Installer</a></li>").arg (win_install);
              msg += QString ("<li><a href=%1.md5sum>PFMABE Windows Installer md5sum</a></li>").arg (win_install);
            }
#endif
          if (!source.isEmpty ())
            {
              msg += QString ("<li><a href=%1>PFMABE Source Code</a></li>").arg (source);
              msg += QString ("<li><a href=%1.md5sum>PFMABE Source Code md5sum</a></li>").arg (source);
            }
          if (!foss.isEmpty ())
            {
              msg += QString ("<li><a href=%1>FOSS Libraries</a></li>").arg (foss);
              msg += QString ("<li><a href=%1.md5sum>FOSS Libraries md5sum</a></li>").arg (foss);
            }
          msg += "</ul><br><br>";

          QMessageBox::information (this, "pfmView", msg);
        }
    }

  reply->deleteLater ();
}

#endif



//!  The "About" help menu item was selected.

void
pfmView::about ()
{
  QMessageBox::about (this, misc.program_version, pfmViewAboutText);
}



//!  The "Acknowledgments" help menu item was selected.

void
pfmView::slotAcknowledgments ()
{
  QMessageBox::about (this, misc.program_version, acknowledgmentsText);
}



//!  The "About Qt" help menu item was selected.

void
pfmView::aboutQt ()
{
  QMessageBox::aboutQt (this, misc.program_version);
}



/*!
************************************************************************************************

   - Coverage map functions.

   - These work the same way as the main map functions (after all, they use the same type of
     map widget).  The coverage map just has fewer possible actions.

   -  This function redraws the coverage bounds (the area we're viewing).

************************************************************************************************/

void 
pfmView::redrawCovBounds ()
{
  if (cov_area_defined)
    {
      mapdef = map->getMapdef ();

      if (cov->rubberbandRectangleIsActive (cov_rb_rectangle)) cov->discardRubberbandRectangle (&cov_rb_rectangle);

      cov->drawRectangle (mapdef.bounds[mapdef.zoom_level].min_x, mapdef.bounds[mapdef.zoom_level].min_y, mapdef.bounds[mapdef.zoom_level].max_x,
                          mapdef.bounds[mapdef.zoom_level].max_y, options.cov_box_color, LINE_WIDTH, Qt::SolidLine, NVTrue);
    }
}



//!  The left mouse button was pressed in the coverage map.

void 
pfmView::covLeftMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)), double lon, double lat)
{
  //  If the popup menu is up discard this mouse press

  if (cov_popup_active)
    {
      cov_popup_active = NVFalse;
      return;
    }


  int32_t *px, *py;
  double *mx, *my;
  NV_F64_XYMBR bounds;


  switch (misc.cov_function)
    {
    case COV_START_DRAW_RECTANGLE:
      cov->discardRubberbandRectangle (&cov_rb_rectangle);

      misc.cov_function = COV_DRAW_RECTANGLE;
      cov->anchorRubberbandRectangle (&cov_rb_rectangle, lon, lat, options.cov_box_color, LINE_WIDTH, Qt::SolidLine);
      break;

    case COV_DRAW_RECTANGLE:

      //  If we have a rubberband rectangle active (moving) we close it and do the zoom in the main map.

      if (cov->rubberbandRectangleIsActive (cov_rb_rectangle))
        {
          cov->closeRubberbandRectangle (cov_rb_rectangle, lon, lat, &px, &py, &mx, &my);


          bounds.min_x = 999999.0;
          bounds.min_y = 999999.0;
          bounds.max_x = -999999.0;
          bounds.max_y = -999999.0;

          for (int32_t i = 0 ; i < 4 ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              if (mx[i] < bounds.min_x) bounds.min_x = mx[i];
              if (my[i] < bounds.min_y) bounds.min_y = my[i];
              if (mx[i] > bounds.max_x) bounds.max_x = mx[i];
              if (my[i] > bounds.max_y) bounds.max_y = my[i];
            }


          //  The first time in we want to enable all of the main buttons after an area has been defined.

          if (!cov_area_defined) setWidgetStates (NVTrue);

          cov_area_defined = NVTrue;

          misc.cov_function = COV_START_DRAW_RECTANGLE;

          zoomIn (bounds, NVTrue);
        }
      break;


    case COV_START_DRAG_RECTANGLE:

      cov_drag_bounds = mapdef.bounds[mapdef.zoom_level];

      cov->setMovingRectangle (&cov_mv_rectangle, cov_drag_bounds.min_x, cov_drag_bounds.min_y, cov_drag_bounds.max_x, cov_drag_bounds.max_y,
                               options.cov_box_color, LINE_WIDTH, NVFalse, Qt::SolidLine);

      cov_start_drag_lat = lat;
      cov_start_drag_lon = lon;

      cov->setCursor (Qt::ClosedHandCursor);
      misc.cov_function = COV_DRAG_RECTANGLE;
      break;
    }
}



//!  The middle mouse button was pressed in the coverage map.

void 
pfmView::covMidMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)), double lon __attribute__ ((unused)),
                      double lat __attribute__ ((unused)))
{
  misc.cov_function = COV_START_DRAW_RECTANGLE;
  cov->setCursor (editRectCursor);

  cov->discardRubberbandRectangle (&cov_rb_rectangle);
  cov->closeMovingRectangle (&cov_mv_rectangle);

  redrawCovBounds ();
}



//!  The right mouse button was pressed in the coverage map.

void 
pfmView::covRightMouse (int32_t mouse_x, int32_t mouse_y, double lon, double lat)
{
  cov_menu_cursor_lon = lon;
  cov_menu_cursor_lat = lat;
  cov_menu_cursor_x = mouse_x;
  cov_menu_cursor_y = mouse_y;


  cov_popup_active = NVTrue;

  covPopupMenu->popup (QCursor::pos ( ));
}



//!  The first popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu0 ()
{
  cov_popup_active = NVFalse;

  covLeftMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  The second popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu1 ()
{
  cov_popup_active = NVFalse;

  covMidMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  The third popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupMenu2 ()
{
  cov_popup_active = NVFalse;

  if (cov_area_defined)
    {
      cov->setCursor (Qt::OpenHandCursor);

      misc.cov_function = COV_START_DRAG_RECTANGLE;
    }
}



//!  The "help" popup menu item was selected in the coverage map.

void 
pfmView::slotCovPopupHelp ()
{
  QWhatsThis::showText (QCursor::pos ( ), covMenuText, cov);
  covMidMouse (cov_menu_cursor_x, cov_menu_cursor_y, cov_menu_cursor_lon, cov_menu_cursor_lat);
}



//!  This slot is called whenever a mouse button (any of them) is pressed in the coverage map.

void 
pfmView::slotCovMousePress (QMouseEvent *e, double lon, double lat)
{
  if (!misc.drawing && !pfm_edit_active)
    {
      if (e->button () == Qt::LeftButton)
        {
          //  Check for the control key modifier.  If it's set, we want to drag the rectangle.

          if (e->modifiers () == Qt::ControlModifier) misc.cov_function = COV_START_DRAG_RECTANGLE;

          covLeftMouse (e->x (), e->y (), lon, lat);
        }
      if (e->button () == Qt::MidButton) covMidMouse (e->x (), e->y (), lon, lat);
      if (e->button () == Qt::RightButton) covRightMouse (e->x (), e->y (), lon, lat);
    }
}



//!  Right now this is just used when the user is dragging the coverage box.  When the user releases the button, the new area is displayed.

void 
pfmView::slotCovMouseRelease (QMouseEvent *e, double lon __attribute__ ((unused)), double lat __attribute__ ((unused)))
{
  int32_t *px, *py;
  double *mx, *my;


  if (!misc.drawing && !pfm_edit_active)
    {
      if (e->button () == Qt::LeftButton && misc.cov_function == COV_DRAG_RECTANGLE)
        {
          cov->getMovingRectangle (cov_mv_rectangle, &px, &py, &mx, &my);

          cov_drag_bounds.min_x = 999999.0;
          cov_drag_bounds.min_y = 999999.0;
          cov_drag_bounds.max_x = -999999.0;
          cov_drag_bounds.max_y = -999999.0;

          for (int32_t i = 0 ; i < 4 ; i++)
            {
              if (misc.dateline && mx[i] < 0.0) mx[i] += 360.0;

              if (mx[i] < cov_drag_bounds.min_x) cov_drag_bounds.min_x = mx[i];
              if (my[i] < cov_drag_bounds.min_y) cov_drag_bounds.min_y = my[i];
              if (mx[i] > cov_drag_bounds.max_x) cov_drag_bounds.max_x = mx[i];
              if (my[i] > cov_drag_bounds.max_y) cov_drag_bounds.max_y = my[i];
            }


          cov->closeMovingRectangle (&cov_mv_rectangle);

          cov->setCursor (editRectCursor);
          misc.cov_function = COV_START_DRAW_RECTANGLE;
          zoomIn (cov_drag_bounds, NVTrue);
        }
    }

  if (e->button () == Qt::MidButton) {};
  if (e->button () == Qt::RightButton) {};
}



//!  The mouse was moved in the coverage map.

void
pfmView::slotCovMouseMove (QMouseEvent *e __attribute__ ((unused)), double lon, double lat)
{
  char ltstring[25], lnstring[25], hem;
  double deg, min, sec;
  NV_F64_COORD2 xy;
  QString y_string, x_string;
  BIN_RECORD bin;


  //  Let other ABE programs know which window we're in.  This is a special case since we have two map windows here.  We're lying about the 
  //  active window ID (process ID) by subtracting 1 from this program's PID.  This makes trackCursor track it locally.

  misc.abe_share->active_window_id = misc.process_id - 1;


  //  Track the cursor position for other ABE programs

  misc.abe_share->cursor_position.y = lat;
  misc.abe_share->cursor_position.x = lon;


  //  Try to find the highest layer with a valid value to display in the status bar.

  xy.y = lat;
  xy.x = lon;
  int32_t hit = -1;

  for (int32_t pfm = 0 ; pfm < misc.abe_share->pfm_count ; pfm++)
    {
      if (misc.abe_share->display_pfm[pfm] && bin_inside_ptr (&misc.abe_share->open_args[pfm].head, xy))
        {
          read_bin_record_xy (misc.pfm_handle[pfm], xy, &bin);

          hit = pfm;
          break;
        }
    }


  //  We only change the status PFM values if we were actually over a PFM data area

  if (hit >= 0)
    {
      pfmLabel->setText (QFileInfo (misc.abe_share->open_args[hit].list_path).fileName ().remove (".pfm"));
      pfmLabel->setToolTip (misc.abe_share->open_args[hit].list_path);
    }


  strcpy (ltstring, fixpos (lat, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  strcpy (lnstring, fixpos (lon, &deg, &min, &sec, &hem, POS_LON, options.position_form));


  if (options.lat_or_east)
    {
      double x = lon * NV_DEG_TO_RAD;
      double y = lat * NV_DEG_TO_RAD;

      pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &x, &y, NULL);

      strcpy (ltstring, QString ("%L1").arg (y, 0, 'f', 2).toLatin1 ());
      strcpy (lnstring, QString ("%L1").arg (x, 0, 'f', 2).toLatin1 ());
    }

  latLabel->setText (ltstring);
  lonLabel->setText (lnstring);


  double diff_lat, diff_lon, anc_lat, anc_lon, dist, az;
  QString string;

  switch (misc.cov_function)
    {
    case COV_DRAW_RECTANGLE:

      cov->dragRubberbandRectangle (cov_rb_rectangle, lon, lat);

      if (cov->rubberbandRectangleIsActive (cov_rb_rectangle))
        {
          cov->dragRubberbandRectangle (cov_rb_rectangle, lon, lat);

          cov->getRubberbandRectangleAnchor (cov_rb_rectangle, &anc_lon, &anc_lat);

          double rect_width, rect_height;

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, anc_lat, lon, &rect_width, &az);
          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, anc_lon, &rect_height, &az);

          invgp (NV_A0, NV_B0, anc_lat, anc_lon, lat, lon, &dist, &az);

          string = QString (" W: %L1m, H: %L2m, Diag: %L3m ").arg (rect_width).arg (rect_height).arg (dist);
          misc.statusProgLabel->setText (string);
        }
      break;

    case COV_DRAG_RECTANGLE:
      diff_lat = lat - cov_start_drag_lat;
      diff_lon = lon - cov_start_drag_lon;

      for (int32_t i = 0 ; i < 4 ; i++)
        {
          cov_drag_bounds.min_x = mapdef.bounds[mapdef.zoom_level].min_x + diff_lon;
          cov_drag_bounds.max_x = mapdef.bounds[mapdef.zoom_level].max_x + diff_lon;
          cov_drag_bounds.min_y = mapdef.bounds[mapdef.zoom_level].min_y + diff_lat;
          cov_drag_bounds.max_y = mapdef.bounds[mapdef.zoom_level].max_y + diff_lat;
        }

      if (cov_drag_bounds.min_x < mapdef.bounds[0].min_x)
        {
          cov_drag_bounds.min_x = mapdef.bounds[0].min_x;
          cov_drag_bounds.max_x = mapdef.bounds[0].min_x + (mapdef.bounds[mapdef.zoom_level].max_x - mapdef.bounds[mapdef.zoom_level].min_x);
        }

      if (cov_drag_bounds.max_x > mapdef.bounds[0].max_x)
        {
          cov_drag_bounds.max_x = mapdef.bounds[0].max_x;
          cov_drag_bounds.min_x = mapdef.bounds[0].max_x - (mapdef.bounds[mapdef.zoom_level].max_x - mapdef.bounds[mapdef.zoom_level].min_x);
        }

      if (cov_drag_bounds.min_y < mapdef.bounds[0].min_y)
        {
          cov_drag_bounds.min_y = mapdef.bounds[0].min_y;
          cov_drag_bounds.max_y = mapdef.bounds[0].min_y + (mapdef.bounds[mapdef.zoom_level].max_y - mapdef.bounds[mapdef.zoom_level].min_y);
        }

      if (cov_drag_bounds.max_y > mapdef.bounds[0].max_y)
        {
          cov_drag_bounds.max_y = mapdef.bounds[0].max_y;
          cov_drag_bounds.min_y = mapdef.bounds[0].max_y - (mapdef.bounds[mapdef.zoom_level].max_y - mapdef.bounds[mapdef.zoom_level].min_y);
        }

      cov->setMovingRectangle (&cov_mv_rectangle, cov_drag_bounds.min_x, cov_drag_bounds.min_y, cov_drag_bounds.max_x, cov_drag_bounds.max_y,
                               options.cov_box_color, LINE_WIDTH, NVFalse, Qt::SolidLine);

      break;
    }
}



//!  We don't do anything pre-coastline draw in the coverage map

void 
pfmView::slotCovPreRedraw (NVMAP_DEF map_covdef __attribute__ ((unused)))
{
}



//!  Slot called after all of the background stuff has been drawn in the coverage map.  This is where we draw the coverage.

void 
pfmView::slotCovPostRedraw (NVMAP_DEF covdef)
{
  double z;
  int32_t pix_z, cov_area_x, cov_area_y, cov_area_width, cov_area_height, cov_area_bin_x, cov_area_bin_y;

  void cov_coasts (MISC *misc, nvMap *cov, NVMAP_DEF *covdef);


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  //  Get the actual map area corner pixels.

  cov->map_to_screen (1, &covdef.bounds[0].min_x, &covdef.bounds[0].max_y, &z, &cov_area_x, &cov_area_y, &pix_z);
  cov->map_to_screen (1, &covdef.bounds[0].max_x, &covdef.bounds[0].min_y, &z, &cov_area_width, &cov_area_height, &pix_z);

  cov_area_width = cov_area_width - cov_area_x;
  cov_area_height = cov_area_height - cov_area_x;


  uint8_t *coverage = NULL;
  QColor c1, prev_c1;
  int32_t cov_start_row, cov_start_col, cov_start_height, cov_start_width;


  //  Remove the box outline prior to drawing.

  cov->discardRubberbandRectangle (&cov_rb_rectangle);


  c1 = options.cov_background_color;

  if (misc.cov_clear)
    {
      cov->fillRectangle (misc.total_mbr.min_x, misc.total_mbr.min_y, misc.total_mbr.max_x, misc.total_mbr.max_y, c1, NVTrue);
    }
  else
    {
      cov->fillRectangle (misc.total_displayed_area.min_x, misc.total_displayed_area.min_y, misc.total_displayed_area.max_x, 
                          misc.total_displayed_area.max_y, c1, NVTrue);
    }


  //  Loop through the open (and displayed) PFMs and paint the coverage for each.

  for (int32_t pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only draw coverage if we're displaying the layer.

      if (misc.abe_share->display_pfm[pfm])
        {
          if (misc.cov_clear)
            {
              cov_start_row = 0;
              cov_start_col = 0;
              cov_start_height = misc.abe_share->open_args[pfm].head.bin_height;
              cov_start_width = misc.abe_share->open_args[pfm].head.bin_width;
            }
          else
            {
              int32_t displayed_area_width = (NINT ((misc.total_displayed_area.max_x - misc.total_displayed_area.min_x) / 
                                                     misc.abe_share->open_args[pfm].head.x_bin_size_degrees));
              int32_t displayed_area_height = (NINT ((misc.total_displayed_area.max_y - misc.total_displayed_area.min_y) / 
                                                      misc.abe_share->open_args[pfm].head.y_bin_size_degrees));
              int32_t displayed_area_row = NINT ((misc.total_displayed_area.min_y - misc.abe_share->open_args[pfm].head.mbr.min_y) / 
                                                  misc.abe_share->open_args[pfm].head.y_bin_size_degrees);
              int32_t displayed_area_column = NINT ((misc.total_displayed_area.min_x - misc.abe_share->open_args[pfm].head.mbr.min_x) / 
                                                     misc.abe_share->open_args[pfm].head.x_bin_size_degrees);

              if (displayed_area_column + displayed_area_width > misc.abe_share->open_args[pfm].head.bin_width - 1)
                displayed_area_width = misc.abe_share->open_args[pfm].head.bin_width - displayed_area_column - 1;

              if (displayed_area_row + displayed_area_height > misc.abe_share->open_args[pfm].head.bin_height - 1)
                displayed_area_height = misc.abe_share->open_args[pfm].head.bin_height - displayed_area_row - 1;


              cov_start_row = displayed_area_row;
              cov_start_col = displayed_area_column;
              cov_start_height = displayed_area_height;
              cov_start_width = displayed_area_width;
            }


          //  Determine how many bins cover a single pixel???

          cov_area_bin_x = misc.abe_share->open_args[pfm].head.bin_width / cov_area_width;
          if (misc.abe_share->open_args[pfm].head.bin_width % cov_area_width) cov_area_bin_x++;

          cov_area_bin_y = misc.abe_share->open_args[pfm].head.bin_height / cov_area_height;
          if (misc.abe_share->open_args[pfm].head.bin_height % cov_area_height) cov_area_bin_y++;


          int32_t checked_sum, verified_sum, cov_col, cov_width;
          NV_I32_COORD2 coord;
          uint8_t has_data;

          int32_t cov_row = cov_start_row / cov_area_bin_y;

          int32_t cov_height = cov_start_height / cov_area_bin_y;

          int32_t size = (cov_area_bin_y + 1) * ((cov_start_width) + cov_area_bin_x + 1);


          //  If size is less than or equal to 0 we have asked for an area outside of the PFM's MBR so we don't want to do anything.

          if (size > 0)
            {
              if ((coverage = (uint8_t *) calloc (size, sizeof (uint8_t))) == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - Unable to allocate coverage map memory.\n", misc.progname, __FILE__, __FUNCTION__, __LINE__);
                  fprintf (stderr, "Requesting %d bytes.\n", size);
                  exit (-1);
                }

              for (int32_t i = cov_row ; i <= cov_row + cov_height ; i++)
                {
                  //  Drawing one row at a time.

                  double start_lat = misc.abe_share->open_args[pfm].head.mbr.min_y + (i * cov_area_bin_y) * 
                    misc.abe_share->open_args[pfm].head.y_bin_size_degrees;
                  double lat = misc.abe_share->open_args[pfm].head.mbr.min_y + ((i + 1) * cov_area_bin_y) * 
                    misc.abe_share->open_args[pfm].head.y_bin_size_degrees;


                  memset (coverage, 0, size);

                  for (int32_t k = 0 ; k <= cov_area_bin_y ; k++)
                    {
                      coord.y = (i * cov_area_bin_y) + k;
                      for (int32_t m = 0 ; m <= cov_start_width ; m++)
                        {
                          coord.x = m + cov_start_col;

                          read_cov_map_index (misc.pfm_handle[pfm], coord, (coverage + k * cov_start_width + m));
                        }
                    }


                  cov_col = cov_start_col / cov_area_bin_x;
                  cov_width = cov_start_width / cov_area_bin_x + cov_col;


                  //  We're only going to paint once the color has changed.  That is, we're making a long skinny rectangle
                  //  until the color needs to change and then we're painting the rectangle.

                  prev_c1 = options.cov_background_color;
                  prev_c1.setAlpha (0);


                  double start_lon = misc.abe_share->open_args[pfm].head.mbr.min_x + ((cov_col - 1) * cov_area_bin_x) * 
                    misc.abe_share->open_args[pfm].head.x_bin_size_degrees;
                  double lon = 0.0;

                  for (int32_t j = cov_col ; j < cov_width ; j++)
                    {
                      lon = misc.abe_share->open_args[pfm].head.mbr.min_x + (j * cov_area_bin_x) *
                        misc.abe_share->open_args[pfm].head.x_bin_size_degrees;

                      has_data = NVFalse;
                      checked_sum = 0;
                      verified_sum = 0;

                      for (int32_t k = 0 ; k < cov_area_bin_y ; k++)
                        {
                          for (int32_t m = 0 ; m < cov_area_bin_x ; m++)
                            {
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_DATA) 
                                has_data = NVTrue;
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_CHECKED) 
                                checked_sum++;
                              if ((*(coverage + k * cov_start_width + (j - cov_col) * cov_area_bin_x + m)) & COV_VERIFIED) 
                                verified_sum++;
                            }
                        }

                      if (has_data)
                        {
                          c1 = options.cov_data_color;

                          if (checked_sum == cov_area_bin_x * cov_area_bin_y) c1 = options.cov_checked_color;
                          if (verified_sum == cov_area_bin_x * cov_area_bin_y) c1 = options.cov_verified_color;

                          c1.setAlpha (255);
                        }
                      else
                        {
                          c1 = options.cov_background_color;


                          //  If it's gonna be white we want it to be clear so it doesn't write over any 
                          //  underlaying PFM coverage layers.

                          c1.setAlpha (0);
                        }


                      if (c1 != prev_c1 && j != cov_col)
                        {
                          cov->fillRectangle (start_lon, start_lat, lon, lat, prev_c1, NVTrue);

                          start_lon = lon;

                          prev_c1 = c1;
                        }
                    }

                  cov->fillRectangle (start_lon, start_lat, lon, lat, prev_c1, NVTrue);
                }


              free (coverage);
            }
        }
    }


  //  Figure out which is the highest active (displayed) layer.

  int32_t hit = 0;
  for (int32_t pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      if (misc.abe_share->display_pfm[pfm]) hit = pfm;
    }


  //  Draw semi-transparent coastlines if they have been requested for the main map.

  if (options.cov_coast) cov_coasts (&misc, cov, &covdef);


  //  Draw a semi-transparent outline around each PFM.

  for (int32_t pfm = misc.abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only draw coverage if we're displaying the layer.

      if (misc.abe_share->display_pfm[pfm])
        {
          if (pfm == hit)
            {
              c1 = options.cov_pfm_box_hot_color;
            }
          else
            {
              c1 = options.cov_pfm_box_cold_color;
            }

          cov->drawPolygon (misc.abe_share->open_args[pfm].head.polygon_count, misc.abe_share->open_args[pfm].head.polygon, 
                            c1, LINE_WIDTH, NVTrue, Qt::SolidLine, NVTrue);
        }
    }


  if (options.display_feature) drawCovFeatures ();


  //  Restore the box outline after drawing.

  redrawCovBounds ();


  cov->setCursor (editRectCursor);



  //  Draw an outline around the entire area.

  cov->drawRectangle (covdef.initial_bounds.min_x, covdef.initial_bounds.min_y, covdef.initial_bounds.max_x,
                      covdef.initial_bounds.max_y, options.cov_box_color, LINE_WIDTH, Qt::SolidLine, NVTrue);

  qApp->restoreOverrideCursor ();
}



//!  Draw the features on the coverage map

void 
pfmView::drawCovFeatures ()
{
  for (uint32_t j = 0 ; j < misc.bfd_header.number_of_records ; j++)
    {
      if ((misc.feature[j].confidence_level || misc.abe_share->layer_type == AVERAGE_DEPTH || misc.abe_share->layer_type == MIN_DEPTH || 
           misc.abe_share->layer_type == MAX_DEPTH) && (!misc.feature[j].parent_record || options.display_children))
        {
          if (misc.feature[j].latitude >= misc.total_mbr.min_y &&
              misc.feature[j].latitude <= misc.total_mbr.max_y &&
              misc.feature[j].longitude >= misc.total_mbr.min_x &&
              misc.feature[j].longitude <= misc.total_mbr.max_x)
            {
              //  Check for the type of feature display

              if (options.display_feature == 1 || (options.display_feature == 2 && misc.feature[j].confidence_level != 5) ||
                  (options.display_feature == 3 && misc.feature[j].confidence_level == 5))
                {
                  //  Check the feature for the feature search string and highlighting.

                  QString feature_info;
                  uint8_t highlight;
                  if (checkFeature (&misc, &options, j, &highlight, &feature_info))
                    {
                      if (misc.feature[j].confidence_level)
                        {
                          if (misc.feature[j].confidence_level == 5)
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE, 
                                                  COV_FEATURE_SIZE, options.cov_verified_feature_color, NVTrue);
                            }
                          else
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE, 
                                                  COV_FEATURE_SIZE, options.cov_feature_color, NVTrue);
                            }
                        }
                      else
                        {
                          if (!misc.surface_val)
                            {
                              cov->fillRectangle (misc.feature[j].longitude, misc.feature[j].latitude, COV_FEATURE_SIZE,
                                                  COV_FEATURE_SIZE, options.cov_inv_feature_color, NVTrue);
                            }
                        }
                    }
                }
            }
        }
    }
}



//!  Kick off the overlays dialog.

void 
pfmView::slotOverlays ()
{
  manageOverlays_dialog = new manageOverlays (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  manageOverlays_dialog->move (x () + width () / 2 - manageOverlays_dialog->width () / 2, y () + height () / 2 - manageOverlays_dialog->height () / 2);

  connect (manageOverlays_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotManageOverlaysDataChanged ()));
}



//!  This slot is called if the overlays data has been changed in the overlays dialog.

void 
pfmView::slotManageOverlaysDataChanged ()
{
  if (!misc.drawing) redrawMap (NVTrue, NVFalse);
}



//!  Kick off the delete/restore file dialog.

void 
pfmView::slotDeleteFile ()
{
  deleteFile_dialog = new deleteFile (this, &options, &misc);

  connect (deleteFile_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotDeleteFileDataChanged ()));
}



//!  This slot is called if one or more input file statuses was changed in the delete/restore file dialog.

void 
pfmView::slotDeleteFileDataChanged ()
{
  slotRedrawCoverage ();
  if (!misc.drawing) redrawMap (NVTrue, NVTrue);
}



//!  Kick off the delete file queue dialog.

void 
pfmView::slotDeleteQueue ()
{
  deleteQueue_dialog = new deleteQueue (this, &options, &misc);

  connect (deleteQueue_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotDeleteFileDataChanged ()));
}



//!  Kick off the change path dialog.

void 
pfmView::slotChangePath ()
{
  changePath_dialog = new changePath (this, &options, &misc);

  connect (changePath_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotChangePathDataChanged ()));
}



//!  This slot is called if one or more input file data paths was changed in the change path dialog.

void 
pfmView::slotChangePathDataChanged ()
{
  //  We've changed the file names in the .ctl file so we have to reread them.

  close_pfm_file (misc.pfm_handle[0]);
  misc.abe_share->open_args[0].checkpoint = 0;
  misc.pfm_handle[0] = open_existing_pfm_file (&misc.abe_share->open_args[0]);
}



//!  Kick off the find feature dialog.

void 
pfmView::slotFindFeature ()
{
  if (misc.bfd_open)
    {
      findFeature_dialog = new findFeature (this, cov, &options, &misc);


      //  Put the dialog in the middle of the screen.

      findFeature_dialog->move (x () + width () / 2 - findFeature_dialog->width () / 2, y () + height () / 2 - findFeature_dialog->height () / 2);
    }
  else
    {
      QMessageBox::warning (this, tr ("pfmView Find Feature"), tr ("No feature file available."));
    }
}



//!  Change chart scale (almost never used anymore).

void 
pfmView::slotDefinePolygonChartScaleChanged ()
{
  redrawMap (NVTrue, NVTrue);
  qApp->processEvents ();
}



//!  Slot called when the user wants to define a feature polygon.

void 
pfmView::slotDefinePolygon ()
{
  if (misc.bfd_open)
    {
      if (options.chart_scale)
        {
          misc.feature_poly_radius = ((options.millimeters / 1000.0) * options.chart_scale) / 2.0;
        }
      else
        {
          //  Default to 1:5000

          misc.feature_poly_radius = ((options.millimeters / 1000.0) * 5000.0) / 2.0;
        }

      misc.def_feature_poly = NVTrue;


      redrawMap (NVTrue, NVTrue);
      qApp->processEvents ();


      definePolygon_dialog = new definePolygon (this, &options, &misc);
      definePolygon_dialog->setModal (true);

      connect (definePolygon_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotEditFeatureDataChanged ()));
      connect (definePolygon_dialog, SIGNAL (definePolygonSignal ()), this, SLOT (slotEditFeatureDefinePolygon ()));
      connect (definePolygon_dialog, SIGNAL (chartScaleChangedSignal ()), this, SLOT (slotDefinePolygonChartScaleChanged ()));

      definePolygon_dialog->show ();
    }
  else
    {
      QMessageBox::warning (this, tr ("pfmView Define Feature Polygon"), tr ("No feature file available."));
    }
}



//!  Slot called when the user wants to output the feature polygons to a Shapefile.

void 
pfmView::slotOutputFeature ()
{
  void outputFeaturePolygons (OPTIONS *options, MISC *misc);

  if (misc.bfd_open)
    {
      outputFeaturePolygons (&options, &misc);
    }
 else
    {
      QMessageBox::warning (this, tr ("pfmView Output Feature"), tr ("No feature file available."));
    }
}



//!  Display the 0 layer PFM header.

void 
pfmView::slotDisplayHeader ()
{
  displayHeader_dialog = new displayHeader (this, &misc);


  //  Put the dialog in the middle of the screen.

  displayHeader_dialog->move (x () + width () / 2 - displayHeader_dialog->width () / 2, y () + height () / 2 - displayHeader_dialog->height () / 2);
}



//  Slot for children to call for standard error output.

void 
pfmView::slotReadStandardError (QProcess *proc)
{
  readStandardError (proc);
}



//  Generic standard error reader that puts info into the message list.

void
pfmView::readStandardError (QProcess *proc)
{
  static QString resp_string = "";


  QByteArray response = proc->readAllStandardError ();


  //  Parse the return response for line feeds

  for (int32_t i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n')
        {
          //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
          //  killing ancillary programs.

          if (!resp_string.contains ("Destroyed while") && !resp_string.contains ("Setting MAXIMUM Cache Size") &&
              !resp_string.contains ("failed to add paths") && !resp_string.contains ("DrvShareLists") &&
              !resp_string.contains ("OpenGL Warning"))
            {
              misc.messages->append (resp_string);
            }

          resp_string = "";
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



//  This is just a slot so that we can start it from the View menu.  The actual work is done in displayMessage.

void
pfmView::slotMessage ()
{
  if (displayMessage_dialog == NULL)
    {
      displayMessage_dialog = new displayMessage (this, &options, &misc);

      connect (displayMessage_dialog, SIGNAL (closed ()), this, SLOT (slotMessageDialogClosed ()));
    }

  displayMessage_dialog->setMessage ();
}



void
pfmView::slotMessageDialogClosed ()
{
  displayMessage_dialog = NULL;
}



//!  Slot called when the user wants to output a file of ASCII data point positions and Z values.

void 
pfmView::slotOutputDataPoints ()
{
  map->setToolTip (tr ("Left click to define points, double click to define last point, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = OUTPUT_POINTS;
  setFunctionCursor (misc.function);
}



//!  Zooms to a specific area based on an input area file.

void 
pfmView::slotZoomToArea ()
{
  static QString filter = tr ("AREA (*.ARE *.are *.afs *.shp)");


  if (!QDir (options.area_dir).exists ()) options.area_dir = options.input_pfm_dir;


  QFileDialog fd (this, tr ("pfmView Zoom to area file")); 
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options.area_dir);


  QStringList filters;
  filters << tr ("AREA (*.ARE *.are *.afs *.shp)");

  fd.setNameFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFile);
  fd.selectNameFilter (filter);

  QStringList files;
  QString newfile;
  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();
      newfile = files.at (0);


      if (!newfile.isEmpty())
        {
          int32_t polygon_count;
          double polygon_x[200], polygon_y[200];
          char path[512];
          NV_F64_XYMBR mbr;

          strcpy (path, newfile.toLatin1 ());

          if (get_area_mbr (path, &polygon_count, polygon_x, polygon_y, &mbr))
            {
              cov_area_defined = NVTrue;

              zoomIn (mbr, NVTrue);


              misc.cov_function = COV_START_DRAW_RECTANGLE;

              redrawCovBounds ();
            }

          options.area_dir = fd.directory ().absolutePath ();
        }
    }
}



//!  Slot called when the user wants to define a rectangular area file.

void 
pfmView::slotDefineRectArea ()
{
  map->setToolTip (tr ("Left click to start rectangle, middle click to abort, right click for menu"));
  misc.save_function = misc.function;
  misc.function = DEFINE_RECT_AREA;
  setFunctionCursor (misc.function);
}



//!  Slot called when the user wants to define a polygonal area file.

void 
pfmView::slotDefinePolyArea ()
{
  map->setToolTip (tr ("Left click to define polygon vertices, double click to define last vertex, middle click to abort"));
  misc.save_function = misc.function;
  misc.function = DEFINE_POLY_AREA;
  setFunctionCursor (misc.function);
}



//!  Define the cursor based on the active function.  Also adds a background color and function message to the status bar.

void 
pfmView::setFunctionCursor (int32_t function)
{
  QString msc;


  discardMovableObjects ();

  switch (function)
    {
    case RECT_EDIT_AREA_3D:
      misc.statusProgLabel->setText (tr (" Rectangular edit mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editRectCursor);
      break;

    case SELECT_HIGH_POINT:
      misc.statusProgLabel->setText (tr (" Select high point mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (highCursor);
      break;

    case SELECT_LOW_POINT:
      misc.statusProgLabel->setText (tr (" Select low point mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (lowCursor);
      break;

    case POLY_EDIT_AREA_3D:
      misc.statusProgLabel->setText (tr (" Polygon edit mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editPolyCursor);
      break;

    case GRAB_CONTOUR:
      misc.statusProgLabel->setText (tr (" Capture contour mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (grabContourCursor);
      break;

    case RECT_FILTER_AREA:
      misc.statusProgLabel->setText (tr (" Rectangle filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterRectCursor);
      break;

    case POLY_FILTER_AREA:
      misc.statusProgLabel->setText (tr (" Polygon filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterPolyCursor);
      break;

    case RECT_FILTER_MASK:
      misc.statusProgLabel->setText (tr (" Rectangle filter mask mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskRectCursor);
      break;

    case POLY_FILTER_MASK:
      misc.statusProgLabel->setText (tr (" Polygon filter mask mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterMaskPolyCursor);
      break;

    case ZOOM_IN_AREA:
      misc.statusProgLabel->setText (tr (" Zoom in rectangle mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (zoomCursor);
      break;

    case OUTPUT_POINTS:
      misc.statusProgLabel->setText (tr (" Output data points mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::CrossCursor);
      break;

    case DEFINE_RECT_AREA:
      misc.statusProgLabel->setText (tr (" Define rectangular area file mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::SizeFDiagCursor);
      break;

    case DEFINE_POLY_AREA:
      misc.statusProgLabel->setText (tr (" Define polygonal area file mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (Qt::ArrowCursor);
      break;

    case DEFINE_FEATURE_POLY_AREA:
      misc.statusProgLabel->setText (tr (" Define polygonal feature area mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (pencilCursor);
      break;

    case DELETE_FEATURE:
      misc.statusProgLabel->setText (tr (" Delete feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (deleteFeatureCursor);
      break;

    case EDIT_FEATURE:
      misc.statusProgLabel->setText (tr (" Edit feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (editFeatureCursor);
      break;

    case ADD_FEATURE:
      misc.statusProgLabel->setText (tr (" Add feature mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (addFeatureCursor);
      break;

    case DRAW_CONTOUR:
      msc = tr (" Draw contour - %L1 ").arg (misc.draw_contour_level, 0, 'f', 2);
      misc.statusProgLabel->setText (msc);
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (drawContourCursor);
      break;

    case REMISP_AREA:
      misc.statusProgLabel->setText (tr (" Rectangular regrid mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (remispCursor);
      break;

    case DRAW_CONTOUR_FILTER:
      msc = tr (" Draw contour for filter - %L1 ").arg (misc.draw_contour_level, 0, 'f', 2);
      misc.statusProgLabel->setText (msc);
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::red);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (drawContourCursor);
      break;

    case REMISP_FILTER:
      misc.statusProgLabel->setText (tr (" Remisp filter mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::yellow);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (filterPolyCursor);
      break;

    case GOOGLE_EARTH:
      misc.statusProgLabel->setText (tr (" Select Google Earth placemark mode "));
      misc.statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::cyan);
      misc.statusProgLabel->setPalette (misc.statusProgPalette);
      map->setCursor (stickpinCursor);
      break;
    }
}



//!  This is the stderr read return from the DNC import QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotImportReadyReadStandardError ()
{
  readStandardError (importProc);
}



//!  Error return from the DNC import process.

void 
pfmView::slotImportError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("Unable to start the import process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("There was a write error to the import process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("There was a read error from the import process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView import process"), tr ("The import process died with an unknown error!"));
      break;
    }
}



//!  This is the return from the DNC import QProcess (when finished normally)

void 
pfmView::slotImportDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (importProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView Import"), tr ("An error occurred during import"));
    }
  else
    {
      //  First we have to remove the "dump_dnc" files that we aren't going to use.

      QString name;
      name = importTempFile + "_coast.trk";
      QFile file (name);
      file.remove ();

      name = importTempFile + ".trk";
      file.setFileName (name);
      file.remove ();

      name = importTempFile + ".llz";
      file.setFileName (name);
      file.remove ();


      if (!misc.bfd_open)
        {
          QDir featureDir;
          name = QString (misc.abe_share->open_args[0].list_path);
          name.replace (".pfm", ".bfd");

          strcpy (misc.abe_share->open_args[0].target_path, name.toLatin1 ());

          name = importTempFile + ".bfd";
          file.setFileName (name);
          file.rename (QString (misc.abe_share->open_args[0].target_path));

          update_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);
        }
      else
        {
          int32_t bfdh;
          BFDATA_HEADER bfd_header;
          BFDATA_RECORD bfd_record;
          BFDATA_POLYGON bfd_polygon;

          name = importTempFile + ".bfd";
          file.setFileName (name);
          char filename[512];
          strcpy (filename, name.toLatin1 ());

          if ((bfdh = binaryFeatureData_open_file (filename, &bfd_header, BFDATA_READONLY)) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (this, "pfmView", tr ("Unable to open feature file\nReason: ") + msg);
              binaryFeatureData_close_file (misc.bfd_handle);
              return;
            }


          //  Read the new feature file and append it to the existing feature file.

          for (uint32_t i = 0 ; i < bfd_header.number_of_records ; i++)
            {
              if (binaryFeatureData_read_record (bfdh, i, &bfd_record) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, "pfmView", tr ("Unable to read feature record\nReason: %1").arg (msg));
                  binaryFeatureData_close_file (misc.bfd_handle);
                  binaryFeatureData_close_file (bfdh);
                  return;
                }


              if (bfd_record.poly_count)
                {
                  if (binaryFeatureData_read_polygon (bfdh, i, &bfd_polygon) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (this, "pfmView", tr ("Unable to read feature polygon\nReason: %1").arg (msg));
                      binaryFeatureData_close_file (misc.bfd_handle);
                      binaryFeatureData_close_file (bfdh);
                      return;
                    }
                }


              if (binaryFeatureData_write_record (misc.bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, NULL) < 0)
                {
                  QString msg = QString (binaryFeatureData_strerror ());
                  QMessageBox::warning (this, "pfmView", tr ("Unable to write feature record\nReason: %1").arg (msg));
                  binaryFeatureData_close_file (misc.bfd_handle);
                  binaryFeatureData_close_file (bfdh);
                  return;
                }
            }


          binaryFeatureData_close_file (bfdh);

          file.remove ();
        }

      slotEditFeatureDataChanged ();
    }
}



//!  Import DNC feature information.

void 
pfmView::slotImport ()
{
  void importDNC (QString dncName, MISC *misc);


  QStringList files;
  QString file;


  if (!QDir (options.dnc_dir).exists ()) options.dnc_dir = options.input_pfm_dir;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Import DNC data"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.output_area_dir);


  fd->setFileMode (QFileDialog::DirectoryOnly);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          options.dnc_dir = fd->directory ().absolutePath ();

          importProc = new QProcess (this);

          QStringList arguments;


          arguments += file;
          importTempFile.sprintf ("./pfmView_import_%05d", misc.process_id);
          arguments += importTempFile;

          connect (importProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                   SLOT (slotImportDone (int, QProcess::ExitStatus)));
          connect (importProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotImportReadyReadStandardError ()));
          connect (importProc, SIGNAL (error (QProcess::ProcessError)), this, 
                   SLOT (slotImportError (QProcess::ProcessError)));


          importProc->start (QString (options.import_prog), arguments);
        }
    }
}



//!  Change the feature file associated with the top (0) layer PFM.

void 
pfmView::slotChangeFeature ()
{
  QStringList files, filters;
  QString file;
  QFileInfo fi = QFileInfo (QString (misc.abe_share->open_args[0].target_path));
  QDir dir = fi.dir ();


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Change feature file"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_pfm_dir);


  filters << tr ("Binary Feature Data file (*.bfd)")
          << tr ("NAVO standard target file (*.xml)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("Binary Feature Data file (*.bfd)"));
  fd->selectFile (QString (misc.abe_share->open_args[0].target_path));


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);

      if (!file.isEmpty())
        {
          strcpy (misc.abe_share->open_args[0].target_path, file.toLatin1 ());

          update_target_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].target_path);

          slotEditFeatureDataChanged ();
        }
    }
}



//!  Change the mosiac (GeoTIFF) file associated with the top (0) layer PFM.

void 
pfmView::slotChangeMosaic ()
{
  QStringList files, filters;
  QString file;

  QFileInfo fi = QFileInfo (QString (misc.abe_share->open_args[0].image_path));
  QDir dir = fi.dir ();


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Change mosaic file"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, dir.absolutePath ());


  filters << tr ("GeoTIFF mosaic file (*.tif *.TIF)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("GeoTIFF mosaic file (*.tif *.TIF)"));
  fd->selectFile (QString (misc.abe_share->open_args[0].image_path));


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);

      if (!file.isEmpty())
        {
          strcpy (misc.abe_share->open_args[0].image_path, file.toLatin1 ());

          update_mosaic_file (misc.pfm_handle[0], misc.abe_share->open_args[0].list_path, misc.abe_share->open_args[0].image_path);


          startMosaic->setEnabled (true);


          //  If the mosaic viewer was running, tell it to switch files.

          misc.abe_share->key = MOSAICVIEW_FORCE_RELOAD;
        }
    }
}



//!  This is the stderr read return from the mosaic viewer QProcess.  Hopefully you won't see anything here.

void 
pfmView::slotMosaicReadyReadStandardError ()
{
  readStandardError (mosaicProc);
}



//!  Error return from the mosaic viewer process.

void 
pfmView::slotMosaicError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("Unable to start the mosaic viewer process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("There was a write error to the mosaic viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("There was a read error from the mosaic viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView mosaic viewer"), tr ("The mosaic viewer process died with an unknown error!"));
      break;
    }

  programTimerCount = 0;
  programTimer->stop ();
  qApp->restoreOverrideCursor ();
}



//!  This is the return from the mosaic viewer QProcess (when finished normally)

void 
pfmView::slotMosaicDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (mosaicProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView Mosaic"), tr ("An error occurred while running the mosaic viewer"));
    }
}



//!  This starts the mosaic viewer.  Most people will use the pfm3D viewer and drape the mosaic on it instead of using this.

void 
pfmView::slotStartMosaicViewer ()
{
  if (!strcmp (misc.abe_share->open_args[0].image_path, "NONE")) return;


  if (mosaicProc)
    {
      if (mosaicProc->state () == QProcess::Running)
        {
          disconnect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
          disconnect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));
          disconnect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));

          mosaicProc->kill ();
        }
      delete mosaicProc;
    }


  program = options.mosaic_prog.section (' ', 0, 0);

  mosaicProc = new QProcess (this);

  QStringList arguments;
  arguments.clear ();

  arguments += QString (misc.abe_share->open_args[0].image_path);
  arguments += "--actionkey00=" + options.mosaic_actkey.section (',', 0, 0);
  arguments += "--actionkey01=" + options.mosaic_actkey.section (',', 1, 1);
  arguments += "--actionkey02=" + options.mosaic_actkey.section (',', 2, 2);
  arguments += QString ("--shared_memory_key=%1").arg (misc.abe_share->ppid);


  QString command;

  command = options.mosaic_prog;
  command.remove (" [MOSAIC_FILE]");


  connect (mosaicProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotMosaicDone (int, QProcess::ExitStatus)));
  connect (mosaicProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMosaicError (QProcess::ProcessError)));
  connect (mosaicProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMosaicReadyReadStandardError ()));


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();
  misc.abe_share->key = WAIT_FOR_START;


  mosaicProc->start (command, arguments);


  //  Starting a half second timer so that we can set the wait cursor.  If we use the started signal from QProcess
  //  the return is instant even though the mosaic viewer isn't visible.  This is just to give the user a warm and
  //  fuzzy.  If the WAIT_FOR_START flag isn't reset in 5 seconds we release anyway.

  programTimerCount = 0;
  programTimer->start (500);
}



//!  This is just to let you know that the ancillary program you tried to start is trying to start ;-)

void 
pfmView::slotProgramTimer ()
{
  if (programTimerCount > 10 || misc.abe_share->key != WAIT_FOR_START)
    {
      programTimer->stop ();

      if (programTimerCount > 10)
        {
          QMessageBox::information (this, "pfmView", tr ("The program %1 has been started but may take some time to appear.\n"
                                                              "Please be patient, it will appear eventually (or die trying ;-)\n\n"
                                                              "IMPORTANT NOTE: Due to processing overhead the system may "
                                                              "freeze for a few moments when %1 is almost ready to appear.  Again, please be patient.").arg (program));
          qApp->processEvents ();
        }

      qApp->restoreOverrideCursor ();
      programTimerCount = 0;
    }
  else
    {
      programTimerCount++;
    }
}



//!  This is the stderr read return from the 3D viewer QProcess.  Hopefully you won't see anything here.

void 
pfmView::slot3DReadyReadStandardError ()
{
  readStandardError (threeDProc);
}



//!  This is the error return from the 3D viewer process.

void 
pfmView::slot3DError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("Unable to start the 3D viewer process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("There was a write error to the 3D viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("There was a read error from the 3D viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmView 3D viewer"), tr ("The 3D viewer process died with an unknown error!"));
      break;
    }

  programTimerCount = 0;
  programTimer->stop ();
  qApp->restoreOverrideCursor ();
}



//!  This is the return from the 3D viewer QProcess (when finished normally)

void 
pfmView::slot3DDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (threeDProc->exitCode ())
    {
      QMessageBox::critical (this, tr ("pfmView 3D Viewer"), tr ("An error occurred while running the 3D viewer"));
    }
}



//!  This starts the 3D surface viewer (with or without draped GeoTIFF data).

void 
pfmView::slotStart3DViewer ()
{
  if (threeDProc)
    {
      if (threeDProc->state () == QProcess::Running)
        {
          disconnect (threeDProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slot3DDone (int, QProcess::ExitStatus)));
          disconnect (threeDProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slot3DError (QProcess::ProcessError)));
          disconnect (threeDProc, SIGNAL (readyReadStandardError ()), this, SLOT (slot3DReadyReadStandardError ()));

          threeDProc->kill ();
        }
      delete threeDProc;
    }


  program = options.threeD_prog.section (' ', 0, 0);

  threeDProc = new QProcess (this);

  QStringList arguments;
  arguments.clear ();

  arguments += QString (misc.abe_share->open_args[0].image_path);
  arguments += QString ("--shared_memory_key=%1").arg (misc.abe_share->ppid);


  QString command;

  command = options.threeD_prog;


  connect (threeDProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slot3DDone (int, QProcess::ExitStatus)));
  connect (threeDProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slot3DError (QProcess::ProcessError)));
  connect (threeDProc, SIGNAL (readyReadStandardError ()), this, SLOT (slot3DReadyReadStandardError ()));


  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();
  misc.abe_share->key = WAIT_FOR_START;


  threeDProc->start (command, arguments);


  //  Starting a half second timer so that we can set the wait cursor.  If we use the started signal from QProcess
  //  the return is instant even though the 3D viewer isn't visible.  This is just to give the user a warm and
  //  fuzzy.  If the WAIT_FOR_START flag isn't reset in 5 seconds we release anyway.

  programTimerCount = 0;
  programTimer->start (500);
}



//!  Export the displayed view to a graphics file (jpg, GeoTIFF, etc.).

void 
pfmView::slotExportImage ()
{
  void export_image (QWidget *parent, MISC *misc, OPTIONS *options, nvMap *map);


  QStringList lst;
  bool ok;

  int32_t w = misc.displayed_area_width[0];
  int32_t h = misc.displayed_area_height[0];

  QString pfm_str = tr ("%1x%2 (PFM resolution)").arg (w).arg (h);
  QString screen_str = tr ("%1x%2 (Screen resolution)").arg (mapdef.draw_width).arg (mapdef.draw_height);

  lst << pfm_str << screen_str << tr ("1280x1024") << tr ("2560x2048") << tr ("5120x4096") << tr ("10240x8192");


  QString res = QInputDialog::getItem (this, "pfmView", tr ("Image resolution"), lst, 0, false, &ok);

  if (ok && !res.isEmpty ())
    {
      w = res.section ('x', 0, 0).toInt ();


      //  Make sure we don't have a problem with the parentheses on the first two options.

      if (res.contains ('('))
        {
          h = res.section ('x', 1, 1).section (' ', 0, 0).toInt ();
        }
      else
        {
          h = res.section ('x', 1, 1).toInt ();
        }

      map->dummyBuffer (w, h, NVTrue);

      qApp->setOverrideCursor (Qt::WaitCursor);
      qApp->processEvents ();

      redrawMap (NVTrue, NVTrue);

     export_image (this, &misc, &options, map);

      qApp->restoreOverrideCursor ();


      map->dummyBuffer (0, 0, NVFalse);
    }

  redrawMap (NVTrue, NVTrue);
 }



//!  Clear the filter masks.

void 
pfmView::clearFilterMasks ()
{
  //  Clear any filter masked areas.
  
  if (misc.poly_filter_mask_count)
    {
      misc.poly_filter_mask.clear ();
      misc.poly_filter_mask_count = 0;
    }
}



//!  Set the stoplight coloring boundaries.

void 
pfmView::setStoplight ()
{
  //  Check for stoplight display

  if (options.stoplight)
    {
      double res;
      bool ok;

      res = QInputDialog::getDouble (this, "pfmView", tr ("Enter the minimum to mid crossover value:"), options.stoplight_min_mid,
                                     -9500.0, 11500.0, 1, &ok);
      if (ok) options.stoplight_min_mid = (float) res;


      res = QInputDialog::getDouble (this, "pfmView", tr ("Enter the maximum to mid crossover value:"), options.stoplight_max_mid,
                                     -9500.0, 11500.0, 1, &ok);
      if (ok) options.stoplight_max_mid = (float) res;


      if (options.stoplight_max_mid < options.stoplight_min_mid)
        {
          float tmp = options.stoplight_max_mid;
          options.stoplight_max_mid = options.stoplight_min_mid;
          options.stoplight_min_mid = tmp;
        }
    }
  else
    {
      //  Make sure that the next time someone turns on stoplight it asks for new crossover values.

      options.stoplight_min_mid = options.stoplight_max_mid = 0.0;
    }


  QString tip = tr ("Toggle stoplight display [crossovers at %L1 and %L2]").arg (options.stoplight_min_mid, 2, 'f', 1).arg (options.stoplight_max_mid, 2, 'f', 1);
  bStoplight->setToolTip (tip);
}



void
pfmView::setStatusAttributes ()
{
  //  Set the attributes for the current "top" PFM.

  for (int32_t i = 0 ; i < NUM_ATTR ; i++)
    {
      misc.attrBoxFlag[i] = 0;

      if (i < misc.abe_share->open_args[0].head.num_bin_attr)
        {
          //  Check to see if the highest layer PFM has CUBE attributes.

          if (!strcmp (misc.abe_share->open_args[0].head.bin_attr_name[i], "###0")) misc.cube_attr_available[0] = NVTrue;

          QString tmp = QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[i]));

          attrBoxName[i]->setText (tmp);
          attrBoxName[i]->setToolTip (tmp +
                                      QString (tr (" - Click to place this attribute in the first attribute slot on the Status tab.")));
          attrBoxName[i]->setWhatsThis (tmp +
                                        QString (tr ("<br><br>Click to place this attribute in the first attribute slot on the "
                                                     "<b>Status</b> tab.  Cyan fade indicates the attribute is already on the "
                                                     "<b>Status</b> tab.  Red fade indicates that it is on the <b>Status</b> tab "
                                                     "and that the surface is colored by this attribute.")));

          attrBoxValue[i]->setToolTip (tr ("This is the value of %1 at the cursor").arg (tmp));

          attrBoxName[i]->show ();
          attrBoxValue[i]->show ();
        }
      else
        {
          attrBoxName[i]->setText ("");

          attrBoxValue[i]->setText ("");

          attrBoxName[i]->hide ();
          attrBoxValue[i]->hide ();
        }
    }


  //  Figure out which attributes that were in the Status tab attributes we still have.

  int32_t atcnt = 0;

  for (int32_t i = 0 ; i < POST_ATTR ; i++)
    {
      misc.attrStatNum[i] = -1;

      for (int32_t j = 0 ; j < misc.abe_share->open_args[0].head.num_bin_attr ; j++)
        {
          QString tmp = QString (pfm_standard_attr_name (1, 0, misc.abe_share->open_args[0].head.bin_attr_name[j]));

          if (options.attrStatName[i] == tmp)
            {
              misc.attrBoxFlag[j] = 1;
              misc.attrStatNum[atcnt] = j;
              options.attrStatName[atcnt] = tmp;
              attrName[atcnt + PRE_ATTR]->setText (tmp);
              atcnt++;
              break;
            }
        }
    }
}
