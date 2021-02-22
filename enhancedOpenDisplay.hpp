/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   enhancedOpen.hpp
 * Author: Russ.Johnson
 *
 * Created on January 28, 2021, 8:48 AM
 */

#ifndef ENHANCEDOPEN_HPP
#define ENHANCEDOPEN_HPP

#include <QtGui>
#include <QLabel>

#include "pfmViewDef.hpp"

class enhancedOpenDisplay : public QDialog {
    Q_OBJECT
    
public:
    
    enhancedOpenDisplay(OPTIONS *options, MISC *misc);
    QDialog::DialogCode showMe();
    QStringList selectedFiles();
    QString defaultOpenMode();
    QString getLastScannedDirectory();
    QString getCurrentFilter();
    
    
private:
    
    QStringList files;
    QString fileOpenMode;
    QString lastScannedDirectory;
    QString currentFilter;
        
    int numFiles;
    int numOpenPFMs;
    QStringList openPfmList;

    QVBoxLayout *mainLayout,*centerLayout,*leftLayout;
    
    QHBoxLayout *btnLayoutTop,*btnLayoutBottom,*cntLayout,*rightLayout,*row1,*row2,*row3;
        
    QLabel *cntLabel,*dirLabel,*filterLabel;
    
    QTableWidget  *tblFiles;
    
    QGroupBox *centerBox,*btnLowerBox,*btnUpperBox, *cntBox, *leftBox, *rightBox;
    
    QPushButton *btnCancel,*btnScan,*btnOpen,*btnChangeDir;
    
    QSignalMapper *fileMapper;
    
    QLineEdit *lineEditFilter,* lineEditDirectory;
    
    QCheckBox *chkMakeDefault,*includeFile[500];
  
    protected slots:
	void slotAddDirClicked();
	void slotDefaultChanged(int);
	void slotFileStateChanged(int);
	void slotFinishedEditing();
	void slotFinishedDir();
	void slotChangeDir();
	
	signals:
	void clicked(int);
	
	
};

#endif /* ENHANCEDOPEN_HPP */

