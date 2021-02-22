/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   enhancedOpen.cpp
 * Author: Russ.Johnson
 * 
 * Created on January 28, 2021, 8:48 AM
 */

#include "enhancedOpenDisplay.hpp"

enhancedOpenDisplay::enhancedOpenDisplay(OPTIONS *options, MISC *misc) {


    numOpenPFMs = misc->abe_share->pfm_count;
    currentFilter = options->lastFileFilter;
    if (currentFilter.length() == 0) currentFilter = "*.pfm";

    for (int i = 0; i < numOpenPFMs; i++) {
	openPfmList << QString(misc->abe_share->open_args[i].list_path);
    }


    mainLayout = new QVBoxLayout();
    this->numFiles = 0;
    this->lastScannedDirectory = options->lastScannedDirectory;

    btnUpperBox = new QGroupBox();
    btnLayoutTop = new QHBoxLayout();

    leftBox = new QGroupBox();
    leftLayout = new QVBoxLayout();

    rightBox = new QGroupBox;
    rightLayout = new QHBoxLayout();



    // Put the left box together
    // Current Directory : --------  |Select Directory|
    // Current Filter    : --------
    // |SCAN|

    dirLabel = new QLabel(this);
    dirLabel->setText("Current Directory: ");
    dirLabel->setAlignment(Qt::AlignRight);
    dirLabel->setFixedWidth(95);
    lineEditDirectory = new QLineEdit(this->lastScannedDirectory);
    connect(lineEditDirectory, SIGNAL(editingFinished()), this, SLOT(slotFinishedDir()));

    btnChangeDir = new QPushButton("CHANGE");
    connect(btnChangeDir, SIGNAL(clicked()), this, SLOT(slotChangeDir()));

    filterLabel = new QLabel(this);
    filterLabel->setText("Filter: ");
    filterLabel->setAlignment(Qt::AlignRight);
    filterLabel->setFixedWidth(95);
    lineEditFilter = new QLineEdit(currentFilter);
    connect(lineEditFilter, SIGNAL(editingFinished()), this, SLOT(slotFinishedEditing()));

    btnScan = new QPushButton("Fetch Files");
    btnScan->setMaximumWidth(150);
    btnScan->setMinimumWidth(100);
    connect(btnScan, SIGNAL(clicked()), this, SLOT(slotAddDirClicked()));

    // add the rows
    row1 = new QHBoxLayout();
    row1->addWidget(dirLabel);
    row1->addWidget(lineEditDirectory);
    row1->addWidget(btnChangeDir);

    row2 = new QHBoxLayout();
    row2->addWidget(filterLabel);
    row2->addWidget(lineEditFilter);

    row3 = new QHBoxLayout();
    row3->addWidget(btnScan);

    leftLayout->addLayout(row1);
    leftLayout->addLayout(row2);
    leftLayout->addLayout(row3);

    leftBox->setLayout(leftLayout);

    // Right Box

    chkMakeDefault = new QCheckBox("Set as default");
    if (options->defaultFileOpen == "regular") {
	chkMakeDefault->setChecked(false);
	this->fileOpenMode = "regular";
    } else {
	this->fileOpenMode = "enhanced";
	chkMakeDefault->setChecked(true);
    }
    connect(chkMakeDefault, SIGNAL(stateChanged(int)), this, SLOT(slotDefaultChanged(int)));

    rightLayout->addWidget(chkMakeDefault);
    rightBox->setLayout(rightLayout);

    // Button Box 

    btnLayoutTop->addWidget(leftBox);
    btnLayoutTop->addWidget(rightBox);
    btnUpperBox->setLayout(btnLayoutTop);

    // COUNT
    cntBox = new QGroupBox();
    cntLayout = new QHBoxLayout();
    cntLabel = new QLabel("Current Number of Open Files : " + QString::number(numOpenPFMs) + " (16 Max)");

    cntLayout->addWidget(cntLabel);
    cntBox->setLayout(cntLayout);

    // CENTER SECTION 


    centerLayout = new QVBoxLayout();
    centerBox = new QGroupBox();

    // build the box that will the files
    tblFiles = new QTableWidget(0, 1, this);
    tblFiles->setColumnWidth(0, 800);

    QTableWidgetItem *header = new QTableWidgetItem();
    header->setText("  ");
    tblFiles->setHorizontalHeaderItem(0, header);
    tblFiles->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);


    QLabel *lblFileList = new QLabel("PFM Files");
    lblFileList->setAlignment(Qt::AlignCenter);
    lblFileList->setStyleSheet("QLabel {font-weight:bold;}");

    centerLayout->addWidget(lblFileList);
    centerLayout->addWidget(tblFiles);

    centerBox->setLayout(centerLayout);

    btnLowerBox = new QGroupBox();
    btnLayoutBottom = new QHBoxLayout();
    btnCancel = new QPushButton("CANCEL");
    btnCancel->setMaximumWidth(100);
    btnCancel->setMinimumWidth(100);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    btnOpen = new QPushButton("OPEN");
    btnOpen->setMaximumWidth(100);
    btnOpen->setMinimumWidth(100);
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(accept()));

    btnLayoutBottom->addStretch();
    btnLayoutBottom->addWidget(btnCancel);
    btnLayoutBottom->addWidget(btnOpen);

    btnLowerBox->setLayout(btnLayoutBottom);

    // ** MAIN LAYOUT  
    mainLayout->addWidget(btnUpperBox);
    mainLayout->addWidget(cntBox);
    mainLayout->addWidget(centerBox);
    mainLayout->addWidget(btnLowerBox);
    setLayout(mainLayout);


    setWindowTitle("Enhanced PFM Open");
    resize(500, 500);
}

