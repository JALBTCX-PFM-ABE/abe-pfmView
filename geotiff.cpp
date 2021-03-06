
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


uint32_t getColorOffset (QString colorInt)
{
  static uint32_t mult;

  if (colorInt.contains ("Alpha"))
    {
      mult = 0x1000000;
    }
  else if (colorInt.contains ("Red"))
    {
      mult = 0x10000;
    }
  else if (colorInt.contains ("Green"))
    {
      mult = 0x100;
    }
  else if (colorInt.contains ("Blue"))
    {
      mult = 0x1;
    }

  return (mult);
}



//!  This paints the GeoTIFF over (or under) the displayed area.

void geotiff (nvMap *map, MISC *misc, OPTIONS *options)
{
  double             lat[2], lon[2], GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, dz[2], 
                     GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat, min_x, min_y, max_x, max_y;
  int32_t            width, height, start_y, end_y, start_x, end_x, z[2];
  static QPixmap     geotiff_save;
  static int32_t     x[2], y[2], wide, high;
  QImage             *full_res_image;
  static NV_F64_XYMBR prev_mbr = {-999.0, -999.0, -999.0, -999.0};
  GDALDataset        *poDataset;
  double             adfGeoTransform[6];



  //  Compute min and max from hatchr start and end since we may only be redrawing a small portion of
  //  the map (i.e. after an edit).

  min_x = misc->total_displayed_area.min_x + (misc->hatchr_start_x * misc->abe_share->open_args[0].head.x_bin_size_degrees);
  min_y = misc->total_displayed_area.min_y + (misc->hatchr_start_y * misc->abe_share->open_args[0].head.y_bin_size_degrees);
  max_x = misc->total_displayed_area.min_x + (misc->hatchr_end_x * misc->abe_share->open_args[0].head.x_bin_size_degrees);
  max_y = misc->total_displayed_area.min_y + (misc->hatchr_end_y * misc->abe_share->open_args[0].head.y_bin_size_degrees);


  //  If we haven't changed anything blast the old pixmap back to the screen instead of
  //  reading it and drawing it again

  if (misc->GeoTIFF_init || fabs (prev_mbr.min_x - min_x) > 0.00000001 ||
      fabs (prev_mbr.min_y - min_y) > 0.00000001 || fabs (prev_mbr.max_x - max_x) > 0.00000001 ||
      fabs (prev_mbr.max_y - max_y) > 0.00000001)
    {
      GDALAllRegister ();


      poDataset = (GDALDataset *) GDALOpen (misc->GeoTIFF_name, GA_ReadOnly);
      if (poDataset != NULL)
        {
          if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
            {
              GeoTIFF_lon_step = adfGeoTransform[1];
              GeoTIFF_lat_step = -adfGeoTransform[5];


              width = poDataset->GetRasterXSize ();
              height = poDataset->GetRasterYSize ();


              GeoTIFF_wlon = adfGeoTransform[0];
              GeoTIFF_nlat = adfGeoTransform[3];


              GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
              GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


              if (max_y >= GeoTIFF_slat && min_y <= GeoTIFF_nlat && max_x >= GeoTIFF_wlon && min_x <= GeoTIFF_elon)
                {
                  full_res_image = new QImage (misc->GeoTIFF_name, "tiff");

                  if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                    {
                      QMessageBox::critical (0, pfmView::tr ("pfmView GeoTIFF import"), 
                                             pfmView::tr ("Unable to open or read GeoTIFF file!"));
                      misc->GeoTIFF_open = NVFalse;
                      delete poDataset;
                      return;
                    }

                  if (options->GeoTIFF_alpha < 255 && full_res_image->hasAlphaChannel ())
                    {
                      QImage alphaChannel = full_res_image->alphaChannel ();

                      for (int32_t i = 0 ; i < height ; i++)
                        {
                          for (int32_t j = 0 ; j < width ; j++)
                            {
                              alphaChannel.setPixel (j, i, options->GeoTIFF_alpha);
                            }
                        }

                      full_res_image->setAlphaChannel (alphaChannel);
                    }


                  /*  This is how I had to read geoTIFFs from Qt 4.7.2 until 4.8.5.  For some unknown reason the above method quit working in
                      Qt 4.7.2 and I didn't check it again until !t 4.8.5 when it was again working.  I'm leaving this here for educational
                      purposes.

                  GDALRasterBand *poBand[4];
                  QString dataType[4], colorInt[4];
                  uint32_t mult[4] = {0, 0, 0, 0};
                  uint32_t alpha = options->GeoTIFF_alpha * 0x1000000;


                  int32_t rasterCount = poDataset->GetRasterCount ();
                  if (rasterCount < 3)
                    {
                      delete poDataset;
                      QMessageBox::critical (0, "pfmView", pfmView::tr ("Not enough raster bands in geoTIFF"));
                      return;
                    }

                  for (int32_t i = 0 ; i < rasterCount ; i++)
                    {
                      poBand[i] = poDataset->GetRasterBand (i + 1);

                      dataType[i] = QString (GDALGetDataTypeName (poBand[i]->GetRasterDataType ()));
                      colorInt[i] = QString (GDALGetColorInterpretationName (poBand[i]->GetColorInterpretation ()));


                      //  We can only handle Byte data (i.e. RGB or ARGB)

                      if (dataType[i] != "Byte")
                        {
                          delete poDataset;
                          QMessageBox::critical (0, "pfmView", pfmView::tr ("Cannot handle %1 data type").arg (dataType[i]));
                          return;
                        }

                      mult[i] = getColorOffset (colorInt[i]);
                    }

                  int32_t nXSize = poBand[0]->GetXSize ();
                  int32_t nYSize = poBand[0]->GetYSize ();

                  full_res_image = new QImage (nXSize, nYSize, QImage::Format_ARGB32);
                  if (full_res_image == NULL || full_res_image->width () == 0 || full_res_image->height () == 0)
                    {
                      QMessageBox::critical (0, "pfmView", pfmView::tr ("Unable to open image!"));
                      delete poDataset;
                      return;
                    }

                  uint32_t *color = new uint32_t[nXSize];
                  uint8_t *pafScanline = (uint8_t *) CPLMalloc (sizeof (uint8_t) * nXSize);

                  for (int32_t i = 0 ; i < nYSize ; i++)
                    {
                      //  If we don't have an alpha band set it to whatever transparancy the user has specified.

                      for (int32_t k = 0 ; k < nXSize ; k++)
                        {
                          if (rasterCount < 4)
                            {
                              color[k] = alpha;
                            }
                          else
                            {
                              color[k] = 0x0;
                            }
                        }


                      //  Read the raster bands.

                      for (int32_t j = 0 ; j < rasterCount ; j++)
                        {
                          poBand[j]->RasterIO (GF_Read, 0, i, nXSize, 1, pafScanline, nXSize, 1, GDT_Byte, 0, 0);
                          for (int32_t k = 0 ; k < nXSize ; k++) color[k] += ((uint32_t) pafScanline[k]) * mult[j];
                        }


                      //  Set the image pixels.

                      for (int32_t k = 0 ; k < nXSize ; k++)
                        {
                          full_res_image->setPixel (k, i, color[k]);
                        }
                    }

                  delete (color);
                  CPLFree (pafScanline);
                  */


                  //  Set the bounds

                  if (min_y < GeoTIFF_slat)
                    {
                      start_y = 0;
                      lat[0] = GeoTIFF_slat;
                    }
                  else
                    {
                      start_y = (int32_t) ((GeoTIFF_nlat - max_y) / GeoTIFF_lat_step);
                      lat[0] = min_y;
                    }
                  if (max_y > GeoTIFF_nlat)
                    {
                      end_y = height;
                      lat[1] = GeoTIFF_nlat;
                    }
                  else
                    {
                      end_y = (int32_t) ((GeoTIFF_nlat - min_y) / GeoTIFF_lat_step);
                      lat[1] = max_y;
                    }

                  if (min_x < GeoTIFF_wlon)
                    {
                      start_x = 0;
                      lon[0] = GeoTIFF_wlon;
                    }
                  else
                    {
                      start_x = (int32_t) ((min_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                      lon[0] = min_x;
                    }
                  if (max_x > GeoTIFF_elon)
                    {
                      end_x = width;
                      lon[1] = GeoTIFF_elon;
                    }
                  else
                    {
                      end_x = (int32_t) ((max_x - GeoTIFF_wlon) / GeoTIFF_lon_step);
                      lon[1] = max_x;
                    }


                  if (start_y < 0)
                    {
                      end_y = (int32_t) ((GeoTIFF_nlat - lat[0]) / GeoTIFF_lat_step);
                      start_y = 0;
                    }

                  if (end_y > height)
                    {
                      end_y = height;
                      start_y = (int32_t) ((GeoTIFF_nlat - lat[1]) / GeoTIFF_lat_step);
                    }


                  //  Number of rows and columns of the GeoTIFF to paint.

                  int32_t rows = end_y - start_y;
                  int32_t cols = end_x - start_x;


                  map->map_to_screen (2, lon, lat, dz, x, y, z);


                  int32_t wide = x[1] - x[0];
                  int32_t high = y[0] - y[1];


                  QPixmap sub_image = QPixmap::fromImage (full_res_image->copy (start_x, start_y, cols, rows));
                  geotiff_save = sub_image.scaled (wide, high, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                  delete full_res_image;


                  //  Blast the pixmap to the screen

                  map->drawPixmap (x[0], y[1], &geotiff_save, 0, 0, wide, high, NVTrue);
                }
              delete poDataset;
            }
        }
      else
        {
          QMessageBox::warning (map, pfmView::tr ("pfmView GeoTIFF"), pfmView::tr ("Unable to read tiff file."));
        }
    }
  else
    {
      //  We didn't change areas and we've still got the old image so blast it to the screen

      map->drawPixmap (x[0], y[1], &geotiff_save, 0, 0, wide, high, NVTrue);
    }

  prev_mbr.min_x = min_x;
  prev_mbr.min_y = min_y;
  prev_mbr.max_x = max_x;
  prev_mbr.max_y = max_y;


  //  Go ahead and plot.

  map->flush ();
}
