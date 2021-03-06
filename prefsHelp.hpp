
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



QString bGrpText = 
  prefs::tr ("Select the format in which you want all geographic positions to be displayed.");

QString orientBoxText = 
  prefs::tr ("Select the orientation for displaying Z values.  Either <b>Depth</b> or <b>Elevation</b>.");

QString depthCheckText = 
  prefs::tr ("Check this box if you want Z values to be displayed as depths (positive Z is down).<br><br>"
             "<b>IMPORTANT NOTE: This orientation value is shared by some of the ABE programs (e.g. pfmEdit3D). "
             "If you set it here it will be used by the other programs.</b>");

QString elevationCheckText = 
  prefs::tr ("Check this box if you want Z values to be displayed as elevations (positive Z is up).<br><br>"
             "<b>IMPORTANT NOTE: This orientation value is shared by some of the ABE programs (e.g. pfmEdit3D). "
             "If you set it here it will be used by the other programs.</b>");

QString contourIntText = 
  prefs::tr ("Set the contour interval to be used by pfmView and also pfmEdit3D.  Setting this "
             "to 0.0 will cause the user defined contour levels to be used.  No contours will "
             "be plotted unless the plot contour button <img source=\":/icons/contour.png\"> has been "
             "toggled on.");

QString contourSmText = 
  prefs::tr ("Set the contour smoothing factor by pfmView and also pfmEdit3D.  This value goes "
             "from 0 (no smoothing) to 10 (max smoothing).");

QString cfiltText = 
  prefs::tr ("Set the envelope value for the contour filter.  When running the contour filter (MISPed data only) "
             "this value is compared to the absolute value of the difference between the MISPed surface built from "
             "the contours and the actual data points.  Any points that have a difference that exceeds this value are "
             "invalidated.");

QString ZFactorText = 
  prefs::tr ("Set the scaling factor for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and pfmEdit3D.  The actual values aren't changed just the displayed values.  "
             "This is useful for comparing depths to GeoTIFFs of scanned charts from NOAA (feet) "
             "or the US Navy (fathoms).");

QString offsetText = 
  prefs::tr ("Set the offset value for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and pfmEdit3D.  The actual values aren't changed just the displayed values.  "
             "This is useful for applying estimated ellipsoid offsets prior to the final datum "
             "shift being applied to the data.<br><br>"
             "<b>IMPORTANT NOTE: The offset value is <i>added</i> to the Z value and is applied <i>after</i> "
             "scaling so the units are in scaled units.</b>");

QString egmText = 
  prefs::tr ("Press this button to retrieve the EGM08 model value of the ellipsoid to geoid (approximate mean sea "
             "level) datum offset for the point nearest the center of the minimum bounding rectangle of the displayed "
             "area.");

QString WFactorText = 
  prefs::tr ("Set the weighting factor for the MISP regridding function.  The general rule of thumb for this value "
             "is to set to 1 to grid randomly spaced point data, 2 to grid randomly spaced swath data, and 3 to grid "
             "near complete coverage swath data.  Set to the negative value to force the grid to be as close as "
             "possible to the original input values.  This option is only useful if you have generated a MISP surface "
             "for the PFM using <b>pfmMisp</b>.");

QString forceText = 
  prefs::tr ("Select this if you wish to force the final output grid to use the original data values at the "
             "location of those values.  This overrides the weight factor in the locations of the original "
             "values.  The weight factor will still have some influence on the grid surface in areas near the "
             "original values.  This option is only useful if you have generated a MISP surface "
             "for the PFM using <b>pfmMisp</b>.");

QString replaceText = 
  prefs::tr ("Select this if you wish to have MISP replace values in the average edited/filtered surface for "
             "bins that contain real data points.  In normal use you want this checked since the average edited/filtered "
             "surface isn't used for much else.  This option is only useful if you have generated a MISP surface "
             "for the PFM using <b>pfmMisp</b>.");

QString zeroText = 
  prefs::tr ("Select this if you wish to restart the color range when you cross zero.  This can be somewhat confusing if "
             "you have large flat areas near zero.");

QString WidthText = 
  prefs::tr ("Set the contour line width/thickness in pixels.  Index contours (if set) will be twice this size.");

QString IndexText = 
  prefs::tr ("Set the index contour interval.  The Nth contour will be drawn at double the normal thickness.  "
             "For example, if this is set to 5, every fifth contour will be double thick.  Set to 0 to disable "
             "index contour emphasis.");

QString filterText = 
  prefs::tr ("Set the statistical filter options in this tab");

