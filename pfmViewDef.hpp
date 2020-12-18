
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



#ifndef _PFM_VIEW_DEF_H_
#define _PFM_VIEW_DEF_H_


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "nvutility.h"
#include "nvutility.hpp"
#include "attributes.hpp"


#include "binaryFeatureData.h"


#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <proj_api.h>

#include "shapefil.h"

#include "pfm.h"
#include "pfm_extras.h"

#include <QtCore>
#include <QtGui>

#ifdef ET_PHONE_HOME
#include <QtNetwork>
#endif

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QSharedMemory>


using namespace std;  // Windoze bullshit - God forbid they should follow a standard


//  Pointer interaction functions.

#define         RECT_EDIT_AREA_3D           0   //!<  Define a rectangular area to edit in 3D
#define         POLY_EDIT_AREA_3D           1   //!<  Define a polygonal area to edit in 3D
#define         ZOOM_IN_AREA                2   //!<  Define a rectangular area to zoom in to
#define         OUTPUT_POINTS               3   //!<  Select data points to output to an ASCII file
#define         DEFINE_RECT_AREA            4   //!<  Define a rectangular area to save as a generic or Army Corps area file
#define         DEFINE_POLY_AREA            5   //!<  Define a polygonal area to save as a generic or Army Corps area file
#define         ADD_FEATURE                 6   //!<  Add a feature to the feature file (optionally creates a feature file)
#define         DELETE_FEATURE              7   //!<  Delete a feature from the feature file
#define         EDIT_FEATURE                8   //!<  Edit a feature in the feature file
#define         DRAW_CONTOUR                9   //!<  Draw a contour to be used for redefining the MISP surface
#define         REMISP_AREA                 10  //!<  Define a rectangular area to regrid with MISP
#define         DRAW_CONTOUR_FILTER         11  //!<  Draw a contour to be used for the MISP surface filter
#define         REMISP_FILTER               12  //!<  Define a polygonal area in which to perform the remisp filter
#define         RECT_FILTER_AREA            13  //!<  Define a rectangular area to filter (statistically)
#define         POLY_FILTER_AREA            14  //!<  Define a polygonal area to filter (statistically)
#define         DEFINE_FEATURE_POLY_AREA    15  //!<  Define a polygon to be associated with a feature point
#define         POLY_FILTER_MASK            16  //!<  Define a polygonal area to mask from filtering
#define         RECT_FILTER_MASK            17  //!<  Define a rectangular area to mask from filtering
#define         SELECT_HIGH_POINT           18  //!<  Define a rectangular area in which to mark a high (minimum Z) point
#define         SELECT_LOW_POINT            19  //!<  Define a rectangular area in which to mark a low (maximum Z) point
#define         GRAB_CONTOUR                20  //!<  Define a polygonal area and insert interpolated contours into the bins
#define         GOOGLE_EARTH                21  //!<  Pick a point in the displayed area to pin when starting Google Earth


//  Coverage area pointer interaction functions.

#define         COV_START_DRAW_RECTANGLE    0   //!<  Start defining a rectangular area
#define         COV_DRAW_RECTANGLE          1   //!<  Define a rectangular area to view
#define         COV_START_DRAG_RECTANGLE    2   //!<  Start the process of dragging the rectangle
#define         COV_DRAG_RECTANGLE          3   //!<  Drag the current rectangular area to a new position


//  Highlight types.

#define         H_NONE                      0   //!<  No highlighting
#define         H_ALL                       1   //!<  Highlight everything (essentially gray shade)
#define         H_CHECKED                   2   //!<  Highlight checked data
#define         H_MODIFIED                  3   //!<  Highlight modified data
#define         H_01                        4   //!<  Highlight bins containing PFM_USER_01 marked data
#define         H_02                        5   //!<  Highlight bins containing PFM_USER_02 marked data
#define         H_03                        6   //!<  Highlight bins containing PFM_USER_03 marked data
#define         H_04                        7   //!<  Highlight bins containing PFM_USER_04 marked data
#define         H_05                        8   //!<  Highlight bins containing PFM_USER_05 marked data
#define         H_06                        9   //!<  Highlight bins containing PFM_USER_06 marked data
#define         H_07                        10  //!<  Highlight bins containing PFM_USER_07 marked data
#define         H_08                        11  //!<  Highlight bins containing PFM_USER_08 marked data
#define         H_09                        12  //!<  Highlight bins containing PFM_USER_09 marked data
#define         H_10                        13  //!<  Highlight bins containing PFM_USER_10 marked data
#define         H_INT                       14  //!<  Highlight interpolated (MISP) bins
#define         H_MULT                      15  //!<  Highlight bins with data from multiple lines
#define         H_COUNT                     16  //!<  Highlight bins with more than options.h_count points
#define         H_IHO_S                     17  //!<  Highlight bins containing possible IHO Special Order outliers
#define         H_IHO_1                     18  //!<  Highlight bins containing possible IHO Order 1 outliers
#define         H_IHO_2                     19  //!<  Highlight bins containing possible IHO Order 2 outliers
#define         H_PERCENT                   20  //!<  Highlight bins containing outliers that fall outside a user defined percentage of depth

