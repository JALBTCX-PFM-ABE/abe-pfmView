
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



#include "pfmView.hpp"


double settings_version = 8.50;


/*!
  These functions store and retrieve the program settings (environment) from a .ini file.  On both Linux and Windows
  the file will be called "misc->qsettings_app".ini (the name of the application with a .ini extension) and will be
  stored in a directory called ABE.config.  On Linux, the ABE.config directory will be stored in your $HOME directory.
  On Windows, it will be stored in your $USERPROFILE folder.  If you make a change to the way a variable is used and
  you want to force the defaults to be restored just change the settings_version to a newer number (I've been using
  the program version number from version.hpp - which you should be updating EVERY time you make a change to the
  program!).  You don't need to change the settings_version though unless you want to force the program to go back
  to the defaults (which can annoy your users).  So, the settings_version won't always match the program version.
*/

uint8_t envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  double saved_version = 0.0;
  QString string;
  QString buttonHotKey[HOTKEYS];


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + QString (misc->qsettings_app) + ".ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + QString (misc->qsettings_app) + ".ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup (misc->qsettings_app);


  //  Recent files, directories, and hot keys should never be affected by the settings version so we want to load them even if the settings version
  //  has changed.

  options->recent_file_count = settings.value (QString ("recent file count"), options->recent_file_count).toInt ();

  for (int32_t i = 0 ; i < options->recent_file_count ; i++)
    {
      string = QString ("recent file %1").arg (i);
      options->recentFile[i] = settings.value (string, options->recentFile[i]).toString ();
    }

  options->overlay_dir = settings.value (QString ("overlay directory"), options->overlay_dir).toString ();
  options->output_area_name_filter = settings.value (QString ("output area name filter"), options->output_area_name_filter).toString ();
  options->output_area_dir = settings.value (QString ("output area directory"), options->output_area_dir).toString ();
  options->output_points_dir = settings.value (QString ("output points directory"), options->output_points_dir).toString ();
  options->output_shape_dir = settings.value (QString ("output shape directory"), options->output_shape_dir).toString ();
  options->input_pfm_dir = settings.value (QString ("input pfm directory"), options->input_pfm_dir).toString ();
  options->geotiff_dir = settings.value (QString ("geotiff directory"), options->geotiff_dir).toString ();
  options->area_dir = settings.value (QString ("area directory"), options->area_dir).toString ();
  options->dnc_dir = settings.value (QString ("dnc directory"), options->dnc_dir).toString ();


  //  We're trying to save the hot keys since these are a pain for the user to change back if they've made major changes 
  //  to them.  We want to use whatever the user had saved even if we change the settings version.  We will still have to
  //  check for hot key conflicts though since there might be new keys with new default values.  To do this we're going
  //  to save the default hot keys (set in set_defaults.cpp) and then do a comparison.  If there are conflicts we'll replace 
  //  the user's setting with the default and then issue a warning.

  uint8_t hotkey_conflict = NVFalse;

  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      //  Save the default so we can check it later.

      buttonHotKey[i] = options->buttonAccel[i];


      QString string = misc->buttonText[i] + QString (" hot key");
      options->buttonAccel[i] = settings.value (string, options->buttonAccel[i]).toString ();
    }


  //  Make sure we have no hotkey duplications.

  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      for (int32_t j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (options->buttonAccel[i].toUpper () == options->buttonAccel[j].toUpper ())
            {
              hotkey_conflict = NVTrue;
              options->buttonAccel[i] = buttonHotKey[i];
              options->buttonAccel[j] = buttonHotKey[j];
            }
        }
    }


  if (hotkey_conflict)
    {
      QMessageBox::warning (0, QString ("pfmView settings"),
                            QString ("Hot key conflicts have been detected when reading program settings!<br><br>") +
                            QString ("Some of your hotkeys have been reset to default values!"));
    }


  //  Unfortunately, when I added the EDIT_MODE_KEY I fat-fingered it to "s" instead of "e".  So, now I'm going to force it back to
  //  "e" if it's set to "s".  This can be removed in a few months - JCD 10/18/15.

  if (options->buttonAccel[EDIT_MODE_KEY] == "s") options->buttonAccel[EDIT_MODE_KEY] = "e";


  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults (from set_defaults.cpp) since they may have changed.

  if (settings_version != saved_version) return (NVFalse);;


  options->contour = settings.value (QString ("contour flag"), options->contour).toBool ();

  options->group_features = settings.value (QString ("group features flag"), options->group_features).toBool ();

  options->contour_width = settings.value (QString ("contour width"), options->contour_width).toInt ();

  options->contour_index = settings.value (QString ("contour index"), options->contour_index).toInt ();


  options->coast = settings.value (QString ("coast flag"), options->coast).toBool ();
  options->cov_coast = settings.value (QString ("coverage coast flag"), options->cov_coast).toBool ();
  options->GeoTIFF_alpha = settings.value (QString ("geotiff alpha"), options->GeoTIFF_alpha).toInt ();


  options->filterSTD = settings.value (QString ("filter standard deviation"), options->filterSTD).toDouble ();

  options->deep_filter_only = settings.value (QString ("deep filter only flag"), options->deep_filter_only).toBool ();

  options->misp_weight = settings.value (QString ("misp weight"), options->misp_weight).toInt ();
  options->misp_force_original = settings.value (QString ("misp force original flag"), options->misp_force_original).toBool ();
  options->misp_replace_all = settings.value (QString ("misp replace all flag"), options->misp_replace_all).toBool ();

  options->position_form = settings.value (QString ("position form"), options->position_form).toInt ();

  int32_t red = settings.value (QString ("contour color/red"), options->contour_color.red ()).toInt ();
  int32_t green = settings.value (QString ("contour color/green"), options->contour_color.green ()).toInt ();
  int32_t blue = settings.value (QString ("contour color/blue"), options->contour_color.blue ()).toInt ();
  int32_t alpha = settings.value (QString ("contour color/alpha"), options->contour_color.alpha ()).toInt ();
  options->contour_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("feature color/red"), options->feature_color.red ()).toInt ();
  green = settings.value (QString ("feature color/green"), options->feature_color.green ()).toInt ();
  blue = settings.value (QString ("feature color/blue"), options->feature_color.blue ()).toInt ();
  alpha = settings.value (QString ("feature color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("feature info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (QString ("feature info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (QString ("feature info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (QString ("feature info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("feature poly color/red"), options->feature_poly_color.red ()).toInt ();
  green = settings.value (QString ("feature poly color/green"), options->feature_poly_color.green ()).toInt ();
  blue = settings.value (QString ("feature poly color/blue"), options->feature_poly_color.blue ()).toInt ();
  alpha = settings.value (QString ("feature poly color/alpha"), options->feature_poly_color.alpha ()).toInt ();
  options->feature_poly_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("verified feature color/red"), options->verified_feature_color.red ()).toInt ();
  green = settings.value (QString ("verified feature color/green"), options->verified_feature_color.green ()).toInt ();
  blue = settings.value (QString ("verified feature color/blue"), options->verified_feature_color.blue ()).toInt ();
  alpha = settings.value (QString ("verified feature color/alpha"), options->verified_feature_color.alpha ()).toInt ();
  options->verified_feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("contour highlight color/red"), options->contour_highlight_color.red ()).toInt ();
  green = settings.value (QString ("contour highlight color/green"), options->contour_highlight_color.green ()).toInt ();
  blue = settings.value (QString ("contour highlight color/blue"), options->contour_highlight_color.blue ()).toInt ();
  alpha = settings.value (QString ("contour highlight color/alpha"), 
                          options->contour_highlight_color.alpha ()).toInt ();
  options->contour_highlight_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("coast color/red"), options->coast_color.red ()).toInt ();
  green = settings.value (QString ("coast color/green"), options->coast_color.green ()).toInt ();
  blue = settings.value (QString ("coast color/blue"), options->coast_color.blue ()).toInt ();
  alpha = settings.value (QString ("coast color/alpha"), options->coast_color.alpha ()).toInt ();
  options->coast_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (QString ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (QString ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (QString ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("filter mask color/red"), options->poly_filter_mask_color.red ()).toInt ();
  green = settings.value (QString ("filter mask color/green"), options->poly_filter_mask_color.green ()).toInt ();
  blue = settings.value (QString ("filter mask color/blue"), options->poly_filter_mask_color.blue ()).toInt ();
  alpha = settings.value (QString ("filter mask color/alpha"), options->poly_filter_mask_color.alpha ()).toInt ();
  options->poly_filter_mask_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage feature color/red"), options->cov_feature_color.red ()).toInt ();
  green = settings.value (QString ("coverage feature color/green"), options->cov_feature_color.green ()).toInt ();
  blue = settings.value (QString ("coverage feature color/blue"), options->cov_feature_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage feature color/alpha"), options->cov_feature_color.alpha ()).toInt ();
  options->cov_feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("coverage coast color/red"), options->cov_coast_color.red ()).toInt ();
  green = settings.value (QString ("coverage coast color/green"), options->cov_coast_color.green ()).toInt ();
  blue = settings.value (QString ("coverage coast color/blue"), options->cov_coast_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage coast color/alpha"), options->cov_coast_color.alpha ()).toInt ();
  options->cov_coast_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage box color/red"), options->cov_box_color.red ()).toInt ();
  green = settings.value (QString ("coverage box color/green"), options->cov_box_color.green ()).toInt ();
  blue = settings.value (QString ("coverage box color/blue"), options->cov_box_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage box color/alpha"), options->cov_box_color.alpha ()).toInt ();
  options->cov_box_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage data color/red"), options->cov_data_color.red ()).toInt ();
  green = settings.value (QString ("coverage data color/green"), options->cov_data_color.green ()).toInt ();
  blue = settings.value (QString ("coverage data color/blue"), options->cov_data_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage data color/alpha"), options->cov_data_color.alpha ()).toInt ();
  options->cov_data_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage background color/red"), options->cov_background_color.red ()).toInt ();
  green = settings.value (QString ("coverage background color/green"), options->cov_background_color.green ()).toInt ();
  blue = settings.value (QString ("coverage background color/blue"), options->cov_background_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage background color/alpha"), options->cov_background_color.alpha ()).toInt ();
  options->cov_background_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage checked color/red"), options->cov_checked_color.red ()).toInt ();
  green = settings.value (QString ("coverage checked color/green"), options->cov_checked_color.green ()).toInt ();
  blue = settings.value (QString ("coverage checked color/blue"), options->cov_checked_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage checked color/alpha"), options->cov_checked_color.alpha ()).toInt ();
  options->cov_checked_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage verified color/red"), options->cov_verified_color.red ()).toInt ();
  green = settings.value (QString ("coverage verified color/green"), options->cov_verified_color.green ()).toInt ();
  blue = settings.value (QString ("coverage verified color/blue"), options->cov_verified_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage verified color/alpha"), options->cov_verified_color.alpha ()).toInt ();
  options->cov_verified_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage pfm box hot color/red"), options->cov_pfm_box_hot_color.red ()).toInt ();
  green = settings.value (QString ("coverage pfm box hot color/green"), options->cov_pfm_box_hot_color.green ()).toInt ();
  blue = settings.value (QString ("coverage pfm box hot color/blue"), options->cov_pfm_box_hot_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage pfm box hot color/alpha"), options->cov_pfm_box_hot_color.alpha ()).toInt ();
  options->cov_pfm_box_hot_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage pfm box cold color/red"), options->cov_pfm_box_cold_color.red ()).toInt ();
  green = settings.value (QString ("coverage pfm box cold color/green"), options->cov_pfm_box_cold_color.green ()).toInt ();
  blue = settings.value (QString ("coverage pfm box cold color/blue"), options->cov_pfm_box_cold_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage pfm box cold color/alpha"), options->cov_pfm_box_cold_color.alpha ()).toInt ();
  options->cov_pfm_box_cold_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (QString ("coverage invalid feature color/red"), options->cov_inv_feature_color.red ()).toInt ();
  green = settings.value (QString ("coverage invalid feature color/green"), options->cov_inv_feature_color.green ()).toInt ();
  blue = settings.value (QString ("coverage invalid feature color/blue"), options->cov_inv_feature_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage invalid feature color/alpha"), options->cov_inv_feature_color.alpha ()).toInt ();
  options->cov_inv_feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (QString ("coverage verified feature color/red"), options->cov_verified_feature_color.red ()).toInt ();
  green = settings.value (QString ("coverage verified feature color/green"), options->cov_verified_feature_color.green ()).toInt ();
  blue = settings.value (QString ("coverage verified feature color/blue"), options->cov_verified_feature_color.blue ()).toInt ();
  alpha = settings.value (QString ("coverage verified feature color/alpha"), options->cov_verified_feature_color.alpha ()).toInt ();
  options->cov_verified_feature_color.setRgb (red, green, blue, alpha);


  options->stoplight = settings.value (QString ("stoplight flag"), options->stoplight).toBool ();
  options->stoplight_min_mid = (double) settings.value (QString ("stoplight min to mid crossover value"),
                                                            (double) options->stoplight_min_mid).toDouble ();
  options->stoplight_max_mid = (double) settings.value (QString ("stoplight max to mid crossover value"),
                                                            (double) options->stoplight_max_mid).toDouble ();

  red = settings.value (QString ("stoplight min color/red"), options->stoplight_min_color.red ()).toInt ();
  green = settings.value (QString ("stoplight min color/green"), options->stoplight_min_color.green ()).toInt ();
  blue = settings.value (QString ("stoplight min color/blue"), options->stoplight_min_color.blue ()).toInt ();
  alpha = settings.value (QString ("stoplight min color/alpha"), options->stoplight_min_color.alpha ()).toInt ();
  options->stoplight_min_color.setRgb (red, green, blue, alpha);

  red = settings.value (QString ("stoplight mid color/red"), options->stoplight_mid_color.red ()).toInt ();
  green = settings.value (QString ("stoplight mid color/green"), options->stoplight_mid_color.green ()).toInt ();
  blue = settings.value (QString ("stoplight mid color/blue"), options->stoplight_mid_color.blue ()).toInt ();
  alpha = settings.value (QString ("stoplight mid color/alpha"), options->stoplight_mid_color.alpha ()).toInt ();
  options->stoplight_mid_color.setRgb (red, green, blue, alpha);

  red = settings.value (QString ("stoplight max color/red"), options->stoplight_max_color.red ()).toInt ();
  green = settings.value (QString ("stoplight max color/green"), options->stoplight_max_color.green ()).toInt ();
  blue = settings.value (QString ("stoplight max color/blue"), options->stoplight_max_color.blue ()).toInt ();
  alpha = settings.value (QString ("stoplight max color/alpha"), options->stoplight_max_color.alpha ()).toInt ();
  options->stoplight_max_color.setRgb (red, green, blue, alpha);

  red = settings.value (QString ("scaled offset z color/red"), options->scaled_offset_z_color.red ()).toInt ();
  green = settings.value (QString ("scaled offset z color/green"), options->scaled_offset_z_color.green ()).toInt ();
  blue = settings.value (QString ("scaled offset z color/blue"), options->scaled_offset_z_color.blue ()).toInt ();
  alpha = settings.value (QString ("scaled offset z color/alpha"), options->scaled_offset_z_color.alpha ()).toInt ();
  options->scaled_offset_z_color.setRgb (red, green, blue, alpha);


  for (int32_t i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      string = QString ("minimum hsv color value %1").arg (i);
      options->min_hsv_color[i] = settings.value (string, options->min_hsv_color[i]).toInt ();
      string = QString ("maximum hsv color value %1").arg (i);
      options->max_hsv_color[i] = settings.value (string, options->max_hsv_color[i]).toInt ();
      string = QString ("minimum hsv locked flag %1").arg (i);
      options->min_hsv_locked[i] = settings.value (string, options->min_hsv_locked[i]).toBool ();
      string = QString ("maximum hsv locked flag %1").arg (i);
      options->max_hsv_locked[i] = settings.value (string, options->max_hsv_locked[i]).toBool ();
      string = QString ("minimum hsv locked value %1").arg (i);
      options->min_hsv_value[i] = (float) settings.value (string, (double) options->min_hsv_value[i]).toDouble ();
      string = QString ("maximum hsv locked value %1").arg (i);
      options->max_hsv_value[i] = (float) settings.value (string, (double) options->max_hsv_value[i]).toDouble ();
    }


  QString en = settings.value (QString ("3D editor name"), QString (options->edit_name_3D)).toString ();
  strcpy (options->edit_name_3D, en.toLatin1 ());


  options->sunopts.sun.x = settings.value (QString ("sunopts sun x"), options->sunopts.sun.x).toDouble ();

  options->sunopts.sun.y = settings.value (QString ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.sun.z = settings.value (QString ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.azimuth = settings.value (QString ("sunopts azimuth"), options->sunopts.azimuth).toDouble ();

  options->sunopts.elevation = settings.value (QString ("sunopts elevation"), options->sunopts.elevation).toDouble ();

  options->sunopts.exag = settings.value (QString ("sunopts exag"), options->sunopts.exag).toDouble ();

  options->sunopts.power_cos = settings.value (QString ("sunopts power cosine"), options->sunopts.power_cos).toDouble ();

  options->sunopts.num_shades = settings.value (QString ("sunopts number of shades"), options->sunopts.num_shades).toInt ();


  options->display_suspect = settings.value (QString ("view suspect sounding positions"), options->display_suspect).toBool ();

  options->display_feature = settings.value (QString ("view feature positions"), options->display_feature).toInt ();
  options->display_children = settings.value (QString ("view feature children positions"), options->display_children).toBool ();
  options->display_selected = settings.value (QString ("view selected sounding positions"), options->display_selected).toBool ();

  options->display_reference = settings.value (QString ("view reference data positions"), options->display_reference).toBool ();

  options->display_feature_info = settings.value (QString ("view feature info"), options->display_feature_info).toBool ();

  options->display_feature_poly = settings.value (QString ("view feature polygon"), options->display_feature_poly).toBool ();

  options->feature_search_string = settings.value (QString ("feature search string"), options->feature_search_string).toString ();

  options->display_minmax = settings.value (QString ("view min and max"), options->display_minmax).toBool ();


  options->auto_redraw = settings.value (QString ("auto redraw"), options->auto_redraw).toBool ();

  options->edit_mode = settings.value (QString ("edit mode [0,1]"), options->edit_mode).toInt ();


  QString up = settings.value (QString ("unload program name"), QString (options->unload_prog)).toString ();
  strcpy (options->unload_prog, up.toLatin1 ());


  QString im = settings.value (QString ("import program name"), QString (options->import_prog)).toString ();
  strcpy (options->import_prog, im.toLatin1 ());


  options->contour_filter_envelope = (float) settings.value (QString ("contour filter envelope"), 
                                                                  (double) options->contour_filter_envelope).toDouble ();

  options->smoothing_factor = settings.value (QString ("contour smoothing factor"), options->smoothing_factor).toInt ();

  options->z_factor = (float) settings.value (QString ("depth scaling factor"), (double) options->z_factor).toDouble ();

  options->z_offset = (float) settings.value (QString ("depth offset value"), (double) options->z_offset).toDouble ();

  options->zero_turnover = settings.value (QString ("zero turnover flag"), options->zero_turnover).toBool ();

  options->auto_close_unload = settings.value (QString ("auto close unload"), options->auto_close_unload).toBool ();

  options->highlight = settings.value (QString ("highlight"), options->highlight).toInt ();
  options->highlight_percent = (float) settings.value (QString ("highlight depth percentage"), (double) options->highlight_percent).toDouble ();

  options->chart_scale = settings.value (QString ("chart scale"), options->chart_scale).toInt ();
  options->fixed_feature_size = settings.value (QString ("fixed feature size flag"), options->fixed_feature_size).toBool ();
  options->millimeters = settings.value (QString ("millimeters"), options->millimeters).toInt ();


  options->cint = (float) settings.value (QString ("contour interval"), (double) options->cint).toDouble ();
  options->layer_type = settings.value (QString ("binned layer type"), options->layer_type).toInt ();


  options->last_rock_feature_desc = settings.value (QString ("last rock feature descriptor index"), options->last_rock_feature_desc).toInt ();

  options->last_offshore_feature_desc = settings.value (QString ("last offshore feature descriptor index"),
                                                        options->last_offshore_feature_desc).toInt ();

  options->last_light_feature_desc = settings.value (QString ("last light feature descriptor index"), options->last_light_feature_desc).toInt ();

  options->last_lidar_feature_desc = settings.value (QString ("last lidar feature descriptor index"), options->last_lidar_feature_desc).toInt ();

  options->last_feature_description = settings.value (QString ("last feature description"), options->last_feature_description).toString ();

  options->last_feature_remarks = settings.value (QString ("last feature remarks"), options->last_feature_remarks).toString ();

  options->feature_search_invert = settings.value (QString ("invert feature search"), options->feature_search_invert).toBool ();

  options->feature_search_type = settings.value (QString ("invert feature type"), options->feature_search_type).toInt ();

  options->overlap_percent = settings.value (QString ("window overlap percentage"), options->overlap_percent).toInt ();

  options->pref_tab = settings.value (QString ("preference tab"), options->pref_tab).toInt ();

  options->lat_or_east = settings.value (QString ("latitude or easting"), options->lat_or_east).toBool ();

  options->num_levels = settings.value (QString ("contour levels"), options->num_levels).toInt ();

  for (int32_t i = 0 ; i < options->num_levels ; i++)
    {
      string = QString ("contour level %1").arg (i);
      options->contour_levels[i] = (float) settings.value (string, (double) options->contour_levels[i]).toDouble ();
    }


  options->iho = settings.value (QString ("CUBE IHO order"), options->iho).toInt ();
  options->capture = settings.value (QString ("CUBE capture percentage"), options->capture).toDouble ();
  options->queue = settings.value (QString ("CUBE queue length"), options->queue).toInt ();
  options->horiz = settings.value (QString ("CUBE orizontal position uncertainty"), options->horiz).toDouble ();
  options->distance = settings.value (QString ("CUBE distance exponent"), options->distance).toDouble ();
  options->min_context = settings.value (QString ("CUBE minimum context"), options->min_context).toDouble ();
  options->max_context = settings.value (QString ("CUBE maximum context"), options->max_context).toDouble ();
  options->disambiguation = settings.value (QString ("CUBE disambiguation method"), options->disambiguation).toInt ();


  //  Save the Status page attribute names.

  for (int32_t i = 0 ; i < POST_ATTR ; i++)
    {
      options->attrStatName[i] = settings.value (QString ("status tab attribute name %1").arg (i), options->attrStatName[i]).toString ();
    }


  //  Retrieve the possible attribute filter range strings.

  options->time_attr_filt = settings.value (options->time_attribute.name + QString (" filter ranges"), options->time_attr_filt).toString ();
  for (int32_t i = 0 ; i < GSF_ATTRIBUTES ; i++)
    options->gsf_attr_filt[i] = settings.value (options->gsf_attribute[i].name + QString (" filter ranges"), options->gsf_attr_filt[i]).toString ();
  for (int32_t i = 0 ; i < HOF_ATTRIBUTES ; i++)
    options->hof_attr_filt[i] = settings.value (options->hof_attribute[i].name + QString (" filter ranges"), options->hof_attr_filt[i]).toString ();
  for (int32_t i = 0 ; i < TOF_ATTRIBUTES ; i++)
    options->tof_attr_filt[i] = settings.value (options->tof_attribute[i].name + QString (" filter ranges"), options->tof_attr_filt[i]).toString ();
  for (int32_t i = 0; i < CZMIL_ATTRIBUTES; i++)
    options->czmil_attr_filt[i] = settings.value (options->czmil_attribute[i].name + QString (" filter ranges"),
                                                  options->czmil_attr_filt[i]).toString ();
  for (int32_t i = 0 ; i < LAS_ATTRIBUTES ; i++)
    options->las_attr_filt[i] = settings.value (options->las_attribute[i].name + QString (" filter ranges"), options->las_attr_filt[i]).toString ();
  for (int32_t i = 0; i < BAG_ATTRIBUTES; i++)
    options->bag_attr_filt[i] = settings.value (options->bag_attribute[i].name + QString (" filter ranges"), options->bag_attr_filt[i]).toString ();


  options->msg_window_x = settings.value (QString ("message window x position"), options->msg_window_x).toInt ();
  options->msg_window_y = settings.value (QString ("message window y position"), options->msg_window_y).toInt ();
  options->msg_width = settings.value (QString ("message window width"), options->msg_width).toInt ();
  options->msg_height = settings.value (QString ("message window height"), options->msg_height).toInt ();

  for (int32_t i = 0 ; i < 2 ; i++) options->editor_no_load[i] = settings.value (QString ("editor no load[%1]").arg (i), options->editor_no_load[i]).toInt ();

  options->feature_radius = (float) settings.value (QString ("feature filter exclusion radius"), (double) options->feature_radius).toDouble ();

  options->screenshot_delay = settings.value (QString ("screenshot delay"), options->screenshot_delay).toInt ();

  options->attrFilterName = settings.value (QString ("otf limiting attribute name"), options->attrFilterName).toString ();

  options->otf_bin_size_meters = (float) settings.value (QString ("otf bin size"), (double) options->otf_bin_size_meters).toDouble ();

  options->h_count = settings.value (QString ("highlight bin count"), options->h_count).toInt ();


  options->overlay_grid = settings.value (QString ("overlay grid flag"), options->overlay_grid).toBool ();
  options->overlay_grid_type = settings.value (QString ("overlay grid type"), options->overlay_grid_type).toInt ();
  options->overlay_grid_spacing = (float) settings.value (QString ("overlay grid spacing"), (double) options->overlay_grid_spacing).toDouble ();
  options->overlay_grid_minutes = (float) settings.value (QString ("overlay grid minutes"), (double) options->overlay_grid_minutes).toDouble ();
  options->overlay_grid_width = settings.value (QString ("overlay grid width"), options->overlay_grid_width).toInt ();
  red = settings.value (QString ("overlay grid color/red"), options->overlay_grid_color.red ()).toInt ();
  green = settings.value (QString ("overlay grid color/green"), options->overlay_grid_color.green ()).toInt ();
  blue = settings.value (QString ("overlay grid color/blue"), options->overlay_grid_color.blue ()).toInt ();
  alpha = settings.value (QString ("overlay grid color/alpha"), options->overlay_grid_color.alpha ()).toInt ();
  options->overlay_grid_color.setRgb (red, green, blue, alpha);

  options->default_chart_scale = settings.value (QString ("default chart scale"), options->default_chart_scale).toInt ();
  options->default_width = (float) settings.value (QString ("default width"), (double) options->default_width).toDouble ();
  options->default_height = (float) settings.value (QString ("default height"), (double) options->default_height).toDouble ();


  options->ge_name = settings.value (QString ("Google Earth name"), options->ge_name).toString ();


  mainWindow->restoreState (settings.value (QString ("main window state")).toByteArray (), (int32_t) (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (QString ("main window geometry")).toByteArray ());

  settings.endGroup ();


  //  We need to get the Z orientation value from the globalABE settings file.  This value is used in some of the ABE programs
  //  to determine whether Z is displayed as depth (the default) or elevation.  We also want to get the font from the global
  //  settings since it will be used in (hopefully) all of the ABE map GUI applications.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  options->z_orientation = settings2.value (QString ("ABE Z orientation factor"), options->z_orientation).toFloat ();

  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  //  IMPORTANT NOTE: Don't add anything to settings2 after the default feature type.  Put it before that.


  //  Default feature type is used in all ABE programs that create or edit features.

  uint8_t ft = 99;
  ft = (uint8_t) settings2.value (QString ("default feature type"), ft).toInt ();


  //  If the user has never set the default feature type we need to inform them of it and let them choose one.

  if (ft >= BFDATA_FEATURE_TYPES)
    {
      QMessageBox msgBox;

      msgBox.setText (pfmView::tr ("A new field has been added to the Binary Feature Data (BFD) format.  This field defines the feature type.  "
                                   "There are two possible feature types, <b>Hydrographic</b> and <b>Informational</b>.  The type of feature used "
                                   "will affect the way Area-Based Editor (ABE) programs mark point data.  For <b>Hydrographic</b> features, ABE will "
                                   "try to mark the nearest associated data point with the PFM_SELECTED_FEATURE flag.  When "
                                   "GSF and/or CZMIL CPF files are the input files for the PFM, these flags will be unloaded to the input files and "
                                   "translated to the corresponding GSF or CPF flags.  If you intend to mostly use features for information purposes "
                                   "(e.g. to mark areas of interest or problem areas) then you should select <b>Informational</b> as the default feature "
                                   "type.  That way the underlying data will not be marked.<br><br>The default feature type can be changed at any time "
                                   "in the feature editing dialog.  Changes made to the feature type in the feature editing dialog will be saved as the new "
                                   "default feature type for all ABE programs.  Also note that feature type was not available for pre-3.0 BFD files. "
                                   "If you are using a preexisting pre-3.0 BFD file you can still force the program to use <b>Informational</b> "
                                   "feature types but they won't be saved to the file.  However, it will prevent the ABE programs from setting the "
                                   "flags on data points.<br><br>"
                                   "Please select the default feature type."));

      QAbstractButton *pButtonYes = msgBox.addButton (pfmView::tr ("Hydrographic"), QMessageBox::YesRole);
      msgBox.addButton (pfmView::tr ("Informational"), QMessageBox::NoRole);
 
      msgBox.exec ();
 
      if (msgBox.clickedButton () == pButtonYes)
        {
          options->default_feature_type = BFDATA_HYDROGRAPHIC;
        }
      else
        {
          options->default_feature_type = BFDATA_INFORMATIONAL;
        }


      settings2.endGroup ();


      //  Now we have to save it so that, if we open pfmEdit3D, it won't show us this message again.

      QSettings settings_g (ini_file2, QSettings::IniFormat);
      settings_g.beginGroup ("globalABE");
      settings_g.setValue (QString ("default feature type"), options->default_feature_type);
      settings_g.endGroup ();
    }
  else
    {
      options->default_feature_type = ft;
      settings2.endGroup ();
    }


  //  We need to get the mosaicView program and hotkeys from the pfmEdit3D settings.
  //  Note that we never save these.  Only pfmEdit3D will be allowed to change these values.
  //  These are stored in the pfmEdit3D envout function in the loop for the ancillary programs.

#ifdef NVWIN3X
  QString ini_file3 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "pfmEdit3D.ini";
#else
  QString ini_file3 = QString (getenv ("HOME")) + "/ABE.config/" + "pfmEdit3D.ini";
#endif

  QSettings settings3 (ini_file3, QSettings::IniFormat);
  settings3.beginGroup (QString ("pfmEdit3D"));


  options->mosaic_prog = settings3.value (QString ("mosaic viewer"), options->mosaic_prog).toString ();
  options->mosaic_hotkey = settings3.value (QString ("mosaic viewer hot key"), options->mosaic_hotkey).toString ();
  options->mosaic_actkey = settings3.value (QString ("mosaic viewer action keys"), options->mosaic_actkey).toString ();

  settings3.endGroup ();


  return (NVTrue);
}


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  QString string;


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/" + QString (misc->qsettings_app) + ".ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/" + QString (misc->qsettings_app) + ".ini";
#endif


  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup (misc->qsettings_app);


  settings.setValue (QString ("recent file count"), options->recent_file_count);

  for (int32_t i = 0 ; i < options->recent_file_count ; i++)
    {
      string = QString ("recent file %1").arg (i);
      settings.setValue (string, options->recentFile[i]);
    }

  settings.setValue (QString ("overlay directory"), options->overlay_dir);
  settings.setValue (QString ("output area name filter"), options->output_area_name_filter);
  settings.setValue (QString ("output area directory"), options->output_area_dir);
  settings.setValue (QString ("output points directory"), options->output_points_dir);
  settings.setValue (QString ("output shape directory"), options->output_shape_dir);
  settings.setValue (QString ("input pfm directory"), options->input_pfm_dir);
  settings.setValue (QString ("geotiff directory"), options->geotiff_dir);
  settings.setValue (QString ("area directory"), options->area_dir);
  settings.setValue (QString ("dnc directory"), options->dnc_dir);


  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      string = misc->buttonText[i] + QString (" hot key");
      settings.setValue (string, options->buttonAccel[i]);
    }


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("contour flag"), options->contour);

  settings.setValue (QString ("group features flag"), options->group_features);

  settings.setValue (QString ("contour width"), options->contour_width);

  settings.setValue (QString ("contour index"), options->contour_index);


  settings.setValue (QString ("coast flag"), options->coast);
  settings.setValue (QString ("coverage coast flag"), options->cov_coast);
  settings.setValue (QString ("geotiff alpha"), options->GeoTIFF_alpha);

  settings.setValue (QString ("main button icon size"), options->main_button_icon_size);

  settings.setValue (QString ("misp weight"), options->misp_weight);
  settings.setValue (QString ("misp force original flag"), options->misp_force_original);
  settings.setValue (QString ("misp replace all flag"), options->misp_replace_all);

  settings.setValue (QString ("deep filter only flag"), options->deep_filter_only);

  settings.setValue (QString ("filter standard deviation"), options->filterSTD);

  settings.setValue (QString ("position form"), options->position_form);


  settings.setValue (QString ("contour color/red"), options->contour_color.red ());
  settings.setValue (QString ("contour color/green"), options->contour_color.green ());
  settings.setValue (QString ("contour color/blue"), options->contour_color.blue ());
  settings.setValue (QString ("contour color/alpha"), options->contour_color.alpha ());


  settings.setValue (QString ("feature color/red"), options->feature_color.red ());
  settings.setValue (QString ("feature color/green"), options->feature_color.green ());
  settings.setValue (QString ("feature color/blue"), options->feature_color.blue ());
  settings.setValue (QString ("feature color/alpha"), options->feature_color.alpha ());


  settings.setValue (QString ("feature info color/red"), options->feature_info_color.red ());
  settings.setValue (QString ("feature info color/green"), options->feature_info_color.green ());
  settings.setValue (QString ("feature info color/blue"), options->feature_info_color.blue ());
  settings.setValue (QString ("feature info color/alpha"), options->feature_info_color.alpha ());


  settings.setValue (QString ("feature poly color/red"), options->feature_poly_color.red ());
  settings.setValue (QString ("feature poly color/green"), options->feature_poly_color.green ());
  settings.setValue (QString ("feature poly color/blue"), options->feature_poly_color.blue ());
  settings.setValue (QString ("feature poly color/alpha"), options->feature_poly_color.alpha ());


  settings.setValue (QString ("verified feature color/red"), options->verified_feature_color.red ());
  settings.setValue (QString ("verified feature color/green"), options->verified_feature_color.green ());
  settings.setValue (QString ("verified feature color/blue"), options->verified_feature_color.blue ());
  settings.setValue (QString ("verified feature color/alpha"), options->verified_feature_color.alpha ());


  settings.setValue (QString ("contour highlight color/red"), options->contour_highlight_color.red ());
  settings.setValue (QString ("contour highlight color/green"), options->contour_highlight_color.green ());
  settings.setValue (QString ("contour highlight color/blue"), options->contour_highlight_color.blue ());
  settings.setValue (QString ("contour highlight color/alpha"), options->contour_highlight_color.alpha ());


  settings.setValue (QString ("coast color/red"), options->coast_color.red ());
  settings.setValue (QString ("coast color/green"), options->coast_color.green ());
  settings.setValue (QString ("coast color/blue"), options->coast_color.blue ());
  settings.setValue (QString ("coast color/alpha"), options->coast_color.alpha ());


  settings.setValue (QString ("background color/red"), options->background_color.red ());
  settings.setValue (QString ("background color/green"), options->background_color.green ());
  settings.setValue (QString ("background color/blue"), options->background_color.blue ());
  settings.setValue (QString ("background color/alpha"), options->background_color.alpha ());


  settings.setValue (QString ("filter mask color/red"), options->poly_filter_mask_color.red ());
  settings.setValue (QString ("filter mask color/green"), options->poly_filter_mask_color.green ());
  settings.setValue (QString ("filter mask color/blue"), options->poly_filter_mask_color.blue ());
  settings.setValue (QString ("filter mask color/alpha"), options->poly_filter_mask_color.alpha ());


  settings.setValue (QString ("coverage feature color/red"), options->cov_feature_color.red ());
  settings.setValue (QString ("coverage feature color/green"), options->cov_feature_color.green ());
  settings.setValue (QString ("coverage feature color/blue"), options->cov_feature_color.blue ());
  settings.setValue (QString ("coverage feature color/alpha"), options->cov_feature_color.alpha ());


  settings.setValue (QString ("coverage coast color/red"), options->cov_coast_color.red ());
  settings.setValue (QString ("coverage coast color/green"), options->cov_coast_color.green ());
  settings.setValue (QString ("coverage coast color/blue"), options->cov_coast_color.blue ());
  settings.setValue (QString ("coverage coast color/alpha"), options->cov_coast_color.alpha ());

  
  settings.setValue (QString ("coverage box color/red"), options->cov_box_color.red ());
  settings.setValue (QString ("coverage box color/green"), options->cov_box_color.green ());
  settings.setValue (QString ("coverage box color/blue"), options->cov_box_color.blue ());
  settings.setValue (QString ("coverage box color/alpha"), options->cov_box_color.alpha ());

  
  settings.setValue (QString ("coverage data color/red"), options->cov_data_color.red ());
  settings.setValue (QString ("coverage data color/green"), options->cov_data_color.green ());
  settings.setValue (QString ("coverage data color/blue"), options->cov_data_color.blue ());
  settings.setValue (QString ("coverage data color/alpha"), options->cov_data_color.alpha ());

  
  settings.setValue (QString ("coverage background color/red"), options->cov_background_color.red ());
  settings.setValue (QString ("coverage background color/green"), options->cov_background_color.green ());
  settings.setValue (QString ("coverage background color/blue"), options->cov_background_color.blue ());
  settings.setValue (QString ("coverage background color/alpha"), options->cov_background_color.alpha ());

  
  settings.setValue (QString ("coverage checked color/red"), options->cov_checked_color.red ());
  settings.setValue (QString ("coverage checked color/green"), options->cov_checked_color.green ());
  settings.setValue (QString ("coverage checked color/blue"), options->cov_checked_color.blue ());
  settings.setValue (QString ("coverage checked color/alpha"), options->cov_checked_color.alpha ());

  
  settings.setValue (QString ("coverage verified color/red"), options->cov_verified_color.red ());
  settings.setValue (QString ("coverage verified color/green"), options->cov_verified_color.green ());
  settings.setValue (QString ("coverage verified color/blue"), options->cov_verified_color.blue ());
  settings.setValue (QString ("coverage verified color/alpha"), options->cov_verified_color.alpha ());

  
  settings.setValue (QString ("coverage pfm box hot color/red"), options->cov_pfm_box_hot_color.red ());
  settings.setValue (QString ("coverage pfm box hot color/green"), options->cov_pfm_box_hot_color.green ());
  settings.setValue (QString ("coverage pfm box hot color/blue"), options->cov_pfm_box_hot_color.blue ());
  settings.setValue (QString ("coverage pfm box hot color/alpha"), options->cov_pfm_box_hot_color.alpha ());

  
  settings.setValue (QString ("coverage pfm box cold color/red"), options->cov_pfm_box_cold_color.red ());
  settings.setValue (QString ("coverage pfm box cold color/green"), options->cov_pfm_box_cold_color.green ());
  settings.setValue (QString ("coverage pfm box cold color/blue"), options->cov_pfm_box_cold_color.blue ());
  settings.setValue (QString ("coverage pfm box cold color/alpha"), options->cov_pfm_box_cold_color.alpha ());

  
  settings.setValue (QString ("coverage invalid feature color/red"), options->cov_inv_feature_color.red ());
  settings.setValue (QString ("coverage invalid feature color/green"), options->cov_inv_feature_color.green ());
  settings.setValue (QString ("coverage invalid feature color/blue"), options->cov_inv_feature_color.blue ());
  settings.setValue (QString ("coverage invalid feature color/alpha"), options->cov_inv_feature_color.alpha ());


  settings.setValue (QString ("coverage verified feature color/red"), options->cov_verified_feature_color.red ());
  settings.setValue (QString ("coverage verified feature color/green"), options->cov_verified_feature_color.green ());
  settings.setValue (QString ("coverage verified feature color/blue"), options->cov_verified_feature_color.blue ());
  settings.setValue (QString ("coverage verified feature color/alpha"), options->cov_verified_feature_color.alpha ());


  settings.setValue (QString ("stoplight flag"), options->stoplight);
  settings.setValue (QString ("stoplight min to mid crossover value"), (double) options->stoplight_min_mid);
  settings.setValue (QString ("stoplight max to mid crossover value"), (double) options->stoplight_max_mid);

  settings.setValue (QString ("stoplight min color/red"), options->stoplight_min_color.red ());
  settings.setValue (QString ("stoplight min color/green"), options->stoplight_min_color.green ());
  settings.setValue (QString ("stoplight min color/blue"), options->stoplight_min_color.blue ());
  settings.setValue (QString ("stoplight min color/alpha"), options->stoplight_min_color.alpha ());

  settings.setValue (QString ("stoplight mid color/red"), options->stoplight_mid_color.red ());
  settings.setValue (QString ("stoplight mid color/green"), options->stoplight_mid_color.green ());
  settings.setValue (QString ("stoplight mid color/blue"), options->stoplight_mid_color.blue ());
  settings.setValue (QString ("stoplight mid color/alpha"), options->stoplight_mid_color.alpha ());

  settings.setValue (QString ("stoplight max color/red"), options->stoplight_max_color.red ());
  settings.setValue (QString ("stoplight max color/green"), options->stoplight_max_color.green ());
  settings.setValue (QString ("stoplight max color/blue"), options->stoplight_max_color.blue ());
  settings.setValue (QString ("stoplight max color/alpha"), options->stoplight_max_color.alpha ());

  settings.setValue (QString ("scaled offset z color/red"), options->scaled_offset_z_color.red ());
  settings.setValue (QString ("scaled offset z color/green"), options->scaled_offset_z_color.green ());
  settings.setValue (QString ("scaled offset z color/blue"), options->scaled_offset_z_color.blue ());
  settings.setValue (QString ("scaled offset z color/alpha"), options->scaled_offset_z_color.alpha ());

  for (int32_t i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      string = QString ("minimum hsv color value %1").arg (i);
      settings.setValue (string, options->min_hsv_color[i]);
      string = QString ("maximum hsv color value %1").arg (i);
      settings.setValue (string, options->max_hsv_color[i]);
      string = QString ("minimum hsv locked flag %1").arg (i);
      settings.setValue (string, options->min_hsv_locked[i]);
      string = QString ("maximum hsv locked flag %1").arg (i);
      settings.setValue (string, options->max_hsv_locked[i]);
      string = QString ("minimum hsv locked value %1").arg (i);
      settings.setValue (string, (double) options->min_hsv_value[i]);
      string = QString ("maximum hsv locked value %1").arg (i);
      settings.setValue (string, (double) options->max_hsv_value[i]);
    }


  settings.setValue (QString ("3D editor name"), QString (options->edit_name_3D));


  settings.setValue (QString ("sunopts sun x"), options->sunopts.sun.x);
  settings.setValue (QString ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (QString ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (QString ("sunopts azimuth"), options->sunopts.azimuth);
  settings.setValue (QString ("sunopts elevation"), options->sunopts.elevation);
  settings.setValue (QString ("sunopts exag"), options->sunopts.exag);
  settings.setValue (QString ("sunopts power cosine"), options->sunopts.power_cos);
  settings.setValue (QString ("sunopts number of shades"), options->sunopts.num_shades);


  settings.setValue (QString ("view suspect sounding positions"), options->display_suspect);
  settings.setValue (QString ("view feature positions"), options->display_feature);
  settings.setValue (QString ("view sub-feature positions"), options->display_children);
  settings.setValue (QString ("view selected sounding positions"), options->display_selected);
  settings.setValue (QString ("view reference data positions"), options->display_reference);
  settings.setValue (QString ("view feature info"), options->display_feature_info);
  settings.setValue (QString ("view feature polygon"), options->display_feature_poly);
  settings.setValue (QString ("feature search string"), options->feature_search_string);
  settings.setValue (QString ("view min and max"), options->display_minmax);

  settings.setValue (QString ("auto redraw"), options->auto_redraw);

  settings.setValue (QString ("edit mode [0,1]"), options->edit_mode);


  settings.setValue (QString ("unload program name"), QString (options->unload_prog));

  settings.setValue (QString ("import program name"), QString (options->import_prog));


  settings.setValue (QString ("contour smoothing factor"), options->smoothing_factor);

  settings.setValue (QString ("contour filter envelope"), (double) options->contour_filter_envelope);

  settings.setValue (QString ("depth scaling factor"), (double) options->z_factor);

  settings.setValue (QString ("depth offset value"), (double) options->z_offset);

  settings.setValue (QString ("zero turnover flag"), options->zero_turnover);

  settings.setValue (QString ("auto close unload"), options->auto_close_unload);

  settings.setValue (QString ("highlight"), options->highlight);
  settings.setValue (QString ("highlight depth percentage"), (double) options->highlight_percent);

  settings.setValue (QString ("chart scale"), (double) options->chart_scale);
  settings.setValue (QString ("fixed feature size flag"), options->fixed_feature_size);
  settings.setValue (QString ("millimeters"), options->millimeters);

  settings.setValue (QString ("contour interval"), options->cint);

  settings.setValue (QString ("binned layer type"), options->layer_type);


  settings.setValue (QString ("last rock feature descriptor index"), options->last_rock_feature_desc);

  settings.setValue (QString ("last offshore feature descriptor index"), options->last_offshore_feature_desc);

  settings.setValue (QString ("last light feature descriptor index"), options->last_light_feature_desc);

  settings.setValue (QString ("last lidar feature descriptor index"), options->last_lidar_feature_desc);

  settings.setValue (QString ("last feature description"), options->last_feature_description);

  settings.setValue (QString ("last feature remarks"), options->last_feature_remarks);

  settings.setValue (QString ("window overlap percentage"), options->overlap_percent);

  settings.setValue (QString ("invert feature search"), options->feature_search_invert);

  settings.setValue (QString ("invert feature type"), options->feature_search_type);

  settings.setValue (QString ("preference tab"), options->pref_tab);

  settings.setValue (QString ("latitude or easting"), options->lat_or_east);

  settings.setValue (QString ("contour levels"), options->num_levels);

  for (int32_t i = 0 ; i < options->num_levels ; i++)
    {
      string = QString ("contour level %1").arg (i);
      settings.setValue (string, options->contour_levels[i]);
    }


  settings.setValue (QString ("CUBE IHO order"), options->iho);
  settings.setValue (QString ("CUBE capture percentage"), options->capture);
  settings.setValue (QString ("CUBE queue length"), options->queue);
  settings.setValue (QString ("CUBE orizontal position uncertainty"), options->horiz);
  settings.setValue (QString ("CUBE distance exponent"), options->distance);
  settings.setValue (QString ("CUBE minimum context"), options->min_context);
  settings.setValue (QString ("CUBE maximum context"), options->max_context);
  settings.setValue (QString ("CUBE disambiguation method"), options->disambiguation);


  //  Restore the Status tab attribute names.

  for (int32_t i = 0 ; i < POST_ATTR ; i++)
    {
      settings.setValue (QString ("status tab attribute name %1").arg (i), options->attrStatName[i]);
    }



  //  This is pretty confusing.  What we're doing is finding out which attribute in the current PFM matches which possible attribute
  //  so that we can save the attribute filter string associated with that attribute (not the attribute number of that attribute in this
  //  particular PFM).  That way, when we open another PFM, if the attributes are in a different order we can assign the correct 
  //  filter ranges to the correct attribute.

  for (int32_t i = 0 ; i < NUM_ATTR ; i++)
    {
      uint8_t done = NVFalse;

      if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->time_attribute.name)
        {
          options->time_attr_filt = options->attrFilterText[i];
          done = NVTrue;
        }

      if (!done)
        {
          for (int32_t j = 0 ; j < GSF_ATTRIBUTES ; j++)
            {
              if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->gsf_attribute[j].name)
                {
                  options->gsf_attr_filt[j] = options->attrFilterText[i];
                  done = NVTrue;
                  break;
                }
            }

          if (!done)
            {
              for (int32_t j = 0 ; j < HOF_ATTRIBUTES ; j++)
                {
                  if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->hof_attribute[j].name)
                    {
                      options->hof_attr_filt[j] = options->attrFilterText[i];
                      done = NVTrue;
                      break;
                    }
                }

              if (!done)
                {
                  for (int32_t j = 0 ; j < TOF_ATTRIBUTES ; j++)
                    {
                      if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->tof_attribute[j].name)
                        {
                          options->tof_attr_filt[j] = options->attrFilterText[i];
                          done = NVTrue;
                          break;
                        }
                    }

                  if (!done)
                    {
                      for (int32_t j = 0 ; j < CZMIL_ATTRIBUTES ; j++)
                        {
                          if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->czmil_attribute[j].name)
                            {
                              options->czmil_attr_filt[j] = options->attrFilterText[i];
                              done = NVTrue;
                              break;
                            }
                        }

                      if (!done)
                        {
                          for (int32_t j = 0 ; j < LAS_ATTRIBUTES ; j++)
                            {
                              if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->las_attribute[j].name)
                                {
                                  options->las_attr_filt[j] = options->attrFilterText[i];
                                  done = NVTrue;
                                  break;
                                }
                            }

                          if (!done)
                            {
                              for (int32_t j = 0 ; j < BAG_ATTRIBUTES ; j++)
                                {
                                  if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[i]) == options->bag_attribute[j].name)
                                    {
                                      options->bag_attr_filt[j] = options->attrFilterText[i];
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


  //  Now we actually save the possible attribute filter range strings.

  settings.setValue (options->time_attribute.name + QString (" filter ranges"), options->time_attr_filt);
  for (int32_t i = 0 ; i < GSF_ATTRIBUTES ; i++) settings.setValue (options->gsf_attribute[i].name + QString (" filter ranges"),
                                                                    options->gsf_attr_filt[i]);
  for (int32_t i = 0 ; i < HOF_ATTRIBUTES ; i++) settings.setValue (options->hof_attribute[i].name + QString (" filter ranges"),
                                                                    options->hof_attr_filt[i]);
  for (int32_t i = 0 ; i < TOF_ATTRIBUTES ; i++) settings.setValue (options->tof_attribute[i].name + QString (" filter ranges"),
                                                                    options->tof_attr_filt[i]);
  for (int32_t i = 0; i < CZMIL_ATTRIBUTES; i++) settings.setValue (options->czmil_attribute[i].name + QString (" filter ranges"),
                                                                    options->czmil_attr_filt[i]);
  for (int32_t i = 0 ; i < LAS_ATTRIBUTES ; i++) settings.setValue (options->las_attribute[i].name + QString (" filter ranges"),
                                                                    options->las_attr_filt[i]);
  for (int32_t i = 0; i < BAG_ATTRIBUTES; i++) settings.setValue (options->bag_attribute[i].name + QString (" filter ranges"),
                                                                  options->bag_attr_filt[i]);


  settings.setValue (QString ("message window x position"), options->msg_window_x);
  settings.setValue (QString ("message window y position"), options->msg_window_y);
  settings.setValue (QString ("message window width"), options->msg_width);
  settings.setValue (QString ("message window height"), options->msg_height);

  for (int32_t i = 0 ; i < 2 ; i++) settings.setValue (QString ("editor no load[%1]").arg (i), options->editor_no_load[i]);

  settings.setValue (QString ("feature filter exclusion radius"), (double) options->feature_radius);

  settings.setValue (QString ("screenshot delay"), options->screenshot_delay);

  settings.setValue (QString ("otf limiting attribute name"), options->attrFilterName);

  settings.setValue (QString ("otf bin size"), (double) options->otf_bin_size_meters);

  settings.setValue (QString ("highlight bin count"), options->h_count);


  settings.setValue (QString ("overlay grid flag"), options->overlay_grid);
  settings.setValue (QString ("overlay grid type"), options->overlay_grid_type);
  settings.setValue (QString ("overlay grid spacing"), (double) options->overlay_grid_spacing);
  settings.setValue (QString ("overlay grid minutes"), (double) options->overlay_grid_minutes);
  settings.setValue (QString ("overlay grid width"), options->overlay_grid_width);
  settings.setValue (QString ("overlay grid color/red"), options->overlay_grid_color.red ());
  settings.setValue (QString ("overlay grid color/green"), options->overlay_grid_color.green ());
  settings.setValue (QString ("overlay grid color/blue"), options->overlay_grid_color.blue ());
  settings.setValue (QString ("overlay grid color/alpha"), options->overlay_grid_color.alpha ());

  settings.setValue (QString ("default chart scale"), options->default_chart_scale);
  settings.setValue (QString ("default width"), (double) options->default_width);
  settings.setValue (QString ("default height"), (double) options->default_height);


  settings.setValue (QString ("Google Earth name"), options->ge_name);


  settings.setValue (QString ("main window state"), mainWindow->saveState ((int32_t) (settings_version * 100.0)));

  settings.setValue (QString ("main window geometry"), mainWindow->saveGeometry ());


  settings.endGroup ();


  //  We need to set the Z orientation value in the globalABE settings file.  This value is used in some of the ABE programs
  //  to determine whether Z is displayed as depth (the default) or elevation.  We also want to get the font from the global
  //  settings since it will be used in (hopefully) all of the ABE map GUI applications.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  settings2.setValue (QString ("ABE Z orientation factor"), options->z_orientation);

  QString fontString = options->font.toString ();
  settings2.setValue (QString ("ABE map GUI font"), fontString);


  //  Default feature type is used in all ABE programs that create or edit features.

  settings2.setValue (QString ("default feature type"), options->default_feature_type);


  settings2.endGroup ();


  //  We need to set a value in the pfmEdit3D qsettings to let it know that pfmView has
  //  left the building.  This will allow them to properly set the running state of any "monitor" type
  //  ancillary programs.  The only time this matters is if you have ancillary programs running from
  //  one of the editors and you have left the editor and then shut down pfmView.  In that case 
  //  the editor will come back up next time thinking that the ancillary programs are still running
  //  and set their button states incorrectly.

#ifdef NVWIN3X
  QString ini_file3 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "pfmEdit3D.ini";
#else
  QString ini_file3 = QString (getenv ("HOME")) + "/ABE.config/" + "pfmEdit3D.ini";
#endif

  QSettings settings3 (ini_file3, QSettings::IniFormat);
  settings3.beginGroup (QString ("pfmEdit3D"));


  settings3.setValue (QString ("pfmView killed"), 1);

  settings3.endGroup ();
}
