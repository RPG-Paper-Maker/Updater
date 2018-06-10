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
#include "engineupdater.h"
#include "dialogprogress.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // The application can now be used even if called from another directory
    QDir bin(qApp->applicationDirPath());
    #ifdef Q_OS_MAC
        bin.cdUp();
        bin.cdUp();
        bin.cdUp();
    #endif
    QDir::setCurrent(bin.absolutePath());

    EngineUpdater engineUpdater;
    DialogEngineUpdate dialog(engineUpdater);

    if (!engineUpdater.readDocumentVersion()) {
        EngineUpdater::startEngineProcess();
        return 0;
    }

    //engineUpdater.writeTrees();

    if (engineUpdater.hasUpdaterExpired()) {
        QMessageBox::warning(nullptr, "Your engine updater has expired.",
                             "Your engine updater has expired. Pleae download "
                             "the newest version in our official website: "
                             "http://rpg-paper-maker.com");
        return 0;
    }
    else {
        if (engineUpdater.hasVersion()) {
            if (EngineUpdater::isNeedUpdate() && engineUpdater.check()) {
                QJsonArray tab;
                engineUpdater.getVersions(tab);
                dialog.updateReleaseText(tab);
                dialog.show();
            }
            else
                EngineUpdater::startEngineProcess();
        }
        else {
            dialog.updateLabel("You can download the newest version of the "
                               "engine. Would you like to continue?");
            dialog.show();
        }
    }

    return a.exec();
}