QString filterSTDText = 
  prefs::tr ("Set the standard deviation value for the polygon filter.  This value can be anywhere from 0.3 to 4.0.  "
             "A value of 2.0 or below is an extreme filter and should be used only for deep water, flat area smoothing.  "
             "A value of 2.4 standard deviations should filter about 5 percent of the data while a value of 4.0 should "
             "only filter out about 0.5 percent.");

QString dFilterText = 
  prefs::tr ("Set this check box to indicate that data will only be filtered in the downward direction when running the "
             "filter.  Downward in this case implies a larger positive value.  The assumption is that the data we are "
             "working with consists of depths not elevations.");

QString featureHighlightText = 
  prefs::tr ("Set this check box to indicate that features found with the feature search string will be displayed in the highlighted "
             "color along with all of the other features in their normal (verified or non-verified) colors.");

QString displayText = 
  prefs::tr ("Set this check box to indicate that features found with the feature search string will be the only features displayed.");

QString invertText = 
  prefs::tr ("Set this check box to indicate that feature searching will be inverted.  That is, only features that <b><i>do not</i></b> "
             "contain the feature search string will be either highlighted or displayed depending on the settings of the <b>Highlight</b> "
             "and <b>Display</b> buttons.");

QString featureRadiusText = 
  prefs::tr ("Set the distance around a feature within which filtering will not be allowed.  Setting this to 0 turns "
             "off <b>ALL</b> feature protection during filtering.  This includes the feature polygon exclusion.");

QString hPercentText = 
  prefs::tr ("Set the percentage value to be used for highlighting points that fall outside of a percentage of "
             "depth from the average edited bin value.  This should be set to the accepted accuracy of the "
             "sonar system with which your data was collected.  For example, the Simrad EM121A has an accepted "
             "accuracy of 0.10 percent of water depth.  Note that highlighting by percentage of depth only works "
             " in depths greater than 20 meters.  Your data should have already been datum or tide corrected.");

QString hCountText = 
  prefs::tr ("Set the valid point count to be used for highlighting bins that contain more valid points than this value.  "
             "For example, if this value is set to 4 and the highlight valid bin count option is checked, bins "
             "containing more than 4 valid points will be highlighted.");

QString milliText = 
  prefs::tr ("Set the millimeters size that will be used to determine feature circle radius for use in defining "
             "features and feature polygons.  To determine the size of a feature in meters just divide this number by 1000 "
             "(to get meters) then multiply by the denominator of the chart scale.");

QString chartScaleText = 
  prefs::tr ("Set the chart scale that will be used to determine feature circle radius for use in defining "
             "features and feature polygons.  The value will be the denominator of the scale fraction with the numerator "
             "always set to 1.  That is, if you enter 5000 then the scale will be 1:5000.  Feature polygons "
             "may be defined by using the <b>Define Feature Polygon</b> entry in the <b>Utilities</b> menu.");

QString overlayGridText = 
  prefs::tr ("Select the way you want to define spacing for the overlaying grid.  Either Meters or Minutes.");

QString dChartScaleText = 
  prefs::tr ("Set the displayed area chart scale that will set when you press the Default Chart Scale button "
             "in the Chart toolbar.  This allows you to save a frequently used chart scale so that you don't have to "
             "manually enter it in the Chart toolbar chart scale field.");

QString dszWidthText = 
  prefs::tr ("Set the displayed area width (in meters) that will set when you press the default displayed area width and height button "
             "in the Chart toolbar.  This allows you to save a frequently used width so that you don't have to "
             "manually enter it in the Chart toolbar width/height field.");

QString dszHeightText = 
  prefs::tr ("Set the displayed area height (in meters) that will set when you press the default displayed area width and height button "
             "in the Chart toolbar.  This allows you to save a frequently used height so that you don't have to "
             "manually enter it in the Chart toolbar width/height field.");

QString overlapText = 
  prefs::tr ("Set the percentage that the window will be overlapped when the arrow keys are used to move the winow.  "
             "This is five percent by default.  The min and max values are 0 and 25 respectively.");

QString conText = 
  prefs::tr ("Set contour and depth display options in this tab");

QString contourLevelsText = 
  prefs::tr ("Set individual contour levels.  Pressing this button will set the standard contour "
             "interval to 0.0 and bring up a table dialog to allow the user to insert contour "
             "levels (one per line, ascending).  The default levels are IHO standard contour "
             "levels.");

