
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



#include "prefs.hpp"
#include "prefsHelp.hpp"


/*!
  This is the preferences dialog.  There are about a bazillion (that's a technical term) different
  options here.  Hopefully the code is easy to follow.
*/

prefs::prefs (QWidget *parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;

  mod_share = *misc->abe_share;
  mod_options = *op;
  mod_misc = *mi;


  contoursD = geotiffD = cubeD = hotKeyD = NULL;


  setModal (true);


  geotiffD = NULL;


  dataChanged = NVFalse;


  setWindowTitle (tr ("pfmView Preferences"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *conBox = new QGroupBox (this);
  QHBoxLayout *conBoxLayout = new QHBoxLayout;
  conBox->setLayout (conBoxLayout);
  QVBoxLayout *conBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *conBoxRightLayout = new QVBoxLayout;
  conBoxLayout->addLayout (conBoxLeftLayout);
  conBoxLayout->addLayout (conBoxRightLayout);


  oGrp = new QButtonGroup (this);
  oGrp->setExclusive (true);
  connect (oGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotOrientationClicked (int)));

  QGroupBox *orientBox = new QGroupBox (tr ("Z Orientation"), this);
  QHBoxLayout *orientBoxLayout = new QHBoxLayout;
  orientBox->setLayout (orientBoxLayout);
  orientBox->setWhatsThis (orientBoxText);

  depthCheck = new QCheckBox (tr ("Depth"), orientBox);
  depthCheck->setToolTip (tr ("Z values displayed as depths"));
  depthCheck->setWhatsThis (depthCheckText);
  oGrp->addButton (depthCheck, 0);
  orientBoxLayout->addWidget (depthCheck);

  elevationCheck = new QCheckBox (tr ("Elevation"), orientBox);
  elevationCheck->setToolTip (tr ("Z values displayed as elevations"));
  elevationCheck->setWhatsThis (elevationCheckText);
  oGrp->addButton (elevationCheck, 1);
  orientBoxLayout->addWidget (elevationCheck);

  conBoxLeftLayout->addWidget (orientBox);


  QGroupBox *intBox = new QGroupBox (tr ("Interval"), this);
  QHBoxLayout *intBoxLayout = new QHBoxLayout;
  intBox->setLayout (intBoxLayout);

  contourInt = new QLineEdit (intBox);
  contourInt->setToolTip (tr ("Change the contour interval"));
  contourInt->setWhatsThis (contourIntText);
  intBoxLayout->addWidget (contourInt);

  bContourLevels = new QPushButton (tr ("Set Contour Levels"), intBox);
  bContourLevels->setToolTip (tr ("Set non-uniform individual contour intervals"));
  bContourLevels->setWhatsThis (contourLevelsText);
  connect (bContourLevels, SIGNAL (clicked ()), this, SLOT (slotContourLevelsClicked ()));
  intBoxLayout->addWidget (bContourLevels);

  conBoxLeftLayout->addWidget (intBox);


  QGroupBox *smBox = new QGroupBox (tr ("Smoothing"), this);
  QHBoxLayout *smBoxLayout = new QHBoxLayout;
  smBox->setLayout (smBoxLayout);
  contourSm = new QSpinBox (smBox);
  contourSm->setRange (0, 10);
  contourSm->setSingleStep (1);
  contourSm->setToolTip (tr ("Change the contour smoothing factor (0-10)"));
  contourSm->setWhatsThis (contourSmText);
  smBoxLayout->addWidget (contourSm);
  conBoxLeftLayout->addWidget (smBox);


  QGroupBox *cfiltBox = new QGroupBox (tr ("Contour filter envelope"), this);
  QHBoxLayout *cfiltBoxLayout = new QHBoxLayout;
  cfiltBox->setLayout (cfiltBoxLayout);
  cfilt = new QDoubleSpinBox (cfiltBox);
  cfilt->setDecimals (2);
  cfilt->setRange (0.0, 100.0);
  cfilt->setSingleStep (10.0);
  cfilt->setToolTip (tr ("Change the contour filter envelope value"));
  cfilt->setWhatsThis (cfiltText);
  cfiltBoxLayout->addWidget (cfilt);
  conBoxLeftLayout->addWidget (cfiltBox);


  QGroupBox *zBox = new QGroupBox (tr ("Z Scale"), this);
  QHBoxLayout *zBoxLayout = new QHBoxLayout;
  zBox->setLayout (zBoxLayout);
  ZFactor = new QComboBox (zBox);
  ZFactor->setToolTip (tr ("Change the Z scale factor"));
  ZFactor->setWhatsThis (ZFactorText);
  ZFactor->setEditable (false);
  ZFactor->addItem (QString ("%L1").arg (1.00000, 0, 'f', 5) + tr (" (no scaling)"));
  ZFactor->addItem (QString ("%L1").arg (3.28084, 0, 'f', 5) + tr (" (meters to feet)"));
  ZFactor->addItem (QString ("%L1").arg (0.34080, 0, 'f', 5) + tr (" (feet to meters)"));
  ZFactor->addItem (QString ("%L1").arg (0.54681, 0, 'f', 5) + tr (" (meters to fathoms)"));
  ZFactor->addItem (QString ("%L1").arg (1.82880, 0, 'f', 5) + tr (" (fathoms to meters)"));
  ZFactor->addItem (QString ("%L1").arg (0.53333, 0, 'f', 5) + tr (" (meters to fathoms, at 4800 ft/sec)"));
  connect (ZFactor, SIGNAL (currentIndexChanged (int)), this, SLOT (slotZFactorChanged (int)));
  zBoxLayout->addWidget (ZFactor);
  conBoxRightLayout->addWidget (zBox);


  QGroupBox *offBox = new QGroupBox (tr ("Z Offset"), this);
  QHBoxLayout *offBoxLayout = new QHBoxLayout;
  offBox->setLayout (offBoxLayout);
  offset = new QDoubleSpinBox (offBox);
  offset->setDecimals (2);
  offset->setRange (-1000.0, 1000.0);
  offset->setSingleStep (10.0);
  offset->setToolTip (tr ("Change the contour/Z offset value"));
  offset->setWhatsThis (offsetText);
  offBoxLayout->addWidget (offset);

  QPushButton *egmButton = new QPushButton (tr ("EGM08", "Earth Gravitational Model 2008"));
  egmButton->setToolTip (tr ("Get the ellipsoid to geoid datum offset from EGM08 model"));
  egmButton->setWhatsThis (egmText);
  connect (egmButton, SIGNAL (clicked ()), this, SLOT (slotEgmClicked (void)));
  offBoxLayout->addWidget (egmButton);
  conBoxRightLayout->addWidget (offBox);


  QGroupBox *widBox = new QGroupBox (tr ("Width"), this);
  QHBoxLayout *widBoxLayout = new QHBoxLayout;
  widBox->setLayout (widBoxLayout);
  Width = new QComboBox (widBox);
  Width->setToolTip (tr ("Change the contour line width/thickness (pixels)"));
  Width->setWhatsThis (WidthText);
  Width->setEditable (true);
  Width->addItem ("1");
  Width->addItem ("2");
  Width->addItem ("3");
  widBoxLayout->addWidget (Width);
  conBoxRightLayout->addWidget (widBox);


  QGroupBox *iBox = new QGroupBox (tr ("Index"), this);
  QHBoxLayout *iBoxLayout = new QHBoxLayout;
  iBox->setLayout (iBoxLayout);
  Index = new QComboBox (iBox);
  Index->setToolTip (tr ("Change the index contour (Nth contour emphasized)"));
  Index->setWhatsThis (IndexText);
  Index->setEditable (true);
  Index->addItem ("0");
  Index->addItem ("2");
  Index->addItem ("4");
  Index->addItem ("5");
  Index->addItem ("10");
  iBoxLayout->addWidget (Index);
  conBoxRightLayout->addWidget (iBox);


  conBoxLeftLayout->addStretch (1);
  conBoxRightLayout->addStretch (1);


  prefTab = new QTabWidget ();
  prefTab->setTabPosition (QTabWidget::North);

  prefTab->addTab (conBox, tr ("Depth/Elevation"));
  prefTab->setTabToolTip (0, tr ("Set the depth/elevation options"));
  prefTab->setTabWhatsThis (0, conText);


  QGroupBox *cfbox = new QGroupBox (this);
  QVBoxLayout *cfboxLayout = new QVBoxLayout;
  cfbox->setLayout (cfboxLayout);


  QGroupBox *cbox = new QGroupBox (this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);
  QVBoxLayout *cboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cboxRightLayout = new QVBoxLayout;
  cboxLayout->addLayout (cboxLeftLayout);
  cboxLayout->addLayout (cboxRightLayout);


  bContourColor = new QPushButton (tr ("Contour"), this);
  bContourColor->setToolTip (tr ("Change contour color"));
  bContourColor->setWhatsThis (contourColorText);
  bContourPalette = bContourColor->palette ();
  connect (bContourColor, SIGNAL (clicked ()), this, SLOT (slotContourColor ()));
  cboxLeftLayout->addWidget (bContourColor);


  bGridColor = new QPushButton (tr ("Overlay Grid"), this);
  bGridColor->setToolTip (tr ("Change overlay grid color"));
  bGridColor->setWhatsThis (gridColorText);
  bGridPalette = bGridColor->palette ();
  connect (bGridColor, SIGNAL (clicked ()), this, SLOT (slotGridColor ()));
  cboxLeftLayout->addWidget (bGridColor);


  bCoastColor = new QPushButton (tr ("Coast"), this);
  bCoastColor->setToolTip (tr ("Change coastline color"));
  bCoastColor->setWhatsThis (coastColorText);
  bCoastPalette = bCoastColor->palette ();
  connect (bCoastColor, SIGNAL (clicked ()), this, SLOT (slotCoastColor ()));
  cboxLeftLayout->addWidget (bCoastColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxLeftLayout->addWidget (bBackgroundColor);


  bFeatureColor = new QPushButton (tr ("Feature"), this);
  bFeatureColor->setToolTip (tr ("Change feature color"));
  bFeatureColor->setWhatsThis (featureColorText);
  bFeaturePalette = bFeatureColor->palette ();
  connect (bFeatureColor, SIGNAL (clicked ()), this, SLOT (slotFeatureColor ()));
  cboxLeftLayout->addWidget (bFeatureColor);


  bFeatureInfoColor = new QPushButton (tr ("Feature information"), this);
  bFeatureInfoColor->setToolTip (tr ("Change feature information text color"));
  bFeatureInfoColor->setWhatsThis (featureInfoColorText);
  bFeatureInfoPalette = bFeatureInfoColor->palette ();
  connect (bFeatureInfoColor, SIGNAL (clicked ()), this, SLOT (slotFeatureInfoColor ()));
  cboxLeftLayout->addWidget (bFeatureInfoColor);


  bFeaturePolyColor = new QPushButton (tr ("Feature polygon"), this);
  bFeaturePolyColor->setToolTip (tr ("Change feature polygonal area color"));
  bFeaturePolyColor->setWhatsThis (featurePolyColorText);
  bFeaturePolyPalette = bFeaturePolyColor->palette ();
  connect (bFeaturePolyColor, SIGNAL (clicked ()), this, SLOT (slotFeaturePolyColor ()));
  cboxLeftLayout->addWidget (bFeaturePolyColor);


  bVerFeatureColor = new QPushButton (tr ("Verified Feature"), this);
  bVerFeatureColor->setToolTip (tr ("Change verified feature color"));
  bVerFeatureColor->setWhatsThis (verFeatureColorText);
  bVerFeaturePalette = bVerFeatureColor->palette ();
  connect (bVerFeatureColor, SIGNAL (clicked ()), this, SLOT (slotVerFeatureColor ()));
  cboxLeftLayout->addWidget (bVerFeatureColor);


  bContourHighlightColor = new QPushButton (tr ("Highlight"), this);
  bContourHighlightColor->setToolTip (tr ("Change contour/feature highlight color"));
  bContourHighlightColor->setWhatsThis (contourHighlightColorText);
  bContourHighlightPalette = bContourHighlightColor->palette ();
  connect (bContourHighlightColor, SIGNAL (clicked ()), this, SLOT (slotContourHighlightColor ()));
  cboxLeftLayout->addWidget (bContourHighlightColor);


  bMaskColor = new QPushButton (tr ("Filter Mask"), this);
  bMaskColor->setToolTip (tr ("Change filter mask color"));
  bMaskColor->setWhatsThis (maskColorText);
  bMaskPalette = bMaskColor->palette ();
  connect (bMaskColor, SIGNAL (clicked ()), this, SLOT (slotMaskColor ()));
  cboxLeftLayout->addWidget (bMaskColor);


  bStoplightMinColor = new QPushButton (tr ("Stoplight Minimum"), this);
  bStoplightMinColor->setToolTip (tr ("Change stoplight minimum color"));
  bStoplightMinColor->setWhatsThis (stoplightMinColorText);
  bStoplightMinPalette = bStoplightMinColor->palette ();
  connect (bStoplightMinColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMinColor ()));
  cboxLeftLayout->addWidget (bStoplightMinColor);


  bStoplightMidColor = new QPushButton (tr ("Stoplight Mid"), this);
  bStoplightMidColor->setToolTip (tr ("Change stoplight mid color"));
  bStoplightMidColor->setWhatsThis (stoplightMidColorText);
  bStoplightMidPalette = bStoplightMidColor->palette ();
  connect (bStoplightMidColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMidColor ()));
  cboxLeftLayout->addWidget (bStoplightMidColor);


  bStoplightMaxColor = new QPushButton (tr ("Stoplight Maximum"), this);
  bStoplightMaxColor->setToolTip (tr ("Change stoplight maximum color"));
  bStoplightMaxColor->setWhatsThis (stoplightMaxColorText);
  bStoplightMaxPalette = bStoplightMaxColor->palette ();
  connect (bStoplightMaxColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMaxColor ()));
  cboxLeftLayout->addWidget (bStoplightMaxColor);


  bCovFeatureColor = new QPushButton (tr ("Coverage Feature"), this);
  bCovFeatureColor->setToolTip (tr ("Change valid coverage feature color"));
  bCovFeatureColor->setWhatsThis (covFeatureColorText);
  bCovFeaturePalette = bCovFeatureColor->palette ();
  connect (bCovFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovFeatureColor ()));
  cboxRightLayout->addWidget (bCovFeatureColor);


  bCovCoastColor = new QPushButton (tr ("Coverage Coastline"), this);
  bCovCoastColor->setToolTip (tr ("Change coastline color in coverage map"));
  bCovCoastColor->setWhatsThis (covCoastColorText);
  bCovCoastPalette = bCovCoastColor->palette ();
  connect (bCovCoastColor, SIGNAL (clicked ()), this, SLOT (slotCovCoastColor ()));
  cboxRightLayout->addWidget (bCovCoastColor);


  bCovBoxColor = new QPushButton (tr ("Coverage Edit Box"), this);
  bCovBoxColor->setToolTip (tr ("Change edit box color in the coverage map"));
  bCovBoxColor->setWhatsThis (covBoxColorText);
  bCovBoxPalette = bCovBoxColor->palette ();
  connect (bCovBoxColor, SIGNAL (clicked ()), this, SLOT (slotCovBoxColor ()));
  cboxRightLayout->addWidget (bCovBoxColor);


  bCovHotPFMColor = new QPushButton (tr ("Coverage Highest Active PFM Box"), this);
  bCovHotPFMColor->setToolTip (tr ("Change highest active PFM box color in the coverage map"));
  bCovHotPFMColor->setWhatsThis (covHotPFMColorText);
  bCovHotPFMPalette = bCovHotPFMColor->palette ();
  connect (bCovHotPFMColor, SIGNAL (clicked ()), this, SLOT (slotCovHotPFMColor ()));
  cboxRightLayout->addWidget (bCovHotPFMColor);


  bCovColdPFMColor = new QPushButton (tr ("Coverage Active PFM Box"), this);
  bCovColdPFMColor->setToolTip (tr ("Change active PFM box color in the coverage map"));
  bCovColdPFMColor->setWhatsThis (covColdPFMColorText);
  bCovColdPFMPalette = bCovColdPFMColor->palette ();
  connect (bCovColdPFMColor, SIGNAL (clicked ()), this, SLOT (slotCovColdPFMColor ()));
  cboxRightLayout->addWidget (bCovColdPFMColor);


  bCovInvFeatureColor = new QPushButton (tr ("Invalid Coverage Feature"), this);
  bCovInvFeatureColor->setToolTip (tr ("Change invalidated coverage feature color"));
  bCovInvFeatureColor->setWhatsThis (covInvFeatureColorText);
  bCovInvFeaturePalette = bCovInvFeatureColor->palette ();
  connect (bCovInvFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovInvFeatureColor ()));
  cboxRightLayout->addWidget (bCovInvFeatureColor);


  bCovVerFeatureColor = new QPushButton (tr ("Verified Coverage Feature"), this);
  bCovVerFeatureColor->setToolTip (tr ("Change verified coverage feature color"));
  bCovVerFeatureColor->setWhatsThis (covVerFeatureColorText);
  bCovVerFeaturePalette = bCovVerFeatureColor->palette ();
  connect (bCovVerFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovVerFeatureColor ()));
  cboxRightLayout->addWidget (bCovVerFeatureColor);


  bCovDataColor = new QPushButton (tr ("Coverage Data"), this);
  bCovDataColor->setToolTip (tr ("Change data color in coverage map"));
  bCovDataColor->setWhatsThis (covDataColorText);
  bCovDataPalette = bCovDataColor->palette ();
  connect (bCovDataColor, SIGNAL (clicked ()), this, SLOT (slotCovDataColor ()));
  cboxRightLayout->addWidget (bCovDataColor);


  bCovBackgroundColor = new QPushButton (tr ("Coverage Background"), this);
  bCovBackgroundColor->setToolTip (tr ("Change background color in coverage map"));
  bCovBackgroundColor->setWhatsThis (covBackgroundColorText);
  bCovBackgroundPalette = bCovBackgroundColor->palette ();
  connect (bCovBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotCovBackgroundColor ()));
  cboxRightLayout->addWidget (bCovBackgroundColor);


  bCovCheckedColor = new QPushButton (tr ("Coverage Checked"), this);
  bCovCheckedColor->setToolTip (tr ("Change checked color in coverage map"));
  bCovCheckedColor->setWhatsThis (covCheckedColorText);
  bCovCheckedPalette = bCovCheckedColor->palette ();
  connect (bCovCheckedColor, SIGNAL (clicked ()), this, SLOT (slotCovCheckedColor ()));
  cboxRightLayout->addWidget (bCovCheckedColor);


  bCovVerifiedColor = new QPushButton (tr ("Coverage Verified"), this);
  bCovVerifiedColor->setToolTip (tr ("Change verified color in coverage map"));
  bCovVerifiedColor->setWhatsThis (covVerifiedColorText);
  bCovVerifiedPalette = bCovVerifiedColor->palette ();
  connect (bCovVerifiedColor, SIGNAL (clicked ()), this, SLOT (slotCovVerifiedColor ()));
  cboxRightLayout->addWidget (bCovVerifiedColor);


  bScaledOffsetZColor = new QPushButton (tr ("Scaled/Offset Z"), this);
  bScaledOffsetZColor->setToolTip (tr ("Change color for scaled and/or offset Z values"));
  bScaledOffsetZColor->setWhatsThis (scaledOffsetZColorText);
  bScaledOffsetZPalette = bScaledOffsetZColor->palette ();
  connect (bScaledOffsetZColor, SIGNAL (clicked ()), this, SLOT (slotScaledOffsetZColor ()));
  cboxRightLayout->addWidget (bScaledOffsetZColor);


  cboxLeftLayout->addStretch (1);
  cboxRightLayout->addStretch (1);


  cfboxLayout->addWidget (cbox);


  bFont = new QPushButton (tr ("Font : "), this);
  bFont->setToolTip (tr ("Change font for pfmView"));
  bFont->setWhatsThis (fontText);
  connect (bFont, SIGNAL (clicked ()), this, SLOT (slotFont ()));
  cfboxLayout->addWidget (bFont);


  prefTab->addTab (cfbox, tr ("Colors/Font"));
  prefTab->setTabToolTip (1, tr ("Set the display colors and text font"));
  prefTab->setTabWhatsThis (1, colorText);


  QGroupBox *filterBox = new QGroupBox (this);
  QVBoxLayout *filterBoxLayout = new QVBoxLayout;
  filterBox->setLayout (filterBoxLayout);


  QGroupBox *sBox = new QGroupBox (tr ("Standard deviation"), this);
  QHBoxLayout *sBoxLayout = new QHBoxLayout;
  sBox->setLayout (sBoxLayout);
  filterSTD = new QDoubleSpinBox (sBox);
  filterSTD->setDecimals (1);
  filterSTD->setRange (0.3, 4.0);
  filterSTD->setSingleStep (0.1);
  filterSTD->setToolTip (tr ("Change the filter standard deviation"));
  filterSTD->setWhatsThis (filterSTDText);
  sBoxLayout->addWidget (filterSTD);
  filterBoxLayout->addWidget (sBox);

  QGroupBox *dBox = new QGroupBox (tr ("Deep filter only"), this);
  dBox->setToolTip (tr ("Toggle filtering of deep values only"));
  QHBoxLayout *dBoxLayout = new QHBoxLayout;
  dBox->setLayout (dBoxLayout);
  dFilter = new QCheckBox (dBox);
  dFilter->setToolTip (tr ("Toggle filtering of deep values only"));
  dFilter->setWhatsThis (dFilterText);
  dBoxLayout->addWidget (dFilter);
  filterBoxLayout->addWidget (dBox);

  QGroupBox *rBox = new QGroupBox (tr ("Feature exclusion radius"), this);
  QHBoxLayout *rBoxLayout = new QHBoxLayout;
  rBox->setLayout (rBoxLayout);
  featureRadius = new QDoubleSpinBox (rBox);
  featureRadius->setDecimals (1);
  featureRadius->setRange (0.0, 100.0);
  featureRadius->setSingleStep (1.0);
  featureRadius->setToolTip (tr ("Change the feature exclusion radius (m)"));
  featureRadius->setWhatsThis (featureRadiusText);
  rBoxLayout->addWidget (featureRadius);
  filterBoxLayout->addWidget (rBox);

  filterBoxLayout->addStretch (1);


  prefTab->addTab (filterBox, tr ("Filter"));
  prefTab->setTabToolTip (2, tr ("Set the statistical filter options"));
  prefTab->setTabWhatsThis (2, filterText);


  //  Set up the miscellaneous tab

  QGroupBox *miscBox = new QGroupBox (this);
  QHBoxLayout *miscBoxLayout = new QHBoxLayout;
  miscBox->setLayout (miscBoxLayout);
  QVBoxLayout *miscBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *miscBoxRightLayout = new QVBoxLayout;
  miscBoxLayout->addLayout (miscBoxLeftLayout);
  miscBoxLayout->addLayout (miscBoxRightLayout);


  QGroupBox *zeroBox = new QGroupBox (tr ("Restart colors at zero"), this);
  zeroBox->setToolTip (tr ("Restart the color range at zero depth"));
  QHBoxLayout *zeroBoxLayout = new QHBoxLayout;
  zeroBox->setLayout (zeroBoxLayout);
  zero = new QCheckBox (zeroBox);
  zero->setToolTip (tr ("Restart the color range at zero depth"));
  zero->setWhatsThis (zeroText);
  zeroBoxLayout->addWidget (zero);
  miscBoxLeftLayout->addWidget (zeroBox);


  QGroupBox *pBox = new QGroupBox (tr ("Highlight depth percentage"), this);
  QHBoxLayout *pBoxLayout = new QHBoxLayout;
  pBox->setLayout (pBoxLayout);
  hPercent = new QDoubleSpinBox (pBox);
  hPercent->setDecimals (2);
  hPercent->setRange (0.01, 5.0);
  hPercent->setSingleStep (0.10);
  hPercent->setToolTip (tr ("Change the highlight depth percentage value"));
  hPercent->setWhatsThis (hPercentText);
  pBoxLayout->addWidget (hPercent);
  miscBoxLeftLayout->addWidget (pBox);


  QGroupBox *cntBox = new QGroupBox (tr ("Highlight valid bin count"), this);
  QHBoxLayout *cntBoxLayout = new QHBoxLayout;
  cntBox->setLayout (cntBoxLayout);
  hCount = new QSpinBox (cntBox);
  hCount->setRange (1, 1000);
  hCount->setSingleStep (1);
  hCount->setToolTip (tr ("Change the highlight valid bin count value"));
  hCount->setWhatsThis (hCountText);
  cntBoxLayout->addWidget (hCount);
  miscBoxLeftLayout->addWidget (cntBox);


  QGroupBox *csBox = new QGroupBox (tr ("Feature scaling"), this);
  QHBoxLayout *csBoxLayout = new QHBoxLayout;
  csBox->setLayout (csBoxLayout);

  fixedSize = new QCheckBox (tr ("Fixed size"), pBox);
  fixedSize->setToolTip (tr ("Set the feature size to a fixed size of 12 pixels (6 for children)"));
  fixedSize->setWhatsThis (tr ("Set the feature size to a fixed size of 12 pixels (6 for children)"));
  csBoxLayout->addWidget (fixedSize);
  connect (fixedSize, SIGNAL (stateChanged (int)), this, SLOT (slotFixedSizeChanged (int)));

  milli = new QSpinBox (pBox);
  milli->setRange (1, 100);
  milli->setSingleStep (5);
  milli->setToolTip (tr ("Change the millimeter setting used in mm@chart scale"));
  milli->setWhatsThis (milliText);
  csBoxLayout->addWidget (milli);

  QLabel *milliLabel = new QLabel (tr ("millimeters @ 1:"));
  csBoxLayout->addWidget (milliLabel);

  chartScale = new QSpinBox (pBox);
  chartScale->setRange (1, 3000000);
  chartScale->setSingleStep (5000);
  chartScale->setToolTip (tr ("Change the size of features based on chart scale"));
  chartScale->setWhatsThis (chartScaleText);
  csBoxLayout->addWidget (chartScale);

  QLabel *scaleLabel = new QLabel (tr ("chart scale"));
  csBoxLayout->addWidget (scaleLabel);

  miscBoxLeftLayout->addWidget (csBox);


  QGroupBox *gridBox = new QGroupBox (tr ("Overlay grid"), this);
  QHBoxLayout *gridBoxLayout = new QHBoxLayout;
  gridBox->setLayout (gridBoxLayout);

  QGroupBox *gtBox = new QGroupBox (tr ("Grid type"), this);
  QHBoxLayout *gtBoxLayout = new QHBoxLayout;
  gtBox->setLayout (gtBoxLayout);

  grid = new QComboBox (gtBox);
  grid->setToolTip (tr ("Select the grid spacing type for the overlay grid"));
  grid->setWhatsThis (overlayGridText);
  grid->setEditable (false);
  grid->addItem ("Meters");
  grid->addItem ("Minutes");
  gtBoxLayout->addWidget (grid);

  gridBoxLayout->addWidget (gtBox);


  QLabel *meterLabel = new QLabel (tr ("Meters"));
  gridBoxLayout->addWidget (meterLabel);

  gridMeters = new QDoubleSpinBox (gridBox);
  gridMeters->setDecimals (1);
  gridMeters->setRange (1, 10000);
  gridMeters->setSingleStep (100);
  gridMeters->setToolTip (tr ("Change the overlay grid spacing in meters"));
  gridMeters->setWhatsThis (tr ("Change the overlay grid spacing in meters"));
  gridBoxLayout->addWidget (gridMeters);


  QLabel *minuteLabel = new QLabel (tr ("Minutes"));
  gridBoxLayout->addWidget (minuteLabel);

  gridMinutes = new QDoubleSpinBox (gridBox);
  gridMinutes->setDecimals (2);
  gridMinutes->setRange (0.01, 10.00);
  gridMinutes->setSingleStep (0.1);
  gridMinutes->setToolTip (tr ("Change the overlay grid spacing in decimal minutes"));
  gridMinutes->setWhatsThis (tr ("Change the overlay grid spacing in decimal minutes"));
  gridBoxLayout->addWidget (gridMinutes);


  miscBoxLeftLayout->addWidget (gridBox);


  QGroupBox *dcsBox = new QGroupBox (tr ("Displayed area default chart scale"), this);
  QHBoxLayout *dcsBoxLayout = new QHBoxLayout;
  dcsBox->setLayout (dcsBoxLayout);

  QLabel *dcsLabel = new QLabel (tr ("1:"));
  dcsBoxLayout->addWidget (dcsLabel);

  dChartScale = new QSpinBox (pBox);
  dChartScale->setRange (100, 3000000);
  dChartScale->setSingleStep (5000);
  dChartScale->setToolTip (tr ("Change the default chart scale for the displayed area"));
  dChartScale->setWhatsThis (dChartScaleText);
  dcsBoxLayout->addWidget (dChartScale, 10);


  miscBoxRightLayout->addWidget (dcsBox);


  QGroupBox *dszBox = new QGroupBox (tr ("Displayed area default size"), this);
  QHBoxLayout *dszBoxLayout = new QHBoxLayout;
  dszBox->setLayout (dszBoxLayout);

  QLabel *dszwLabel = new QLabel (tr ("Width"));
  dszBoxLayout->addWidget (dszwLabel);

  dszWidth = new QDoubleSpinBox (pBox);
  dszWidth->setRange (10.0, 100000.0);
  dszWidth->setSingleStep (100.0);
  dszWidth->setDecimals (2);
  dszWidth->setToolTip (tr ("Change the default width (in meters) for the displayed area"));
  dszWidth->setWhatsThis (dszWidthText);
  dszBoxLayout->addWidget (dszWidth, 10);


  QLabel *dszhLabel = new QLabel (tr ("Height"));
  dszBoxLayout->addWidget (dszhLabel);

  dszHeight = new QDoubleSpinBox (pBox);
  dszHeight->setRange (10.0, 100000.0);
  dszHeight->setSingleStep (100.0);
  dszHeight->setDecimals (2);
  dszHeight->setToolTip (tr ("Change the default height (in meters) for the displayed area"));
  dszHeight->setWhatsThis (dszHeightText);
  dszBoxLayout->addWidget (dszHeight, 10);


  miscBoxRightLayout->addWidget (dszBox);


  QGroupBox *oBox = new QGroupBox (tr ("Window overlap percentage"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  overlap = new QSpinBox (oBox);
  overlap->setRange (0, 95);
  overlap->setSingleStep (1);
  overlap->setToolTip (tr ("Change the move window overlap percentage"));
  overlap->setWhatsThis (overlapText);
  oBoxLayout->addWidget (overlap);
  miscBoxRightLayout->addWidget (oBox);


  QGroupBox *textSearchBox = new QGroupBox (tr ("Feature search string"), this);
  QHBoxLayout *textSearchBoxLayout = new QHBoxLayout;
  textSearchBox->setLayout (textSearchBoxLayout);
  textSearch = new QLineEdit (textSearchBox);
  textSearch->setToolTip (tr ("Highlight features based on text search"));
  textSearch->setWhatsThis (textSearchText);
  textSearchBoxLayout->addWidget (textSearch);
  miscBoxRightLayout->addWidget (textSearchBox);


  invert = new QCheckBox (tr ("Invert"), textSearchBox);
  invert->setToolTip (tr ("Search for features that <b><i>do not</i></b> contain the search string"));
  invert->setWhatsThis (invertText);
  textSearchBoxLayout->addWidget (invert);


  QRadioButton *highlight = new QRadioButton (tr ("Highlight"));
  highlight->setWhatsThis (featureHighlightText);
  QRadioButton *display = new QRadioButton (tr ("Display"));
  display->setWhatsThis (displayText);

  hGrp = new QButtonGroup (this);
  hGrp->setExclusive (true);
  connect (hGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHighlightClicked (int)));

  hGrp->addButton (highlight, 0);
  hGrp->addButton (display, 1);

  textSearchBoxLayout->addWidget (highlight);
  textSearchBoxLayout->addWidget (display);


  bHotKeys = new QPushButton (tr ("Button Hot Keys"), this);
  bHotKeys->setToolTip (tr ("Change hot key sequences for buttons"));
  bHotKeys->setWhatsThis (hotKeysText);
  connect (bHotKeys, SIGNAL (clicked ()), this, SLOT (slotHotKeys ()));
  miscBoxRightLayout->addWidget (bHotKeys);


  QGroupBox *iconBox = new QGroupBox (tr ("Button Icon Size"), this);
  QHBoxLayout *iconBoxLayout = new QHBoxLayout;
  iconBox->setLayout (iconBoxLayout);

  iconSize = new QComboBox (iconBox);
  iconSize->setToolTip (tr ("Set the size (in pixels) of the main button icons"));
  iconSize->setWhatsThis (iconSizeText);
  iconSize->setEditable (false);
  iconSize->addItem ("16");
  iconSize->addItem ("20");
  iconSize->addItem ("24");
  iconSize->addItem ("28");
  iconSize->addItem ("32");
  iconBoxLayout->addWidget (iconSize);

  miscBoxRightLayout->addWidget (iconBox);


  QGroupBox *unloadBox = new QGroupBox (tr ("Auto close unload"), this);
  unloadBox->setToolTip (tr ("Automatically close unload dialogs on normal completion"));
  QHBoxLayout *unloadBoxLayout = new QHBoxLayout;
  unloadBox->setLayout (unloadBoxLayout);
  unload = new QCheckBox (unloadBox);
  unload->setToolTip (tr ("Automatically close unload dialogs on normal completion"));
  unload->setWhatsThis (prefUnloadText);
  unloadBoxLayout->addWidget (unload);
  miscBoxRightLayout->addWidget (unloadBox);


  bGeotiff = new QPushButton (tr ("GeoTIFF transparency"), this);
  bGeotiff->setToolTip (tr ("Change the amount of transparency for GeoTIFF overlay"));
  bGeotiff->setWhatsThis (prefsGeotiffText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  miscBoxLeftLayout->addWidget (bGeotiff);


  QGroupBox *geBox = new QGroupBox (tr ("Google Earth"), this);
  QHBoxLayout *geBoxLayout = new QHBoxLayout;
  geBox->setLayout (geBoxLayout);

  geName = new QLineEdit (geBox);
  geName->setReadOnly (true);
  geName->setToolTip (tr ("Google Earth executable name"));
  geName->setWhatsThis (geNameText);
  geBoxLayout->addWidget (geName);
  miscBoxLeftLayout->addWidget (geBox, 1);


  geBrowse = new QPushButton (tr ("Browse"), geBox);
  geBrowse->setToolTip (tr ("Browse for the Google Earth executable"));
  geBrowse->setWhatsThis (geBrowseText);
  connect (geBrowse, SIGNAL (clicked ()), this, SLOT (slotGeBrowseClicked ()));
  geBoxLayout->addWidget (geBrowse);


  miscBoxLeftLayout->addStretch (1);
  miscBoxRightLayout->addStretch (1);


  prefTab->addTab (miscBox, tr ("Miscellaneous"));
  prefTab->setTabToolTip (3, tr ("Set miscellaneous options"));
  prefTab->setTabWhatsThis (3, miscText);


  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  QGroupBox *formBox = new QGroupBox (this);
  QVBoxLayout *formBoxLayout = new QVBoxLayout;
  formBox->setLayout (formBoxLayout);

  bGrp = new QButtonGroup (this);
  bGrp->setExclusive (true);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPositionClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  formBoxLayout->addWidget (hdms);
  formBoxLayout->addWidget (hdm_);
  formBoxLayout->addWidget (hd__);
  formBoxLayout->addWidget (sdms);
  formBoxLayout->addWidget (sdm_);
  formBoxLayout->addWidget (sd__);

  formBoxLayout->addStretch (1);


  prefTab->addTab (formBox, tr ("Position Format"));
  prefTab->setTabToolTip (4, tr ("Set the position display format"));
  prefTab->setTabWhatsThis (4, bGrpText);


  QGroupBox *sunBox = new QGroupBox (this);
  QVBoxLayout *sunBoxLayout = new QVBoxLayout;
  sunBox->setLayout (sunBoxLayout);


  QGroupBox *angBox = new QGroupBox (tr ("Angle"), this);
  QHBoxLayout *angBoxLayout = new QHBoxLayout;
  angBox->setLayout (angBoxLayout);
  sunAz = new QDoubleSpinBox (angBox);
  sunAz->setDecimals (1);
  sunAz->setRange (0.0, 359.9);
  sunAz->setSingleStep (1.0);
  sunAz->setValue (mod_options.sunopts.azimuth);
  sunAz->setWrapping (true);
  sunAz->setToolTip (tr ("Change the sun angle (0.0-360.0)"));
  sunAz->setWhatsThis (sunAzText);
  angBoxLayout->addWidget (sunAz);
  sunBoxLayout->addWidget (angBox);


  QGroupBox *eleBox = new QGroupBox (tr ("Elevation"), this);
  QHBoxLayout *eleBoxLayout = new QHBoxLayout;
  eleBox->setLayout (eleBoxLayout);
  sunEl = new QDoubleSpinBox (eleBox);
  sunEl->setDecimals (1);
  sunEl->setRange (0.0, 90.0);
  sunEl->setSingleStep (1.0);
  sunEl->setValue (mod_options.sunopts.elevation);
  sunEl->setWrapping (true);
  sunEl->setToolTip (tr ("Change the sun elevation (0.0-90.0)"));
  sunEl->setWhatsThis (sunElText);
  eleBoxLayout->addWidget (sunEl);
  sunBoxLayout->addWidget (eleBox);


  QGroupBox *exaBox = new QGroupBox (tr ("Exaggeration"), this);
  QHBoxLayout *exaBoxLayout = new QHBoxLayout;
  exaBox->setLayout (exaBoxLayout);
  sunEx = new QDoubleSpinBox (exaBox);
  sunEx->setDecimals (1);
  sunEx->setRange (0.0, 359.9);
  sunEx->setSingleStep (1.0);
  sunEx->setValue (mod_options.sunopts.exag);
  sunEx->setWrapping (true);
  sunEx->setToolTip (tr ("Change the sun Z exaggeration (1-10)"));
  sunEx->setWhatsThis (sunExText);
  exaBoxLayout->addWidget (sunEx);
  sunBoxLayout->addWidget (exaBox);

  sunBoxLayout->addStretch (1);


  prefTab->addTab (sunBox, tr ("Sun Shading"));
  prefTab->setTabToolTip (5, tr ("Set sun shading options"));
  prefTab->setTabWhatsThis (5, sunText);


  QGroupBox *cubeBox = new QGroupBox (this);
  QHBoxLayout *cubeBoxLayout = new QHBoxLayout;
  cubeBox->setLayout (cubeBoxLayout);
  QVBoxLayout *cubeBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cubeBoxRightLayout = new QVBoxLayout;
  cubeBoxLayout->addLayout (cubeBoxLeftLayout);
  cubeBoxLayout->addLayout (cubeBoxRightLayout);


  QGroupBox *wBox = new QGroupBox (tr ("MISP weight factor"), this);
  QHBoxLayout *wBoxLayout = new QHBoxLayout;
  wBox->setLayout (wBoxLayout);
  WFactor = new QComboBox (wBox);
  WFactor->setToolTip (tr ("Change the MISP weighting factor"));
  WFactor->setWhatsThis (WFactorText);
  WFactor->setEditable (true);
  WFactor->addItem ("1");
  WFactor->addItem ("2");
  WFactor->addItem ("3");
  if (mod_misc.average_type)
    {
      WFactor->setEnabled (true);
    }
  else
    {
      WFactor->setEnabled (false);
    }
  wBoxLayout->addWidget (WFactor);
  cubeBoxLeftLayout->addWidget (wBox);


  QGroupBox *forceBox = new QGroupBox (tr ("MISP force original input value"), this);
  forceBox->setToolTip (tr ("Force MISP to use the original input value when available"));
  QHBoxLayout *forceBoxLayout = new QHBoxLayout;
  forceBox->setLayout (forceBoxLayout);
  force = new QCheckBox (forceBox);
  force->setToolTip (tr ("Force MISP to use the original input value when available"));
  force->setWhatsThis (forceText);
  if (mod_misc.average_type)
    {
      force->setEnabled (true);
    }
  else
    {
      force->setEnabled (false);
    }
  forceBoxLayout->addWidget (force);
  cubeBoxLeftLayout->addWidget (forceBox);


  QGroupBox *replaceBox = new QGroupBox (tr ("MISP replace all values"), this);
  replaceBox->setToolTip (tr ("Replace average edited surface values in all bins"));
  QHBoxLayout *replaceBoxLayout = new QHBoxLayout;
  replaceBox->setLayout (replaceBoxLayout);
  replace = new QCheckBox (replaceBox);
  replace->setToolTip (tr ("Replace average edited surface values in all bins"));
  replace->setWhatsThis (replaceText);
  if (mod_misc.average_type)
    {
      replace->setEnabled (true);
    }
  else
    {
      replace->setEnabled (false);
    }
  replaceBoxLayout->addWidget (replace);
  cubeBoxLeftLayout->addWidget (replaceBox);


  QGroupBox *ihoBox = new QGroupBox (tr ("IHO order"), this);
  QHBoxLayout *ihoBoxLayout = new QHBoxLayout;
  ihoBox->setLayout (ihoBoxLayout);
  ihoBoxLayout->setSpacing (10);

  ihoOrder = new QComboBox (ihoBox);
  ihoOrder->setToolTip (tr ("Set the IHO order for the CUBE process"));
  ihoOrder->setEditable (false);
  ihoOrder->addItem ("IHO Special Order");
  ihoOrder->addItem ("IHO Order 1a/1b");
  ihoOrder->addItem ("IHO Order 2");
  ihoBoxLayout->addWidget (ihoOrder);


  cubeBoxLeftLayout->addWidget (ihoBox);


  QGroupBox *captureBox = new QGroupBox (tr ("Capture percentage"), this);
  QHBoxLayout *captureBoxLayout = new QHBoxLayout;
  captureBox->setLayout (captureBoxLayout);
  captureBoxLayout->setSpacing (10);

  capture = new QDoubleSpinBox (captureBox);
  capture->setDecimals (1);
  capture->setRange (1.0, 10.0);
  capture->setSingleStep (1.0);
  capture->setWrapping (true);
  capture->setToolTip (tr ("Set the capture percentage for the CUBE process"));
  captureBoxLayout->addWidget (capture);


  cubeBoxLeftLayout->addWidget (captureBox);


  QGroupBox *queueBox = new QGroupBox (tr ("Queue length"), this);
  QHBoxLayout *queueBoxLayout = new QHBoxLayout;
  queueBox->setLayout (queueBoxLayout);
  queueBoxLayout->setSpacing (10);

  queue = new QSpinBox (queueBox);
  queue->setRange (1, 20);
  queue->setSingleStep (1);
  queue->setWrapping (true);
  queue->setToolTip (tr ("Set the queue length for the CUBE process"));
  queueBoxLayout->addWidget (queue);


  cubeBoxLeftLayout->addWidget (queueBox);


  QGroupBox *horizBox = new QGroupBox (tr ("Horizontal position uncertainty"), this);
  QHBoxLayout *horizBoxLayout = new QHBoxLayout;
  horizBox->setLayout (horizBoxLayout);
  horizBoxLayout->setSpacing (10);

  horiz = new QDoubleSpinBox (horizBox);
  horiz->setDecimals (1);
  horiz->setRange (0.0, 100.0);
  horiz->setSingleStep (1.0);
  horiz->setWrapping (true);
  horiz->setToolTip (tr ("Set the horizontal position uncertainty for the CUBE process"));
  horizBoxLayout->addWidget (horiz);


  cubeBoxRightLayout->addWidget (horizBox);


  QGroupBox *distanceBox = new QGroupBox (tr ("Distance exponent"), this);
  QHBoxLayout *distanceBoxLayout = new QHBoxLayout;
  distanceBox->setLayout (distanceBoxLayout);
  distanceBoxLayout->setSpacing (10);

  distance = new QDoubleSpinBox (distanceBox);
  distance->setDecimals (1);
  distance->setRange (0.0, 4.0);
  distance->setSingleStep (1.0);
  distance->setWrapping (true);
  distance->setToolTip (tr ("Set the distance exponent for the CUBE process"));
  distanceBoxLayout->addWidget (distance);


  cubeBoxRightLayout->addWidget (distanceBox);


  QGroupBox *minContextBox = new QGroupBox (tr ("Minimum context"), this);
  QHBoxLayout *minContextBoxLayout = new QHBoxLayout;
  minContextBox->setLayout (minContextBoxLayout);
  minContextBoxLayout->setSpacing (10);

  minContext = new QDoubleSpinBox (minContextBox);
  minContext->setDecimals (1);
  minContext->setRange (0.0, 10.0);
  minContext->setSingleStep (1.0);
  minContext->setWrapping (true);
  minContext->setToolTip (tr ("Set the minimum context for the CUBE process"));
  minContextBoxLayout->addWidget (minContext);


  cubeBoxRightLayout->addWidget (minContextBox);


  QGroupBox *maxContextBox = new QGroupBox (tr ("Maximum context"), this);
  QHBoxLayout *maxContextBoxLayout = new QHBoxLayout;
  maxContextBox->setLayout (maxContextBoxLayout);
  maxContextBoxLayout->setSpacing (10);

  maxContext = new QDoubleSpinBox (maxContextBox);
  maxContext->setDecimals (1);
  maxContext->setRange (0.0, 10.0);
  maxContext->setSingleStep (1.0);
  maxContext->setWrapping (true);
  maxContext->setToolTip (tr ("Set the maximum context for the CUBE process"));
  maxContextBoxLayout->addWidget (maxContext);


  cubeBoxRightLayout->addWidget (maxContextBox);


  QGroupBox *std2confBox = new QGroupBox (tr ("Std2Conf"), this);
  QHBoxLayout *std2confBoxLayout = new QHBoxLayout;
  std2confBox->setLayout (std2confBoxLayout);
  std2confBoxLayout->setSpacing (10);

  std2conf = new QComboBox (std2confBox);
  std2conf->setToolTip (tr ("Set the standard deviation to confidence scale for the CUBE process"));
  std2conf->setEditable (false);
  std2conf->addItem (QString ("%L1").arg (2.56, 0, 'f', 2));
  std2conf->addItem (QString ("%L1").arg (1.96, 0, 'f', 2));
  std2conf->addItem (QString ("%L1").arg (1.69, 0, 'f', 2));
  std2conf->addItem (QString ("%L1").arg (1.00, 0, 'f', 2));
  std2confBoxLayout->addWidget (std2conf);


  cubeBoxRightLayout->addWidget (std2confBox);


  QGroupBox *disBox = new QGroupBox (tr ("Disambiguation method"), this);
  QHBoxLayout *disBoxLayout = new QHBoxLayout;
  disBox->setLayout (disBoxLayout);
  disBoxLayout->setSpacing (10);

  disambiguation = new QComboBox (disBox);
  disambiguation->setToolTip (tr ("Set the disambiguation method for the CUBE process"));
  disambiguation->setEditable (false);
  disambiguation->addItem ("Prior");
  disambiguation->addItem ("Likelihood");
  disambiguation->addItem ("Posterior");
  disBoxLayout->addWidget (disambiguation);

  cubeBoxRightLayout->addWidget (disBox);

  cubeBoxLeftLayout->addStretch (1);
  cubeBoxRightLayout->addStretch (1);


  //  Only add the CUBE preferences if we have the cube program.

  if (misc->cube_available)
    {
      prefTab->addTab (cubeBox, tr ("MISP/CUBE"));
      prefTab->setTabToolTip (6, tr ("Set MISP/CUBE options"));
      prefTab->setTabWhatsThis (6, cubeBoxText);
    }
  else
    {
      prefTab->addTab (cubeBox, tr ("MISP"));
      prefTab->setTabToolTip (6, tr ("Set MISP options"));
      prefTab->setTabWhatsThis (6, mispBoxText);
      ihoBox->hide ();
      captureBox->hide ();
      queueBox->hide ();
      horizBox->hide ();
      distanceBox->hide ();
      minContextBox->hide ();
      maxContextBox->hide ();
      std2confBox->hide ();
      disBox->hide ();
    }

  vbox->addWidget (prefTab);


  setFields ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  bRestoreDefaults = new QPushButton (tr ("Restore Defaults"), this);
  bRestoreDefaults->setToolTip (tr ("Restore all preferences to the default state"));
  bRestoreDefaults->setWhatsThis (restoreDefaultsText);
  connect (bRestoreDefaults, SIGNAL (clicked ()), this, SLOT (slotRestoreDefaults ()));
  actions->addWidget (bRestoreDefaults);

  QPushButton *applyButton = new QPushButton (tr ("OK"), this);
  applyButton->setToolTip (tr ("Accept changes and close dialog"));
  applyButton->setWhatsThis (applyPrefsText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyPrefs ()));
  actions->addWidget (applyButton);

  QPushButton *closeButton = new QPushButton (tr ("Cancel"), this);
  closeButton->setToolTip (tr ("Discard changes and close dialog"));
  closeButton->setWhatsThis (closePrefsText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);


  //  Set the current tab to the last used tab.

  prefTab->setCurrentIndex (options->pref_tab);
  connect (prefTab, SIGNAL (currentChanged (int)), this, SLOT (slotPrefTabChanged (int)));


  show ();
}



