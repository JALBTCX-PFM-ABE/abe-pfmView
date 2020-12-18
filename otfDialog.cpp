
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



#include "otfDialog.hpp"
#include "otfDialogHelp.hpp"


otfDialog::otfDialog (QWidget *parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  mod_options = *op;
  mod_misc = *mi;


  setWindowTitle (tr ("pfmView OTF settings"));


  setSizeGripEnabled (true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *otfSizeBox = new QGroupBox (tr ("OTF bin size"), this);
  QHBoxLayout *otfSizeBoxLayout = new QHBoxLayout;
  otfSizeBox->setLayout (otfSizeBoxLayout);
  otfSize = new QDoubleSpinBox (otfSizeBox);
  otfSize->setDecimals (2);
  otfSize->setRange (0.10, 1852.0);
  otfSize->setSingleStep (1.0);
  otfSize->setValue (mod_options.otf_bin_size_meters);
  otfSize->setToolTip (tr ("Change the on-the-fly gridding bin size (meters)"));
  otfSize->setWhatsThis (otfSizeText);
  otfSizeBoxLayout->addWidget (otfSize);


  vbox->addWidget (otfSizeBox);


  //  If we've got more than one PFM opened we don't want to allow range limiting in the OTF bins.

  if (mod_misc.abe_share->pfm_count > 1) mod_misc.otf_attr = -1;


  QGroupBox *attrBox = new QGroupBox (tr ("Limiting attribute"), this);
  QHBoxLayout *attrBoxLayout = new QHBoxLayout;
  attrBox->setLayout (attrBoxLayout);
  attrCombo = new QComboBox (attrBox);
  attrCombo->setToolTip (tr ("Set an attribute to limit the data used in the OTF computation"));
  attrCombo->setWhatsThis (attrComboText);
  attrCombo->setEditable (false);
  attrCombo->addItem (tr ("None"));

  for (int32_t i = 0 ; i < mod_misc.abe_share->open_args[0].head.num_ndx_attr ; i++)
      attrCombo->addItem (mod_misc.abe_share->open_args[0].head.ndx_attr_name[i]);

  attrCombo->setCurrentIndex (mod_misc.otf_attr + 1);
  connect (attrCombo, SIGNAL (currentIndexChanged (int)), this, SLOT (slotAttrComboChanged (int)));
  attrBoxLayout->addWidget (attrCombo);


  vbox->addWidget (attrBox);


  QGroupBox *attrFilterBox = new QGroupBox (tr ("OTF attribute limits"), this);
  QVBoxLayout *attrFilterBoxLayout = new QVBoxLayout;
  attrFilterBox->setLayout (attrFilterBoxLayout);


  QHBoxLayout *attrLimitsLayout = new QHBoxLayout;
  attrFilterBoxLayout->addLayout (attrLimitsLayout);


  attrFilterMin = new QLabel (tr ("Min:     "));
  attrLimitsLayout->addWidget (attrFilterMin);

  attrFilterMax = new QLabel (tr ("Max:     "));
  attrLimitsLayout->addWidget (attrFilterMax);


  QHBoxLayout *attrRangeLayout = new QHBoxLayout;
  attrFilterBoxLayout->addLayout (attrRangeLayout);


  QLabel *attrFilterRangeLabel = new QLabel (tr ("Attribute ranges"), this);
  attrRangeLayout->addWidget (attrFilterRangeLabel);

  attrRangeText = new QLineEdit ("", this);
  attrRangeText->setAlignment (Qt::AlignLeft);
  attrRangeText->setWhatsThis (attrRangeTextText);
  if (mod_misc.otf_attr < 0)
    {
      attrRangeText->setText ("");
    }
  else
    {
      attrRangeText->setText (mod_options.attrFilterText[mod_misc.otf_attr]);
    }
  attrRangeLayout->addWidget (attrRangeText, 10);


  vbox->addWidget (attrFilterBox);


  //  If we've got more than one PFM opened we don't want to allow range limiting in the OTF bins.

  if (mod_misc.abe_share->pfm_count > 1)
    {
      attrCombo->setEnabled (false);
      attrRangeText->setEnabled (false);
    }
  else
    {
      if (mod_misc.otf_attr >= 0)
        {
          //  We want to 1 base the numbers if the attribute is the CZMIL channel number.

          if (QString (mod_misc.abe_share->open_args[0].head.ndx_attr_name[mod_misc.otf_attr]).contains ("CZMIL Channel number"))
            {
              attrFilterMin->setText (tr ("Min: %1").arg (mod_misc.abe_share->open_args[0].head.min_ndx_attr[mod_misc.otf_attr] + 1));
              attrFilterMax->setText (tr ("Max: %1").arg (mod_misc.abe_share->open_args[0].head.max_ndx_attr[mod_misc.otf_attr] + 1));
            }
          else
            {
              attrFilterMin->setText (tr ("Min: %1").arg (mod_misc.abe_share->open_args[0].head.min_ndx_attr[mod_misc.otf_attr]));
              attrFilterMax->setText (tr ("Max: %1").arg (mod_misc.abe_share->open_args[0].head.max_ndx_attr[mod_misc.otf_attr]));
            }
        }
    }

  QHBoxLayout *actions = new QHBoxLayout (0);
  actions->setMargin (5);
  actions->setSpacing (5);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Accept changes"));
  acceptButton->setWhatsThis (acceptOTFText);
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotAccept ()));
  actions->addWidget (acceptButton);

  QPushButton *closeButton = new QPushButton (tr ("Cancel"), this);
  closeButton->setToolTip (tr ("Discard changes"));
  closeButton->setWhatsThis (closeOTFText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  //  Move the dialog to the center of the parent if possible.

  QRect tmp = parent->frameGeometry ();
  int32_t window_x = tmp.x ();
  int32_t window_y = tmp.y ();


  tmp = parent->geometry ();
  int32_t width = tmp.width ();
  int32_t height = tmp.height ();


  QSize dlg = sizeHint ();
  int32_t dlg_width = dlg.width ();

  move (window_x + width / 2 - dlg_width / 2, window_y + height / 2);


  show ();
}