QDialog::DialogCode enhancedOpenDisplay::showMe() {
    this->setWindowModality(Qt::ApplicationModal);
    this->exec();
    return static_cast<QDialog::DialogCode> (this->result());
}

QStringList enhancedOpenDisplay::selectedFiles() {


    for (int i = 0; i < numFiles; i++) {
	if (includeFile[i]->isChecked() && includeFile[i]->isEnabled()) {
	    this->files << includeFile[i]->text();
	}
    }
    return this->files;
}

QString enhancedOpenDisplay::defaultOpenMode() {
    return this->fileOpenMode;
}

QString enhancedOpenDisplay::getLastScannedDirectory() {
    return this->lastScannedDirectory;
}

QString enhancedOpenDisplay::getCurrentFilter() {
    return currentFilter;
}

void enhancedOpenDisplay::slotAddDirClicked() {

    QString fileName;

    // clear out any of the table rows from previous scans
    tblFiles->setRowCount(0);
    this->numFiles = 0;


    QSignalMapper *fileMapper = new QSignalMapper(this);
    connect(fileMapper, SIGNAL(mapped(int)), this, SLOT(slotFileStateChanged(int)));

    QTableWidgetItem *header = new QTableWidgetItem();


    header->setText("Results for scan of Directory " + this->lastScannedDirectory + " and filter " + currentFilter);
    tblFiles->setHorizontalHeaderItem(0, header);

    QDirIterator it(this->lastScannedDirectory, QStringList() << currentFilter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
	tblFiles->insertRow(this->numFiles);

	fileName = it.next();
	includeFile[this->numFiles] = new QCheckBox(fileName, this);

	fileMapper->setMapping(includeFile[this->numFiles], this->numFiles);
	connect(includeFile[this->numFiles], SIGNAL(clicked()), fileMapper, SLOT(map()));


	// check to see if this file is already open.  If it is then disable the check box
	for (int i = 0; i < numOpenPFMs; i++) {
	    if (openPfmList.at(i) == fileName) {
		includeFile[this->numFiles]->setChecked(true);
		includeFile[this->numFiles]->setDisabled(true);
	    }
	}

	tblFiles->setCellWidget(this->numFiles, 0, includeFile[this->numFiles]);

	this->numFiles++;
    }

    connect(fileMapper, SIGNAL(mapped(int)), this, SLOT(slotDefaultChanged(int)));
    tblFiles->setRowCount(this->numFiles);
    tblFiles->repaint();


}

void enhancedOpenDisplay::slotDefaultChanged(int state) {
    if (state) {
	this->fileOpenMode = "enhanced";
    } else {
	this->fileOpenMode = "regular";
    }
}

void enhancedOpenDisplay::slotFileStateChanged(int itemClicked) {


    // get the state of the item clicked
    if (includeFile[itemClicked]->isChecked()) {
	if (numOpenPFMs == 16) {
	    includeFile[itemClicked]->setChecked(false);
	    QMessageBox::warning(this, tr("pfmView Open PFM Structure"),
		    tr("You have reached the max number of PFMs that can be opened"));
	}
	else {
	    numOpenPFMs++;
	}
    } else {
	numOpenPFMs--;
    }

    this->cntLabel->setText("Current Number of Open Files : " + QString::number(numOpenPFMs) + " (16 Max)");
}

void enhancedOpenDisplay::slotChangeDir() {
    QFileDialog dialog(this);
    this->lastScannedDirectory = dialog.getExistingDirectory(0, tr("Select folder to scan for PFM"), this->lastScannedDirectory,
	    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    lineEditDirectory->setText(this->lastScannedDirectory);
}

void enhancedOpenDisplay::slotFinishedEditing() {
    
    QString filter = lineEditFilter->text();
    if (filter.length() == 0) {
	filter = "*.pfm";
	currentFilter = filter;
	lineEditFilter->setText(filter);
	return;
    }
    
    QString last4 = filter.right(4).toLower();
    
    if (last4 == ".pfm") {
	currentFilter = lineEditFilter->text();
    } else {
	filter += ".pfm";
	currentFilter = filter;
	lineEditFilter->setText(filter);
    }
    

}

void enhancedOpenDisplay::slotFinishedDir() {
    this->lastScannedDirectory = lineEditDirectory->text();
}