#define         NUM_HIGHLIGHTS              21


#define         NUMSHADES                   256    //!<  Number of shades to be used for sunshading
#define         NUMHUES                     128    //!<  Number of hues to be used for coloring the surface
#define         NUM_SURFACES                12     //!<  Number of available surface types (including OTF)
#define         NUM_SCALE_LEVELS            17     //!<  Number of color scale boxes
#define         CONTOUR_POINTS              1000   //!<  Max number of points in a drawn contour
#define         DEFAULT_SEGMENT_LENGTH      0.25   //!<  Default segment length for contours in scribe.cpp
#define         NUM_OVERLAYS                10     //!<  Max number of overlays
#define         LINE_WIDTH                  2      //!<  Standard line width
#define         COV_FEATURE_SIZE            2      //!<  Standard coverage feature size
#define         POLYGON_POINTS              NVMAP_POLYGON_POINTS   //!<  Maximum number of points in a polygon
#define         NUM_TOOLBARS                8      //!<  Number of toolbars defined
#define         OTF_OFFSET                  100    //!<  Arbitrary number used to offset the MIN_FILTERED_DEPTH and MAX_FILTERED_DEPTH values
                                                   //!<  in order to indicate that we are doing on-the-fly binning to build the surface
#define         MARKER_W                    15     //!<  Marker (cursor point tracker) width
#define         MARKER_H                    10     //!<  Marker height
#define         MAX_RECENT                  10     //!<  Maximum number of files in the Open Recent... file list
#define         PRE_ATTR                    5      //!<  Number of built in attributes for coloring prior to the PFM bin attributes
#define         POST_ATTR                   10     //!<  Number of attribute slots on "Status" tab.


//  Button hotkeys that are editable by the user.  These are used as indexes into the options.buttonAccel, misc.buttonText,
//  misc.button, and misc.buttonIcon arrays.  To add or remove from this list you need to change it here, in set_defaults.cpp
//  and in pfmView.cpp (look for the tool buttons that have one of these defined names attached).

#define         REDRAW_KEY                  0
#define         REDRAW_COVERAGE_KEY         1
#define         TOGGLE_CONTOUR_KEY          2
#define         TOGGLE_GRID_KEY             3
#define         DISPLAY_AVG_EDITED_KEY      4
#define         DISPLAY_MIN_EDITED_KEY      5
#define         DISPLAY_MAX_EDITED_KEY      6
#define         DISPLAY_AVG_UNEDITED_KEY    7
#define         DISPLAY_MIN_UNEDITED_KEY    8
#define         DISPLAY_MAX_UNEDITED_KEY    9
#define         DISPLAY_MIN_EDITED_OTF_KEY  10
#define         DISPLAY_MAX_EDITED_OTF_KEY  11
#define         TOGGLE_GEOTIFF_KEY          12
#define         EDIT_MODE_KEY               13

#define         HOTKEYS                     14


//  Overlay types

#define         ISS60_AREA                  1
#define         GENERIC_AREA                2
#define         ISS60_ZONE                  3
#define         GENERIC_ZONE                4
#define         ISS60_SURVEY                5
#define         TRACK                       6
#define         GENERIC_YXZ                 7
#define         SHAPE                       8
#define         ACE_AREA                    9


//!  Filter undo structure

typedef struct
{
  int8_t            pfm;
  NV_I32_COORD2     coord;
  int32_t           index;
  uint32_t          val;
} UNDO_RECORD;


//!  Output data points structure

typedef struct
{
  NV_F64_COORD2   xy;
  float           avg;
  float           min;
  float           max;
} OUTPUT_DATA_POINTS;


//!  Overlay structure.

typedef struct
{
  int32_t     file_type;                  //!<  File type (see ISS60_AREA above)
  uint8_t     display;                    //!<  Set to NVTrue if it is to be displayed
  QColor      color;                      //!<  Color to draw the overlay
  char        filename[512];              //!<  Overlay file name
} OVERLAY;