QString contoursTableText = 
  prefs::tr ("Use this table to set individual contour levels.  Levels should be set in ascending "
             "order.");

QString prefsGeotiffText = 
  prefs::tr ("Click this button to pop up a dialog with tools to allow you to change the amount of "
             "transparency used for a GeoTIFF overlay.");

QString geNameText = 
  prefs::tr ("This is the name of the Google Earth executable.  This field is not editable.  If you want to set the path to the Google Earth executable, "
             "click the <b>Browse</b> Button.");

QString geBrowseText = 
  prefs::tr ("Click this button to select the Google Earth executable file.");

QString cubeBoxText = 
  prefs::tr ("Set the Minimum Curvature Spline Interpolation (MISP) and Combined Uncertainty Bathymetric Estimator (CUBE) options in this tab.  "
             "Unless you really know what you're doing you should leave all of the CUBE settings as they are.  The only one that "
             "might be user modifiable without disastrous results is the IHO order selection.");

QString mispBoxText = 
  prefs::tr ("Set the Minimum Curvature Spline Interpolation (MISP) options in this tab.");

QString transparencyText = 
  prefs::tr ("This dialog is used to set the amount of transparency used in the GeoTIFF overlay.  Move the "
             "slider to change the value.  When the slider is changed the picture will be redrawn.");

QString hotKeysText = 
  prefs::tr ("Click this button to change the accelerator key sequences associated with some of "
             "the function buttons.");

QString hotKeyTableText = 
  prefs::tr ("Accelerator key sequences (hot keys) for some of the buttons in the main window may be changed "
             "in this dialog.  Up to four comma separated key values with modifiers may be used.  The format is "
             "illustrated by the following examples:\n\n"
             "    Q\n"
             "    Shift+q\n"
             "    Ctrl+q\n"
             "    Ctrl+Alt+q\n"
             "    Ctrl+x,Ctrl+c\n\n");

QString closeHotKeyText = 
  prefs::tr ("Click this button to close the hot key dialog.");

QString closeContoursText = 
  prefs::tr ("This button closes the contour level dialog.");

QString sunText = 
  prefs::tr ("Set sun shading options in this tab");

QString sunAzText = 
  prefs::tr ("Set the sun angle.  Sun angle is the direction from which the sun appears to be "
             "illuminating the sun shaded surface.  The default value is 30 degrees which would be "
             "from the upper right of the display.");

QString sunElText = 
  prefs::tr ("Set the sun elevation.  Sun elevation is the apparent elevation of the sun above the "
             "horizon for the sun shaded surface.  The default value is 30 degrees.  Acceptable "
             "values are from 0.0 (totally dark) to 90.0 (directly overhead).");

QString sunExText = 
  prefs::tr ("Set the sun shading Z exaggeration.  This is the apparent exaggeration of the surface "
             "in the Z direction.  The default is 1.0 (no exaggeration).");

QString textSearchText = 
  prefs::tr ("Set the feature text search string.  The program will do a case insensitive search of all "
             "displayed feature descriptions and remarks for this text string.  Features that contain the "
             "text string will be highlighted.");

QString iconSizeText = 
  prefs::tr ("Set the size of the main window buttons.  The options are 16, 20, 24, 28, or 32 pixels.<br><br>"
             "<b>IMPORTANT NOTE: The icons size will change when you click the <i>Apply</i> button but "
             "the locations will not be entirely correct until you have exited and restarted the program.</b>");

QString prefUnloadText = 
  prefs::tr ("Select this if you wish unload dialogs that appear when unloading the PFM to automatically close "
             "when the unload process has completed without error.  If there is an error the dialog will not close "
             "and a warning message will be displayed.");

QString miscText = 
  prefs::tr ("This tab contains settings for a number of miscellaneous options including MISP surface "
             "parameters, button hot keys, button icon size, and the feature search string.");

QString colorText = 
  prefs::tr ("Set color options in this tab.");

QString contourColorText = 
  prefs::tr ("Click this button to change the color that is used to plot contours.  "
             "After selecting a color the map will be redrawn.");

QString gridColorText = 
  prefs::tr ("Click this button to change the color that is used to draw the overlay grid.  "
             "After selecting a color the map will be redrawn.");

QString backgroundColorText = 
  prefs::tr ("Click this button to change the color that is used as NULL background.  After "
             "selecting a color the map will be redrawn.");

QString coastColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the coasts.  After "
             "selecting a color the map will be redrawn.");

