
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



/*  C/Motif pfm_view replaced by C++/Qt pfmView on 02/25/05.  All comments for pfm_view
    have been left in for historical (hysterical ???) purposes.  JCD  */


#ifndef VERSION

#define     VERSION     "PFM Software - pfmView V10.03 - 08/07/19"

#endif

/*< <pre>


******************* pfm_view comments *********************


    Version 1.0
    Jan C. Depner
    05/14/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.01
    Jan C. Depner
    02/23/00

    Lots of groovy enhancements (sound, logo).


    Version 2.1
    Jan C. Depner
    03/29/00

    Uses version 2.1 of the pfm library.  Added view of class1 and class2 data.
    Removed 'normalize colors' button.


    Version 2.2
    Jan C. Depner
    04/06/00

    Allows the use of polygons to define editing areas.


    Version 2.3
    Jan C. Depner
    04/16/00

    Changed message from scrolled_list widget to list widget.  Display 
    distance with each operation in the window.


    Version 2.31
    Jan C. Depner
    04/23/00

    Fixed a bunch of small annoying bugs.  Added a few minor enhancements.


    Version 2.32
    Jan C. Depner
    05/02/00

    Speeded up the coverage map drawing.


    Version 2.33
    Jan C. Depner
    06/22/00

    Removed Sound.  Added extract, unload, delete files, restore files to
    utilities pulldown.


    Version 2.34
    Jan C. Depner
    08/20/00

    Minor changes, mostly having to do with SHOALS data.


    Version 2.35
    Jan C. Depner
    08/28/00

    Overlay data.


    Version 2.36
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 2.37
    Jan C. Depner
    10/03/00

    Leave the rectangle or polygon on the screen even if you don't change 
    anything.  Added ability to output single point bin records to ASCII
    file.  Added multiple file coverage highlighting.


    Version 2.38
    Jan C. Depner
    11/27/00

    Removed the landmask stuff from the coverage map drawing.


    Version 2.40
    Jan C. Depner
    12/06/00

    Uses the coverage map placed at the end of the bin file in PFM 3.0.


    Version 2.41
    Jan C. Depner
    12/28/00

    Added a nice percent spinner for setting the area checked or unchecked.


    Version 2.42
    Jan C. Depner
    01/29/01

    Simplified view_cbs and hatchr.  Made hatchr only redraw edited data not 
    the whole display.  A bit faster now.


    Version 2.43
    Jan C. Depner
    01/31/01

    Added the ability to filter and clear edits for the displayed area.


    Version 2.44
    Jan C. Depner
    02/07/01

    Fixed a minor hiccup in the scroll bars.


    Version 2.45
    Jan C. Depner
    02/21/01

    Passing scale to open_pfm_file as a pointer.


    Version 2.46
    Jan C. Depner
    05/06/01

    Added the demo version checking.  Removed ACME Software references.


    Version 2.47
    Jan C. Depner
    06/05/01

    Added selected sounding check.


    Version 2.48
    Jan C. Depner
    06/10/01

    Always highlight land.


    Version 3.0
    Jan C. Depner
    07/16/01

    Uses version 4.0 of the PFM library.  Major changes!!!!


    Version 3.1
    Jan C. Depner
    10/01/01

    A whole bunch of little annoying things have been fixed.  You can now
    go outside the bounds to define a rectangle in the coverage map.  The
    attributes don't show up in the layer types unless they were loaded.
    Other little goodies.


    Version 3.11
    Jan C. Depner
    10/23/01

    Made the program shell "pfm_edit_demo" if running the demo version.  Fixed
    small bug in find min/max bins.  Added max standard deviation to find
    min/max bins.


    Version 3.12
    Jan C. Depner
    10/30/01

    Added the ability to open a file on the command line.  Added confirmation
    to the "Clear Edits" option.  Added dialog to allow user to do sounding
    selection for a specific area.


    Version 3.13
    Jan C. Depner
    11/14/01

    Save the "Auto Redraw After Edit" option to .pfm_view file.  Corrected
    center of minimum circle on "Find Min/Max Bins".


    Version 3.14
    Jan C. Depner
    12/11/01

    Fixed bug when in polygon edit mode and no auto redraw.


    Version 3.2
    Jan C. Depner
    06/05/02

    Added changes to support PFM 4.1.  Changes affect CLASS flags and 
    attributes.


    Version 3.21
    Jan C. Depner
    07/02/02

    Clearing edits now sets the PFM_MODIFIED flag if it clears any flags.


    Version 4.0
    Jan C. Depner
    07/12/02

    Used Jeff Parker's (SAIC) static color allocating idea to increase
    drawing speed.  At least an order of magnitude faster!!!  Why didn't
    I think of this!?


    Version 4.01
    Jan C. Depner
    09/05/02

    Changed the viewer to support other surfaces in the average and average
    filtered fields.  This is getting ready for the CUBE navigation surface.


    Version 4.02
    Jan C. Depner
    09/19/02

    Removed PFM_USER_05 and replaced it with PFM_INTERPOLATED.  Don't forget,
    today is National Talk Like A Pirate Day (Aaaaarrrrr, just ask Dave Barry
    me bucko).


    Version 4.03
    Jan C. Depner
    11/06/02

    Removed DEMO_VERSION.


    Version 4.1
    Jan C. Depner
    11/13/02

    Added contour smoothing.


    Version 4.11
    Jan C. Depner
    12/13/02

    Fixed a bug in the "Clear edits for displayed area" function.


    Version 4.12
    Jan C. Depner
    07/03/03

    Added ability to recompute bin values for the displayed area.


    Version 4.2
    Jan C. Depner
    07/18/03

    Went to 32 bits for validity fields.


    Version 4.21
    Jan C. Depner
    08/07/03

    Changed the way we display negative soundings (topo LIDAR).  Range of data
    is now computed from max to zero and from zero to min to get color values.


    Version 4.22
    Jan C. Depner
    08/11/03

    Added target search to utilities.  Added min and max Z to first message
    window after redraw.


    Version 4.3
    Jan C. Depner
    08/20/03

    Added unload of displayed area to edit options.  Fixed small bug that
    showed up when largest possible max and min values were displayed.


    Version 4.4
    Jan C. Depner
    09/25/03

    Fixed bug when displaying interpolated data.  Made highlight interpolated
    data highlight on interpolated cells when you fill all the cells (-a
    option in pfm_misp).


    Version 4.41
    Jan C. Depner
    11/25/03

    Kicked number of overlays to 40.  Fixed closing of ISS60 .ARE overlays.


    Version 4.5
    Jan C. Depner
    02/20/04

    Modified to use the new (Wade Ladner) XML format target files.  The old
    HYDRO .tgt format is no longer supported.


    Version 4.6
    Jan C. Depner
    04/06/04

    Improvements to sunshade.  Fixed sunshade cell size.  Latest updates to
    Wade's XML target format.  Cleaned up old "demo" stuff.


    Version 4.61
    Jan C. Depner
    08/04/04

    Added remarks to description in the find target utility.  Added Display
    Reference option.


    Version 4.7
    Jan C. Depner
    08/17/04

    Added ability to display GeoTIFF as a surface.  Added conversion factor
    for Z values (useful when looking at charts in feet or fathoms).


    Version 4.71
    Jan C. Depner
    10/17-16/04

    A few minor enhancements - display deleted targets when displaying min,
    max, or average depth (invalid data).  Optionally update coverage map
    automatically on a change of area or surface validity.  Added green
    targets to coverage map to indicate deleted targets when displaying
    invalid data.  Fix for setting color range when all values are negative.
    Speeded up the find target function in utilities.  Written while crossing
    the international dateline (Continental 0002) from Palau to Hawaii on 
    10/17/04, 10/16/04 ;-)


    Version 4.72
    Jan C. Depner
    11/05/04

    Added distance tracking to OUTPUT_POINTS.  Added ability to define area
    files (DEFINE_AREA).


    Version 4.73
    Jan C. Depner
    11/12/04

    Bug fix for new compiler in FC3.


    Version 4.74
    Jan C. Depner
    12/09/04

    Made Edit and View menus tear-offs.


    Version 4.75
    Jan C. Depner
    01/14/05

    Added text output for point overlays.  Just add text after lat, lon, in 
    .pts file.


    Version 4.8
    Jan C. Depner
    01/19/05

    Support for attributes.



******************* pfmView comments *********************


    Version 1.0
    Jan C. depner
    02/25/05

    This is the first version of the C++/Qt replacement for pfm_view.  As such, 
    I have gone back to 1.0 but didn't want to lose the earlier documentation
    of the C/Motif pfm_view program.


    Version 1.01
    Jan C. depner
    03/01/05

    Added the ability to display snippets, edit remarks, and validate/invalidate 
    targets in findTarget.  Changed a bunch of NV_CHAR things to QString.  Check for
    an already running PFM/ABE, SABER, or Fledermaus application.


    Version 1.02
    Jan C. depner
    03/09/05

    The program now checks to see if you are already running PFM/ABE, SABER, or Fledermaus
    and allows you to gracefully abort or to delete shared memory and continue.


    Version 1.03
    Jan C. depner
    04/19/05

    Changed all of the QVBox's to QVBoxLayout's.  This allows the dialogs to auto
    size correctly.  I'm learning ;-)


    Version 1.04
    Jan C. depner
    04/26/05

    Disable attribute buttons when drawing if they are present.  Fix zoom out screwup.


    Version 1.05
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.06
    Jan C. Depner
    05/17/05

    Checking for drawing_canceled in displayMinMax and overlayFlags.


    Version 1.07
    Jan C. Depner
    05/31/05

    Disabled key press events when pfm_edit_active.


    Version 1.08
    Jan C. Depner
    07/28/05

    Allow the user to open another file instead of forcing him to exit to change
    files.


    Version 1.09
    Jan C. Depner
    08/23/05

    Added extended help to start up browser using the Area_Based_Editor.html file.


    Version 1.10
    Jan C. Depner
    10/26/05

    Now uses the PFM 4.6 handle file or list file instead of the PFM directory name.


    Version 2.0
    Jan C. Depner
    11/08/05

    PFM version 5.0 changes.


    Version 2.01
    Jan C. Depner
    01/29/06

    Updated map and rproj.


    Version 2.02
    Jan C. Depner
    03/07/06

    Finished target updates.


    Version 3.0
    Jan C. Depner
    04/03/06

    Massive Qt changes - new cursors, left click to finish operations, right click menu in some modes, fixed 
    stretch, replaced QVBox, QHBox and others with layouts.


    Version 3.1
    Jan C. Depner
    05/11/06

    Added remisp function.  Runs automatically after edit when the average_filt_name is AVERAGE MISP SURFACE, 
    MINIMUM MISP SURFACE, or MAXIMUM MISP SURFACE.  There is also a function and button.  Added ability to draw and
    insert contour data into the PFM.  This also creates a GSF file in the pfm.data directory.
    Added area based filter function.


    Version 3.11
    Jan C. Depner
    05/17/06

    Save the last target description to place in the description field on next add_target.


    Version 3.12
    Jan C. Depner
    05/25/06

    Added 1:50,000 coastlines, added coast on/off button to main tool bar, removed coast selection buttons from
    preferences.  Coast selection is now based on area displayed.


    Version 3.13
    Jan C. Depner
    06/01/06

    Changed "set checked/verified" buttons to a popup menu to save a bit of space.  Fixed bug in
    right click popup menu function.


    Version 3.14
    Jan C. Depner
    06/05/06

    Removed inside.cpp.  Moved to utility.


    Version 3.15
    Jan C. Depner
    06/13/06

    Added index contour and contour line thickness options to preferences.  Changed snippet viewer to 
    targetPic.


    Version 3.16
    Jan C. Depner
    07/07/06

    Don't open projected PFMs.


    Version 3.17
    Jan C. Depner
    07/31/06

    Changed cov cursor to the edit_rect cursor from SizeAllCursor.  Jamie made me do it ;-)


    Version 3.2
    Jan C. Depner
    08/17/06

    Changed draw_contour functionality.  Added window overlap percentage to prefs and env_in_out.


    Version 3.21
    Jan C. Depner
    08/22/06

    Fixed MISP boundary problem.  Disallow contour drawing outside of PFM boundaries.


    Version 3.22
    Jan C. Depner
    09/13/06

    Had to change map.cpp and map.hpp to nvmap.hpp and nvmap.cpp in utility.


    Version 3.23
    Jan C. Depner
    09/29/06

    Added depth to UpdateTarget dialog.


    Version 3.24
    Jan C. Depner
    01/16/07

    Only send heading to targetPic for UNISIPS images.  The LIDAR ones may already be rotated.


    Version 3.25
    Jan C. Depner
    01/26/07

    Only update load progress bar at 20% increments.


    Version 3.26
    Jan C. Depner
    02/12/07

    Check exit status from unload process and issue error message if needed.


    Version 3.27
    Jan C. Depner
    02/15/07

    Fix contour drawing by adding some polyline functions to nvmap.cpp in utility library.


    Version 3.28
    Jan C. Depner
    02/16/07

    Allocate one more than gridcols in remisp.c (see comments).


    Version 3.29
    Jan C. Depner
    02/18/07

    Correct bug in remisp.cpp.  We weren't sending the right and upper interpolated data (doughnut
    data) to misp.  This had no effect with real data.


    Version 4.00
    Jan C. Depner
    04/02/07

    Qt 4 port and many minor improvements.


    Version 4.01
    Jan C. Depner
    04/30/07

    Correct bug in pfmView.cpp setMainButtons - don't try to play with bSurface buttons until
    a file has been opened.


    Version 4.02
    Jan C. Depner
    05/30/07

    Added alpha values to saved colors.


    Version 4.03
    Jan C. Depner
    06/25/07

    Switched to using GDAL to read TIFF info and QIMage to read TIFF data.


    Version 4.04
    Jan C. Depner
    07/31/07

    Added ability to apply offset in addition to scaling.


    Version 4.05
    Jan C. Depner
    08/06/07

    Added ability to switch target files.  Use MEDIUM_TARGET instead of SHORT_TARGET to speed up 
    search and display of targets.


    Version 4.06
    Jan C. Depner
    08/21/07

    Changed target marker to circle instead of square - much more elegant ;-)  Added zoomToArea
    utility.


    Version 4.07
    Jan C. Depner
    08/24/07

    Fixed geotiff transparency problem.  Added ability to underlay the geotiff.
    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 4.08
    Jan C. Depner
    09/04/07

    Only write a new line to the line file for each single new GSF file created when drawing
    contours.


    Version 4.09
    Jan C. Depner
    09/12/07

    Changed upper limit on percent overlap to 95 instead of 25 (request from BNPC folks).


    Version 4.10
    Jan C. Depner
    09/17/07

    Added EGM96 button to "offset" in prefs.  Fixed redraw bug after edit.


    Version 4.11
    Jan C. Depner
    09/19/07

    Cleaned up remisp progress bar code.


    Version 4.20
    Jan C. Depner
    10/10/07

    Shells the mosaicView program.


    Version 4.21
    Jan C. Depner
    10/15/07

    Tracks cursor in other ABE applications.


    Version 5.00
    Jan C. Depner
    12/03/07

    Replaced POSIX shared memory with QSharedMemory.  Added fflush calls after prints to stderr since flush
    is not automatic in Windows.  Sets the CHILD_PROCESS_FORCE_EXIT key on exit.
    Modifies the set_defaults.cpp file on build to reflect the correct location of the extended help file.


    Version 5.01
    Jan C. Depner
    12/06/07

    Fixed edit area redraw problem.


    Version 5.02
    Jan C. Depner
    12/11/07

    Changed default toolbar locations.


    Version 5.03
    Jan C. Depner
    12/13/07

    Save working directory locations.  Changed QWidgetItem to QCheckBox to deal with bug in Qt 4.4 snapshot.


    Version 5.04
    Jan C. Depner
    12/28/07

    Uses multiple movable objects as added to nvMap.


    Version 5.10
    Jan C. Depner
    01/04/08

    Now uses the process ID plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 5.20
    Jan C. Depner
    01/08/08

    Handles "area move" requests on return from pfmEdit.


    Version 5.21
    Jan C. Depner
    01/14/08

    Uses check_target_schema (from utility library) to make sure that the schema in target files is correct.


    Version 5.22
    Jan C. Depner
    01/23/08

    Fix problem with saving the last target description and remarks for use in the next update target dialog.


    Version 5.23
    Jan C. Depner
    02/07/08

    When defining rectangular or polygonal area files, add them to the overlay list.


    Version 5.24
    Jan C. Depner
    03/12/08

    Use "." as command line argument to force file open dialogs to use the current working directory
    instead of the saved last-used directory.  Added warning when displaying more bins than pixels.


    Version 5.25
    Jan C. Depner
    03/13/08

    Kill or reload the mosaicView program if it was running when we switch files.


    Version 5.26
    Jan C. Depner
    03/14/08

    Set wait cursor when drawing target polygons since it can take a long time.  Try to force an
    SMB/NFS cache flush by closing and opening the file on setAreaChecked operations so that 
    remote clients will be kept up to date on the status of editing by others.  Of course, it may
    not work ;-)


    Version 5.27
    Jan C. Depner
    03/28/08

    Added black outline on min/max indicators.  Switched from a signed misp weight factor to an
    unsigned 1 to 3 value and a force original input flag.  Confirmation dialog for unload button.
    Export image files in JPG, PNG, GeoTIFF, BMP, or XPM format.


    Version 5.28
    Jan C. Depner
    03/31/08

    Moved highlight buttons to a menu button in the toolbar.  We were running out of toolbar space
    and we don't use those very much anyway.  Added bin size to the status bar.  Added highlight IHO
    Special Order, Order 1, and Order 2 menu options.  Added highlight percentage of depth menu option.
    The IHO and percentage of depth highlight options were suggested by Glenn Carson.


    Version 5.29
    Jan C. Depner
    04/01/08

    Added acknowledgments to the Help pulldown menu.


    Version 6.00
    Jan C. Depner
    04/03/08

    Removed sunshade button (we always sunshade).  Removed number of soundings and standard deviation
    as surfaces.  These and the attributes are now used for coloring the sunshaded surface.  Handle 
    the two newer reserved attributes (CUBE).  Display attribute values in their own status bar.
    Cleaned up the highlight icon mess.  Damn!  Now I have to rewrite the documentation!  Again!


    Version 6.01
    Jan C. Depner
    04/04/08

    bSurface bug fix.


    Version 6.02
    Jan C. Depner
    04/07/08

    Replaced getuid with windows_getuid inside and ifdef NVWIN3X so we didn't have any conflicts
    on Linux/UNIX.  Replaced single .h and .hpp files from utility library with include of nvutility.h
    and nvutility.hpp


    Version 6.10
    Jan C. Depner
    04/14/08

    Added the new ABE register functions so we can hook up to programs that we didn't shell.


    Version 6.11
    Jan C. Depner
    04/15/08

    Added target protection for the filter process.


    Version 6.12
    Jan C. Depner
    04/21/08

    Invert the colors for color_by_attribute.


    Version 6.20
    Jan C. Depner
    04/22/08

    Added CUBE process button and functionality.


    Version 6.21
    Jan C. Depner
    04/23/08

    Added CUBE settings to prefs dialog.


    Version 6.22
    Jan C. Depner
    04/24/08

    Cleaned up the way extended help was being handled.


    Version 6.30
    Jan C. Depner
    05/29/08

    Added ability to change the PFM input files in the PFM control file.  Also, check entered contour levels for out
    of range condition.


    Version 6.31
    Jan C. Depner
    06/03/08

    Fixed problem of writing out beam number 0 to contour generated GSF files.


    Version 6.32
    Jan C. Depner
    06/05/08

    Added UNIX/Windows path substitution to the change path capability.


    Version 6.33
    Jan C. Depner
    06/06/08

    Add Output Data Points files to overlay list.


    Version 6.34
    Jan C. Depner
    06/10/08

    Now issues the ZOOM_TO_MBR_COMMAND to all connected ABE applications when you press the "z" key
    (or whatever key you have set for mosaicView's zoom in pfmEdit).


    Version 6.35
    Jan C. Depner
    06/30/08

    Fix bug in dealing with ZOOM_TO_MBR_COMMAND from pfmEdit when not linked.


    Version 7.00
    Jan C. Depner
    07/10/08

    Now handles up to MAX_ABE_PFMS (see ABE.h in utility library) PFM files at one time.
    Happy Birthday to Jennifer and Chris.


    Version 7.01
    Jan C. Depner
    07/22/08

    Fixed multiple free of misc.target.  Also, now checks validity of data in bin when the mouse moves over
    the bin.  This way we will get decent values from underlying layers when the higher level layers have
    empty bins.


    Version 7.02
    Jan C. Depner
    07/28/08

    writeGSFFile will only write out points that fall inside the PFM area(s).  Also, moved some code from
    pfmView.cpp to external functions (trying to remove code bloat from pfmView.cpp).


    Version 7.03
    Jan C. Depner
    07/29/08

    Fixed display of verified data in the coverage map.


    Version 7.12
    Jan C. Depner
    09/15/08

    Fixed target display with multiple PFMs.


    Version 7.13
    Jan C. Depner
    10/20/08

    Save target search string to settings.


    Version 7.20
    Jan C. Depner
    11/07/08

    Broke two toolbars up into 7 toolbars.  Do a better job of saving toolbar and window positions and sizes.
    Added ability to change tool button icon sizes.  Added View menu so that if you turn off all of the tool bars
    you can easily figure out how to turn them back on again (right click in the menu bar gives the same menu).
    Allow resizing of main button icons.  Added Preferences to Edit menu.


    Version 7.21
    Jan C. Depner
    12/04/08

    Now adds PFM_SELECTED_FEATURE to points that have a target added.


    Version 7.22
    Jan C. Depner
    12/08/08

    Added ability to define filter mask areas.


    Version 7.23
    Jan C. Depner
    12/15/08

    Added screenshot capture to updateTarget.


    Version 7.24
    Jan C. Depner
    12/16/08

    Fix "array out of bounds" SOD in pfmFilter.cpp.


    Version 7.25
    Jan C. Depner
    01/23/09

    Removed event accept from keyPressEvent handler.


    Version 7.26
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 7.27
    Jan C. Depner
    02/03/09

    Added option to consolidate targets within a target polygon.


    Version 7.30
    Jan C. Depner
    02/11/09

    pfm3D can now tell pfmView to kick off a polygon edit.  Added coordination between pfmView, pfmEdit3D,
    and pfm3D.  When pfmEdit3D is started pfm3D will unload it's display lists and memory unless you move
    the cursor back into pfm3D in which case it will reload until you leave again.  Lists and memory will
    be reloaded upon exit from pfmEdit3D.


    Version 7.40
    Jan C. Depner
    03/31/09

    Replaced support for NAVO standard XML target file with Binary Feature Data (BFD) file support.


    Version 7.41
    Jan C. Depner
    04/08/09

    Changes to deal with "flush" argument on all nvmapp.cpp (nvutility library) drawing functions.


    Version 7.42
    Jan C. Depner
    04/10/09

    Can now select shoalest or current when grouping features.  Also, added option to delete
    feature polygons (and automatically un-group members).


    Version 7.43
    Jan C. Depner
    04/17/09

    Added rectangular filter and filter mask buttons as well as a clear filter mask button.


    Version 7.44
    Jan C. Depner
    04/23/09

    Changed the acknowledgments help to include Qt and a couple of others.


    Version 7.45
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 7.46
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 7.47
    Jan C. Depner
    05/22/09

    Dealing with multiple resize events when "Display content in resizing windows" is 
    enabled.


    Version 7.48
    Jan C. Depner
    06/01/09

    Speeded up contouring by writing to pixmap and then blasting to screen.


    Version 7.49
    Jan C. Depner
    06/02/09

    Added ability to set high and low points and add them to the BFD file (this is for BNPC production).


    Version 7.50
    Jan C. Depner
    06/03/09

    Fixed dateline issues (I hope).


    Version 7.60
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.  Added support for using GMT Surface for interpolated average surface.


    Version 7.61
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 7.62
    Jan C. Depner
    07/17/09

    YADP - Yet Another Dateline Problem.  This time it was in the contour drawing code.


    Version 7.63
    Jan C. Depner
    07/28/09

    Changed use of _sleep to Sleep on MinGW (Windows).


    Version 7.64
    Jan C. Depner
    07/29/09

    Added tool tips to tool bars.
    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 7.65
    Jan C. Depner
    08/03/09

    Changed the unload button to an unload menu allowing you to unload the entire file if you want to.


    Version 7.66
    Jan C. Depner
    08/17/09

    Made Standard Deviation symbol (diamond) smaller when displaying min/max/std points.  Added message box
    with unload information.  Added ability to plot ESRI SHAPE files as overlays (I think ;-)


    Version 7.67
    Jan C. Depner
    08/26/09

    Added support for automatic filter masking when returning from the point editor.


    Version 7.68
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 7.69
    Jan C. Depner
    09/23/09

    Support for flagging verified features (confidence = 5).


    Version 7.70
    Jan C. Depner
    11/16/09

    Added ability to do a MISP filter.  This lets you draw contours and then define an area to be filtered.  The contours
    are used to compute the MISP surface then the depth data is decimated to closely match the MISPed surface.  After
    that, the MBR of the area is reMISPed.


    Version 7.71
    Jan C. Depner
    12/02/09

    Replaced get_egm96 with get_egm08.


    Version 7.72
    Jan C. Depner
    12/11/09

    Added support for overlay of Army Corps area files.


    Version 7.73
    Jan C. Depner
    12/15/09

    Added a ton of comments to the code.  Also, displayMinMax now works on all of the PFM layers instead 
    of just the top (0) layer.


    Version 7.74
    Jan C. Depner
    12/29/09

    Stopped the filter from filtering points when there is not enough surrounding valid data.


    Version 7.75
    Jan C. Depner
    01/08/10

    Stopped remisp from gridding over land masked points.


    Version 7.76
    Jan C. Depner
    01/19/10

    Fixed startup message bug.


    Version 8.00
    Jan C. Depner
    01/22/10

    Completely reworked the GUI.  Moved the coverage map inside on the left.  Moved the status info to the left in a
    tabbed widget along with PFM layer display checkboxes on the second page.  Also, replaced all QProgressDialogs with the
    status bar QProgressBar (much more elegant ;-)


    Version 8.01
    Jan C. Depner
    01/25/10

    Moved the OTF grid to shared memory so we can use it in pfm3D (if it's up).  BTW - New Orleans Saints,
    2009 NFC Champions... Who Dat!!!


    Version 8.02
    Jan C. Depner
    01/28/10

    Can now open multiple PFMs at once.  Fixed bug in coverage when editing multiple PFMs but some were not in 
    the edited area.  It would try to allocate negative memory... DOH!


    Version 8.03
    Jan C. Depner
    02/10/10

    Added support for running pfmView from within pfmWDBView (the mother of all PFM database viewers).


    Version 8.04
    Jan C. Depner
    03/05/10

    Added option to turn off the infamous TPOSIAFPS message after the first time.


    Version 8.05
    Jan C. Depner
    03/08/10

    Added support for drawing contours when called from pfmWDBView (have to set proper attributes).


    Version 8.06
    Jan C. Depner
    03/11/10

    Wasn't displaying bin size properly when bins were defined in minutes.


    Version 8.07
    Jan C. Depner
    03/19/10

    Fixed bug in OTF display.  Fixed bug in hatchr when called from paint_otf_surface and highlight
    or attribute was set.


    Version 8.08
    Jan C. Depner
    03/26/10

    setAreaChecked now handles all displayed PFM layers if you have more than one PFM opened.


    Version 8.10
    Jan C. Depner
    03/29/10

    Added Open Recent... entry to the file menu.


    Version 8.11
    Jan C. Depner
    03/31/10

    Made #define statements for the overlay types (e.g. ACE_AREA, ISS60_zone).


    Version 8.12
    Jan C. Depner
    04/01/10

    Fixed the security key dialog for hand-drawn contours.  April Fools.  No, just kidding, I really did fix it ;-)


    Version 8.13
    Jan C. Depner
    04/05/10

    Added error messages for not finding or opening the security.cfg file in loadSecurity.cpp.


    Version 8.14
    Jan C. Depner
    04/09/10

    Added diagonal movement of the viewed area using the PgUpm PgDn, Home, and End keypad keys.


    Version 8.20
    Jan C. Depner
    04/13/10

    Added color scale on a new tab widget page.  Made color restart at zero optional.  Removed some of the verbiage in the 
    status area.  Combined with a possible 10 bin attributes it was eating up too much real estate.


    Version 8.21
    Jan C. Depner
    04/19/10

    Fixed unload of multiple displayed PFM layers.  Some GUI tweaks to clean up the display.  Changed preferences dialog to use
    a QTabWidget so we don't eat up the entire screen.


    Version 8.22
    Jan C. Depner
    04/22/10

    Added delete file queue to the Edit menu.  This option allows you to mark files for deletion in pfmEdit and/or pfmEdit3D
    and then delete them in pfmView.  This option also allows you to delete files from more than one PFM.


    Version 8.23
    Jan C. Depner
    04/30/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 8.24
    Jan C. Depner
    06/03/10

    Added confirm dialog to verify features function.


    Version 8.25
    Jan C. Depner
    07/06/10

    Allow user to change (and save) min and max HSV colors for surface drawing.  Also, these can come in on the command
    line as --min_hsv_color and/or --max_hsv_color.  Allow the user to lock the min and max scale values.


    Version 8.26
    Jan C. Depner
    07/13/10

    Added command line options for locked min and max HSV values (--min_hsv_value and --max_hsv_value).  This is (for the
    moment) for Optech International.  I may use it for pfmWDBView later.


    Version 8.27
    Jan C. Depner
    07/20/10

    Fix land mask check bug in remisp.cpp.


    Version 8.28
    Jan C. Depner
    09/01/10

    Now automatically renames "deleted" file with .pfmView_deleted appended to the name and takes that off of the 
    name when you restore a file.  Also, added invert and display/highlight to feature search string ability.


    Version 8.29
    Jan C. Depner
    09/08/10

    Changes to handle Qt 4.6.3.


    Version 8.30
    Jan C. Depner
    09/16/10

    Added color by (avg-min), (max-avg), and (max-min).  Also added stoplight coloring.


    Version 8.31
    Jan C. Depner
    09/17/10

    Fixed bug displaying feature info.  Removed "highlight by depth band" since stoplight coloring is better.


    Version 8.32
    Jan C. Depner
    09/20/10

    Added support for command line areas or bounds to support CZMIL.


    Version 8.33
    Jan C. Depner
    10/01/10

    Fixed bug in writeGSFFile.cpp that caused segfault.


    Version 8.40
    Jan C. Depner
    10/08/10

    Removed GSF hand-drawn contour file generation.  Nobody was using it anyway.


    Version 8.41
    Jan C. Depner
    12/07/10

    Cleaned up the hotkey and accelerator handling quite a bit.  TORA! TORA! TORA!


    Version 8.42
    Jan C. Depner
    12/14/10

    You can now drag the pre-existing rectangle in the coverage map.


    Version 8.43
    Jan C. Depner
    12/16/10

    Force full redraw when returning from editor (with changes) if contouring is turned on.  It is 
    really hard to merge the contours correctly otherwise.


    Version 8.44
    Jan C. Depner
    12/21/10

    Allow top and bottom scale values to be locked for any "color by" option.


    Version 8.45
    Jan C. Depner
    01/05/11

    Added hotkey check in the preferences dialog to prevent conflicts.  I really should have done this a long time
    ago but it was way down on the priority list ;-)


    Version 8.46
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 8.47
    Jan C. Depner
    01/16/11

    Positioned non-modal popup dialogs in the center of the screen.  Changed default color bands to red-blue for all
    except color by depth (red-magenta).


    Version 8.48
    Jan C. Depner
    01/26/11

    Run misp_proc or surf_proc in a thread so we get a reasonable progress bar update.


    Version 8.49
    Jan C. Depner
    03/21/11

    Fixed filter feature radius bug.


    Version 8.50
    Jan C. Depner
    03/28/11

    Try to save the user's hot key definitions even across setting version changes.


    Version 8.51
    Jan C. Depner
    04/06/11

    Fixed highlight percentage of depth (gotta divide by 100... DOH!)  Thanx and a tip of the hat to Sheldon Powe
    for catching that one.


    Version 8.52
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks are now handled in ABE shared memory and are no longer passed as
    arguments to the editors.


    Version 8.53
    Jan C. Depner
    04/11/11

    Properly populate the file name when creating a new feature file.  It was leaving it blank on Windows.


    Version 8.54
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.


    Version 8.55
    Jan C. Depner
    05/10/11

    Minor bug fix handling generic point overlay files.


    Version 8.56
    Jan C. Depner
    05/13/11

    Fixed the way the color scale boxes are displayed when min or max is locked.


    Version 8.57
    Jan C. Depner
    06/06/11

    Modified the way we handle hand-drawn contours in order to eliminate jagged contours.  We now mark
    hand-drawn contour depths as PFM_DATA (not normally used in DEPTH_RECORD) and then look for this flag
    when remisp'ing the data.  If the flag is set, the point will be replaced with the misp'ed point but
    it will remain as a depth record (i.e. not interpolated).  This is a stop-gap fix until I can get
    NAVO to use CHRTR2 for gridding.


    Version 8.58
    Jan C. Depner
    06/23/11

    At the request of NAVO I have changed the "color by" drop down menu to a separate toolbar.  It actually
    works better this way since we tend to change the color options quite a bit.


    Version 8.59
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.
    Removed the . on the command line option since the current working directory is in the sidebar.  Added menu to 
    allow displaying of all, unverified, verified, or no features (as opposed to just on/off).


    Version 8.60
    Jan C. Depner
    06/30/11

    You can now color by count, std, avg-min, max-avg, and max-min in OTF mode.  This required quite a bit more memory
    but it was a needed feature.


    Version 8.61
    Jan C. Depner
    07/13/11

    Added highlight by number of valid poiints in bin.


    Version 8.62
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 8.63
    Jan C. Depner
    07/28/11

    Added display of average on-the-fly grid (edited and unedited).


    Version 8.64
    Jan C. Depner
    07/29/11

    Added output of "pfmView killed" qsetting to pfmEdit and pfmEdit3D qsettings so that those programs
    will know that pfmView was shut down since the last time they were running.  This allows us to 
    properly set the "monitor" type ancillary program "running" states since pfmView issues a kill
    command to all of the associated ancillary programs when it exits.


    Version 8.65
    Jan C. Depner
    10/18/11

    Fixed bug when writing hand drawn contours to the PFM.  It wasn't retrieving the
    pfmView_hand_drawn_contour line number so it was setting the line number to 0.
    Thanx and a tip of the hat to Laura Casey.


    Version 8.66
    Jan C. Depner
    10/24/11

    Added GRAB_CONTOUR mode to capture interpolated contours and place them into the PFM
    as if they were hand-drawn contours (MISP'ed PFMs only).  Thanx and yet another tip of
    the hat to Laura Casey.


    Version 8.67
    Jan C. Depner
    10/27/11

    After an edit (and remisp) I wasn't replacing the MISPed surface in bins with original
    data.  DOH!


    Version 8.68
    Jan C. Depner
    11/02/11

    Made replacing the MISP surface in bins with original data an option (just like it is in
    pfmMisp).  The option defaults to NVTrue and can be changed in the preferences dialog.
    Change the average filtered surface button to the cube_surface button if layer 0 PFM
    is the CUBE Surface.


    Version 8.69
    Jan C. Depner
    11/18/11

    Removed support for the GMT surface since no one was using (or wanted) it.


    Version 8.70
    Jan C. Depner
    11/30/11

    Converted .xpm icons to .png icons.


    Version 8.71
    Jan C. Depner
    12/29/11

    Fixed screenshot.cpp to grab both screens on Windows.  It will also scale the windows if
    they aren't the same height.


    Version 8.72
    Jan C. Depner
    03/07/12

    Removed qApp->processEvents () from resize callback to get rid of a crash that occurred on Windows when
    resizing.


    Version 8.73
    Jan C. Depner
    04/02/12

    Changed startup message to explain new Hockey Puck filter in pfmEdit(3D).


    Version 8.74
    Jan C. Depner
    06/20/12

    Removed support for CZMIL until the formats are finished.


    Version 8.75
    Jan C. Depner
    08/15/12

    Fixed changePath on Windows.  God, how I hate Windoze!


    Version 8.76
    Jan C. Depner
    08/23/12

    Place dummy name in the PFM label on the left side of the window prior to opening a file.  This stops
    the program from generating a resize when the name is added to the label after opening a file.  It 
    appeared to generate a resize randomly but it seems that it was caused by opening the file and moving
    the cursor rapidly into the coverage box area.  Having the dummy name seems to fix the problem.


    Version 8.77
    Jan C. Depner
    09/27/12

    Added ability to output defined area files as ESRI Shapefiles.


    Version 8.78
    Jan C. Depner (PFM Software)
    09/19/13

    Added highlight modified cells option to the highlight list.


    Version 8.79
    Jan C. Depner (PFM Software)
    09/20/13

    Added "Set multiple coverage checked" and "Set valid multiple coverage checked" options to the check menu.


    Version 8.80
    Jan C. Depner (PFM Software)
    12/09/13

    Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
    the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
    Linux or, in the registry (shudder) on Windows.


    Version 8.81
    Jan C. Depner (PFM Software)
    12/17/13

    Removed the landmask code.  Now that we have a really good coastline based on SWBD (and a mask built from it)
    we don't really need to draw the landmask anymore.  If you want to see where the landmask will be, just look
    at the coastline.


    Version 8.82
    Jan C. Depner (PFM Software)
    02/26/14

    Cleaned up "Set but not used" variables that show up using the 4.8.2 version of gcc.


    Version 8.83
    Jan C. Depner (PFM Software)
    02/28/14

    Reverted to using Qt to read geoTIFFs.  It quit working in Qt 4.7.2 but is working again.  It's
    much faster than using GDAL and it's probably a lot more bulletproof.


    Version 8.84
    Jan C. Depner (PFM Software)
    03/013/14

    Changed constant EPS to MISP_EPS to avoid collision with EPS defined in nvutility.


    Version 8.85
    Jan C. Depner (PFM Software)
    03/17/14

    Removed WLF support.  Top o' the mornin' to ye!
    Also, fixed the CZMIL part of get_feature_time.cpp.


    Version 8.86
    Jan C. Depner (PFM Software)
    03/19/14

    - Changed the pointer to the ABE user guide HTML file from PFM/Area_Based_Editor.html to PFM_ABE_User_Guide.html.
    - Straightened up the Open Source acknowledgments.


    Version 8.87
    Jan C. Depner (PFM Software)
    05/07/14

    - Fixed string literal problems in format strings.
    - Check error returnsS from fgets calls.


    Version 8.88
    Jan C. Depner (PFM Software)
    05/27/14

    - Added the new LGPL licensed GSF library to the acknowledgments.


    Version 8.89
    Jan C. Depner (PFM Software)
    06/19/14

    - Removed PFMWDB support.  No one was using it.  It seemed like a good idea but I guess not.


    Version 8.90
    Jan C. Depner (PFM Software)
    06/24/14

    - Moved the unload process to an external dialog (per layer) so that the user can move on
      with editing without waiting for it to finish.  Mosty for CZMIL processing where unload
      can be very time consuming.


    Version 8.91
    Jan C. Depner (PFM Software)
    06/30/14

    - Replaced all of the old, borrowed icons with new, public domain icons.  Mostly from the Tango set
      but a few from flavour-extended and 32pxmania.


    Version 8.92
    Jan C. Depner (PFM Software)
    07/04/14

    - Removed support for old NAVO target files.
    - Oh say can you see....


    Version 8.93
    Jan C. Depner (PFM Software)
    07/11/14

    - Removed the startup message.  It was only useful (if at all) when I was adding new features 
      like a madman.


    Version 8.94
    Jan C. Depner (PFM Software)
    07/19/14

    - Fixed the CR/LF Windows conundrum in unloadDialog.cpp.


    Version 8.95
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 8.96
    Jan C. Depner (PFM Software)
    07/29/14

    - Fixed errors discovered by cppcheck.


    Version 8.97
    Jan C. Depner (PFM Software)
    11/20/14

    - Moved the selection of 2D vs 3D editor to the preferences dialog.  The 2D editor has been out of date
    since June 2013 and this removes two buttons from the toolbar.


    Version 9.00
    Jan C. Depner (PFM Software)
    12/23/14

    - Now honors the regen signal from pfmEdit3D that is sent after pfmEdit3D runs czmilReprocess.


    Version 9.01
    Jan C. Depner (PFM Software)
    12/26/14

    - Replaced most realloc arrays with std::vectors to adress speed issues on Windows.


    Version 9.02
    Jan C. Depner (PFM Software)
    01/17/15

    - Added QMessageBox to display number of points invalidted when filter is run.


    Version 9.03
    Jan C. Depner (PFM Software)
    01/31/15

    - Now adds first point to end of polygons defined in the Utilities->Define Polygonal Area File menu entry.


    Version 9.04
    Jan C. Depner (PFM Software)
    02/11/15

    - Got rid of the compile time CZMIL naming BS.  It now checks the ABE_CZMIL environment variable at run time.


    Version 9.05
    Jan C. Depner (PFM Software)
    02/13/15

    - To give better feedback to shelling programs in the case of errors I've added the program name to all
      output to stderr.


    Version 9.06
    Jan C. Depner (PFM Software)
    02/14/15

    - More error message cleanup.  All non-fatal error messages and fatal error messages from child processes (like
      pfmEdit3D) will now be displayed in a message dialog that will automatically pop up when a message is sent to
      stderr.  Happy Valentines day!


    Version 9.07
    Jan C. Depner (PFM Software)
    02/19/15

    - Fixed the redraw issue when running czmilReprocess in the editor (areaRegen).
    - Added the ability to start the editors in "no invalid" mode.  No invalid data will be loaded when starting the editor.
      Thanx and a giant tip of the hat to Brandon Ellis (NGC) for suggesting no loading the invalid data.


    Version 9.08
    Jan C. Depner (PFM Software)
    02/20/15

    - Switched the 2 editor buttons to a QMenu with 4 possible editor modes.


    Version 9.09
    Jan C. Depner (PFM Software)
    02/22/15

    - Added "Save as" option to error messages dialog.


    Version 9.10
    Jan C. Depner (PFM Software)
    02/27/15

    - Fixed the "areaRegen" stuff in slotEditDone.  The problem was that pfmEdit3D was modifying the bounds (like
      it's supposed to) but, in the case of czmilReprocess which causes the areaRegen, we wanted to restart the editor
      with the exact same area.


    Version 9.11
    Jan C. Depner (PFM Software)
    03/13/15

    - Fixed changePath.cpp when it dealt with a "deleted" input file.


    Version 9.12
    Jan C. Depner (PFM Software)
    03/31/15

    - Removed the color by "average minus minimum" and "maximum minus average" buttons.  The color by "maximum
      minus minimum" did a better job.  The other two were redundant.  Two fewer buttons, yay!


    Version 9.13
    Jan C. Depner (PFM Software)
    04/04/15

    - Added a PFMABE release check for public domain versions.  It will attempt to check the latest version
      on http://pfmabe.software.


    Version 9.14
    Jan C. Depner (PFM Software)
    04/13/15

    - Don't display CUBE button if navo_pfm_cube (proprietary) is not available on system.


    Version 9.15
    Jan C. Depner (PFM Software)
    05/26/15

    - Fixed the ET_PHONE_HOME stuff.  I had forgotten to add the QNetworkAccessManager and QNetworkRequest stuff.


    Version 9.16
    Jan C. Depner (PFM Software)
    06/29/15

    - Added semi-transparent coastlines to the coverage map.  Also made all coverage map colors modifiable in the preferences
      dialog.


    Version 9.17
    Jan C. Depner (PFM Software)
    06/30/15

    - Added Northing and Easting values to left display.


    Version 9.18
    Jan C. Depner (PFM Software)
    07/14/15

    - Made alternate background color for information labels match standard background.
    - Fixed the Preferences dialog so that it would properly scale the Z offset if the Z scaling factor was changed.


    Version 9.19
    Jan C. Depner (PFM Software)
    07/18/15

    - Hard-wired the information area fields background and text colors to white, gray, and black so that changes in the 
      system window colors won't do weird things to them.


    Version 9.20
    Jan C. Depner (PFM Software)
    07/21/15

    - Removed one line of code that supported the (obsolete) CZMIL attributeViewer program.


    Version 9.21
    Jan C. Depner (PFM Software)
    08/05/15

    - Removed "color by" buttons and moved that functionality to the attribute name labels (e.g. Depth, Point count, StdDev,
      or a PFM NDX attribute name.  Whichever field is currently being used for "color by" will have a background fading from red
      on the right to the normal background on the left.
    - Moved "Bin size" field from info section of notebook to PFM name slot in notebook.


    Version 9.22
    Jan C. Depner (PFM Software)
    08/07/15

    - Made scaleBox use the attribute format when coloring by attribute.


    Version 9.30
    Jan C. Depner (PFM Software)
    08/15/15

    - Added option to Preferences to display Z values to the user as either elevations or depths.


    Version 9.31
    Jan C. Depner (PFM Software)
    08/19/15

    - Select high point and low point options now save as either depth or elevation depending on the Z orientation.
      The points are also scaled and offset based on the settings in the Preferences dialog.
    - Fixed the What's This text for the displayMinMax button so that it would be correct based on the Z orientation.
    - Centered the displayMinMax shapes over the points.
    - Made the displayMinMax function compute the actual valid min, max, and max STD regardless of the surface being
      viewed.  It used to use the surface so, if you were viewing the minimum surface you would get the minimum bin,
      the maximum STD bin, and the maximum of the minimum bins.  What we really wanted was the minimum or all bins,
      the maximum standard deviation, and the maximum of all bins.


    Version 9.32
    Jan C. Depner (PFM Software)
    10/16/15

    - Added "Filter displayed area" button.
    - Added "UNDO" to filter.
    - Fixed the "Switch back to edit mode" key.


    Version 9.33
    Jan C. Depner (PFM Software)
    11/05/15

    - Made the Preferences dialog open on the last used tab used instead of just defaulting to the first tab.


    Version 9.34
    Jan C. Depner (PFM Software)
    12/08/15

    - Fixed bug encountered when trying to create feature file.
    - Allowed the user to define a feature without underlying point data (e.g. for LiDAR areas with no data returns).
    - Added Tools menu option to allow user to output features with associated polygons to a Shapefile.
    - Changed to EPSG 4326 definition for Shapefile .prj files.


    Version 9.35
    Jan C. Depner (PFM Software)
    01/25/16

    - Moved displayMessage from pfmView.cpp to an external QDialog class, displayMessage.cpp.


    Version 9.36
    Jan C. Depner (PFM Software)
    02/29/16

    - Added ability to shell Google Earth to view displayed area.
    - Added Google Earth, Firefox, and Internet Explorer to the acknowledgments help text.
    - Happy leap year!


    Version 9.37
    Jan C. Depner (PFM Software)
    03/21/16

    - Made Google Earth work better using NetworkLinks.


    Version 9.38
    Jan C. Depner (PFM Software)
    04/08/16

    - Made the "Highlight multiple coverage" option ignore PFM_DELETED and PFM_REFERENCE data.


    Version 9.39
    Jan C. Depner (PFM Software)
    04/16/16

    - Added the ability to put attribute range limits on the data used to build the on-the-fly (OTF) grid surfaces.  For example, you
      may want to only use CZMIL data where the "CZMIL Classification" is one type (e.g. bare earth), or you may only want to
      use GSF data where the "GSF Heave" is below a certain threshold.


    Version 9.40
    Jan C. Depner (PFM Software)
    05/11/16

    - Moved include of release.hpp from pfmView.hpp to pfmView.cpp so that the program doesn't get rebuilt completely every
      time we build it.


    Version 9.41
    Jan C. Depner (PFM Software)
    07/06/16

    - Turned off the ET_PHONE_HOME stuff in the mk.linux and mk.mingw files.  I left the code in case I need to do
      something similar in the future.


    Version 9.42
    Jan C. Depner (PFM Software)
    07/12/16

    - Added 3 new CZMIL attributes that were added in version 3.0 of the CZMIL API.  These are CZMIL D_index,
      CZMIL D_index_cube, and CZMIL User data.
    - Removed experimental CWF interest point amplitude attribute.  D_index will work better and it comes
      right out of the CPF file so there isn't a huge amount of overhead.


    Version 9.43
    Jan C. Depner (PFM Software)
    07/23/16

    - Now automatically runs CUBE on just the edited subset of the surface if there is only one surface displayed,
      it is a cube surface, and the CUBE program is available.  The option to rerun CUBE on the entire surface is
      still available (but probably pointless).
    - Removed CUBE verbiage from help text when the CUBE program is not available.


    Version 9.44
    Jan C. Depner (PFM Software)
    07/26/16

    - When exporting GeoTIFF image pfmView now checks for Well-known Text (WKT) in the PFM header.  If it is found, it
      is used instead of the default EPSG:4326.  When using CZMIL as input files, some PFMs may use NAD83 instead of
      WGS84 and that should be defined in the PFM header.


    Version 9.45
    Jan C. Depner (PFM Software)
    08/05/16

    - Moved the filter dialog to the center of the main display so you cant't miss it.


    Version 9.46
    Jan C. Depner (PFM Software)
    08/08/16

    - Added ability to change the text font for the entire application and all other ABE map GUI applications.
    - Fixed problem with preferences not opening on the last used tab when a value had been changed previously.
    - To avoid possible confusion, removed translation setup from QSettings in env_in_out.cpp.


    Version 9.47
    Jan C. Depner (PFM Software)
    08/15/16

    - Added ability to draw overlay grid with grid spacing set to either meters (nominal) or minutes.
    - Added ability to set size of feature circles based on millimeters at chart scale or, by default, at a
      fixed size of 12 pixels (6 for child features).
    - Changed "-" to ">" in the OTF attribute range strings so that we could use negative values as part of a range.


    Version 9.48
    Jan C. Depner (PFM Software)
    08/16/16

    - Added Chart toolbar to display width and height of displayed area in meters.
    - Added ability to modify width and/or height in the Chart toolbar to change the displayed area.


    Version 9.49
    Jan C. Depner (PFM Software)
    08/17/16

    - Added chart scale to the Chart toolbar to display the nominal chart scale.
    - Added ability to modify the chart scale in the Chart toolbar to change the displayed area.
    - Moved the Stoplight button to the View toolbar since it was the lone entry left in the color toolbar.
    - Added default width, height, and chart scale to the Preferences dialog.
    - Moved MISP options to the MISP/CUBE tab in the Preferences dialog.
    - Changed the way dragging the area works in the coverage map.  Now you just do Ctrl-left mouse, move the box,
      and when you release the mouse button it redraws at the location of the box.  It is much more intuitive this
      way.


    Version 9.50
    Jan C. Depner (PFM Software)
    08/18/16

    - Unmarks PFM_SELECTED_FEATURE on depth record if the nearest feature point is invalidated (confidence set to 0).


    Version 9.51
    Jan C. Depner (PFM Software)
    08/20/16

    - Added support for Binary Feature Data (BFD) 3.0 feature type field.  Now, features will be marked or unmarked
      in the data based on whether the feature in question is Hydrographic (marked) or Informational (unmarked).
      This is in addition to unmarking for confidence set to 0.


    Version 9.52
    Jan C. Depner (PFM Software)
    08/21/16

    - Now uses PFM_DESIGNATED_SOUNDING for child features.


    Version 9.53
    Jan C. Depner (PFM Software)
    08/23/16

    - Removed the ability to launch the old 2D editor.


    Version 9.54
    Jan C. Depner (PFM Software)
    10/26/16

    - Switched from using the old SAIC CUBE implementation (navo_pfm_cube) to my CUBE implementation (cube_pfm).


    Version 9.55
    Jan C. Depner (PFM Software)
    06/04/17

    - Fixed a possible problem with opening and reading HOF and TOF files.  It probably isn't a problem.


    Version 9.56
    Jan C. Depner (PFM Software)
    06/08/17

    - Fixed labeling of CZMIL user flags.


    Version 9.57
    Jan C. Depner (PFM Software)
    09/18/17

    - A bunch of changes to support doing translations in the future.  There is a generic
      pfmView_xx.ts file that can be run through Qt's "linguist" to translate to another language.
    - Replaced all the hard-coded browser starting code with a call to QDesktopServices.  DOH!


    Version 9.58
    Jan C. Depner (PFM Software)
    04/20/18

    - Added "no invalid"/"no reference" button.
    - Changed "Edit mode" button back to a single button (instead of a menu) and added mode selection
      (rectangle/polygon) to the right-click menu.


    Version 9.59
    Jan C. Depner (PFM Software)
    05/20/18

    - Added ability to define the Google Earth executable if it's not found in your PATH.


    Version 9.60
    Jan C. Depner (PFM Software)
    07/17/18

    - Added LAS 1.4 attributes to the LAS attributes


    Version 10.00
    Jan C. Depner (PFM Software)
    05/18/19

    - Changed to support PFMv7.  This is mostly for the new "num_valid" bin layer, 10 user flags,
      and 20 bin attributes.


    Version 10.01
    Jan C. Depner (PFM Software)
    07/10/19

    - Added support for using PFM_USER_06 for Hockey Puck filtered CZMIL data.


    Version 10.02
    Jan C. Depner (PFM Software)
    07/23/19

    - Added CZMIL Urban noise flags attribute.


    Version 10.03
    Jan C. Depner (PFM Software)
    08/07/19

    - Added support for SHAPE files (via the utility library) for "Zoom To Area File".

</pre>*/
