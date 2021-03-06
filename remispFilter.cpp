
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
#include "misp.h"


#define         MISP_EPS    1.0e-3         /* epsilon criteria for finding winner */
#define         FILTER 9


/*!
  This is very similar to the remisp function except that it is used to filter an area where the user has hand
  drawn contours.  This is used for certain applications to smooth out very flat surfaces (usually in deep water)
  so that they are esthetically more pleasing to the eye (a terrible idea if you ask me).
*/

remispFilter::remispFilter (MISC *mi, OPTIONS *op, double *x, double *y, int32_t pc)
{
  std::vector<NV_F64_COORD3> xyz_array;
  int32_t            out_count = 0, misp_weight;
  NV_I32_COORD2      coord;
  BIN_RECORD         bin;
  DEPTH_RECORD       *depth;
  int32_t            recnum;
  NV_F64_COORD3      xyz;
  NV_F64_XYMBR       grid_mbr[MAX_ABE_PFMS];
  int32_t            gridcols[MAX_ABE_PFMS], gridrows[MAX_ABE_PFMS], col_filter[MAX_ABE_PFMS], row_filter[MAX_ABE_PFMS];
  double             half_x[MAX_ABE_PFMS], half_y[MAX_ABE_PFMS];
  uint8_t            land_mask_flag = NVFalse;
  QString            progText;
  gridThread         grid_thread;



  options = op;
  misc = mi;
  mx = x;
  my = y;
  poly_count = pc;
  complete = NVFalse;


  //  Figure out the width, start row, start column, end row, and end column

  NV_F64_XYMBR mbr;

  mbr.min_x = 999999.0;
  mbr.min_y = 999999.0;
  mbr.max_x = -999999.0;
  mbr.max_y = -999999.0;
  for (int32_t i = 0 ; i < poly_count ; i++)
    {
      if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;

      mbr.min_x = qMin (mbr.min_x, mx[i]);
      mbr.min_y = qMin (mbr.min_y, my[i]);
      mbr.max_x = qMax (mbr.max_x, mx[i]);
      mbr.max_y = qMax (mbr.max_y, my[i]);
    }


  misp_weight = options->misp_weight;


  //  First we retrieve all valid data points from all available PFM layers (only those that lie outside of our polygon)

  for (int32_t pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      land_mask_flag = NVFalse;

      if (misc->abe_share->display_pfm[pfm] && misc->average_type[pfm])
	{
          //  Check for the land mask flag in PFM_USER_10 in any of the PFM layers.

          if (!strcmp (misc->abe_share->open_args[pfm].head.user_flag_name[9], "Land masked point")) land_mask_flag = NVTrue;


	  //  Adjust the MBR to the nearest grid point.  Note that part or all of the MBR may be outside
	  //  of the PFM bounds if we are handling more than one layer.

	  grid_mbr[pfm].min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
	    (NINT ((mbr.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) / 
		   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

	  grid_mbr[pfm].max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
	    (NINT ((mbr.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
		   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

	  grid_mbr[pfm].min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
	    (NINT ((mbr.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
		   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

	  grid_mbr[pfm].max_x = misc->abe_share->open_args[pfm].head.mbr.min_x +
	    (NINT ((mbr.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
		   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


	  //  Add the filter border to the MBR

	  grid_mbr[pfm].min_x -= ((double) FILTER * misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  grid_mbr[pfm].min_y -= ((double) FILTER * misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
	  grid_mbr[pfm].max_x += ((double) FILTER * misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  grid_mbr[pfm].max_y += ((double) FILTER * misc->abe_share->open_args[pfm].head.y_bin_size_degrees);


	  //  Number of rows and columns in the area

	  gridcols[pfm] = NINT ((grid_mbr[pfm].max_x - grid_mbr[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  gridrows[pfm] = NINT ((grid_mbr[pfm].max_y - grid_mbr[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees);

	  row_filter[pfm] = gridrows[pfm] - FILTER;
	  col_filter[pfm] = gridcols[pfm] - FILTER;



          misc->statusProg->setRange (0, gridrows[pfm]);
	  progText = pfmView::tr (" Reading PFM data from %1 ").arg (QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm"));
          misc->statusProgLabel->setText (progText);
          misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
          misc->statusProgLabel->setPalette (misc->statusProgPalette);
          misc->statusProg->setTextVisible (true);
          qApp->processEvents();


	  //  Get the data that falls in the MBR (but outside our polygon) from the current PFM and save it to memory.

	  half_x[pfm] = misc->abe_share->open_args[pfm].head.x_bin_size_degrees * 0.5;
	  half_y[pfm] = misc->abe_share->open_args[pfm].head.y_bin_size_degrees * 0.5;

	  for (int32_t i = 0 ; i < gridrows[pfm] ; i++)
	    {
	      //  Compute the latitude of the center of the bin.

	      NV_F64_COORD2 xy;
	      xy.y = grid_mbr[pfm].min_y + i * misc->abe_share->open_args[pfm].head.y_bin_size_degrees + half_y[pfm];

	      misc->statusProg->setValue (i);
	      qApp->processEvents();

	      for (int32_t j = 0 ; j < gridcols[pfm] ; j++)
		{
		  //  Compute the longitude of the center of the bin.

		  xy.x = grid_mbr[pfm].min_x + j * misc->abe_share->open_args[pfm].head.x_bin_size_degrees + half_x[pfm];


		  //  Get the PFM coordinates of the center of the bin

		  compute_index_ptr (xy, &coord, &misc->abe_share->open_args[pfm].head);


		  //  Make sure that we are inside of the PFM

		  if (coord.x >= 0 && coord.x < misc->abe_share->open_args[pfm].head.bin_width && coord.y >= 0 && coord.y < misc->abe_share->open_args[pfm].head.bin_height)
		    {
		      read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


		      //  If we have soundings in the bin, go get them

		      if (bin.num_soundings)
			{
			  if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
			    {
			      uint8_t found = NVFalse;
			      for (int32_t k = 0 ; k < recnum ; k++)
				{
				  switch (misc->average_type[pfm])
				    {
				      //  Use every point (this is normally the case).

				    case 0:
				    case 2:
				    case 5:
				      if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)) && 
                                          depth[k].xyz.z < misc->abe_share->open_args[pfm].head.null_depth && 
                                          !inside_polygon2 (mx, my, poly_count, depth[k].xyz.x, depth[k].xyz.y))
                                        add_point (&xyz_array, depth[k].xyz, &out_count);
				      break;


				      //  Use the minimum value in the bin.

				    case 1:
				    case 4:
				      if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                                          fabs (depth[k].xyz.z - bin.min_filtered_depth) < MISP_EPS &&
                                          depth[k].xyz.z < misc->abe_share->open_args[pfm].head.null_depth && 
                                          !inside_polygon2 (mx, my, poly_count, depth[k].xyz.x, depth[k].xyz.y))
					{
					  add_point (&xyz_array, depth[k].xyz, &out_count);
					  found = NVTrue;
					}
				      break;
    

				      //  Use the maximum value in the bin.
    
				    case 3:
				    case 6:
				      if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                                          fabs (depth[k].xyz.z - bin.max_filtered_depth) < MISP_EPS &&
                                          depth[k].xyz.z < misc->abe_share->open_args[pfm].head.null_depth && 
                                          !inside_polygon2 (mx, my, poly_count, depth[k].xyz.x, depth[k].xyz.y))
					{
					  add_point (&xyz_array, depth[k].xyz, &out_count);
					  found = NVTrue;
					}
				      break;
				    }
				  if (found) break;
				}
			      free (depth);
			    }
			}
		      else
			{
			  //  If there was no depth data in the border area (filter size) we want to load previously
			  //  interpolated (MISP/SURF gridded) data in order to smoothly edge match to existing interpolated data
			  //  outside of the MBR.

			  if (bin.validity & PFM_INTERPOLATED)
			    {
			      if (i < FILTER || j < FILTER || i >= row_filter[pfm] || j >= col_filter[pfm])
				{
				  xyz.x = bin.xy.x;
				  xyz.y = bin.xy.y;
				  xyz.z = bin.avg_filtered_depth;
				  add_point (&xyz_array, xyz, &out_count);
				}
			    }
			}
		    }
		}
	    }
	}
    }


  //  Now add in all of the drawn contours that fall inside of our polygon.

  for (int32_t i = 0 ; i < misc->filt_contour_count ; i++)
    {
      if (misc->filt_contour[i].x >= -180.0 && inside_polygon2 (mx, my, poly_count, misc->filt_contour[i].x, misc->filt_contour[i].y))
        add_point (&xyz_array, misc->filt_contour[i], &out_count);
    }


  if (!out_count) return;


  //  We have all of the input data (from any visible PFM) in memory so now we run MISP or SURF for each PFM layer that 
  //  has had MISP or SURF run on it before (average_type > 0, this is checked in pfmView.cpp).

  for (int32_t pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      //  Only deal with those that are being displayed and have been gridded.

      if (misc->abe_share->display_pfm[pfm] && misc->average_type[pfm])
	{
	  //  We're going to let MISP/SURF handle everything in zero based units of the bin size.  That is, we subtract off the
	  //  west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
	  //  the south latitude.  This will give us values that range from 0.0 to gridcols[pfm] in longitude and 0.0 to gridrows[pfm]
	  //  in latitude.

	  NV_F64_XYMBR new_mbr;
	  new_mbr.min_x = 0.0;
	  new_mbr.min_y = 0.0;
	  new_mbr.max_x = (double) gridcols[pfm];
	  new_mbr.max_y = (double) gridrows[pfm];


	  //  Initialize the MISP engine.

          if (options->misp_force_original) misp_weight = -misp_weight;
          misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, misp_weight, new_mbr);

          progText = pfmView::tr (" Loading data into MISP for %1 ").arg (QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm"));


          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, out_count);
	  qApp->processEvents();


	  //  Don't allow the progress bar to slow things down too much.

	  int32_t inc = out_count / 20;
          if (!inc) inc = 1;


	  for (int32_t i = 0 ; i < out_count ; i++)
	    {
	      if (!(out_count % inc))
		{
		  misc->statusProg->setValue (i);
		  qApp->processEvents();
		}

	      xyz.x = (xyz_array[i].x - grid_mbr[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
	      xyz.y = (xyz_array[i].y - grid_mbr[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
	      xyz.z = xyz_array[i].z;
              misp_load (xyz);
	    }


          progText = pfmView::tr (" Computing MISP surface for %1 ").arg (QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm"));

          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, 0);
	  qApp->processEvents();


          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, 0);
          misc->statusProg->setValue (-1);
          qApp->processEvents ();


          //  We're starting the grid processing concurrently using a thread.  Note that we're using the Qt::DirectConnection type
          //  for the signal/slot connections.  This causes all of the signals emitted from the thread to be serviced immediately.
          //  Why are we running misp_proc in a thread???  Because it's the only way to get the stupid progress bar to update so
          //  that the user will know that the damn program is still running.  Sheesh!

          complete = NVFalse;
          connect (&grid_thread, SIGNAL (completed ()), this, SLOT (slotGridCompleted ()), Qt::DirectConnection);

          grid_thread.grid (misc->average_type[pfm]);


          //  We can't move on until the thread is complete but we want to keep our progress bar updated.  This is a bit tricky 
          //  because you can't update the progress bar from within slots connected to thread signals.  Those slots are considered part
          //  of the mispThread and not part of the GUI thread.  When the thread is finished we move on to the retrieval step.

          while (!complete)
            {
#ifdef NVWIN3X
              Sleep (50);
#else
              usleep (50000);
#endif

              qApp->processEvents ();
            }


          progText = tr (" Retrieving MISP data for %1 ").arg (QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm"));


          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, gridrows[pfm]);
	  qApp->processEvents();


          //  Allocating one more than gridcols[pfm] due to constraints of old chrtr (see comments in misp_funcs.c)

          float *array = (float *) malloc ((gridcols[pfm] + 1) * sizeof (float));

          if (array == NULL)
            {
              fprintf (stderr, "%s %s %s %d - array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
              exit (-1);
            }


          for (int32_t i = 0 ; i < gridrows[pfm] ; i++)
            {
              misc->statusProg->setValue (i);
              qApp->processEvents();


              if (!misp_rtrv (array)) break;


              //  Only use data that aren't in the filter border

              if (i >= FILTER && i <= row_filter[pfm])
                {
                  //  Compute the latitude of the center of the bin.

                  NV_F64_COORD2 xy;
                  xy.y = grid_mbr[pfm].min_y + i * misc->abe_share->open_args[pfm].head.y_bin_size_degrees + half_y[pfm];


                  for (int32_t j = 0 ; j < gridcols[pfm] ; j++)
                    {
                      //  Only use data that aren't in the filter border

                      if (j >= FILTER && j <= col_filter[pfm])
                        {
                          //  Compute the longitude of the center of the bin.

                          xy.x = grid_mbr[pfm].min_x + j * misc->abe_share->open_args[pfm].head.x_bin_size_degrees + half_x[pfm];


                          //  Make sure we're inside the PFM bounds.  Note that we're not using the MBR.  The reason
                          //  being that the PFM bounds may be a polygon.

                          if (bin_inside_ptr (&misc->abe_share->open_args[pfm].head, xy))
                            {
                              //  Make sure we're inside of our polygon.

                              if (inside_polygon2 (mx, my, poly_count, xy.x, xy.y))
                                {
                                  //  Get the PFM coordinates of the center of the bin

                                  compute_index_ptr (xy, &coord, &misc->abe_share->open_args[pfm].head);


                                  //  Read the bin record and depth array.

                                  read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


                                  //  This is a special case.  We don't want to replace land masked bins.  If you really need to
                                  //  do that then you need to delete the land mask values.

                                  if (!land_mask_flag || !(bin.validity & PFM_USER_10))
                                    {
                                      //  Scan the depth array.

                                      if (bin.num_soundings)
                                        {
                                          if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
                                            {
                                              //  Find the single point that is closest to the average from the MISP grid.

                                              double min_diff = 999999999.0;
                                              int32_t ndx = -1;

                                              for (int32_t k = 0 ; k < recnum ; k++)
                                                {
                                                  if (!(depth[k].validity & (PFM_DELETED | PFM_INVAL | PFM_REFERENCE)))
                                                    {
                                                      if (fabs (depth[k].xyz.z - array[j]) < min_diff)
                                                        {
                                                          ndx = k;
                                                          min_diff = fabs (depth[k].xyz.z - array[j]);
                                                        }
                                                    }
                                                }


                                              //  Invalidate all but the closest point.

                                              if (ndx >= 0)
                                                {
                                                  for (int32_t k = 0 ; k < recnum ; k++)
                                                    {
                                                      if (!(depth[k].validity & (PFM_DELETED | PFM_INVAL | PFM_REFERENCE)))
                                                        {
                                                          if (k != ndx || fabs (depth[k].xyz.z - array[j]) > options->contour_filter_envelope)
                                                            {
                                                              depth[k].validity |= PFM_MANUALLY_INVAL;


                                                              //  Save the modified depth array back to the PFM

                                                              int32_t rc = update_depth_record_index (misc->pfm_handle[pfm], &depth[k]);
                                                              if (rc != SUCCESS)
                                                                {
                                                                  QString msg = tr ("%1 %2 %3 - Error on depth status update - %4\n").arg
                                                                    (__FILE__).arg (__FUNCTION__).arg (__LINE__).arg (pfm_error_str (rc));
                                                                  misc->messages->append (msg);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

          free (array);
	}
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (false);
  qApp->processEvents();


  xyz_array.clear ();


  //  Finally, we have to remisp now that we have invalidated the data points that weren't near the gridded surface.

  remisp (misc, options, &mbr);
}






remispFilter::~remispFilter ()
{
}



void 
remispFilter::add_point (std::vector <NV_F64_COORD3> *xyz_array, NV_F64_COORD3 xyz, int32_t *count)
{
  try
    {
      xyz_array->resize (*count + 1);
    }
  catch (std::bad_alloc&)
    {
      fprintf (stderr, "%s %s %s %d - xyz_array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
      exit (-1);
    }


  (*xyz_array)[*count] = xyz;

  (*count)++;
}



void remispFilter::slotGridCompleted ()
{
  complete = NVTrue;
}