prefs::~prefs ()
{
}



void
prefs::slotPrefTabChanged (int tab)
{
  //  Force it to be the current tab in both the original and the copy, in case we change something.

  mod_options.pref_tab = options->pref_tab = tab;
}



/*!
  The following functions (until further notice) are just servicing pushbuttons.  They're intuitively obvious to the
  most casual observer.
*/

void
prefs::slotContourColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.contour_color, this, tr ("pfmView Contour Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.contour_color = clr;

  setFields ();
}



void
prefs::slotGridColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.overlay_grid_color, this, tr ("pfmView Grid Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.overlay_grid_color = clr;

  setFields ();
}



void
prefs::slotFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_color, this, tr ("pfmView Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_color = clr;

  setFields ();
}



void
prefs::slotVerFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.verified_feature_color, this, tr ("pfmView Verified Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.verified_feature_color = clr;

  setFields ();
}



void
prefs::slotCovVerFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_verified_feature_color, this, tr ("pfmView Coverage Verified Feature Color"),
				QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_verified_feature_color = clr;

  setFields ();
}



void
prefs::slotFeatureInfoColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_info_color, this, tr ("pfmView Feature Information Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_info_color = clr;

  setFields ();
}



void
prefs::slotFeaturePolyColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_poly_color, this, tr ("pfmView Feature Polygon Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_poly_color = clr;

  setFields ();
}



