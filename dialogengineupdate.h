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

#ifndef DIALOGENGINEUPDATE_H
#define DIALOGENGINEUPDATE_H

#include <QDialog>
#include <QJsonArray>
#include <dialogprogress.h>
#include "engineupdater.h"

// -------------------------------------------------------
//
//  CLASS DialogEngineUpdate
//
//  Dialog box displaying all the release notes.
//
// -------------------------------------------------------

namespace Ui {
class DialogEngineUpdate;
}

class DialogEngineUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEngineUpdate(EngineUpdater& enginUpdater,
                                QWidget *parent = 0);
    ~DialogEngineUpdate();
    void updateReleaseText(QJsonArray& tab);
    void updateLabel(QString label);

private:
    Ui::DialogEngineUpdate *ui;
    EngineUpdater& m_engineUpdater;
    DialogProgress m_progress;
    bool m_update;

public slots:
    virtual void accept();
    virtual void reject();

private slots:
    void on_checkBoxShow_toggled(bool checked);
    void on_downloadCompleted();
};

#endif // DIALOGENGINEUPDATE_H
