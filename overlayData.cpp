
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
#include "survey.hpp"


//!  This function draws the overlays on the display.

void overlayData (nvMap *map, MISC *misc, OPTIONS *options)
{
  uint8_t       first = NVTrue;
  FILE          *fp;
  char          string[256], tmp[256], desc[128], lat_hemi = 'N', lon_hemi = 'E', val;
  double        tmp_x[2], tmp_y[2], minBounds[4], maxBounds[4], dep;
  int32_t       lat_deg = 0, lat_min = 0, lon_deg = 0, lon_min = 0, file, ping, count, type, numShapes, x, y, z;
  float         lat_sec = 0.0, lon_sec = 0.0;
  SHPHandle     shpHandle;
  SHPObject     *shape = NULL;

  static std::vector<double> lat, lon;


  for (int32_t i = 0 ; i < NUM_OVERLAYS ; i++)
    {
      if (misc->overlays[i].file_type && misc->overlays[i].display)
        {
          if ((fp = fopen (misc->overlays[i].filename, "r")) != NULL)
            {
              switch (misc->overlays[i].file_type)
                {
                case ISS60_AREA:
                case ISS60_ZONE:
                  count = 0;
                  while (fgets (string, sizeof (string), fp) != NULL)
                    {
                      if (!strncmp (string, "POINT=", 6))
                        {
                          try
                            {
                              lat.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              lon.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lon array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }

                          strtok (string, ";");
                          strcpy (tmp, strtok (NULL, ";"));
                          posfix (tmp, &lat[count], POS_LAT);
                          strcpy (tmp, strtok (NULL, ";"));
                          posfix (tmp, &lon[count], POS_LON);

                          count++;
                        }
                    }


                  //  Clip the polygon

                  for (int32_t k = 0 ; k < count ; k++)
                    {
                      int32_t j = (k + 1) % count;


                      //  Don't clip the original points, clip a copy.

                      tmp_x[0] = lon[k];
                      tmp_y[0] = lat[k];
                      tmp_x[1] = lon[j];
                      tmp_y[1] = lat[j];


                      if (clip (&tmp_x[0], &tmp_y[0], &tmp_x[1], &tmp_y[1], misc->total_displayed_area))
                        {
                          map->drawLine (tmp_x[0], tmp_y[0], tmp_x[1], tmp_y[1],
                                         misc->overlays[i].color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                        }
                    }

                  break;


                case GENERIC_AREA:
                case GENERIC_ZONE:
                case ACE_AREA:
                  count = 0;


                  //  Army Corps only

                  if (misc->overlays[i].file_type == ACE_AREA)
                    {
                      while (fgets (string, sizeof (string), fp) != NULL)
                        {
                          try
                            {
                              lat.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              lon.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lon array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }


                          if (strchr (string, ','))
                            {
                              sscanf (string, "%lf,%lf", &lon[count], &lat[count]);
                            }
                          else
                            {
                              sscanf (string, "%lf %lf", &lon[count], &lat[count]);
                            }

                          count++;
                        }
                    }


                  //  Generic files.

                  else
                    {
                      while (fget_coord (fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, &lon_hemi, &lon_deg, &lon_min, &lon_sec))
                        {
                          try
                            {
                              lat.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }

                          try
                            {
                              lon.resize (count + 1);
                            }
                          catch (std::bad_alloc&)
                            {
                              fprintf (stderr, "%s %s %s %d - lon array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                       strerror (errno));
                              exit (-1);
                            }

                          lat[count] = (double) lat_deg + (double) lat_min / 60.0 + (double) lat_sec / 3600.0;
                          if (lat_hemi == 'S') lat[count] = -lat[count];

                          lon[count] = (double) lon_deg + (double) lon_min / 60.0 + (double) lon_sec / 3600.0;
                          if (lon_hemi == 'W') lon[count] = -lon[count];

                          count++;
                        }
                    }


                  //  Clip the polygon

                  for (int32_t k = 0 ; k < count ; k++)
                    {
                      int32_t j = (k + 1) % count;


                      //  Don't clip the original points, clip a copy.

                      tmp_x[0] = lon[k];
                      tmp_y[0] = lat[k];
                      tmp_x[1] = lon[j];
                      tmp_y[1] = lat[j];


                      if (clip (&tmp_x[0], &tmp_y[0], &tmp_x[1], &tmp_y[1], misc->total_displayed_area))
                        {
                          map->drawLine (tmp_x[0], tmp_y[0], tmp_x[1], tmp_y[1],
                                         misc->overlays[i].color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                        }
                    }

                  break;


                case ISS60_SURVEY:
                  SURVEY srv;

                  if (get_dsurvey (fp, &srv))
                    {
                      for (int32_t j = 0 ; j < srv.n ; j++)
                        {
                          if (clip (&srv.line[j].p1.x, &srv.line[j].p1.y, &srv.line[j].p2.x, &srv.line[j].p2.y,
                                    misc->total_displayed_area))
                            {
                              map->drawLine (srv.line[j].p1.x, srv.line[j].p1.y, srv.line[j].p2.x, srv.line[j].p2.y,
                                             misc->overlays[i].color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                            }
                        }

                      free_dsurvey (&srv);
                    }
                  break;


                case TRACK:
                  try
                    {
                      lat.resize (2);
                    }
                  catch (std::bad_alloc&)
                    {
                      fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                      exit (-1);
                    }

                  try
                    {
                      lon.resize (2);
                    }
                  catch (std::bad_alloc&)
                    {
                      fprintf (stderr, "%s %s %s %d - lon array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
                      exit (-1);
                    }


                  while (fgets (string, sizeof (string), fp) != NULL)
                    {
                      if (!strncmp (string, "FILE", 4) || !strncmp (string, "LEVEL", 5)) 
                        {
                          first = NVTrue;
                        }
                      else
                        {
                          sscanf (string, "%1c,%d,%d,%lf,%lf", &val, &file, &ping, &lat[1], &lon[1]);


                          if (val == '-') first = NVTrue;


                          if (!first) 
                            {
                              tmp_x[0] = lon[0];
                              tmp_y[0] = lat[0];
                              tmp_x[1] = lon[1];
                              tmp_y[1] = lat[1];

                              if (clip (&tmp_x[0], &tmp_y[0], &tmp_x[1], &tmp_y[1], misc->total_displayed_area))
                                {
                                  map->drawLine (tmp_x[0], tmp_y[0], tmp_x[1], tmp_y[1], 
                                                 misc->overlays[i].color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                                }
                            }
                          lon[0] = lon[1];
                          lat[0] = lat[1];

                          first = NVFalse;
                        }
                    }
                  break;


                case GENERIC_YXZ:
                  while (ngets (string, sizeof (string), fp) != NULL)
                    {
                      if (strlen (string) > 2)
                        {
                          strcpy (tmp, strtok (string, ","));
                          posfix (tmp, &tmp_y[0], POS_LAT);
                          strcpy (tmp, strtok (NULL, ","));
                          posfix (tmp, &tmp_x[0], POS_LON);
                          strcpy (desc, strtok (NULL, ","));

                          if (tmp_y[0] >= misc->total_displayed_area.min_y && tmp_y[0] <= misc->total_displayed_area.max_y &&
                              tmp_x[0] >= misc->total_displayed_area.min_x && tmp_x[0] <= misc->total_displayed_area.max_x)
                            {
                              if (strlen (desc) > 1)
                                {
                                  map->map_to_screen (1, &tmp_x[0], &tmp_y[0], &dep, &x, &y, &z);

                                  map->drawText (QString (desc), x + 9, y + 6, misc->overlays[i].color, NVFalse);
                                }

                              map->fillCircle (tmp_x[0], tmp_y[0], 10, 0.0, 360.0, misc->overlays[i].color, NVFalse);
                            }
                        }
                    }
                  break;


                case SHAPE:

		  fclose (fp);


                  //  Open shape file

                  shpHandle = SHPOpen (misc->overlays[i].filename, "rb");

                  if (shpHandle != NULL)
                    {
                      //  Get shape file header info

                      SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


                      //  Read all shapes

                      for (int32_t k = 0 ; k < numShapes ; k++)
                        {
                          shape = SHPReadObject (shpHandle, k);


                          //  Get all vertices

                          if (shape->nVertices >= 2)
                            {
                              count = 0;
                              for (int32_t j = 0 ; j < shape->nVertices ; j++)
                                {
                                  try
                                    {
                                      lat.resize (count + 1);
                                    }
                                  catch (std::bad_alloc&)
                                    {
                                      fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                               strerror (errno));
                                      exit (-1);
                                    }

                                  try
                                    {
                                      lon.resize (count + 1);
                                    }
                                  catch (std::bad_alloc&)
                                    {
                                      fprintf (stderr, "%s %s %s %d - lat array - %s\n", misc->progname, __FILE__, __FUNCTION__, __LINE__,
                                               strerror (errno));
                                      exit (-1);
                                    }


                                  lon[j] = shape->padfX[j];
                                  lat[j] = shape->padfY[j];

                                  count++;
                                }


                              //  Clip polygon to the minimum bounding rectangle

                              for (int32_t k = 0 ; k < count ; k++)
                                {
                                  if (lon[k] < misc->total_displayed_area.min_x) lon[k] = misc->total_displayed_area.min_x;
                                  if (lon[k] > misc->total_displayed_area.max_x) lon[k] = misc->total_displayed_area.max_x;
                                  if (lat[k] < misc->total_displayed_area.min_y) lat[k] = misc->total_displayed_area.min_y;
                                  if (lat[k] > misc->total_displayed_area.max_y) lat[k] = misc->total_displayed_area.max_y;
                                }

                              //map->fillPolygon (count, lon, lat, misc->overlays[i].color, NVTrue);
                              map->drawPolygon (count, lon.data (), lat.data (), misc->overlays[i].color, 2, NVTrue, Qt::SolidLine, NVTrue);
                            }

                          SHPDestroyObject (shape);
                        }
                      SHPClose (shpHandle);
                    }
                  break;
                }
            }
        }
    }



  lat.clear ();
  lon.clear ();


  //  Draw filtering contours if available (MISP surface only).

  if (misc->filt_contour_count)
    {
      uint8_t start = NVTrue;

      for (int32_t i = 0 ; i < misc->filt_contour_count ; i++)
        {
          if (start)
            {
              tmp_x[0] = misc->filt_contour[i].x;
              tmp_y[0] = misc->filt_contour[i].y;
              start = NVFalse;
            }
          else
            {
              if (misc->filt_contour[i].x < -180.0)
                {
                  start = NVTrue;
                }
              else
                {
                  tmp_x[1] = misc->filt_contour[i].x;
                  tmp_y[1] = misc->filt_contour[i].y;

                  if (tmp_x[0] >= misc->total_displayed_area.min_x && tmp_x[0] <= misc->total_displayed_area.max_x &&
                      tmp_x[1] >= misc->total_displayed_area.min_x && tmp_x[1] <= misc->total_displayed_area.max_x &&
                      tmp_y[0] >= misc->total_displayed_area.min_y && tmp_y[0] <= misc->total_displayed_area.max_y &&
                      tmp_y[1] >= misc->total_displayed_area.min_y && tmp_y[1] <= misc->total_displayed_area.max_y)
                    {
                      map->drawLine (tmp_x[0], tmp_y[0], tmp_x[1], tmp_y[1], options->contour_highlight_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                    }

                  tmp_x[0] = tmp_x[1];
                  tmp_y[0] = tmp_y[1];
                }
            }
        }
    }

  map->flush ();
}