void
prefs::slotCoastColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.coast_color, this, tr ("pfmView Coastline Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.coast_color = clr;

  setFields ();
}



void
prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("pfmView Background Color"));

  if (clr.isValid ()) mod_options.background_color = clr;

  setFields ();
}



void
prefs::slotContourHighlightColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.contour_highlight_color, this, tr ("pfmView Highlighted Contour Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.contour_highlight_color = clr;

  setFields ();
}



void
prefs::slotMaskColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.poly_filter_mask_color, this, tr ("pfmView Filter Mask Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.poly_filter_mask_color = clr;

  setFields ();
}



void
prefs::slotCovFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_feature_color, this, tr ("pfmView Coverage Map Feature Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_feature_color = clr;

  setFields ();
}



void
prefs::slotCovBoxColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_box_color, this, tr ("pfmView Coverage Map Edit Box Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_box_color = clr;

  setFields ();
}



void
prefs::slotCovDataColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_data_color, this, tr ("pfmView Coverage Map Data Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_data_color = clr;

  setFields ();
}



void
prefs::slotCovCheckedColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_checked_color, this, tr ("pfmView Coverage Map Checked Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_checked_color = clr;

  setFields ();
}



void
prefs::slotCovVerifiedColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_verified_color, this, tr ("pfmView Coverage Map Verified Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_verified_color = clr;

  setFields ();
}