//!  Filter mask structure.

typedef struct
{
  double      x[POLYGON_POINTS];          //!<  Array of polygon X values
  double      y[POLYGON_POINTS];          //!<  Array of polygon Y values
  int32_t     count;                      //!<  Number of points in the mask polygon
  uint8_t     displayed;                  //!<  Set to NVTrue if this mask has been displayed since last redraw
} FILTER_MASK;


//!  The OPTIONS structure contains all those variables that can be saved to the users pfmView QSettings.

typedef struct
{
  uint8_t     contour;                    //!<  To contour or not to contour...
  uint8_t     coast;                      //!<  Coastline flag
  uint8_t     cov_coast;                  //!<  Coverage coastline flag
  int32_t     position_form;              //!<  Position format number
  int32_t     contour_width;              //!<  Contour line width/thickness in pixels
  int32_t     contour_index;              //!<  Contour index (Nth contour emphasized)
  QColor      contour_color;              //!<  Color to be used for contours
  QColor      contour_highlight_color;    //!<  Color to be used for highlighted contours
  QColor      feature_color;              //!<  Color to be used for features
  QColor      feature_info_color;         //!<  Color to be used for feature information text display
  QColor      feature_poly_color;         //!<  Color to be used for feature polygon display
  QColor      verified_feature_color;     //!<  Color to be used for verified features (confidence = 5)
  QColor      coast_color;                //!<  Color to be used for coast
  QColor      background_color;           //!<  Color to be used for background
  QColor      cov_feature_color;          //!<  Color to be used for features on coverage map
  QColor      cov_inv_feature_color;      //!<  Color to be used for invalid features on the coverage map
  QColor      cov_verified_feature_color; //!<  Color to be used for verified features (confidence = 5) on the coverage map
  QColor      poly_filter_mask_color;     //!<  Color to be used for filter masks
  QColor      cov_coast_color;            //!<  Color to be used for coastline on the coverage map
  QColor      cov_box_color;              //!<  Color to be used for the edit box on the coverage map
  QColor      cov_pfm_box_hot_color;      //!<  Color to be used for the PFM surrounding boxes of the highest active layer in the coverage map
  QColor      cov_pfm_box_cold_color;     //!<  Color to be used for the PFM surrounding boxes of the other active layers in the coverage map
  QColor      cov_data_color;             //!<  Color to be used for normal data areas in the coverage map
  QColor      cov_background_color;       //!<  Color to be used for empty data areas (e.g. background) in the coverage map
  QColor      cov_checked_color;          //!<  Color to be used for checked data areas in the coverage map
  QColor      cov_verified_color;         //!<  Color to be used for verified data areas in the coverage map
  QColor      scaled_offset_z_color;      //!<  Color to be used for printing scale or offset Z values in the information box
  QFont       font;                       //!<  Font used for pfmView and all other ABE map GUI applications (change in Preferences dialog)
  uint8_t     stoplight;                  //!<  Set if we're going to display using stoplight coloring
  float       stoplight_min_mid;          //!<  Value for crossover from minimum to mid
  float       stoplight_max_mid;          //!<  Value for crossover from amximum to mid
  QColor      stoplight_max_color;        //!<  Color to be used for stoplight display max (normally red for depth, green for other).
  QColor      stoplight_mid_color;        //!<  Color to be used for stoplight display mid (normally yellow).
  QColor      stoplight_min_color;        //!<  Color to be used for stoplight display min (normally green for depth, red for other).
  int32_t     stoplight_min_index;        //!<  Index into the HSV color arry for the min stoplight color (not saved to qsettings)
  int32_t     stoplight_mid_index;        //!<  Index into the HSV color arry for the mid stoplight color (not saved to qsettings)
  int32_t     stoplight_max_index;        //!<  Index into the HSV color arry for the max stoplight color (not saved to qsettings)
  uint16_t    min_hsv_color[NUM_SCALE_LEVELS];     //!<  HSV color wheel index for color to be used for minimum values in surface (default 0)
  uint16_t    max_hsv_color[NUM_SCALE_LEVELS];     //!<  HSV color wheel index for color to be used for maximum values in surface (default 315)
  float       min_hsv_value[NUM_SCALE_LEVELS];     //!<  Locked minimum data value for minimum HSV color
  float       max_hsv_value[NUM_SCALE_LEVELS];     //!<  Locked maximum data value for maximum HSV color
  uint8_t     min_hsv_locked[NUM_SCALE_LEVELS];    //!<  Set if min HSV is locked to a value
  uint8_t     max_hsv_locked[NUM_SCALE_LEVELS];    //!<  Set if max HSV is locked to a value
  char        edit_name_3D[256];          //!<  3D Editor name
  SUN_OPT     sunopts;                    //!<  Sun shading options
  uint8_t     display_suspect;            //!<  Flag to show suspect data
  int32_t     display_feature;            //!<  0 = no features, 1 = all features, 2 = unverified features, 3 = verified features,
  uint8_t     display_children;           //!<  Flag to show grouped feature children locations
  uint8_t     display_selected;           //!<  Flag to show selected soundings
  uint8_t     display_reference;          //!<  Flag to show reference soundings
  uint8_t     display_feature_info;       //!<  Flag to show feature info
  uint8_t     display_feature_poly;       //!<  Flag to show feature polygonal area if it exists
  uint8_t     display_minmax;             //!<  Flag to show min and max points
  int32_t     edit_mode;                  //!<  0 - rectangle, 1 - polygon
  uint8_t     auto_redraw;                //!<  Flag for auto redraw after edit
  char        unload_prog[256];           //!<  Unload program name
  char        import_prog[256];           //!<  DNC import program name
  int32_t     smoothing_factor;           //!<  Contour smoothing factor (0-10)
  float       z_factor;                   //!<  Conversion factor for Z values. May be used to convert m to ft...
  float       z_offset;                   //!<  Offset value for Z values.
  int32_t     highlight;                  //!<  See H_NONE and others above for definitions
  QString     buttonAccel[HOTKEYS];       //!<  Accelerator key sequences for some of the buttons
  float       feature_radius;             //!<  Filter exclusion distance for features
  int32_t     last_rock_feature_desc;     //!<  Index of last used rock, wreck, obstruction feature description
  int32_t     last_offshore_feature_desc; //!<  Index of last used offshore installation feature description
  int32_t     last_light_feature_desc;    //!<  Index of last used light, buoy, beacon feature description
  int32_t     last_lidar_feature_desc;    //!<  Index of last used lidar feature description
  QString     last_feature_description;   //!<  String containing the last feature descriptor used
  QString     last_feature_remarks;       //!<  String containing the last feature remarks used
  QString     feature_search_string;      //!<  Text to search for when highlighting features
  int32_t     misp_weight;                //!<  Weight value for misp_init in remisp
  uint8_t     misp_force_original;        //!<  Force original flag for remisp
  uint8_t     misp_replace_all;           //!<  Set if you want to replace the surface in bins with original input data
  float       filterSTD;                  //!<  Filter standard deviation
  uint8_t     deep_filter_only;           //!<  Filter only in the downward direction (assumes positive is depth)
  int32_t     overlap_percent;            //!<  Window overlap percentage when moving with arrow keys
  int32_t     GeoTIFF_alpha;              //!<  25 - almost invisible, 255 - opaque, range in between - some transparency
  QString     mosaic_prog;                //!<  Mosaic viewer program - from pfmEdit3D via abe_share or QSettings
  QString     mosaic_hotkey;              //!<  mosaicView hot keys - from pfmEdit3D via abe_share or QSettings
  QString     mosaic_actkey;              //!<  mosaicView hot keys - from pfmEdit3D via abe_share or QSettings
  QString     threeD_prog;                //!<  3D viewer program.
  QString     threeD_hotkey;              //!<  3D viewer hot keys.
  QString     threeD_actkey;              //!<  3D viewer hot keys.
  QString     overlay_dir;                //!<  Last directory searched for overlays
  QString     output_area_dir;            //!<  Last directory in which an area file was saved
  QString     output_area_name_filter;    //!<  Last output area file name filter used (*.are, *.afs, *.shp)
  QString     output_points_dir;          //!<  Last directory in which an output data points file was saved
  QString     output_shape_dir;           //!<  Last directory in which a shape file of feature polygons was written
  QString     input_pfm_dir;              //!<  Last directory searched for PFM files
  QString     geotiff_dir;                //!<  Last directory searched for GeoTIFF files
  QString     area_dir;                   //!<  Last directory searched for area files
  QString     dnc_dir;                    //!<  Last directory searched for DNC files
  QString     recentFile[MAX_RECENT];     //!<  Saved file names for Open Recent...
  int32_t     recent_file_count;          //!<  Number of files in Open Recent... list
  float       highlight_percent;          //!<  Percentage of depth for highlight.
  int32_t     chart_scale;                //!<  Chart scale used for defining feature circle radius for polygon definition
  uint8_t     fixed_feature_size;         //!<  Set to True to use a fixed feature size
  int32_t     millimeters;                //!<  Millimeters at chart scale (used to define feature size)
  int32_t     layer_type;                 //!<  Type of bin data/contour to display (same as misc.abe_share->layer_type)
  double      otf_bin_size_meters;        //!<  Bin size for on-the-fly gridding
  float       cint;                       //!<  Contour interval (same as misc.abe_share->cint)
  int32_t     num_levels;                 //!<  Number of contour levels (same as misc.abe_share->num_levels)
  float       contour_levels[200];        //!<  User defined contour levels (same as misc.abe_share->contour_levels)
  int32_t     screenshot_delay;           //!<  Delay (in seconds) of the screenshot in the updateFeature dialog.
  uint8_t     group_features;             //!<  Group feature flag for Define Feature Polygon entry of the Utilities menu
  float       contour_filter_envelope;    //!<  Contour filter envelope value
  int32_t     main_button_icon_size;      //!<  Main button icon size (16, 24, 32)
  QColor      color_array[2][NUMHUES + 1][NUMSHADES]; //!<  arrays of surface colors
  uint8_t     zero_turnover;              //!<  Set if we restart the colors at zero
  int32_t     feature_search_type;        //!<  0 = highlight result of search, 1 = only display result of search
  uint8_t     feature_search_invert;      //!<  if set, search for features NOT containing search string
  int32_t     h_count;                    //!<  Highlight bins with more than this number of points
  uint8_t     auto_close_unload;          //!<  If set, automatically close unload dialogs on normal finish
  int32_t     msg_width;                  //!<  Message window width
  int32_t     msg_height;                 //!<  Message window height
  int32_t     msg_window_x;               //!<  Message window x position
  int32_t     msg_window_y;               //!<  Message window y position
  uint8_t     editor_no_load[2];          //!<  This is set to run pfmEdit3D in "no invalid" [0] and/or "no reference" [1] mode.
  float       z_orientation;              /*!<  This is set to 1.0 to display numeric data to the user as depths or -1.0 to display them as
                                                elevations.  This value is stored in a separate QSettings .ini file because it is used by a
                                                number of other programs (e.g. pfmEdit3D).  */
  int32_t     pref_tab;                   /*!<  This is the tab number of the last Preferences tab viewed (so we can go back to the same tab
                                                to start).  */
  uint8_t     lat_or_east;                //!<  Set to 0 to display latitude and longitude, set to 1 to display northing and easting.
  QString     attrFilterText[NUM_ATTR];   //!<  Text ranges for the attribute limits on the on-the-fly (OTF) bin generation.
  QString     attrFilterName;             //!<  This is the name of the currently active OTF limiting attribute
  QString     attrStatName[POST_ATTR];    //!<  Names of selected attributes on the Status tab
  uint8_t     default_feature_type;       //!<  Default feature type used when creating features (BFDATA_HYDROGRAPHIC or BFDATA_INFORMATIONAL).

  uint8_t     overlay_grid;               //!<  True if user wants grid overlay
  int32_t     overlay_grid_type;          //!<  0 = spacing in meters, 1 = spacing in minutes
  float       overlay_grid_spacing;       //!<  Overlay grid spacing in (nominal) meters
  float       overlay_grid_minutes;       //!<  Overlay grid spacing in decimal minutes
  int32_t     overlay_grid_width;         //!<  Overlay grid line width/thickness in pixels
  QColor      overlay_grid_color;         //!<  Color to be used for overlay grid
  int32_t     default_chart_scale;        //!<  Default displayed area chart scale
  float       default_width;              //!<  Default displayed area chart width
  float       default_height;             //!<  Default displayed area chart height
  QString     ge_name;                    //!<  Name of the Google Earth executable or script



  //  The following relate to PFM attributes.

  QString     time_attr_filt;
  QString     gsf_attr_filt[GSF_ATTRIBUTES];
  QString     hof_attr_filt[HOF_ATTRIBUTES];
  QString     tof_attr_filt[TOF_ATTRIBUTES];
  QString     czmil_attr_filt[CZMIL_ATTRIBUTES];
  QString     las_attr_filt[LAS_ATTRIBUTES];
  QString     bag_attr_filt[BAG_ATTRIBUTES];
  ATTR_BOUNDS time_attribute;
  int16_t     time_attribute_num;
  ATTR_BOUNDS gsf_attribute[GSF_ATTRIBUTES];
  int16_t     gsf_attribute_num[GSF_ATTRIBUTES];
  ATTR_BOUNDS hof_attribute[HOF_ATTRIBUTES];
  int16_t     hof_attribute_num[HOF_ATTRIBUTES];
  ATTR_BOUNDS tof_attribute[TOF_ATTRIBUTES];
  int16_t     tof_attribute_num[TOF_ATTRIBUTES];
  ATTR_BOUNDS czmil_attribute[CZMIL_ATTRIBUTES];
  int16_t     czmil_attribute_num[CZMIL_ATTRIBUTES];
  ATTR_BOUNDS las_attribute[LAS_ATTRIBUTES];
  int16_t     las_attribute_num[LAS_ATTRIBUTES];
  ATTR_BOUNDS bag_attribute[BAG_ATTRIBUTES];
  int16_t     bag_attribute_num[BAG_ATTRIBUTES];
  int16_t     czmil_flag_count;
  QString     czmil_flag_name[CZMIL_FLAGS];
  int16_t     czmil_flag_num[CZMIL_FLAGS];


  //!  CUBE parameters (see CUBE documentation)

  int32_t     iho;
  double      capture;
  int32_t     queue;
  double      horiz;
  double      distance;
  double      min_context;
  double      max_context;
  int32_t     std2conf;                   //!<  This is one less than the number needed in navo_pfm_cube
  int32_t     disambiguation;
} OPTIONS;


