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
    m_engineUpdater(engineUpdater),
    m_update(false)
{
    ui->setupUi(this);
    setFixedSize(geometry().width(), geometry().height());

    m_progress.connect(&m_engineUpdater, SIGNAL(progress(int, QString)),
                       &m_progress, SLOT(setValueLabel(int, QString)));
    m_progress.connect(&m_engineUpdater, SIGNAL(progressDescription(QString)),
                       &m_progress, SLOT(setDescription(QString)));
    m_progress.connect(&m_engineUpdater, SIGNAL(setCount(int)),
                       &m_progress, SLOT(setCount(int)));
    m_progress.connect(&m_engineUpdater, SIGNAL(addOne()),
                       &m_progress, SLOT(addOne()));
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
    m_update = true;
}

// -------------------------------------------------------

void DialogEngineUpdate::updateLabel(QString label) {
    ui->label->setText(label);
    ui->scrollArea->hide();
    ui->checkBoxShow->hide();
    setFixedSize(geometry().width(), geometry().height() - 370);
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void DialogEngineUpdate::accept() {
    this->hide();
    m_progress.show();
    if (m_update)
        m_engineUpdater.update();
    else
        m_engineUpdater.downloadEngine();
    if (!m_engineUpdater.messageError().isEmpty())
        QMessageBox::critical(this, "Error", m_engineUpdater.messageError());
    else
        QMessageBox::information(this, "Done!", "Download finished correctly!");
    m_progress.close();
    qApp->quit();
    EngineUpdater::startEngineProcess();

    QDialog::accept();
}

// -------------------------------------------------------

void DialogEngineUpdate::reject() {
    this->hide();
    qApp->quit();
    EngineUpdater::startEngineProcess();

    QDialog::reject();
}

// -------------------------------------------------------

void DialogEngineUpdate::on_checkBoxShow_toggled(bool checked) {
    m_engineUpdater.changeNeedUpdate(checked);
}