void
prefs::slotCovCoastColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_coast_color, this, tr ("pfmView Coverage Map Coast Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_coast_color = clr;

  setFields ();
}



void
prefs::slotCovBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_background_color, this, tr ("pfmView Coverage Map Background Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_background_color = clr;

  setFields ();
}



void
prefs::slotCovHotPFMColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_pfm_box_hot_color, this, tr ("pfmView Coverage Map Highest Active PFM Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_pfm_box_hot_color = clr;

  setFields ();
}



void
prefs::slotCovColdPFMColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_pfm_box_cold_color, this, tr ("pfmView Coverage Map Highest Active PFM Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_pfm_box_cold_color = clr;

  setFields ();
}



void
prefs::slotCovInvFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_inv_feature_color, this, tr ("pfmView Coverage Map Invalid Feature Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_inv_feature_color = clr;

  setFields ();
}



void
prefs::slotStoplightMinColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_min_color, this, tr ("pfmView Stoplight Min Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_min_color = clr;

  setFields ();
}



void
prefs::slotStoplightMidColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_mid_color, this, tr ("pfmView Stoplight Mid Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_mid_color = clr;

  setFields ();
}



void
prefs::slotStoplightMaxColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_max_color, this, tr ("pfmView Stoplight Max Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_max_color = clr;

  setFields ();
}



