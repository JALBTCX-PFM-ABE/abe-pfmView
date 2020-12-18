
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



/*!
  This function places markers on the minimum, maximum, and max standard deviation value in the displayed area.
  The actual values used are computed in compute_layer_min_max.cpp which is called from paint_surface.cpp.
*/

void displayMinMax (nvMap *map, OPTIONS *options, MISC *misc)
{
  double              x[3], y[3], dz = 0.0;
  int32_t             cx, cy, cz, px[4], py[4];


  int32_t min_pfm = misc->displayed_valid_min_pfm;
  int32_t max_pfm = misc->displayed_valid_max_pfm;
  int32_t std_pfm = misc->displayed_valid_std_pfm;

  NV_I32_COORD2 min_coord = misc->displayed_valid_min_coord;
  NV_I32_COORD2 max_coord = misc->displayed_valid_max_coord;
  NV_I32_COORD2 std_coord = misc->displayed_valid_std_coord;


  //  Minimum value.

  y[0] = misc->displayed_area[min_pfm].min_y + ((double) (min_coord.y - misc->displayed_area_row[min_pfm]) + 0.5) * 
    misc->abe_share->open_args[min_pfm].head.y_bin_size_degrees;
  x[0] = misc->displayed_area[min_pfm].min_x + ((double) (min_coord.x - misc->displayed_area_column[min_pfm])+ 0.5) * 
    misc->abe_share->open_args[min_pfm].head.x_bin_size_degrees;


  //  Maximum value.

  y[1] = misc->displayed_area[max_pfm].min_y + ((double) (max_coord.y - misc->displayed_area_row[max_pfm]) + 0.5) * 
    misc->abe_share->open_args[max_pfm].head.y_bin_size_degrees;
  x[1] = misc->displayed_area[max_pfm].min_x + ((double) (max_coord.x - misc->displayed_area_column[max_pfm]) + 0.5) * 
    misc->abe_share->open_args[max_pfm].head.x_bin_size_degrees;


  //  Maximum standard deviation.

  y[2] = misc->displayed_area[std_pfm].min_y + ((double) (std_coord.y - misc->displayed_area_row[std_pfm]) + 0.5) * 
    misc->abe_share->open_args[std_pfm].head.y_bin_size_degrees;
  x[2] = misc->displayed_area[std_pfm].min_x + ((double) (std_coord.x - misc->displayed_area_column[std_pfm]) + 0.5) * 
    misc->abe_share->open_args[std_pfm].head.x_bin_size_degrees;


  map->map_to_screen (1, &x[0], &y[0], &dz, &cx, &cy, &cz);
  map->fillCircle (cx, cy, 14, 0.0, 360.0, options->contour_color, NVFalse);

  map->map_to_screen (1, &x[1], &y[1], &dz, &cx, &cy, &cz);
  map->fillRectangle (cx - 7, cy - 7, 15, 15, options->contour_color, NVFalse);

  map->map_to_screen (1, &x[2], &y[2], &dz, &cx, &cy, &cz);
  px[0] = cx - 8;
  py[0] = cy;
  px[1] = cx;
  py[1] = cy - 8;
  px[2] = cx + 8;
  py[2] = cy;
  px[3] = cx;
  py[3] = cy + 8;
  map->fillPolygon (4, px, py, options->contour_color, NVTrue);
}
