/*
    RPG Paper Maker Copyright (C) 2017 Marie Laporte

    This file is part of RPG Paper Maker.

    RPG Paper Maker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RPG Paper Maker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "dialogengineupdate.h"
#include "ui_dialogengineupdate.h"
#include <QThread>
#include <QMessageBox>

// -------------------------------------------------------
//
//  CONSTRUCTOR / DESTRUCTOR / GET / SET
//
// -------------------------------------------------------

DialogEngineUpdate::DialogEngineUpdate(EngineUpdater &engineUpdater,
                                       QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEngineUpdate),
    m_engineUpdater(engineUpdater)
{
    ui->setupUi(this);
    m_progress.connect(&m_engineUpdater, SIGNAL(progress(int, QString)),
                       &m_progress, SLOT(setValueLabel(int, QString)));
}

DialogEngineUpdate::~DialogEngineUpdate()
{
    delete ui;
}

// -------------------------------------------------------
//
//  INTERMEDIARY FUNCTIONS
//
// -------------------------------------------------------

void DialogEngineUpdate::updateReleaseText(QJsonArray& tab) {
    ui->widgetReleaseNotes->updateText(tab);
}

// -------------------------------------------------------

void DialogEngineUpdate::updateLabel(QString label) {
    ui->label->setText(label);
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void DialogEngineUpdate::accept() {
    this->hide();
    m_progress.show();
    m_engineUpdater.downloadEngine();
    if (!m_engineUpdater.messageError().isEmpty())
        QMessageBox::critical(this, "Error", m_engineUpdater.messageError());
    else
        QMessageBox::information(this, "Done!", "Download finished correctly!");
    m_progress.close();

    QDialog::accept();
}