void
prefs::slotScaledOffsetZColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.scaled_offset_z_color, this, tr ("pfmView Scaled/Offset Z Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.scaled_offset_z_color = clr;

  setFields ();
}



void
prefs::slotFont ()
{
  bool ok;
  QFont font = QFontDialog::getFont (&ok, mod_options.font, this);
  if (ok)
    {
      mod_options.font = font;

      setFields ();
    }
}



void
prefs::slotFixedSizeChanged (int state)
{
  if (state)
    {
      mod_options.fixed_feature_size = NVTrue;
    }
  else
    {
      mod_options.fixed_feature_size = NVFalse;
    }

  setFields ();
}



void
prefs::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//!  Restore all of the application defaults (this calls set_defaults and gets the original defaults).

void
prefs::slotRestoreDefaults ()
{
  void set_defaults (MISC *misc, OPTIONS *options, uint8_t restore);


  set_defaults (misc, options, NVTrue);

  mod_options = *options;
  mod_share = *misc->abe_share;
  mod_misc = *misc;

  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);

  setFields ();


  mod_misc.GeoTIFF_init = NVTrue;


  *misc->abe_share = mod_share;
  *options = mod_options;
  *misc = mod_misc;

  emit dataChangedSignal (NVTrue);

  close ();
}



void
prefs::slotGeBrowseClicked ()
{
  QStringList files, filters;
  QString file;
  QFileInfo fi = QFileInfo (mod_options.ge_name);
  QDir dir = fi.dir ();


  QFileDialog *fd = new QFileDialog (this, tr ("pfmView Google Earth name"));
  fd->setViewMode (QFileDialog::List);


  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFile (mod_options.ge_name);


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);

      if (!file.isEmpty())
        {
          mod_options.ge_name = file;
          geName->setText (mod_options.ge_name);
        }
    }
}



void
prefs::slotPositionClicked (int id)
{
  mod_options.position_form = id;
}



void
prefs::slotHighlightClicked (int id)
{
  mod_options.feature_search_type = id;
}