otfDialog::~otfDialog ()
{
}



void
otfDialog::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
otfDialog::slotAttrComboChanged (int index)
{
  mod_misc.otf_attr = index - 1;

  if (index > 0)
    {
      //  We want to 1 base the numbers if the attribute is the CZMIL channel number.

      if (QString (mod_misc.abe_share->open_args[0].head.ndx_attr_name[mod_misc.otf_attr]).contains ("CZMIL Channel number"))
        {
          attrFilterMin->setText (tr ("Min: %1").arg (mod_misc.abe_share->open_args[0].head.min_ndx_attr[mod_misc.otf_attr] + 1));
          attrFilterMax->setText (tr ("Max: %1").arg (mod_misc.abe_share->open_args[0].head.max_ndx_attr[mod_misc.otf_attr] + 1));
        }
      else
        {
          attrFilterMin->setText (tr ("Min: %1").arg (mod_misc.abe_share->open_args[0].head.min_ndx_attr[mod_misc.otf_attr]));
          attrFilterMax->setText (tr ("Max: %1").arg (mod_misc.abe_share->open_args[0].head.max_ndx_attr[mod_misc.otf_attr]));
        }


      attrRangeText->setText (mod_options.attrFilterText[mod_misc.otf_attr]);
      mod_options.attrFilterName = QString (mod_misc.abe_share->open_args[0].head.ndx_attr_name[mod_misc.otf_attr]);

      attrRangeText->setEnabled (true);
    }
  else
    {
      attrFilterMin->setText (tr ("Min:     "));
      attrFilterMax->setText (tr ("Max:     "));
      attrRangeText->setText ("");
      attrRangeText->setEnabled (false);
      mod_options.attrFilterName = "None";
    }
}



void
otfDialog::slotAccept ()
{
  if (mod_misc.otf_attr >= 0)
    {
      QString rangeText = attrRangeText->text ();


      //  First check for invalid characters.

      for (int32_t i = 0 ; i < rangeText.length () ; i++)
        {
          if (!(rangeText.at (i).isDigit ()))
            {
              if (rangeText.at (i) != ',' && rangeText.at (i) != '-' && rangeText.at (i) != '>' && rangeText.at (i) != '.')
                {
                  QMessageBox::warning (this, tr ("pfmView OTF settings"),
                                        tr ("Invalid character %1 in attribute filter range text").arg (rangeText.at (i)));
                  return;
                }
            }
        }


      //  We're actually going to do the following computations here, and in the OTF painter.  The reason being that we have to
      //  check the ranges here but we may not be running OTF gridding at the moment so we don't want to carry around arrays of ranges.

      //  Split the string into sections divided by commas.

      QStringList ranges = rangeText.split (",", QString::SkipEmptyParts);


      int32_t range_count = ranges.size ();


      //  Allocate the memory for the filter ranges.

      float *amin, *amax;

      amin = (float *) calloc (range_count, sizeof (float));
      if (amin == NULL)
        {
          perror ("Allocating amin memory in otfDialog::slotAccept");
          exit (-1);
        }


      amax = (float *) calloc (range_count, sizeof (float));
      if (amax == NULL)
        {
          perror ("Allocating amax memory in otfDialog::slotAccept");
          exit (-1);
        }


      for (int32_t i = 0 ; i < range_count ; i++)
        {
          if (ranges.at (i).contains ('>'))
            {
              QString aminString = ranges.at (i).section ('>', 0, 0);
              QString amaxString = ranges.at (i).section ('>', 1, 1);

              amin[i] = aminString.toFloat ();
              amax[i] = amaxString.toFloat ();
            }
          else
            {
              amin[i] = amax[i] = ranges.at (i).toFloat ();
            }


          //  Check for inverted range.

          if (amin[i] > amax[i])
            {
              QMessageBox::warning (this, tr ("pfmView OTF settings"),
                                    tr ("Range minimum %1 greater than range maximum %2").arg (amin[i]).arg (amax[i]));
              free (amin);
              free (amax);
              return;
            }
        }


      free (amin);
      free (amax);


      //  Save the ranges for this attribute.

      mod_options.attrFilterText[mod_misc.otf_attr] = rangeText;
      mod_options.attrFilterName = QString (mod_misc.abe_share->open_args[0].head.ndx_attr_name[mod_misc.otf_attr]);
    }


  //  Save the changed data back to the original structures.

  misc->otf_attr = mod_misc.otf_attr;
  if (misc->otf_attr >= 0) options->attrFilterText[misc->otf_attr] = mod_options.attrFilterText[misc->otf_attr];

  options->attrFilterName = mod_options.attrFilterName;
  options->otf_bin_size_meters = otfSize->value ();


  emit dataChangedSignal ();


  close ();
}



void
otfDialog::slotClose ()
{
  close ();
}
