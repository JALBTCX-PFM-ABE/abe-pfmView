
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



//!  This file contains help documentation for the otfDialog.cpp file.

QString otfSizeText = 
  otfDialog::tr ("This is the on-the-fly (OTF) gridding bin size in meters.  Try to keep it reasonable or it could take a very long time to "
                 "paint.");

QString attrComboText = 
  otfDialog::tr ("Select an attribute to use to limit the data that is used to paint the surface.  For example, you may want to only use "
                 "CZMIL data where the <b>CZMIL Classification</b> is one type (e.g. bare earth), or you may only want to use GSF data "
                 "where the <b>GSF Heave</b> is below a certain threshold.  Note that the on-the-fly (OTF) gridding parameters button "
                 "<img source=\":/icons/set_otf_size.png\"> will be changed to this icon - <img source=\":/icons/set_otf_size_attr.png\"> "
                 "if a limiting attribute is selected.");

QString attrRangeTextText = 
  otfDialog::tr ("This is where you may set the attribute limits for the OTF data surface.  These limits will be used to filter the data prior "
                 "to computing the OTF surface.  The <b>Attribute ranges</b> field will accept a combination of values separated by commas (,) "
                 "and/or right arrows (>).  For example, the text <b>2.0,7.2>26.8,30.0</b> will cause all data having a value (for the selected attribute) "
                 "of 2.0, or in the range 7.2 to 26.8 inclusive, or having a value of 30.0 to be used to compute the OTF surface.  All data outside "
                 "those ranges will be ignored.<br><br>"
                 "<b>IMPORTANT NOOTE: The <i>Min:</i> and <i>Max:</i> fields are populated with the minimum and maximum allowable values for the "
                 "specified attribute.  Be aware that you may ask for ranges that are not in the data for the current displayed area.  In other words, "
                 "you may see nothing after setting the ranges and doing OTF gridding.</b>");

QString acceptOTFText = 
  otfDialog::tr ("Clicking the <b>Accept</b> button causes the program to save all changes made in the pfmView OTF settings dialog.  Note that, if you "
                 "are not actually running OTF gridding the surface will not be redrawn.  The on-the-fly (OTF) gridding parameters button "
                 "<img source=\":/icons/set_otf_size.png\"> may be changed to or from this icon - <img source=\":/icons/set_otf_size_attr.png\"> "
                 "if you have changed the limiting attribute.");

QString closeOTFText = 
  otfDialog::tr ("Clicking this button discards all changes made to the pfmView OTF settings.");