void
prefs::slotCloseContours ()
{
  //  Get the values.

  QString string;
  int32_t status, j;
  float tmp;

  j = 0;
  for (int32_t i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      string = contoursTable->item (i, 0)->text ();
      status = sscanf (string.toLatin1 (), "%f", &tmp);
      if (status == 1)
        {
          mod_share.contour_levels[j] = tmp;
          j++;
        }
    }

  mod_share.num_levels = j + 1;


  setFields ();

  contoursD->close ();
}



void
prefs::slotZFactorChanged (int index)
{
  //  If we changed the Z scaling factor and we have a Z offset set we need to divide the Z offsetby the old scale factor and then
  //  multiply the Z offset by the new scale factor otherwise things get decidedly screwy.

  mod_options.z_offset /= save_scale_factor;

  switch (index)
    {
    case 0:
      mod_options.z_factor = 1.0;
      break;

    case 1:
      mod_options.z_factor = 3.28084;
      break;

    case 2:
      mod_options.z_factor = 0.34080;
      break;

    case 3:
      mod_options.z_factor = 0.54681;
      break;

    case 4:
      mod_options.z_factor = 1.82880;
      break;

    case 5:
      mod_options.z_factor = 0.53333;
      break;
    }

  save_scale_factor = mod_options.z_factor;

  mod_options.z_offset *= save_scale_factor;

  offset->setValue (mod_options.z_offset);
}



void
prefs::slotOrientationClicked (int id)
{
  if (id)
    {
      mod_options.z_orientation = -1.0;
    }
  else
    {
      mod_options.z_orientation = 1.0;
    }
}



//!  This function is used to set user defined contour levels.

