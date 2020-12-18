
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


//!  Outputs feature polygons to Shapefile.

void outputFeaturePolygons (OPTIONS *options, MISC *misc)
{
  SHPHandle           shp_hnd;
  SHPObject           *shape;
  DBFHandle           dbf_hnd;  
  FILE                *prj_fp;
  char                shape_name[1024], prj_file[512];
  double              *x, *y, *z, *m;


  //  Loop through the feature data and make sure that there is at least one valid feature with a polygon.

  uint8_t poly = NVFalse;
  for (uint32_t i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      if (misc->feature[i].poly_count && misc->feature[i].poly_type && misc->feature[i].confidence_level >= 3)
        {
          poly = NVTrue;
          break;
        }
    }


  if (!poly)
    {
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("No valid polygons defined in feature file."));
      return;
    }


  QString name = QString (misc->abe_share->open_args[0].list_path);
  name.remove (".pfm").append (".shp");
  strcpy (shape_name, name.toLatin1 ());

  if (!QDir (options->output_shape_dir).exists ()) options->output_shape_dir = options->input_pfm_dir;

  QFileDialog fd (0, pfmView::tr ("pfmView Output Feature Shape File"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options->output_shape_dir);

  fd.setNameFilter (pfmView::tr ("Shapefile (*.shp)"));
  fd.setFileMode (QFileDialog::AnyFile);

  fd.selectFile (QString (shape_name));

  QStringList files;
  QString file;
  if (fd.exec () == QDialog::Accepted)
    {
      files = fd.selectedFiles ();

      file = files.at (0);

      if (file.isEmpty()) return;

      if (!file.endsWith (".shp")) file.append (".shp");


      //  Check for a pre-existing file

      strcpy (shape_name, file.toLatin1 ());
    }
  else
    {
      return;
    }


  //  Truncate the shapefile name so that we can use the libshp open routines.

  shape_name[strlen (shape_name) - 4] = 0;


  if ((shp_hnd = SHPCreate (shape_name, SHPT_POLYGON)) == NULL)
    {
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create .shp file."));
      return;
    }


  if ((dbf_hnd = DBFCreate (shape_name)) == NULL)
    {
      SHPClose (shp_hnd);
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create .dbf file."));
      return;
    }


  //  Make attributes

  if (DBFAddField (dbf_hnd, "Record#", FTInteger, 10, 0) == -1)
    {
      SHPClose (shp_hnd);
      DBFClose (dbf_hnd);  
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create Record# attribute."));
      return;
    }


  if (DBFAddField (dbf_hnd, "Remarks", FTString, 129, 0) == -1)
    {
      SHPClose (shp_hnd);
      DBFClose (dbf_hnd);  
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create Remarks attribute."));
      return;
    }


  if (DBFAddField (dbf_hnd, "Description", FTString, 129, 0) == -1)
    {
      SHPClose (shp_hnd);
      DBFClose (dbf_hnd);  
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create Description attribute."));
      return;
    }


  //  Stupid freaking .prj file

  strcpy (prj_file, shape_name);
  strcat (prj_file, ".prj");

  if ((prj_fp = fopen (prj_file, "w")) == NULL)
    {
      SHPClose (shp_hnd);
      DBFClose (dbf_hnd);  
      QMessageBox::warning (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("Unable to create .prj file."));
      return;
    }

  fprintf (prj_fp,
           "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]\n");
  fclose (prj_fp);


  int32_t shape_number = 0;

  for (uint32_t i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      if (misc->feature[i].poly_count && misc->feature[i].poly_type && misc->feature[i].confidence_level >= 3)
        {
          BFDATA_POLYGON bfd_polygon;

          binaryFeatureData_read_polygon (misc->bfd_handle, i, &bfd_polygon);

          x = (double *) calloc (misc->feature[i].poly_count, sizeof (double));
          y = (double *) calloc (misc->feature[i].poly_count, sizeof (double));
          z = (double *) calloc (misc->feature[i].poly_count, sizeof (double));
          m = (double *) calloc (misc->feature[i].poly_count, sizeof (double));

          for (uint32_t j = 0 ; j < misc->feature[i].poly_count ; j++)
            {
              x[j] = bfd_polygon.longitude[j];
              y[j] = bfd_polygon.latitude[j];
            }

          shape = SHPCreateObject (SHPT_POLYGON, -1, 0, NULL, NULL, misc->feature[i].poly_count, x, y, z, m);
          SHPWriteObject (shp_hnd, -1, shape);
          SHPDestroyObject (shape);

          free (x);
          free (y);
          free (z);
          free (m);

          DBFWriteIntegerAttribute (dbf_hnd, shape_number, 0, misc->feature[i].record_number);
          DBFWriteStringAttribute (dbf_hnd, shape_number, 1, misc->feature[i].remarks);
          DBFWriteStringAttribute (dbf_hnd, shape_number, 2, misc->feature[i].description);

          shape_number++;
        }
    }


  SHPClose (shp_hnd);
  DBFClose (dbf_hnd);  

  QMessageBox::information (0, pfmView::tr ("pfmView Output Feature"), pfmView::tr ("%1 polygons written to Shapefile %2").arg (shape_number).arg
                            (shape_name) + QString (".shp"));
}
