
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



#ifndef FINDFEATURE_H
#define FINDFEATURE_H

#include "pfm.h"
#include "pfmViewDef.hpp"


class findFeature:public QDialog
{
  Q_OBJECT 


    public:

  findFeature (QWidget *parent = 0, nvMap *co = NULL, OPTIONS *op = NULL, MISC *mi = NULL);
  ~findFeature ();


 signals:


 protected:

  nvMap           *cov;

  QTimer          *findTimer;

  QLineEdit       *searchString;

  QListWidget     *featureBox;

  QString         srch_strng;

  QStringList     featureList;

  OPTIONS         *options;

  MISC            *misc;

  int32_t         hot_id, prev_hot_id;


  void resetFeature (int id);


  protected slots:

  void slotFindTimer ();
  void slotTextEdited (const QString &text);
  void slotItemClicked (QListWidgetItem *item);
  void slotHelp ();
  void slotClose ();

 private:
};

#endif