void
prefs::slotContourLevelsClicked ()
{
  if (contoursD) slotCloseContours ();


  QString string;

  mod_share.cint = 0.0;
  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  
  contoursD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  contoursD->setWindowTitle (tr ("pfmView Contour Levels"));

  QVBoxLayout *vbox = new QVBoxLayout (contoursD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  contoursTable = new QTableWidget (MAX_CONTOUR_LEVELS, 1, this);
  contoursTable->setWhatsThis (contoursTableText);
  contoursTable->setAlternatingRowColors (true);
  QTableWidgetItem *chItem = new QTableWidgetItem (tr ("Contour level"));
  contoursTable->setHorizontalHeaderItem (0, chItem);

  QTableWidgetItem *cntItem[MAX_CONTOUR_LEVELS];

  for (int32_t i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      if (i < mod_share.num_levels)
        {
          string.sprintf ("%f", mod_share.contour_levels[i]);
        }
      else
        {
          string.sprintf (" ");
        }
      cntItem[i] = new QTableWidgetItem (string); 
      contoursTable->setItem (i, 0, cntItem[i]);
    }
  contoursTable->resizeColumnsToContents ();


  vbox->addWidget (contoursTable, 1);

  int32_t w = qMin (800, contoursTable->columnWidth (0) + 60);
  contoursD->resize (w, 600);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (contoursD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), contoursD);
  closeButton->setToolTip (tr ("Close the contour levels dialog"));
  closeButton->setWhatsThis (closeContoursText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseContours ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  contoursD->move (x () + width () / 2 - contoursD->width () / 2, y () + height () / 2 - contoursD->height () / 2);

  contoursD->show ();
}



//!  This function sets the GeoTIFF transparency level (it uses a dummy GeoTIFF).

void 
prefs::slotGeotiff ()
{
  if (geotiffD) geotiffD->close ();


  geotiffD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  geotiffD->setWindowTitle (tr ("pfmView GeoTIFF transparency"));


  QImage tmp_layer1 = QImage (":/icons/average.png");
  layer1 = tmp_layer1.convertToFormat (QImage::Format_ARGB32);

  geotiff_w = layer1.width ();
  geotiff_h = layer1.height ();


  QImage tmp_layer2 = QImage (":/icons/geotiff.png");
  layer2 = tmp_layer2.convertToFormat (QImage::Format_ARGB32);


  geotiffD->resize (geotiff_w + 20, geotiff_h + 120);



  QVBoxLayout *vbox = new QVBoxLayout (geotiffD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QFrame *frame = new QFrame (geotiffD);
  frame->setFrameStyle (QFrame::Panel | QFrame::Sunken);


  //  Seriously cheating here - I'm just using the map class so I don't have to make a special widget just
  //  to paint the demo part of the transparency dialog.

  NVMAP_DEF mapdef;
  mapdef.projection = 3;
  mapdef.draw_width = geotiff_w;
  mapdef.draw_height = geotiff_h;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;
 
  mapdef.border = 0;
  mapdef.background_color = Qt::white;
  mapdef.landmask = NVFalse;
  mapdef.coasts = NVFalse;


  geotiff = new nvMap (geotiffD, &mapdef);
  connect (geotiff, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotPreRedraw (NVMAP_DEF)));

  geotiff->setWhatsThis (transparencyText);


  vbox->addWidget (frame);


  QVBoxLayout *fBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (geotiff);
  fBox->addLayout (hBox);



  QGroupBox *sbox = new QGroupBox (tr ("GeoTIFF transparency"), geotiffD);
  QHBoxLayout *sboxLayout = new QHBoxLayout;
  sbox->setLayout (sboxLayout);

  QString tmp;
  tmp.sprintf ("%03d", mod_options.GeoTIFF_alpha);
  geotiff_left = new QLabel (tmp, geotiffD);
  geotiff_left->setWhatsThis (transparencyText);
  sboxLayout->addWidget (geotiff_left);

  geotiffScroll = new QScrollBar (Qt::Horizontal, geotiffD);
  geotiffScroll->setRange (25, 255);
  geotiffScroll->setSliderPosition (mod_options.GeoTIFF_alpha);
  geotiffScroll->setTracking (NVFalse);
  connect (geotiffScroll, SIGNAL (valueChanged (int)), this, SLOT (slotGeotiffScrollValueChanged (int)));
  connect (geotiffScroll, SIGNAL (sliderMoved (int)), this, SLOT (slotGeotiffScrollSliderMoved (int)));
  geotiffScroll->setToolTip (tr ("Move to change the amount of transparency in the GeoTIFF overlay"));
  geotiffScroll->setWhatsThis (transparencyText);
  sboxLayout->addWidget (geotiffScroll, 10);

  QLabel *right = new QLabel (tr ("Opaque"), geotiffD);
  right->setWhatsThis (transparencyText);
  sboxLayout->addWidget (right);


  vbox->addWidget (sbox);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (geotiffD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), geotiffD);
  closeButton->setToolTip (tr ("Close the GeoTIFF transparancy dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseGeotiff ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  geotiffD->move (x () + width () / 2 - geotiffD->width () / 2, y () + height () / 2 - geotiffD->height () / 2);

  geotiffD->show ();

  geotiff->enableSignals ();

  geotiff->redrawMap (NVTrue);
}



void 
prefs::slotPreRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  QColor pixel;


  for (int32_t i = 0 ; i < geotiff_w ; i++)
    {
      for (int32_t j = 0 ; j < geotiff_h ; j++)
        {
          pixel = QColor (layer2.pixel (i, j));
          pixel.setAlpha (mod_options.GeoTIFF_alpha);
          layer2.setPixel (i, j, pixel.rgba ());
        }
    }

  QPixmap lay1 = QPixmap::fromImage (layer1);
  QPixmap lay2 = QPixmap::fromImage (layer2);

  geotiff->drawPixmap (0, 0, &lay1, 0, 0, geotiff_w, geotiff_h, NVTrue);
  geotiff->drawPixmap (0, 0, &lay2, 0, 0, geotiff_w, geotiff_h, NVTrue);

  geotiff->setCursor (Qt::ArrowCursor);
}



void 
prefs::slotGeotiffScrollValueChanged (int value)
{
  mod_options.GeoTIFF_alpha = value;

  geotiff->redrawMap (NVTrue);
}



void 
prefs::slotGeotiffScrollSliderMoved (int value)
{
  QString tmp;
  tmp.sprintf ("%03d", value);
  geotiff_left->setText (tmp);
}



void 
prefs::slotCloseGeotiff ()
{
  geotiffD->close ();
}



//!  This is the hotkey changing dialog.

void 
prefs::slotHotKeys ()
{
  if (hotKeyD) slotCloseHotKeys ();

  hotKeyD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  hotKeyD->setWindowTitle (tr ("pfmView Hot Keys"));

  QVBoxLayout *vbox = new QVBoxLayout (hotKeyD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  hotKeyTable = new QTableWidget (HOTKEYS, 2, this);
  hotKeyTable->setWhatsThis (hotKeyTableText);
  hotKeyTable->setAlternatingRowColors (true);
  QTableWidgetItem *bItemH = new QTableWidgetItem (tr ("Button"));
  hotKeyTable->setHorizontalHeaderItem (0, bItemH);
  QTableWidgetItem *kItemH = new QTableWidgetItem (tr ("Key"));
  hotKeyTable->setHorizontalHeaderItem (1, kItemH);

  QTableWidgetItem *bItem[HOTKEYS], *kItem[HOTKEYS];


  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      bItem[i] = new QTableWidgetItem (mod_misc.buttonIcon[i], mod_misc.buttonText[i]);
      bItem[i]->setFlags (Qt::ItemIsEnabled);

      hotKeyTable->setItem (i, 0, bItem[i]);
      kItem[i] = new QTableWidgetItem (mod_options.buttonAccel[i]);
      hotKeyTable->setItem (i, 1, kItem[i]);
    }
  hotKeyTable->resizeColumnsToContents ();
  hotKeyTable->resizeRowsToContents ();

  vbox->addWidget (hotKeyTable, 1);

  int32_t w = qMin (800, hotKeyTable->columnWidth (0) + hotKeyTable->columnWidth (1) + 40);
  int32_t h = qMin (600, hotKeyTable->rowHeight (0) * HOTKEYS + 75);
  hotKeyD->resize (w, h);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (hotKeyD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), hotKeyD);
  closeButton->setToolTip (tr ("Close the hot key dialog"));
  closeButton->setWhatsThis (closeHotKeyText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseHotKeys ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  hotKeyD->move (x () + width () / 2 - hotKeyD->width () / 2, y () + height () / 2 - hotKeyD->height () / 2);

  hotKeyD->show ();
}



void 
prefs::slotCloseHotKeys ()
{
  for (int32_t i = 0 ; i < HOTKEYS ; i++) mod_options.buttonAccel[i] = hotKeyTable->item (i, 1)->text ();

  hotKeyD->close ();
}



//!  Get the EGM value nearest to the center of the displayed area.

void 
prefs::slotEgmClicked ()
{
  double lat, lon;

  lat = misc->total_displayed_area.min_y + (misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / 2.0;
  lon = misc->total_displayed_area.min_x + (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / 2.0;

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();

  float datum_offset = get_egm08 (lat, lon);
  cleanup_egm08 ();

  qApp->restoreOverrideCursor ();

  if (datum_offset < 999999.0) 
    {
      offset->setValue ((double) datum_offset);
    }
  else
    {
      QMessageBox::warning (this, "pfmView", tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
    }
}



//!  Preliminary field/button setting function.

void 
prefs::setFields ()
{
  QString string;


  //  Save the scale factor so we can play around with it and the offset value without messing up the input values.

  save_scale_factor = mod_options.z_factor;


  bGrp->button (mod_options.position_form)->setChecked (true);


  int32_t hue, sat, val;

  mod_options.coast_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCoastPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.coast_color);
  bCoastPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.coast_color);
  bCoastColor->setPalette (bCoastPalette);


  mod_options.contour_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bContourPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bContourPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bContourPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bContourPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bContourPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.contour_color);
  bContourPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.contour_color);
  bContourColor->setPalette (bContourPalette);


  mod_options.overlay_grid_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bGridPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bGridPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bGridPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bGridPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bGridPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.overlay_grid_color);
  bGridPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.overlay_grid_color);
  bGridColor->setPalette (bGridPalette);


  mod_options.feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_color);
  bFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_color);
  bFeatureColor->setPalette (bFeaturePalette);


  mod_options.verified_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bVerFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.verified_feature_color);
  bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.verified_feature_color);
  bVerFeatureColor->setPalette (bVerFeaturePalette);


  mod_options.cov_verified_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_verified_feature_color);
  bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_verified_feature_color);
  bCovVerFeatureColor->setPalette (bCovVerFeaturePalette);


  mod_options.feature_info_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_info_color);
  bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_info_color);
  bFeatureInfoColor->setPalette (bFeatureInfoPalette);


  mod_options.feature_poly_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_poly_color);
  bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_poly_color);
  bFeaturePolyColor->setPalette (bFeaturePolyPalette);


  mod_options.background_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bBackgroundPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.background_color);
  bBackgroundPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.background_color);
  bBackgroundColor->setPalette (bBackgroundPalette);


  mod_options.contour_highlight_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bContourHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bContourHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bContourHighlightPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.contour_highlight_color);
  bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.contour_highlight_color);
  bContourHighlightColor->setPalette (bContourHighlightPalette);


  mod_options.poly_filter_mask_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bMaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bMaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bMaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bMaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bMaskPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.poly_filter_mask_color);
  bMaskPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.poly_filter_mask_color);
  bMaskColor->setPalette (bMaskPalette);


  mod_options.cov_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_feature_color);
  bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_feature_color);
  bCovFeatureColor->setPalette (bCovFeaturePalette);


  mod_options.cov_coast_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovCoastPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_coast_color);
  bCovCoastPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_coast_color);
  bCovCoastColor->setPalette (bCovCoastPalette);


  mod_options.cov_box_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovBoxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovBoxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovBoxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovBoxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovBoxPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_box_color);
  bCovBoxPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_box_color);
  bCovBoxColor->setPalette (bCovBoxPalette);


  mod_options.cov_data_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovDataPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovDataPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovDataPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovDataPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovDataPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_data_color);
  bCovDataPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_data_color);
  bCovDataColor->setPalette (bCovDataPalette);


  mod_options.cov_background_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovBackgroundPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_background_color);
  bCovBackgroundPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_background_color);
  bCovBackgroundColor->setPalette (bCovBackgroundPalette);


  mod_options.cov_checked_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovCheckedPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovCheckedPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovCheckedPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovCheckedPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovCheckedPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_checked_color);
  bCovCheckedPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_checked_color);
  bCovCheckedColor->setPalette (bCovCheckedPalette);


  mod_options.cov_verified_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovVerifiedPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovVerifiedPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovVerifiedPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovVerifiedPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovVerifiedPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_verified_color);
  bCovVerifiedPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_verified_color);
  bCovVerifiedColor->setPalette (bCovVerifiedPalette);


  mod_options.cov_pfm_box_hot_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovHotPFMPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovHotPFMPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovHotPFMPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovHotPFMPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovHotPFMPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_pfm_box_hot_color);
  bCovHotPFMPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_pfm_box_hot_color);
  bCovHotPFMColor->setPalette (bCovHotPFMPalette);


  mod_options.cov_pfm_box_cold_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovColdPFMPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovColdPFMPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovColdPFMPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovColdPFMPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovColdPFMPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_pfm_box_cold_color);
  bCovColdPFMPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_pfm_box_cold_color);
  bCovColdPFMColor->setPalette (bCovColdPFMPalette);


  mod_options.cov_inv_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_inv_feature_color);
  bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_inv_feature_color);
  bCovInvFeatureColor->setPalette (bCovInvFeaturePalette);


  //  Before we do the stoplight colors we want to find the nearest one in the color array so we can use the already
  //  defined shades to do sunshading.

  for (int32_t j = 0 ; j < 3 ; j++)
    {
      QColor tmp;

      switch (j)
        {
        case 0:
          tmp = mod_options.stoplight_min_color;
          break;

        case 1:
          tmp = mod_options.stoplight_mid_color;
          break;

        case 2:
          tmp = mod_options.stoplight_max_color;
          break;
        }

      int32_t min_hue_diff = 99999;
      int32_t closest = -1;

      for (int32_t i = 0 ; i < NUMHUES ; i++)
        {
          int32_t hue_diff = abs (tmp.hue () - mod_options.color_array[0][i][255].hue ());

          if (hue_diff < min_hue_diff)
            {
              min_hue_diff = hue_diff;
              closest = i;
            }
        }

      switch (j)
        {
        case 0:
          mod_options.stoplight_min_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_min_index = closest;
          break;

        case 1:
          tmp = mod_options.stoplight_mid_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_mid_index = closest;
          break;

        case 2:
          tmp = mod_options.stoplight_max_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_max_index = closest;
          break;
        }
    }



  mod_options.stoplight_min_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMinPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_min_color);
  bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_min_color);
  bStoplightMinColor->setPalette (bStoplightMinPalette);

  mod_options.stoplight_mid_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMidPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_mid_color);
  bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_mid_color);
  bStoplightMidColor->setPalette (bStoplightMidPalette);

  mod_options.stoplight_max_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_max_color);
  bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_max_color);
  bStoplightMaxColor->setPalette (bStoplightMaxPalette);


  mod_options.scaled_offset_z_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bScaledOffsetZPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bScaledOffsetZPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bScaledOffsetZPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bScaledOffsetZPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bScaledOffsetZPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.scaled_offset_z_color);
  bScaledOffsetZPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.scaled_offset_z_color);
  bScaledOffsetZColor->setPalette (bScaledOffsetZPalette);


  QString fontString = tr ("Font : ") + mod_options.font.toString ();
  bFont->setText (fontString);


  if (mod_options.z_orientation > 0.0)
    {
      depthCheck->setChecked (true);
    }
  else
    {
      elevationCheck->setChecked (true);
    }

  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  contourSm->setValue (mod_options.smoothing_factor);

  cfilt->setValue (mod_options.contour_filter_envelope);

  if (fabs (mod_options.z_factor - 1.0) < 0.0000001)
    {
      ZFactor->setCurrentIndex (0);
    }
  else if (fabs (mod_options.z_factor - 3.28084) < 0.0000001)
    {
      ZFactor->setCurrentIndex (1);
    }
  else if (fabs (mod_options.z_factor - 0.34080) < 0.0000001)
    {
      ZFactor->setCurrentIndex (2);
    }
  else if (fabs (mod_options.z_factor - 0.54681) < 0.0000001)
    {
      ZFactor->setCurrentIndex (3);
    }
  else if (fabs (mod_options.z_factor - 1.82880) < 0.0000001)
    {
      ZFactor->setCurrentIndex (4);
    }
  else if (fabs (mod_options.z_factor - 0.53333) < 0.0000001)
    {
      ZFactor->setCurrentIndex (5);
    }

  offset->setValue (mod_options.z_offset);

  string.sprintf ("%d", mod_options.contour_width);
  Width->lineEdit ()->setText (string);

  string.sprintf ("%d", mod_options.contour_index);
  Index->lineEdit ()->setText (string);

  string.sprintf ("%d", mod_options.misp_weight);
  WFactor->lineEdit ()->setText (string);
  force->setChecked (mod_options.misp_force_original);
  replace->setChecked (mod_options.misp_replace_all);

  zero->setChecked (mod_options.zero_turnover);

  filterSTD->setValue (mod_options.filterSTD);

  dFilter->setChecked (mod_options.deep_filter_only);

  featureRadius->setValue (mod_options.feature_radius);

  hPercent->setValue (mod_options.highlight_percent);

  hCount->setValue (mod_options.h_count);

  fixedSize->setChecked (mod_options.fixed_feature_size);

  if (mod_options.fixed_feature_size)
    {
      milli->setEnabled (false);
      chartScale->setEnabled (false);
    }
  else
    {
      milli->setEnabled (true);
      chartScale->setEnabled (true);
    }

  milli->setValue (mod_options.millimeters);
  chartScale->setValue (mod_options.chart_scale);

  grid->setCurrentIndex (mod_options.overlay_grid_type);
  gridMeters->setValue (mod_options.overlay_grid_spacing);
  gridMinutes->setValue (mod_options.overlay_grid_minutes);

  geName->setText (mod_options.ge_name);

  dszWidth->setValue (mod_options.default_width);
  dszHeight->setValue (mod_options.default_height);
  dChartScale->setValue (mod_options.default_chart_scale);

  overlap->setValue (mod_options.overlap_percent);

  sunAz->setValue (mod_options.sunopts.azimuth);
  sunEl->setValue (mod_options.sunopts.elevation);
  sunEx->setValue (mod_options.sunopts.exag);

  textSearch->setText (mod_options.feature_search_string);


  switch (mod_options.main_button_icon_size)
    {
    case 16:
      iconSize->setCurrentIndex (0);
      break;

    case 20:
      iconSize->setCurrentIndex (1);
      break;

    case 24:
    default:
      iconSize->setCurrentIndex (2);
      break;

    case 28:
      iconSize->setCurrentIndex (3);
      break;

    case 32:
      iconSize->setCurrentIndex (4);
      break;
    }

  unload->setChecked (mod_options.auto_close_unload);

  ihoOrder->setCurrentIndex (mod_options.iho);
  queue->setValue (mod_options.queue);
  capture->setValue (mod_options.capture);
  horiz->setValue (mod_options.horiz);
  distance->setValue (mod_options.distance);
  minContext->setValue (mod_options.min_context);
  maxContext->setValue (mod_options.max_context);
  std2conf->setCurrentIndex (mod_options.std2conf);
  disambiguation->setCurrentIndex (mod_options.disambiguation);

  hGrp->button (mod_options.feature_search_type)->setChecked (true);
  invert->setChecked (mod_options.feature_search_invert);
}



//!  Get the values from the fields/buttons and issue a change signal if needed.