//!  General stuff (miscellaneous).

typedef struct
{
  int32_t     function;                   //!<  Active edit or zoom function
  int32_t     save_function;              //!<  Save last function
  int32_t     last_edit_function;         //!<  Last edit function (so we can always come up in edit mode).
  int32_t     cov_function;               //!<  Active coverage map function
  uint8_t     coverage_drawing;           //!<  Set if coverage is drawing
  int32_t     poly_count;                 //!<  Polygon point count
  double      polygon_x[POLYGON_POINTS];  //!<  Polygon X
  double      polygon_y[POLYGON_POINTS];  //!<  Polygon Y
  int32_t     poly_filter_mask_count;     //!<  Number of polygonal filter mask areas defined
  std::vector<FILTER_MASK> poly_filter_mask; //!<  Pointer to array of polygonal filter mask areas.
  std::vector<UNDO_RECORD> undo;          //!<  Filter undo record array pointer (filtered_count keeps track of the size).
  int32_t     filtered_count;             //!<  Number of points actually marked as invaid when the filter is run.
  int32_t     maxd;                       //!<  Maximum contour density per grid cell
  char        GeoTIFF_name[512];          //!<  GeoTIFF file name
  uint8_t     GeoTIFF_open;               //!<  Set if GeoTIFF file has been opened.
  uint8_t     GeoTIFF_init;               //!<  Set if GeoTIFF is new
  int32_t     display_GeoTIFF;            //!<  0 - no display, 1 - display on top, -1 display underneath data
  int32_t     color_index;                /*!<  0 for normal colors, otherwise 1 for number, 2 for std, 3 for max-min, 4 for number valid, 
                                                attribute + PRE_ATTR.  */
  int32_t     attrStatNum[POST_ATTR];     //!<  Numbers of attributes being displayed on the Status tab
  int32_t     attrBoxFlag[NUM_ATTR];      //!<  Set to 1 if attribute is being displayed on the Status tab, otherwise 0
  uint8_t     drawing;                    //!<  set if we are drawing the surface
  uint8_t     drawing_canceled;           //!<  set to cancel drawing
  uint8_t     surface_val;                //!<  surface valid or invalid
  uint8_t     prev_surface_val;           //!<  used to test for changed surface
  uint8_t     clear;                      //!<  clear entire area flag
  uint8_t     cov_clear;                  //!<  clear entire coverage area flag
  NV_F64_XYMBR displayed_area[MAX_ABE_PFMS]; //!<  displayed area for each of the PFM(s)
  NV_F64_XYMBR total_displayed_area;      //!<  displayed area for all of the displayed PFMs
  NV_F64_XYMBR prev_total_displayed_area; //!<  previously displayed total area
  int32_t     displayed_area_width[MAX_ABE_PFMS]; //!<  displayed area width in columns
  int32_t     displayed_area_height[MAX_ABE_PFMS]; //!<  displayed area height in rows
  int32_t     displayed_area_column[MAX_ABE_PFMS]; //!<  displayed area start column
  int32_t     displayed_area_row[MAX_ABE_PFMS]; //!<  displayed area start row
  float       displayed_area_min;         //!<  min Z value for displayed area
  float       displayed_area_max;         //!<  max Z value for displayed area
  float       displayed_area_range;       //!<  Z range for displayed area
  float       displayed_area_attr_min;    //!<  Minimum PFM attribute value (if coloring by attribute)
  float       displayed_area_attr_max;    //!<  Maximum PFM attribute value (if coloring by attribute)
  float       displayed_area_attr_range;  //!<  PFM attribute range (if coloring by attribute)
  BFDATA_SHORT_FEATURE *feature;          //!<  Feature (target) data array
  int32_t     bfd_handle;                 //!<  BFD (target) file handle
  BFDATA_HEADER  bfd_header;              //!<  Header for the current BFD file
  BFDATA_RECORD  new_record;              //!<  Record built when adding a new feature
  uint8_t     bfd_open;                   //!<  Set if we have an open bfd file
  int32_t     hatchr_start_x;             //!<  start column for hatchr
  int32_t     hatchr_end_x;               //!<  end column for hatchr
  int32_t     hatchr_start_y;             //!<  start row for hatchr
  int32_t     hatchr_end_y;               //!<  end row for hatchr
  float       *current_row;               //!<  One D array of data to pass to contour package
  float       *current_attr;              //!<  One D array of attribute data to pass to contour package
  uint8_t     *current_flags;             //!<  One D array of edit flags to pass to render function
  float       *next_row;                  //!<  One D array of data for sunshading
  OVERLAY     overlays[NUM_OVERLAYS];     //!<  overlay file information
  std::vector<OUTPUT_DATA_POINTS> output_points; //!<  output data points
  int32_t     output_point_count;         //!<  number of points for OUTPUT_POINTS
  NV_I32_COORD2 center_point;             //!<  center of drawing area in pixels
  float       sin_array[3600];            //!<  Pre-computed sine values at 1/10 degree
  float       cos_array[3600];            //!<  Pre-computed cosine values at 1/10 degree
  QIcon       buttonIcon[HOTKEYS];        //!<  Tool button icons for tools having accelerators
  QString     buttonText[HOTKEYS];        //!<  Tooltip text for the buttons that have editable accelerators
  QToolButton *button[HOTKEYS];           //!<  Buttons that have editable accelerators
  QString     html_help_tag;              /*!<  Help tag for the currently running section of code.  This comes from
                                                the PFM_ABE html help documentation.  */
  int32_t     nearest_feature;
  int32_t     nearest_pfm;
  float       draw_contour_level;         //!<  Value inserted when drawing contours.
  QColor      widgetForegroundColor;      //!<  The normal widget foreground color.
  int32_t     feature_polygon_flag;
  QSharedMemory *abeShare;                //!<  ABE's shared memory pointer.
  QSharedMemory *abeRegister;             //!<  ABE's process register
  ABE_SHARE   *abe_share;                 //!<  Pointer to the ABE_SHARE structure in shared memory.
  ABE_REGISTER *abe_register;             //!<  Pointer to the ABE_REGISTER structure in shared memory.
  uint8_t     linked;                     //!<  Set if we have linked to another ABE application.
  uint8_t     cube_available;             //!<  NVTrue if the navo_pfm_cube program is available (in the PATH)
  uint8_t     googleearth_available;      //!<  NVTrue if googleearth is available (in the PATH)
  QString     qsettings_app;              //!<  Application name for QSettings
  NV_I32_COORD2 add_feature_coord;        //!<  PFM bin coordinates used if we added a feature
  int32_t     add_feature_index;          //!<  PFM depth array index in add_feature_coord PFM bin if we added a feature
  QWidget     *map_widget;                //!<  Map widget;
  int32_t     cov_width;                  //!<  Coverage window width
  int32_t     cov_height;                 //!<  Coverage window height
  int32_t     width;                      //!<  Main window width
  int32_t     height;                     //!<  Main window height
  int32_t     window_x;                   //!<  Main window x position
  int32_t     window_y;                   //!<  Main window y position
  uint8_t     def_feature_poly;           //!<  Set to true when using Utilities->Define Feature Polygon
  float       feature_poly_radius;        //!<  Radius used for defining feature polygons based on chart scale
  uint8_t     dateline;                   //!<  Set to NVTrue if any of the PFMs cross the dateline.
  std::vector<NV_F64_COORD3> filt_contour; //!<  Contours drawn for filtering a MISP surface
  int32_t     filt_contour_count;         //!<  Number of filter contours drawn
  uint8_t     otf_surface;                //!<  Whether we're doing an on-the-fly binned surface instead of one of the
                                          //!<  normal surfaces
  OTF_GRID_RECORD *otf_grid;              //!<  Pointer to allocated on-the-fly grid
  QSharedMemory *otfShare;                //!<  otf_grid shared memory pointer
  int32_t     process_id;                 //!<  This program's process ID
  QStatusBar  *progStatus;                //!<  Progress (and message) status bar
  QProgressBar *statusProg;               //!<  Generic progress bar in status area
  QLabel      *statusProgLabel;           //!<  Generic progress bar label
  QPalette    statusProgPalette;          //!<  Background color palette for the progress bar label
  uint8_t     tposiafps;                  //!<  If set, don't show the famous TPOSIAFPS message again this session.
  scaleBox    *scale[NUM_SCALE_LEVELS];
  float       color_min;                  //!<  Minimum value used for color range
  float       color_max;                  //!<  Maximum value used for color range
  float       color_range;                //!<  Color range
  int8_t      unload_type[100];           //!<  The type of unload for each unload dialog, 0 = file, 1 = area, -1 = not running
  NV_F64_XYMBR unload_area[100];          //!<  The area for the unload process
  QPixmap     unload_cov_map;             //!<  Pixmap of the coverage map at the time of the unload
  int32_t     unload_window_x;            //!<  The unload dialog X location
  int32_t     unload_window_y;            //!<  The unload dialog Y location
  int32_t     unload_window_width;        //!<  The unload dialog width
  int32_t     unload_window_height;       //!<  The unload dialog height
  char        program_version[256];       //!<  Program version name, based on the ABE_CZMIL environment variable.
  char        progname[256];              /*!<  This is the program name.  It will be used in all output to stderr so that shelling programs (like abe)
                                                will know what program printed out the error message.  */
  QStringList *messages;                  //!<  Message list
  int32_t     num_messages;               //!<  Number of messages in messageList
  projPJ      pj_utm;                     //!<  Proj4 UTM projection structure
  projPJ      pj_latlon;                  //!<  Proj4 latlon projection structure
  int32_t     otf_attr;                   /*!<  This is set to -1 to indicate that we aren't limiting OTF binning by attribute.  Otherwise it is 
                                                set to the limiting attribute number (in the current PFM).  */


  //  The following values are used by displayMinMax and are computed in compute_layer_min_max.

  float       displayed_valid_min;        //!<  Minimum valid Z bin value for displayed area (regardless of surface viewed)
  NV_I32_COORD2 displayed_valid_min_coord;//!<  PFM coordinates of the min valid value in the displayed area
  int32_t     displayed_valid_min_pfm;    //!<  PFM layer of the min valid value in the displayed area
  float       displayed_valid_max;        //!<  Maximum valid Z value for displayed area (regardless of surface viewed)
  NV_I32_COORD2 displayed_valid_max_coord;//!<  PFM coordinates of the max valid value in the displayed area
  int32_t     displayed_valid_max_pfm;    //!<  PFM layer of the max valid value in the displayed area
  float       displayed_valid_std;        //!<  Maximum STD value for displayed area
  NV_I32_COORD2 displayed_valid_std_coord;//!<  PFM coordinates of the max STD in the displayed area
  int32_t     displayed_valid_std_pfm;    //!<  PFM layer of the max STD in the displayed area


  //  The following concern PFMs as layers.  There are a few things from ABE_SHARE that also need to be 
  //  addressed when playing with layers - open_args, display_pfm, and pfm_count.

  NV_F64_XYMBR total_mbr;                 //!<  MBR of all of the displayed PFMs
  double      ss_cell_size_x[MAX_ABE_PFMS]; //!<  Sunshading X cell size
  double      ss_cell_size_y[MAX_ABE_PFMS]; //!<  Sunshading Y cell size
  char        attr_format[MAX_ABE_PFMS][NUM_ATTR][20]; //!<  Formats to use for displaying attributes
  int32_t     pfm_handle[MAX_ABE_PFMS];   //!<  PFM file handle
  int32_t     average_type[MAX_ABE_PFMS]; /*!<  Type of average filtered surface, -1 - CUBE, 0 - normal, 1 - minimum MISP, 
                                                2 - average MISP, 3 - maximum MISP  */
  uint8_t     cube_attr_available[MAX_ABE_PFMS]; //!<  NVTrue is CUBE attributes are in the file
  uint8_t     pfm_alpha[MAX_ABE_PFMS];
  int32_t     last_saved_contour_record[MAX_ABE_PFMS]; //!<  Record number of the last record saved from the drawn contour file.
  uint8_t     contour_in_pfm[MAX_ABE_PFMS]; //!<  NVTrue if a drawn contour enters the PFM (temporary use)
} MISC;


#endif
