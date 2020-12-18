
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



#include "unloadDialog.hpp"


/*!
  This dialog keeps track of unloading of PFM files in a separate dialog so that the user can move on with editing while
  the unload process runs in the background.
*/

unloadDialog::unloadDialog (QWidget *parent, OPTIONS *op, MISC *mi, int32_t dlg, int32_t pfm):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  dialog = dlg;


  unloadProc = NULL;


  QString filename = QString (pfm_basename (misc->abe_share->open_args[pfm].list_path));


  setWindowTitle (tr ("pfmView Unload %1").arg (filename));


  setSizeGripEnabled (true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QLabel *label = new QLabel (this);;
  label->setPixmap (misc->unload_cov_map);


  vbox->addWidget (label);


  unloadList = new QTextEdit (this);


  vbox->addWidget (unloadList, 1);


  QHBoxLayout *actions = new QHBoxLayout (0);
  actions->setMargin (5);
  actions->setSpacing (5);
  vbox->addLayout (actions);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  if (unloadProc != NULL) delete unloadProc;

  unloadProc = new QProcess (this);


  QStringList arguments;
  QString arg;


  if (misc->unload_type[dialog])
    {
      arg.sprintf ("-a %.9f,%.9f,%.9f,%.9f", misc->unload_area[dialog].max_y, misc->unload_area[dialog].min_y, misc->unload_area[dialog].max_x,
                   misc->unload_area[dialog].min_x);
      arguments += arg;
    }


  //  Make sure that the file name is last on the command line.  For some reason getopt on Windows doesn't like it otherwise.

  char native_path[1024];
  QString nativePath;


  //  Convert to native separators so that we get nice file names on Windows.

  nativePath = QDir::toNativeSeparators (QString (misc->abe_share->open_args[pfm].list_path));
  strcpy (native_path, nativePath.toLatin1 ());

  arg.sprintf ("""%s""", native_path);
  arguments += arg;


  connect (unloadProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotUnloadDone (int, QProcess::ExitStatus)));
  connect (unloadProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotUnloadReadyReadStandardError ()));
  connect (unloadProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotUnloadError (QProcess::ProcessError)));


  unloadProc->start (QString (options->unload_prog), arguments);


  this->resize (misc->unload_window_width, misc->unload_window_height);
  this->move (misc->unload_window_x, misc->unload_window_y);


  show ();
}



unloadDialog::~unloadDialog ()
{
}



void 
unloadDialog::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  misc->unload_type[dialog] = -1;
}



void 
unloadDialog::get_bounds ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  misc->unload_window_x = tmp.x ();
  misc->unload_window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  misc->unload_window_width = tmp.width ();
  misc->unload_window_height = tmp.height ();
}



//!  Error callback from the unload process.

void 
unloadDialog::slotUnloadError (QProcess::ProcessError error)
{
  get_bounds ();
  emit unloadCompleteSignal (dialog, error);
}



//!  This is the return from the unload QProcess (when finished)

void 
unloadDialog::slotUnloadDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  get_bounds ();

  unloadProc->closeReadChannel (QProcess::StandardError);


  if (unloadProc->exitCode ())
    {
      emit unloadCompleteSignal (dialog, -1);
    }
  else
    {
      emit unloadCompleteSignal (dialog, -99);
    }
}



/*!
  This handles standard error read from the unload QProcess.
*/

void 
unloadDialog::slotUnloadReadyReadStandardError ()
{
  static QString resp_string = "";
  static uint8_t last_line = NVFalse;


  QByteArray response = unloadProc->readAllStandardError ();


  //  Parse the return response for carriage returns and line feeds

  for (int32_t i = 0 ; i < response.length () ; i++)
    {
      if (response.at (i) == '\n')
        {
          if (last_line)
            {
              unloadList->moveCursor (QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
              unloadList->moveCursor (QTextCursor::End, QTextCursor::KeepAnchor);
              unloadList->textCursor ().removeSelectedText ();
            }

          last_line = NVFalse;

          unloadList->append (resp_string);

          resp_string = "";
        }
      else if (response.at (i) == '\r')
        {
          uint8_t wincheck = NVTrue;


          //  CR/LF check for Windows.

#ifdef NVWIN3X

          if (i != response.length () - 1)
            {
              if (response.at (i + 1) == '\n')
                {
                  wincheck = NVFalse;
                }
            }

#endif

          if (wincheck)
            {
              if (last_line)
                {
                  unloadList->moveCursor (QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                  unloadList->moveCursor (QTextCursor::End, QTextCursor::KeepAnchor);
                  unloadList->textCursor ().removeSelectedText ();
                }

              last_line = NVTrue;

              unloadList->insertPlainText (resp_string); 

              resp_string = "";
            }
        }
      else
        {
          resp_string += response.at (i);
        }
    }
}



void
unloadDialog::slotClose ()
{
  misc->unload_type[dialog] = -1;
  close ();
}