QString contourHighlightColorText = 
  prefs::tr ("Click this button to change the color that is used to highlight the selected "
             "contour interval when drawing contours and/or features highlighted by text search.  "
             "After selecting a color the map will be redrawn.");

QString maskColorText = 
  prefs::tr ("Click this button to change the color that is used to draw the polygon filter mask areas.  "
             "If you set the alpha value to anything other than 255 the area will be drawn as a filled "
             "polygon with some amount of transparency.  If you set it to 255 the area will be drawn as "
             "an empty polygon.  After selecting a color the map will be redrawn.");

QString featureColorText = 
  prefs::tr ("Click this button to change the color that is used to plot features.  "
             "After selecting a color the map will be redrawn.<br><br>"
             "<b>IMPORTANT NOTE: Setting the alpha value to less than 255 will cause the features to be drawn as semi-transparent, filled circles "
             "instead of as an empty circle.</b>");

QString verFeatureColorText = 
  prefs::tr ("Click this button to change the color that is used to plot verified features (confidence = 5).  "
             "After selecting a color the map will be redrawn.");

QString covVerFeatureColorText = 
  prefs::tr ("Click this button to change the color that is used to plot verified features (confidence = 5) on the coverage map.  "
             "After selecting a color the map will be redrawn.");

QString featureInfoColorText = 
  prefs::tr ("Click this button to change the color that is used to write the feature information "
             "next to the featuresplot features.  After selecting a color the map will be redrawn.");

QString featurePolyColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the polygonal areas that "
             "are associated with specific features.  Note that if you set the alpha channel to "
             "less than 255 the area will be plotted as a filled polygon with the semi-transparency "
             "that is set for the alpha channel (0 = transparent, 255 = opaque).  After selecting a "
             "color the map will be redrawn.");

QString covFeatureColorText = 
  prefs::tr ("Click this button to change the color that is used to plot features on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covCoastColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the coastline on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covBoxColorText = 
  prefs::tr ("Click this button to change the color that is used to draw the edit box on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covDataColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the valid data coverage on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covCheckedColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the valid/checked data coverage on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covVerifiedColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the valid/verified data coverage on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covBackgroundColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the background (i.e. no data) on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covHotPFMColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the outline of the PFM of the highest active PFM on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covColdPFMColorText = 
  prefs::tr ("Click this button to change the color that is used to plot the outline of all of the lower active PFMs on the coverage "
             "map.  After selecting a color the coverage map will be redrawn.");

QString covInvFeatureColorText = 
  prefs::tr ("Click this button to change the color that is used to plot invalidated features on the "
             "coverage map.  After selecting a color the coverage map will be redrawn.");

QString stoplightMinColorText = 
  prefs::tr ("Click this button to change the color that is used to draw data below the stoplight mid to min crossover "
             "value.  This defaults to red for depths and green for other attributes (like standard deviation).");

QString stoplightMidColorText = 
  prefs::tr ("Click this button to change the color that is used to draw data between the mid to min crossover and the mid to max crossover "
             "values.  This defaults to yellow.");

QString stoplightMaxColorText = 
  prefs::tr ("Click this button to change the color that is used to draw data above the stoplight mid to max crossover "
             "value.  This defaults to green for depths and red for other attributes (like standard deviation).");

QString scaledOffsetZColorText = 
  prefs::tr ("Click this button to change the color that is used to draw Z values in the information box when the Z value is being "
             "scaled and/or offset (for example meters to feet and/or EGM2008 Z offset aplied).");

QString fontText = 
  prefs::tr ("Click this button to change the font used for text in the application.<br><br>"
             "<font color=\"#ff0000\"><b>IMPORTANT NOTE: The font that you set in pfmView will be used in all other ABE map GUI applications "
             "(e.g. pfmEdit3D, pfm3D, mosaicView, chartsPic, etc.) upon restart of the application.  This is the only place you can change "
             "fonts for the ABE map GUI applications.</b></font>");

QString restoreDefaultsText = 
  prefs::tr ("Click this button to restore all of the preferences to the original default values.  "
             "This includes <b><i>all</i></b> of the system preferences, not just those in the "
             "preferences dialog.  This includes things like the highlight option, surface viewed, "
             "contours viewed, contour levels, etc.");

QString closePrefsText = 
  prefs::tr ("Click this button to discard all preference changes and close the dialog.  The main map and "
             "coverage map will not be redrawn.");

QString applyPrefsText = 
  prefs::tr ("Click this button to apply all changes and close the dialog.  The main map and coverage map will "
             "be redrawn.");
