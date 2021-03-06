
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
  Sets an area checked, unchecked, verified, or unverified.  These are all PFM validity bit settings.  pfmView will
  display them in different ways if requested.
*/

void setAreaChecked (MISC *misc, int32_t type, uint8_t area)
{
  int32_t             i, j, mask = 0, width, height, row, column, recnum;
  QString             string;
  BIN_RECORD          bin_record;
  DEPTH_RECORD        *depth;
  uint8_t             checkit;
  uint32_t            prev_line;


  for (int32_t pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
          //  Adjust bounds to nearest grid point for setting displayed area, otherwise do entire file(s).

          if (area)
            {
              adjust_bounds (misc, pfm);
              row = misc->displayed_area_row[pfm];
              column = misc->displayed_area_column[pfm];
              width = misc->displayed_area_width[pfm];
              height = misc->displayed_area_height[pfm];
            }
          else
            {
              row = 0;
              column = 0;
              width = misc->abe_share->open_args[pfm].head.bin_width;            
              height = misc->abe_share->open_args[pfm].head.bin_height;             
            }


          switch (type)
            {
            case 0:
              string = pfmView::tr (" Setting bin flags to CHECKED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_CHECKED;
              break;

            case 1:
              string = pfmView::tr (" Setting bin flags to UNCHECKED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_CHECKED;
              break;

            case 2:
              string = pfmView::tr (" Setting bin flags to VERIFIED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_VERIFIED;
              break;

            case 3:
              string = pfmView::tr (" Setting bin flags to UNVERIFIED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_VERIFIED;
              break;

            case 4:
              string = pfmView::tr (" Setting multi-coverage bin flags to CHECKED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_CHECKED;
              break;

            case 5:
              string = pfmView::tr (" Setting valid multi-coverage bin flags to CHECKED for PFM %1 of %2").arg (pfm + 1).arg (misc->abe_share->pfm_count);
              mask = PFM_CHECKED;
              break;
            }


          misc->statusProg->setRange (0, height);
          misc->statusProgLabel->setText (string);
          misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
          misc->statusProgLabel->setPalette (misc->statusProgPalette);
          misc->statusProg->setTextVisible (true);
          qApp->processEvents();


          //  Loop for the height of the displayed area.

          for (j = 0 ; j < height ; j++)
            {
              misc->statusProg->setValue (j);
              qApp->processEvents();


              //  Loop for the width of the displayed area.
    
              for (i = 0 ; i < width ; i++)
                {
                  bin_record.coord.x = column + i;
                  bin_record.coord.y = row + j;
                  checkit = NVTrue;


                  //  To check or not to check, that is the question.

                  switch (type)
                    {
                    case 0:
                      bin_record.validity |= PFM_CHECKED;
                      break;

                    case 1:
                      bin_record.validity &= ~PFM_CHECKED;
                      break;

                    case 2:
                      bin_record.validity |= PFM_VERIFIED;
                      break;

                    case 3:
                      bin_record.validity &= ~PFM_VERIFIED;
                      break;

                    case 4:
                      checkit = NVFalse;

                      if (!read_depth_array_index (misc->pfm_handle[pfm], bin_record.coord, &depth, &recnum))
                        {
                          prev_line = depth[0].line_number;
                          for (int32_t j = 1 ; j < recnum ; j++)
                            {
                              if (depth[j].line_number != prev_line)
                                {
                                  checkit = NVTrue;
                                  break;
                                }

                              prev_line = depth[j].line_number;
                            }
                          free (depth);
                        }

                      if (checkit) bin_record.validity |= PFM_CHECKED;
                      break;

                    case 5:
                      checkit = NVFalse;

                      if (!read_depth_array_index (misc->pfm_handle[pfm], bin_record.coord, &depth, &recnum))
                        {
                          prev_line = UINT32_MAX;
                          for (int32_t j = 0 ; j < recnum ; j++)
                            {
                              if (!(depth[j].validity & PFM_INVAL))
                                {
                                  if (prev_line < UINT32_MAX)
                                    {
                                      if (depth[j].line_number != prev_line)
                                        {
                                          checkit = NVTrue;
                                          break;
                                        }
                                    }

                                  prev_line = depth[j].line_number;
                                }
                            }
                          free (depth);
                        }

                      if (checkit) bin_record.validity |= PFM_CHECKED;
                      break;
                    }


                  //  Only set or unset the CHECKED or VERIFIED flag.

                  if (checkit) write_bin_record_validity_index (misc->pfm_handle[pfm], &bin_record, mask);
                }
            }
        }
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (false);
  qApp->processEvents();
}
