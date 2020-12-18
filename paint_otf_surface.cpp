
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


//!  This function paints the surface when using on-the-fly (OTF) binning (actual painting occurs in hatchr.cpp).

void paint_otf_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, uint8_t cov_area_defined)
{
  double          x_bin_size_degrees;
  double          y_bin_size_degrees;
  int32_t         ndx = 0;
  static int32_t  width = 0, height = 0, size = 0;


  void hatchr (nvMap *map, OPTIONS *options, uint8_t clear, NV_F64_XYMBR mbr, NV_F64_XYMBR edit_mbr, float min_z, float max_z, float range,
               double x_bin_size, double y_bin_size, int32_t height, int32_t start_x, int32_t end_x, float ss_null, double cell_size_x,
               double cell_size_y, float *current_row, float *next_row, float *current_attr, uint8_t *current_flags, uint8_t alpha,
               int32_t row, int32_t attribute);
  void geotiff (nvMap *map, MISC *misc, OPTIONS *options);
  void overlayData (nvMap *map, MISC *misc, OPTIONS *options);
  void setScale (float min_z, float max_z, float range, int32_t flip, MISC *misc, OPTIONS *options, uint8_t min_lock, uint8_t max_lock);



  if (!cov_area_defined) return;


  misc->total_displayed_area.min_y = mapdef->bounds[mapdef->zoom_level].min_y;
  misc->total_displayed_area.min_x = mapdef->bounds[mapdef->zoom_level].min_x;
  misc->total_displayed_area.max_y = mapdef->bounds[mapdef->zoom_level].max_y;
  misc->total_displayed_area.max_x = mapdef->bounds[mapdef->zoom_level].max_x;


  if (misc->total_displayed_area.min_x > misc->total_displayed_area.max_x)
    {
      //  Only adjust the lons if the displayed area does not cross the dateline.
            
      if ((misc->total_displayed_area.max_x > 0.0 && misc->total_displayed_area.min_x > 0.0) || 
	  (misc->total_displayed_area.max_x < 0.0 && misc->total_displayed_area.min_x < 0.0))
	{
	  double temp = misc->total_displayed_area.min_x;
	  misc->total_displayed_area.min_x = misc->total_displayed_area.max_x;
	  misc->total_displayed_area.max_x = temp;
	}
    }


  //  Save the displayed area for other ABE programs.  Some key off of the editor/viewer area (displayed_area)
  //  and some key off of just the viewer area (viewer_displayed_area) so, at this point, we set them
  //  both.

  misc->abe_share->displayed_area = misc->abe_share->viewer_displayed_area = misc->total_displayed_area;


  //  Compute center point for sun angle arrow and bin size computation

  double center_map_x = misc->total_displayed_area.min_x + (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / 2.0;
  double center_map_y = misc->total_displayed_area.min_y + (misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / 2.0;
  double dum_map_z;
  int32_t dum_z;
  map->map_to_screen (1, &center_map_x, &center_map_y, &dum_map_z, &misc->center_point.x, &misc->center_point.y, &dum_z);


  //  Compute the bin size in degrees at the center of the displayed area.

  double new_x, new_y;
  NV_F64_XYMBR mbr;
  newgp (center_map_y, center_map_x, 0.0, options->otf_bin_size_meters, &new_y, &new_x);
  y_bin_size_degrees = misc->abe_share->otf_y_bin_size = new_y - center_map_y;
  newgp (center_map_y, center_map_x, 90.0, options->otf_bin_size_meters, &new_y, &new_x);
  x_bin_size_degrees = misc->abe_share->otf_x_bin_size = new_x - center_map_x;


  //  Fill the background with the background color so we don't have to paint the background in hatchr.
  //  Also so we can display the GeoTIFF under the data if requested.

  //  Whole area

  if (misc->clear)
    {
      //  If we asked to clear then we've probably changed the area so we want to free and allocate the grid.

      if (misc->otf_grid != NULL)
        {
          misc->otfShare->detach ();
          misc->otf_grid = NULL;
          misc->abe_share->otf_width = misc->abe_share->otf_height = 0;
        }

      mbr = misc->total_displayed_area;
    }


  //  Partial area (edited area)

  else
    {
      mbr = misc->abe_share->edit_area;
    }

  map->fillRectangle (mbr.min_x, mbr.min_y, mbr.max_x, mbr.max_y, options->background_color, NVFalse);


  float null_depth;
  if (options->layer_type == MIN_FILTERED_DEPTH || options->layer_type == MIN_DEPTH)
    {
      null_depth = misc->abe_share->otf_null_value = CHRTRNULL;
    }
  else
    {
      null_depth = misc->abe_share->otf_null_value = -CHRTRNULL;
    }


  //  Allocate the otf grid if needed.

  if (misc->otf_grid == NULL)
    {
      //  Figure out how many bins we'll need.

      width = misc->abe_share->otf_width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees) + 1;
      height = misc->abe_share->otf_height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees) + 1;

      size = width * height * sizeof (OTF_GRID_RECORD);

      QString skey;
      skey.sprintf ("%d_abe_otf_grid", misc->process_id);

      misc->otfShare = new QSharedMemory (skey);

      if (!misc->otfShare->create (size, QSharedMemory::ReadWrite)) misc->otfShare->attach (QSharedMemory::ReadWrite);

      misc->otf_grid = (OTF_GRID_RECORD *) misc->otfShare->data ();

      misc->otfShare->lock ();


      //  Clear the memory block.

      memset (misc->otf_grid, 0, size);

      for (int32_t i = 0 ; i < height ; i++)
        {
          for (int32_t j = 0 ; j < width ; j++)
            {
              misc->otf_grid[i * width + j].min = CHRTRNULL;
              misc->otf_grid[i * width + j].max = -CHRTRNULL;
              misc->otf_grid[i * width + j].cnt = OTF_GRID_MAX;
            }
        }


      misc->displayed_area_min = CHRTRNULL;
      misc->displayed_area_max = -CHRTRNULL;
    }


  //  If we're just redrawing after an edit we need to clear the edited area.  To do this we check to see if the OTF bin is in
  //  the displayed area.  If it is, we set the bin to null depth so that it will get replaced prior to drawing.

  if (!misc->clear)
    {
      for (int32_t i = 0 ; i < height ; i++)
        {
          double bin_lat = misc->total_displayed_area.min_y + (double) i * y_bin_size_degrees;
          for (int32_t j = 0 ; j < width ; j++)
            {
              double bin_lon = misc->total_displayed_area.min_x + (double) j * x_bin_size_degrees;
              if (bin_lat >= misc->abe_share->edit_area.min_y && bin_lat < misc->abe_share->edit_area.max_y &&
                  bin_lon >= misc->abe_share->edit_area.min_x && bin_lon < misc->abe_share->edit_area.max_x)
                {
                  misc->otf_grid[i * width + j].cnt = OTF_GRID_MAX;
                  misc->otf_grid[i * width + j].min = CHRTRNULL;
                  misc->otf_grid[i * width + j].max = -CHRTRNULL;
                  misc->otf_grid[i * width + j].avg = 0.0;
                  misc->otf_grid[i * width + j].std = 0.0;
                }
            }
        }
    }


  //  Set up our attribute limits if needed.

  float *amin = NULL, *amax = NULL;
  int32_t range_count = 0;

  if (misc->otf_attr >= 0)
    {
      QString rangeText = options->attrFilterText[misc->otf_attr];


      //  Split the string into sections divided by commas.

      QStringList ranges = rangeText.split (",", QString::SkipEmptyParts);


      range_count = ranges.size ();


      //  Allocate the memory for the filter ranges.

      amin = (float *) calloc (range_count, sizeof (float));
      if (amin == NULL)
        {
          perror ("Allocating amin memory in paint_otf_surface");
          exit (-1);
        }


      amax = (float *) calloc (range_count, sizeof (float));
      if (amax == NULL)
        {
          perror ("Allocating amax memory in paint_otf_surface");
          exit (-1);
        }


      for (int32_t i = 0 ; i < range_count ; i++)
        {
          if (ranges.at (i).contains ('-'))
            {
              QString aminString = ranges.at (i).section ('-', 0, 0);
              QString amaxString = ranges.at (i).section ('-', 1, 1);

              amin[i] = aminString.toFloat ();
              amax[i] = amaxString.toFloat ();
            }
          else
            {
              amin[i] = amax[i] = ranges.at (i).toFloat ();
            }


          //  We have to switch back from 1 based numbers if the attribute is the CZMIL channel number.

          if (QString (misc->abe_share->open_args[0].head.ndx_attr_name[misc->otf_attr]).contains ("CZMIL Channel number"))
            {
              amin[i] -= 1.0;
              amax[i] -= 1.0;
            }
        }
    }


  uint8_t min_lock = NVFalse, max_lock = NVFalse;
  int32_t attribute = 0;

  for (int32_t pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
          adjust_bounds (misc, pfm);


          //  Redraw everything

          if (misc->clear)
            {
              misc->hatchr_start_x = 0;
              misc->hatchr_end_x = misc->displayed_area_width[pfm];
              misc->hatchr_start_y = 0;
              misc->hatchr_end_y = misc->displayed_area_height[pfm];
            }


          //  Redraw a portion (what we edited)

          else
            {
              NV_F64_XYMBR tmp_mbr = misc->abe_share->edit_area;


              //  Adjust edit bounds to nearest grid point (with a border of 1 grid point to clean up the edges).

              tmp_mbr.min_y = misc->abe_share->open_args[pfm].head.mbr.min_y +
                (NINT ((misc->abe_share->edit_area.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
                       misc->abe_share->open_args[pfm].head.y_bin_size_degrees) - 1) *
                misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
              tmp_mbr.max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
                (NINT ((misc->abe_share->edit_area.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
                       misc->abe_share->open_args[pfm].head.y_bin_size_degrees) + 1) * 
                misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
              tmp_mbr.min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
                (NINT ((misc->abe_share->edit_area.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
                       misc->abe_share->open_args[pfm].head.x_bin_size_degrees) - 1) *
                misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
              tmp_mbr.max_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
                (NINT ((misc->abe_share->edit_area.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
                       misc->abe_share->open_args[pfm].head.x_bin_size_degrees) + 1) *
                misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


              tmp_mbr.min_y = qMax (tmp_mbr.min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
              tmp_mbr.max_y = qMin (tmp_mbr.max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
              tmp_mbr.min_x = qMax (tmp_mbr.min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
              tmp_mbr.max_x = qMin (tmp_mbr.max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);



              misc->hatchr_start_x = NINT ((tmp_mbr.min_x - misc->displayed_area[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
              misc->hatchr_end_x = misc->hatchr_start_x + (NINT ((tmp_mbr.max_x - tmp_mbr.min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees));

              misc->hatchr_start_y = NINT ((tmp_mbr.min_y - misc->displayed_area[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
              misc->hatchr_end_y = misc->hatchr_start_y + (NINT ((tmp_mbr.max_y - tmp_mbr.min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees));
            }


          //  If the width or height is 0 or negative we have asked for an area outside of the PFM's MBR so we don't want to do anything.

          if (misc->displayed_area_width[pfm] > 0 && misc->displayed_area_height[pfm] > 0)
            {
              //  We only want to do attributes if we're coloring by number, stddev, avg - min, max - avg, or max - min.

              if (misc->color_index > 0 && misc->color_index < PRE_ATTR) attribute = misc->color_index;

              misc->current_row = (float *) calloc (misc->abe_share->otf_width, sizeof (float));
              if (misc->current_row == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - current_row - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                  exit (-1);
                }

              misc->next_row = (float *) calloc (misc->abe_share->otf_width, sizeof (float));
              if (misc->next_row == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - next_row - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                  exit (-1);
                }

              if (attribute)
                {
                  misc->current_attr = (float *) calloc (misc->abe_share->otf_width, sizeof (float));
                  if (misc->current_attr == NULL)
                    {
                      fprintf (stderr, "%s %s %s %d - current_attr - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                      exit (-1);
                    }
                }


              misc->statusProg->setRange (misc->hatchr_start_y, misc->hatchr_end_y);
              QString title = pfmView::tr (" Reading %1 of %2 : ").arg (misc->abe_share->pfm_count - pfm).arg (misc->abe_share->pfm_count) +
                QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName () + " ";
              misc->statusProgLabel->setText (title);
              misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
              misc->statusProgLabel->setPalette (misc->statusProgPalette);
              misc->statusProg->setTextVisible (true);
              qApp->processEvents();


              //  We only want to update the progress bar at about 20% increments.  This makes things go marginally faster.

              int32_t prog_inc = (misc->hatchr_end_y - misc->hatchr_start_y) / 5;
              if (!prog_inc) prog_inc = 1;


              //  Read input data row by row.  Note that hatchr_start_y and hatchr_end_y may not be the same as the
              //  entire displayed area since we may only be redrawing a small edited portion of the display.

              NV_I32_COORD2 coord;
              DEPTH_RECORD *depth;
              int32_t numrecs;
              for (int32_t jj = misc->hatchr_start_y ; jj <= misc->hatchr_end_y ; jj++)
                {
                  if (!((jj - misc->hatchr_start_y) % prog_inc))
                    {
                      misc->statusProg->setValue (jj);
                      qApp->processEvents();
                    }


                  coord.y = misc->displayed_area_row[pfm] + jj;
                  for (int32_t kk = misc->hatchr_start_x ; kk <= misc->hatchr_end_x ; kk++)
                    {
                      coord.x = misc->displayed_area_column[pfm] + kk;
                      if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &numrecs))
                        {
                          for (int32_t mm = 0 ; mm < numrecs ; mm++)
                            {
                              //  Compute the bin

                              int32_t bin_x = NINT ((depth[mm].xyz.x - misc->total_displayed_area.min_x) / x_bin_size_degrees);
                              int32_t bin_y = NINT ((depth[mm].xyz.y - misc->total_displayed_area.min_y) / y_bin_size_degrees);


                              //  Make sure the point is within our otf grid.

                              if (bin_x >= 0 && bin_x < width - 1 && bin_y >= 0 && bin_y < height - 1)
                                {
                                  ndx = bin_y * width + bin_x;


                                  if (options->layer_type == MIN_FILTERED_DEPTH || options->layer_type == MAX_FILTERED_DEPTH ||
                                      options->layer_type == AVERAGE_FILTERED_DEPTH)
                                    {
                                      if (!(depth[mm].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                                        {
                                          //  Check for attribute limits if needed (using range_count because it is faster to check for non-zero).

                                          uint8_t show_it = NVTrue;

                                          if (range_count)
                                            {
                                              for (int32_t rng = 0 ; rng < range_count ; rng++)
                                                {
                                                  if (depth[mm].attr[misc->otf_attr] > amax[rng] || depth[mm].attr[misc->otf_attr] < amin[rng])
                                                    {
                                                      show_it = NVFalse;
                                                      break;
                                                    }
                                                }
                                            }


                                          if (show_it)                                                  
                                            {
                                              //  Using avg as sum and std as sum of squares until all bins are loaded.

                                              misc->otf_grid[ndx].avg += depth[mm].xyz.z;
                                              misc->otf_grid[ndx].std += (depth[mm].xyz.z * depth[mm].xyz.z);
                                              misc->otf_grid[ndx].cnt++;


                                              if (misc->otf_grid[ndx].min > depth[mm].xyz.z) 
                                                {
                                                  misc->otf_grid[ndx].min = depth[mm].xyz.z;
                                                  misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                                  misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                                }

                                              if (misc->otf_grid[ndx].max < depth[mm].xyz.z)
                                                {
                                                  misc->otf_grid[ndx].max = depth[mm].xyz.z;
                                                  misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                                  misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                                }
                                            }
                                        }
                                    }
                                  else
                                    {
                                      if (!(depth[mm].validity & (PFM_DELETED | PFM_REFERENCE)))
                                        {
                                          //  Check for attribute limits if needed (using range_count because it is faster to check for non-zero).

                                          uint8_t show_it = NVTrue;

                                          if (range_count)
                                            {
                                              for (int32_t rng = 0 ; rng < range_count ; rng++)
                                                {
                                                  if (depth[mm].attr[misc->otf_attr] > amax[rng] || depth[mm].attr[misc->otf_attr] < amin[rng])
                                                    {
                                                      show_it = NVFalse;
                                                      break;
                                                    }
                                                }
                                            }


                                          if (show_it)                                                  
                                            {
                                              //  Using avg as sum and std as sum of squares until all bins are loaded.

                                              misc->otf_grid[ndx].avg += depth[mm].xyz.z;
                                              misc->otf_grid[ndx].std += (depth[mm].xyz.z * depth[mm].xyz.z);
                                              misc->otf_grid[ndx].cnt++;


                                              if (misc->otf_grid[ndx].min > depth[mm].xyz.z) 
                                                {
                                                  misc->otf_grid[ndx].min = depth[mm].xyz.z;
                                                  misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                                  misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                                }

                                              if (misc->otf_grid[ndx].max < depth[mm].xyz.z)
                                                {
                                                  misc->otf_grid[ndx].max = depth[mm].xyz.z;
                                                  misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                                  misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                          free (depth);
                        }
                    }


                  if (qApp->hasPendingEvents ())
                    {
                      qApp->processEvents();
                      if (misc->drawing_canceled) break;
                    }
                }
            }
        }

      if (misc->drawing_canceled) break;
    }


  //  Free the attribute ranges if needed.

  if (range_count)
    {
      free (amin);
      free (amax);
    }


  //  Compute the average and standard deviation.

  for (int32_t i = 0 ; i < misc->abe_share->otf_height ; i++)
    {
      for (int32_t j = 0 ; j < misc->abe_share->otf_width ; j++)
        {
          ndx = i * misc->abe_share->otf_width + j;


          //  Only compute average and standard deviation if we have valid points.

          if (misc->otf_grid[ndx].cnt >= OTF_GRID_MAX)
            {
              misc->otf_grid[ndx].cnt -= OTF_GRID_MAX;

              if (misc->otf_grid[ndx].cnt)
                {
                  misc->otf_grid[ndx].avg = misc->otf_grid[ndx].avg / (double) misc->otf_grid[ndx].cnt;

                  if (misc->otf_grid[ndx].cnt > 1)
                    {
                      double variance = ((misc->otf_grid[ndx].std -
                                              ((double) misc->otf_grid[ndx].cnt * (misc->otf_grid[ndx].avg * misc->otf_grid[ndx].avg))) /
                                             ((double) misc->otf_grid[ndx].cnt - 1.0));

                      if (variance >= 0)
                        {
                          misc->otf_grid[ndx].std = sqrt (variance);
                        }
                      else
                        {
                          misc->otf_grid[ndx].std = 0.0;
                        }
                    }
                  else
                    {
                      misc->otf_grid[ndx].std = 0.0;
                    }
                }
              else
                {
                  misc->otf_grid[ndx].avg = null_depth;
                  misc->otf_grid[ndx].std = 0.0;
                }
            }
        }
    }


  if (attribute)
    {
      //  Compute the min and max attributes.

      for (int32_t i = 0 ; i < misc->abe_share->otf_height ; i++)
        {
          for (int32_t j = 0 ; j < misc->abe_share->otf_width ; j++)
            {
              ndx = i * misc->abe_share->otf_width + j;


              //  Only compute min and max if we have valid points.

              if (misc->otf_grid[ndx].cnt)
                {
                  switch (attribute)
                    {
                    case 1:
                      misc->displayed_area_attr_min = qMin ((float) misc->otf_grid[ndx].cnt, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax ((float) misc->otf_grid[ndx].cnt, misc->displayed_area_attr_max);
                      break;

                    case 2:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].std, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].std, misc->displayed_area_attr_max);
                      break;

                    case 3:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].avg - misc->otf_grid[ndx].min, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].avg - misc->otf_grid[ndx].min, misc->displayed_area_attr_max);
                      break;

                    case 4:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].max - misc->otf_grid[ndx].avg, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].max - misc->otf_grid[ndx].avg, misc->displayed_area_attr_max);
                      break;

                    case 5:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].max - misc->otf_grid[ndx].min, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].max - misc->otf_grid[ndx].min, misc->displayed_area_attr_max);
                      break;
                    }
                }
            }
        }

      misc->color_min = misc->displayed_area_attr_min;
      misc->color_max = misc->displayed_area_attr_max;
    }
  else
    {
      misc->color_min = misc->displayed_area_min;
      misc->color_max = misc->displayed_area_max;
    }


  if (options->min_hsv_locked[attribute])
    {
      if (misc->color_min < options->min_hsv_value[attribute])
        {
          misc->color_min = options->min_hsv_value[attribute];
          min_lock = NVTrue;
        }
    }

  if (options->max_hsv_locked[attribute])
    {
      if (misc->color_max > options->max_hsv_value[attribute])
        {
          misc->color_max = options->max_hsv_value[attribute];
          max_lock = NVTrue;
        }
    }


  misc->color_range = misc->color_max - misc->color_min;
  if (misc->color_range == 0.0) misc->color_range = 1.0;


  misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
  if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;

  if (attribute)
    {
      misc->displayed_area_attr_range = misc->displayed_area_attr_max - misc->displayed_area_attr_min;
      if (misc->displayed_area_attr_range == 0.0) misc->displayed_area_attr_range = 1.0;
    }



  misc->color_range = misc->color_max - misc->color_min;

  misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
  if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (false);
  qApp->processEvents ();


  if (!misc->drawing_canceled)
    {
      //  Set the scale colors for the current range.

      if (attribute)
        {
          setScale (misc->displayed_area_attr_min, misc->displayed_area_attr_max, misc->displayed_area_attr_range, attribute, misc, options, min_lock,
                    max_lock);
        }
      else
        {
          setScale (misc->displayed_area_min, misc->displayed_area_max, misc->displayed_area_range, attribute, misc, options, min_lock, max_lock);
        }


      //  Set these for pfm3D.

      misc->abe_share->otf_min_z = misc->displayed_area_min;
      misc->abe_share->otf_max_z = misc->displayed_area_max;


      //  Redraw everything

      if (misc->clear)
        {
          misc->hatchr_start_x = 0;
          misc->hatchr_end_x = width;
          misc->hatchr_start_y = 0;
          misc->hatchr_end_y = height;
        }


      //  Redraw a portion (what we edited)

      else
        {
          NV_F64_XYMBR tmp_mbr = misc->abe_share->edit_area;


          //  Adjust edit bounds to nearest grid point (with a border of 1 grid point to clean up the edges).

          tmp_mbr.min_y = misc->total_displayed_area.min_y + (NINT ((misc->abe_share->edit_area.min_y - misc->total_displayed_area.min_y) /
                                                                    y_bin_size_degrees) - 1) * y_bin_size_degrees;
          tmp_mbr.max_y = misc->total_displayed_area.min_y + (NINT ((misc->abe_share->edit_area.max_y - misc->total_displayed_area.min_y) /
                                                                    y_bin_size_degrees) + 1) * y_bin_size_degrees;
          tmp_mbr.min_x = misc->total_displayed_area.min_x + (NINT ((misc->abe_share->edit_area.min_x - misc->total_displayed_area.min_x) /
                                                                    x_bin_size_degrees) - 1) * x_bin_size_degrees;
          tmp_mbr.max_x = misc->total_displayed_area.min_x + (NINT ((misc->abe_share->edit_area.max_x - misc->total_displayed_area.min_x) /
                                                                    x_bin_size_degrees) + 1) * x_bin_size_degrees;


          tmp_mbr.min_y = qMax (tmp_mbr.min_y, misc->total_displayed_area.min_y);
          tmp_mbr.max_y = qMin (tmp_mbr.max_y, misc->total_displayed_area.max_y);
          tmp_mbr.min_x = qMax (tmp_mbr.min_x, misc->total_displayed_area.min_x);
          tmp_mbr.max_x = qMin (tmp_mbr.max_x, misc->total_displayed_area.max_x);



          misc->hatchr_start_x = NINT ((tmp_mbr.min_x - misc->total_displayed_area.min_x) / x_bin_size_degrees);
          misc->hatchr_end_x = misc->hatchr_start_x + (NINT ((tmp_mbr.max_x - tmp_mbr.min_x) / x_bin_size_degrees));

          misc->hatchr_start_y = NINT ((tmp_mbr.min_y - misc->total_displayed_area.min_y) / y_bin_size_degrees);
          misc->hatchr_end_y = misc->hatchr_start_y + (NINT ((tmp_mbr.max_y - tmp_mbr.min_y) / y_bin_size_degrees));
        }

      misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
      if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;

      misc->statusProg->setRange (misc->hatchr_start_y, misc->hatchr_end_y);
      misc->statusProgLabel->setText (pfmView::tr (" Painting surface "));
      misc->statusProg->setTextVisible (true);
      qApp->processEvents();


      //  We only want to update the progress bar at about 20% increments.  This makes things go marginally faster.

      int32_t prog_inc = height / 5;
      if (!prog_inc) prog_inc = 1;


      for (int32_t jj = misc->hatchr_start_y ; jj < misc->hatchr_end_y ; jj++)
        {
          if (!((jj - misc->hatchr_start_y) % prog_inc))
            {
              misc->statusProg->setValue (jj);
              qApp->processEvents();
            }

          if (attribute)
            {
              for (int32_t kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (attribute)
                    {
                    case 1:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].cnt;
                      break;

                    case 2:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].std;
                      break;

                    case 3:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].avg - misc->otf_grid[jj * width + kk].min;
                      break;

                    case 4:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].max - misc->otf_grid[jj * width + kk].avg;
                      break;

                    case 5:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].max - misc->otf_grid[jj * width + kk].min;
                      break;
                    }
                }
            }

          if (jj == misc->hatchr_end_y - 1)
            {
              for (int32_t kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (options->layer_type)
                    {
                    case MIN_FILTERED_DEPTH:
                    case MIN_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].min;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].min;
                      break;

                    case MAX_FILTERED_DEPTH:
                    case MAX_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].max;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].max;
                      break;

                    case AVERAGE_FILTERED_DEPTH:
                    case AVERAGE_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      break;
                    }
                }
            }
          else
            {
              for (int32_t kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (options->layer_type)
                    {
                    case MIN_FILTERED_DEPTH:
                    case MIN_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].min;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].min;
                      break;

                    case MAX_FILTERED_DEPTH:
                    case MAX_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].max;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].max;
                      break;

                    case AVERAGE_FILTERED_DEPTH:
                    case AVERAGE_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].avg;
                      break;
                    }
                }
            }


          //  HSV fill and sunshade.

          hatchr (map, options, misc->clear, misc->total_displayed_area, misc->abe_share->edit_area, misc->color_min, misc->color_max,
                  misc->color_range, x_bin_size_degrees, y_bin_size_degrees, misc->abe_share->otf_height, misc->hatchr_start_x, misc->hatchr_end_x,
                  null_depth, options->otf_bin_size_meters, options->otf_bin_size_meters, misc->current_row, misc->next_row, misc->current_attr,
                  NULL, misc->pfm_alpha[0], jj, attribute);


          if (qApp->hasPendingEvents ())
            {
              qApp->processEvents();
              if (misc->drawing_canceled) break;
            }
        }


      //  Flush all of the saved fillRectangles from hatchr to screen

      map->flush ();
    }


  free (misc->current_row);
  free (misc->next_row);
  if (attribute) free (misc->current_attr);

  misc->otfShare->unlock ();


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (false);
  qApp->processEvents();


  if (!misc->drawing_canceled)
    {
      //  Overlay any files that have been requested.

      overlayData (map, misc, options);


      //  Display suspect, feature, selected, or reference info if enabled.

      if (options->display_suspect || options->display_feature || options->display_selected || options->display_reference)
        overlayFlag (map, options, misc, NVTrue, NVTrue, NVTrue);


      //  Display filter masks if any are defined.

      for (int32_t i = 0 ; i < misc->poly_filter_mask_count ; i++)
        {
          if (!misc->poly_filter_mask[i].displayed)
            {
              //  Check to see if this mask (that we are going to redraw) overlaps another (that we weren't planning on redrawing).
              //  If so, set the flag to redraw the other one.

              for (int32_t j = 0 ; j < misc->poly_filter_mask_count ; j++)
                {
                  if (j != i && misc->poly_filter_mask[j].displayed)
                    {
                      if (polygon_collision2 (misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y, misc->poly_filter_mask[i].count,
                                              misc->poly_filter_mask[j].x, misc->poly_filter_mask[j].y, misc->poly_filter_mask[j].count))
                        misc->poly_filter_mask[j].displayed = NVFalse;
                    }
                }
            }
        }


      for (int32_t i = 0 ; i < misc->poly_filter_mask_count ; i++)
        {
          if (!misc->poly_filter_mask[i].displayed)
            {
              if (options->poly_filter_mask_color.alpha () < 255)
                {
                  map->fillPolygon (misc->poly_filter_mask[i].count, misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y, 
                                    options->poly_filter_mask_color, NVTrue);
                }
              else
                {
                  //  We don't have to worry about clipping this because moving the area discards the mask areas.

                  map->drawPolygon (misc->poly_filter_mask[i].count, misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y,
                                    options->poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
                }
              misc->poly_filter_mask[i].displayed = NVTrue;
            }
        }


      //  Flag minimum, maximum, and maximum std dev if enabled.

      if (options->display_minmax) displayMinMax (map, options, misc);
    }
}
