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
#include <QTimer>

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
    //engineUpdater.writeTrees();
    DialogEngineUpdate dialog(engineUpdater);

    if (!engineUpdater.readDocumentVersion()) {
        EngineUpdater::startEngineProcess();
        QTimer::singleShot(5000, qApp, SLOT(quit()));
        return 0;
    }    

    if (engineUpdater.hasUpdaterExpired())
    {
        dialog.setToUpgradeUpdater();
        dialog.show();
    }
    else
    {
        if (engineUpdater.hasVersion()) {
            if (EngineUpdater::isNeedUpdate() && engineUpdater.check()) {
                QJsonArray tab;
                engineUpdater.getVersions(tab);
                dialog.updateReleaseText(tab);
                dialog.show();
            }
            else {
                EngineUpdater::startEngineProcess();
                QTimer::singleShot(5000, qApp, SLOT(quit()));
            }
        }
        else {
            dialog.updateLabel("You can download RPG Paper Maker now. Would you "
                "like to continue with these options?");
            dialog.show();
        }
    }

    return a.exec();
}
