
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


/*!<  This function draws the coastlines on the coverage map.  We do that here because there is no simple way to do
      it using the built in coastline plotting in nvmap given the way we have to draw the coverage in pfmView.
      This was basically stolen directly from the nvmap function in the utility library.  */

void cov_coasts (MISC *misc, nvMap *cov, NVMAP_DEF *covdef)
{
  static char files[9][12] = {"coasts.dat", "rivers.dat", "bounds.dat", "wvsfull.dat", "wvs250k.dat", "wvs1.dat", "wvs3.dat", "wvs12.dat", "wvs43.dat"};
  void wdbplt(char *file, double slatd, double nlatd, double wlond, double elond, nvMap *map, QColor color, int32_t line_width);


  //  Coastlines - note that we use the old wdbplt for the lower resolutions.

  float xsize = covdef->bounds[covdef->zoom_level].max_x - covdef->bounds[covdef->zoom_level].min_x;
  float ysize = covdef->bounds[covdef->zoom_level].max_y - covdef->bounds[covdef->zoom_level].min_y;


  //  This is the 1:50,000 scale coastline.  It does not have data north of 60N or south of 57S.

  if (check_coast (COAST_50K) && xsize < 5.0 && ysize < 5.0 && covdef->bounds[covdef->zoom_level].max_y < 60.0 &&
      covdef->bounds[covdef->zoom_level].min_y > -56.0)
    {
      int32_t slat = (int32_t) (covdef->bounds[covdef->zoom_level].min_y + 90.0) - 90;
      int32_t nlat = (int32_t) (covdef->bounds[covdef->zoom_level].max_y + 90.0) - 89;
      int32_t wlon = (int32_t) (covdef->bounds[covdef->zoom_level].min_x + 180.0) - 180;
      int32_t elon = (int32_t) (covdef->bounds[covdef->zoom_level].max_x + 180.0) - 179;

      double *coast_x, *coast_y;
      int32_t segCount;

      for (int32_t i = slat ; i <= nlat ; i++)
        {
          for (int32_t j = wlon ; j <= elon ; j++)
            {
              while ((segCount = read_coast (COAST_50K, j, i, &coast_x, &coast_y)))
                {
                  uint8_t in = NVFalse;
                  for (int32_t k = 0 ; k < segCount ; k++)
                    {
                      if (misc->dateline && coast_x[k] < 0.0) coast_x[k] += 360.0;

                      if (cov->checkBounds (coast_x[k], coast_y[k]))
                        {
                          if (k && in) cov->drawLine (coast_x[k - 1], coast_y[k - 1], coast_x[k], coast_y[k], covdef->coast_color,
                                                      covdef->coast_thickness, NVFalse, Qt::SolidLine);
                          in = NVTrue;
                        }
                      else
                        {
                          in = NVFalse;
                        }
                    }

                  free (coast_x);
                  free (coast_y);
                }
            }
        }

      cov->update ();
    }
  else if (check_coast (GSHHS_ALL) && (xsize < 10.0 || ysize < 10.0))
    {
      int32_t slat = (int32_t) (covdef->bounds[covdef->zoom_level].min_y + 90.0) - 90;
      int32_t nlat = (int32_t) (covdef->bounds[covdef->zoom_level].max_y + 90.0) - 89;
      int32_t wlon = (int32_t) (covdef->bounds[covdef->zoom_level].min_x + 180.0) - 180;
      int32_t elon = (int32_t) (covdef->bounds[covdef->zoom_level].max_x + 180.0) - 179;
      double *coast_x, *coast_y;
      int32_t segCount;

      for (int32_t i = slat ; i <= nlat ; i++)
        {
          for (int32_t j = wlon ; j <= elon ; j++)
            {
              while ((segCount = read_coast (GSHHS_ALL, j, i, &coast_x, &coast_y)))
                {
                  uint8_t in = NVFalse;
                  for (int32_t k = 0 ; k < segCount ; k++)
                    {
                      if (misc->dateline && coast_x[k] < 0.0) coast_x[k] += 360.0;

                      if (cov->checkBounds (coast_x[k], coast_y[k]))
                        {
                          if (k && in) cov->drawLine (coast_x[k - 1], coast_y[k - 1], coast_x[k], coast_y[k], covdef->coast_color,
                                                      covdef->coast_thickness, NVFalse, Qt::SolidLine);
                          in = NVTrue;
                        }
                      else
                        {
                          in = NVFalse;
                        }
                    }


                  free (coast_x);
                  free (coast_y);
                }
            }
        }
      cov->update ();
    }
  else if (xsize < 20.0 || ysize < 20.0)
    {
      wdbplt (files[5], covdef->bounds[covdef->zoom_level].min_y, covdef->bounds[covdef->zoom_level].max_y, covdef->bounds[covdef->zoom_level].min_x,
              covdef->bounds[covdef->zoom_level].max_x, cov, covdef->coast_color, covdef->coast_thickness);
    }
  else if (xsize < 50.0 || ysize < 50.0)
    {
      wdbplt (files[6], covdef->bounds[covdef->zoom_level].min_y, covdef->bounds[covdef->zoom_level].max_y, covdef->bounds[covdef->zoom_level].min_x,
              covdef->bounds[covdef->zoom_level].max_x, cov, covdef->coast_color, covdef->coast_thickness);
    }
  else if (xsize < 90.0 || ysize < 90.0)
    {
      wdbplt (files[7], covdef->bounds[covdef->zoom_level].min_y, covdef->bounds[covdef->zoom_level].max_y, covdef->bounds[covdef->zoom_level].min_x,
              covdef->bounds[covdef->zoom_level].max_x, cov, covdef->coast_color, covdef->coast_thickness);
    }
  else
    {
      wdbplt (files[8], covdef->bounds[covdef->zoom_level].min_y, covdef->bounds[covdef->zoom_level].max_y, covdef->bounds[covdef->zoom_level].min_x,
              covdef->bounds[covdef->zoom_level].max_x, cov, covdef->coast_color, covdef->coast_thickness);
    }
}