void
prefs::slotApplyPrefs ()
{
  int32_t tmp_i, status;
  float tmp_f;
  uint8_t feature_search_changed = NVFalse;


  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;


  if (mod_options.z_orientation != options->z_orientation) dataChanged = NVTrue;

  QString tmp = contourInt->text ();
  status = sscanf (tmp.toLatin1 (), "%f", &tmp_f);
  if (status == 1) mod_share.cint = tmp_f;
  if (mod_share.cint != misc->abe_share->cint) dataChanged = NVTrue;

  tmp = contourSm->text ();
  status = sscanf (tmp.toLatin1 (), "%d", &tmp_i);
  if (status == 1) mod_options.smoothing_factor = tmp_i;
  if (mod_options.smoothing_factor != options->smoothing_factor) dataChanged = NVTrue;

  tmp = cfilt->text ();
  status = sscanf (tmp.toLatin1 (), "%f", &tmp_f);
  if (status == 1) mod_options.contour_filter_envelope = tmp_f;
  if (mod_options.contour_filter_envelope != options->contour_filter_envelope) dataChanged = NVTrue;


  //  We don't have to retrieve this since we have slotZFactorChanged.

  if (mod_options.z_factor != options->z_factor) dataChanged = NVTrue;


  mod_options.z_offset = offset->value ();
  if (mod_options.z_offset != options->z_offset) dataChanged = NVTrue;


  tmp = Width->currentText ();
  status = sscanf (tmp.toLatin1 (), "%d", &tmp_i);
  if (status == 1) mod_options.contour_width = tmp_i;
  if (mod_options.contour_width != options->contour_width) dataChanged = NVTrue;


  tmp = Index->currentText ();
  status = sscanf (tmp.toLatin1 (), "%d", &tmp_i);
  if (status == 1) mod_options.contour_index = tmp_i;
  if (mod_options.contour_index != options->contour_index) dataChanged = NVTrue;


  tmp = WFactor->currentText ();
  status = sscanf (tmp.toLatin1 (), "%d", &tmp_i);
  if (status == 1) mod_options.misp_weight = tmp_i;
  if (mod_options.misp_weight != options->misp_weight) dataChanged = NVTrue;

  mod_options.misp_force_original = force->isChecked ();
  if (mod_options.misp_force_original != options->misp_force_original) dataChanged = NVTrue;

  mod_options.misp_replace_all = replace->isChecked ();
  if (mod_options.misp_replace_all != options->misp_replace_all) dataChanged = NVTrue;

  mod_options.zero_turnover = zero->isChecked ();
  if (mod_options.zero_turnover != options->zero_turnover) dataChanged = NVTrue;

  mod_options.filterSTD = filterSTD->value ();
  if (mod_options.filterSTD != options->filterSTD) dataChanged = NVTrue;

  mod_options.deep_filter_only = dFilter->isChecked ();
  if (mod_options.deep_filter_only != options->deep_filter_only) dataChanged = NVTrue;

  mod_options.feature_radius = featureRadius->value ();
  if (mod_options.feature_radius != options->feature_radius) dataChanged = NVTrue;

  mod_options.highlight_percent = hPercent->value ();
  if (mod_options.highlight_percent != options->highlight_percent) dataChanged = NVTrue;

  mod_options.h_count = hCount->value ();
  if (mod_options.h_count != options->h_count) dataChanged = NVTrue;

  mod_options.chart_scale = chartScale->value ();
  if (mod_options.chart_scale != options->chart_scale) dataChanged = NVTrue;

  mod_options.millimeters = milli->value ();
  if (mod_options.millimeters != options->millimeters) dataChanged = NVTrue;

  if (mod_options.fixed_feature_size != options->fixed_feature_size) dataChanged = NVTrue;

  mod_options.overlay_grid_type = grid->currentIndex ();
  if (mod_options.overlay_grid_type != options->overlay_grid_type) dataChanged = NVTrue;
  mod_options.overlay_grid_spacing = gridMeters->value ();
  if (mod_options.overlay_grid_spacing != options->overlay_grid_spacing) dataChanged = NVTrue;
  mod_options.overlay_grid_minutes = gridMinutes->value ();
  if (mod_options.overlay_grid_minutes != options->overlay_grid_minutes) dataChanged = NVTrue;

  if (mod_options.ge_name != options->ge_name) dataChanged = NVTrue;

  mod_options.default_chart_scale = dChartScale->value ();
  if (mod_options.default_chart_scale != options->default_chart_scale) dataChanged = NVTrue;
  mod_options.default_width = dszWidth->value ();
  if (mod_options.default_width != options->default_width) dataChanged = NVTrue;
  mod_options.default_height = dszHeight->value ();
  if (mod_options.default_height != options->default_height) dataChanged = NVTrue;


  tmp = overlap->text ();
  status = sscanf (tmp.toLatin1 (), "%d", &tmp_i);
  if (status == 1) mod_options.overlap_percent = tmp_i;
  if (mod_options.overlap_percent != options->overlap_percent) dataChanged = NVTrue;


  mod_options.sunopts.azimuth = sunAz->value ();
  if (mod_options.sunopts.azimuth != options->sunopts.azimuth) dataChanged = NVTrue;
  mod_options.sunopts.elevation = sunEl->value ();
  if (mod_options.sunopts.elevation != options->sunopts.elevation) dataChanged = NVTrue;
  mod_options.sunopts.exag = sunEx->value ();
  if (mod_options.sunopts.exag != options->sunopts.exag) dataChanged = NVTrue;


  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);


  if (mod_options.feature_search_type != options->feature_search_type) dataChanged = NVTrue;
  mod_options.feature_search_invert = invert->isChecked ();
  if (mod_options.feature_search_invert != options->feature_search_invert) dataChanged = NVTrue;


  mod_options.feature_search_string = textSearch->text ();
  if (mod_options.feature_search_string != options->feature_search_string || mod_options.feature_search_type != options->feature_search_type ||
      mod_options.feature_search_invert != options->feature_search_invert)
    {
      dataChanged = NVTrue;
      feature_search_changed = NVTrue;
    }


  switch (iconSize->currentIndex ())
    {
    case 0:
      mod_options.main_button_icon_size = 16;
      break;

    case 1:
      mod_options.main_button_icon_size = 20;
      break;

    case 2:
      mod_options.main_button_icon_size = 24;
      break;

    case 3:
      mod_options.main_button_icon_size = 28;
      break;

    case 4:
      mod_options.main_button_icon_size = 32;
      break;
    }
  if (mod_options.main_button_icon_size != options->main_button_icon_size) dataChanged = NVTrue;

  mod_options.auto_close_unload = unload->isChecked ();
  if (mod_options.auto_close_unload != options->auto_close_unload) dataChanged = NVTrue;

  mod_options.iho = ihoOrder->currentIndex ();
  if (mod_options.iho != options->iho) dataChanged = NVTrue;
  mod_options.capture = capture->value ();
  if (mod_options.capture != options->capture) dataChanged = NVTrue;
  mod_options.queue = queue->value ();
  if (mod_options.queue != options->queue) dataChanged = NVTrue;
  mod_options.horiz = horiz->value ();
  if (mod_options.horiz != options->horiz) dataChanged = NVTrue;
  mod_options.distance = distance->value ();
  if (mod_options.distance != options->distance) dataChanged = NVTrue;
  mod_options.min_context = minContext->value ();
  if (mod_options.min_context != options->min_context) dataChanged = NVTrue;
  mod_options.max_context = maxContext->value ();
  if (mod_options.max_context != options->max_context) dataChanged = NVTrue;
  mod_options.std2conf = std2conf->currentIndex ();
  if (mod_options.std2conf != options->std2conf) dataChanged = NVTrue;
  mod_options.disambiguation = disambiguation->currentIndex ();
  if (mod_options.disambiguation != options->disambiguation) dataChanged = NVTrue;


  if (mod_options.GeoTIFF_alpha != options->GeoTIFF_alpha)
    {
      dataChanged = NVTrue;
      mod_misc.GeoTIFF_init = NVTrue;
    }


  if (mod_options.contour_color != options->contour_color) dataChanged = NVTrue;
  if (mod_options.overlay_grid_color != options->overlay_grid_color) dataChanged = NVTrue;
  if (mod_options.coast_color != options->coast_color) dataChanged = NVTrue;
  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.feature_color != options->feature_color) dataChanged = NVTrue;
  if (mod_options.feature_info_color != options->feature_info_color) dataChanged = NVTrue;
  if (mod_options.feature_poly_color != options->feature_poly_color) dataChanged = NVTrue;
  if (mod_options.contour_highlight_color != options->contour_highlight_color) dataChanged = NVTrue;
  if (mod_options.poly_filter_mask_color != options->poly_filter_mask_color) dataChanged = NVTrue;
  if (mod_options.cov_feature_color != options->cov_feature_color) dataChanged = NVTrue;
  if (mod_options.cov_inv_feature_color != options->cov_inv_feature_color) dataChanged = NVTrue;
  if (mod_options.verified_feature_color != options->verified_feature_color) dataChanged = NVTrue;
  if (mod_options.cov_verified_feature_color != options->cov_verified_feature_color) dataChanged = NVTrue;
  if (mod_options.stoplight_min_color != options->stoplight_min_color) dataChanged = NVTrue;
  if (mod_options.stoplight_mid_color != options->stoplight_mid_color) dataChanged = NVTrue;
  if (mod_options.stoplight_max_color != options->stoplight_max_color) dataChanged = NVTrue;
  if (mod_options.cov_coast_color != options->cov_coast_color) dataChanged = NVTrue;
  if (mod_options.cov_box_color != options->cov_box_color) dataChanged = NVTrue;
  if (mod_options.cov_data_color != options->cov_data_color) dataChanged = NVTrue;
  if (mod_options.cov_background_color != options->cov_background_color) dataChanged = NVTrue;
  if (mod_options.cov_checked_color != options->cov_checked_color) dataChanged = NVTrue;
  if (mod_options.cov_verified_color != options->cov_verified_color) dataChanged = NVTrue;
  if (mod_options.cov_pfm_box_hot_color != options->cov_pfm_box_hot_color) dataChanged = NVTrue;
  if (mod_options.cov_pfm_box_cold_color != options->cov_pfm_box_cold_color) dataChanged = NVTrue;
  if (mod_options.scaled_offset_z_color != options->scaled_offset_z_color) dataChanged = NVTrue;
  if (mod_options.font != options->font) dataChanged = NVTrue;



  //  Make sure we have no hotkey duplications.

  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      for (int32_t j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (mod_options.buttonAccel[i].toUpper () == mod_options.buttonAccel[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("pfmView preferences"),
                                    tr ("Hot key for <b>%1</b> button conflicts with hotkey for <b>%2</b> button!<br><br>"
                                        "Resetting button hotkeys for these two buttons.").arg (misc->buttonText[i]).arg (misc->buttonText[j]));

              mod_options.buttonAccel[i] = options->buttonAccel[i];
              mod_options.buttonAccel[j] = options->buttonAccel[j];
              dataChanged = NVFalse;

              hotKeyD->close ();
              hotKeyD = NULL;

              return;
            }
        }
    }


  //  Now check for changes to the hotkeys.

  for (int32_t i = 0 ; i < HOTKEYS ; i++)
    {
      if (mod_options.buttonAccel[i] != options->buttonAccel[i])
        {
          dataChanged = NVTrue;
          emit hotKeyChangedSignal (i);
          break;
        }
    }


  if (mod_share.num_levels != misc->abe_share->num_levels) dataChanged = NVTrue;


  for (int32_t i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      if (mod_share.contour_levels[i] != misc->abe_share->contour_levels[i])
        {
          dataChanged = NVTrue;
          break;
        }
    }


  setFields ();


  hide ();


  if (dataChanged)
    {
      *misc->abe_share = mod_share;
      *options = mod_options;
      *misc = mod_misc;

      emit dataChangedSignal (feature_search_changed);
    }

  close ();
}



void
prefs::slotClosePrefs ()
{
  close ();
}
