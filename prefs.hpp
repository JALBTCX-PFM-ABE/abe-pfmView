
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



#ifndef PREFS_H
#define PREFS_H

#include "pfm.h"
#include "pfmViewDef.hpp"


class prefs:public QDialog
{
  Q_OBJECT 


    public:

  prefs (QWidget *parent = 0, OPTIONS *op = NULL, MISC *mi = NULL);
  ~prefs ();


 signals:

  void dataChangedSignal (uint8_t feature_search_changed);
  void hotKeyChangedSignal (int32_t i);


 protected:

  ABE_SHARE       mod_share;

  OPTIONS         *options, mod_options;

  MISC            *misc, mod_misc;

  QTabWidget      *prefTab;

  QButtonGroup    *bGrp, *cGrp, *hGrp, *oGrp;

  QDialog         *contoursD, *geotiffD, *cubeD, *hotKeyD;

  nvMap           *geotiff;

  QImage          layer1, layer2;

  QScrollBar      *geotiffScroll;

  int32_t         geotiff_w, geotiff_h;

  uint8_t         dataChanged;

  float           save_scale_factor;

  QTableWidget    *contoursTable, *hotKeyTable;

  QPushButton     *bTextColor, *bContourColor, *bGridColor, *bFeatureColor, *bVerFeatureColor, *bCovVerFeatureColor, *bFeatureInfoColor, *bFeaturePolyColor,
                  *bCoastColor, *bBackgroundColor, *bContourHighlightColor, *bMaskColor, *bCovFeatureColor, *bCovCoastColor, *bCovDataColor,
                  *bCovBackgroundColor, *bCovCheckedColor, *bCovVerifiedColor, *bCovInvFeatureColor, *bStoplightMinColor, *bStoplightMidColor,
                  *bCovHotPFMColor, *bCovColdPFMColor, *bCovBoxColor, *bStoplightMaxColor, *bScaledOffsetZColor, *bFont, *bRestoreDefaults, *geBrowse;

  QPalette        bContourPalette, bGridPalette, bFeaturePalette, bVerFeaturePalette, bCovVerFeaturePalette, bFeatureInfoPalette, bFeaturePolyPalette,
                  bCoastPalette, bBackgroundPalette, bContourHighlightPalette, bMaskPalette, bCovFeaturePalette, bCovCoastPalette, bCovDataPalette,
                  bCovBackgroundPalette, bCovCheckedPalette, bCovVerifiedPalette, bCovInvFeaturePalette, bCovHotPFMPalette, bCovColdPFMPalette,
                  bCovBoxPalette, bStoplightMinPalette, bStoplightMidPalette, bStoplightMaxPalette, bScaledOffsetZPalette;

  QPushButton     *bContourLevels, *bGeotiff, *bCube, *bHotKeys;

  QCheckBox       *force, *replace, *zero, *unload, *dFilter, *invert, *depthCheck, *elevationCheck, *fixedSize;

  QLineEdit       *contourInt, *textSearch, *geName;
  
  QDoubleSpinBox  *sunEx, *sunAz, *sunEl, *offset, *filterSTD, *featureRadius, *hPercent, *capture, *horiz, *distance, *minContext,
                  *maxContext, *cfilt, *gridMeters, *gridMinutes, *dszWidth, *dszHeight;

  QComboBox       *ZFactor, *WFactor, *Width, *Index, *iconSize, *ihoOrder, *std2conf, *disambiguation, *grid;

  QSpinBox        *contourSm, *overlap, *queue, *milli, *chartScale, *hCount, *dChartScale;

  QPixmap         helpIcon;

  QLabel          *geotiff_left;

  void setFields ();


  protected slots:

  void slotPrefTabChanged (int tab);
  void slotContourColor ();
  void slotGridColor ();
  void slotFeatureColor ();
  void slotVerFeatureColor ();
  void slotCovVerFeatureColor ();
  void slotFeatureInfoColor ();
  void slotFeaturePolyColor ();
  void slotCoastColor ();
  void slotBackgroundColor ();
  void slotContourHighlightColor ();
  void slotMaskColor ();
  void slotCovFeatureColor ();
  void slotCovCoastColor ();
  void slotCovBoxColor ();
  void slotCovDataColor ();
  void slotCovBackgroundColor ();
  void slotCovCheckedColor ();
  void slotCovVerifiedColor ();
  void slotCovHotPFMColor ();
  void slotCovColdPFMColor ();
  void slotCovInvFeatureColor ();
  void slotStoplightMinColor ();
  void slotStoplightMidColor ();
  void slotStoplightMaxColor ();
  void slotScaledOffsetZColor ();
  void slotFont ();
  void slotFixedSizeChanged (int state);
  void slotHelp ();
  void slotRestoreDefaults ();
  void slotGeBrowseClicked ();
  void slotPositionClicked (int id);
  void slotHighlightClicked (int id);
  void slotZFactorChanged (int index);
  void slotOrientationClicked (int id);
  void slotContourLevelsClicked ();
  void slotGeotiff ();
  void slotPreRedraw (NVMAP_DEF mapdef);
  void slotGeotiffScrollValueChanged (int value);
  void slotGeotiffScrollSliderMoved (int value);
  void slotCloseGeotiff ();
  void slotHotKeys ();
  void slotEgmClicked ();
  void slotCloseHotKeys ();
  void slotCloseContours ();
  void slotApplyPrefs ();
  void slotClosePrefs ();

 private:
};

#endif
