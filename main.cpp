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
#include <QThread>

/*
#include "common.h"
#include <QDir>
#include <QJsonDocument>
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EngineUpdater engineUpdater(argc == 2 ? argv[1] : "");
    DialogProgress progress;
    progress.connect(&engineUpdater, SIGNAL(progress(int, QString)),
                     &progress, SLOT(setValueLabel(int, QString)));
    progress.connect(&engineUpdater, SIGNAL(finished()),
                     &progress, SLOT(close()));
    QThread* thread = new QThread(&progress);
    engineUpdater.moveToThread(thread);


    //engineUpdater.writeTrees();

    /*
    QJsonDocument json;
    QJsonObject obj;
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             Common::pathCombine("Content", "test.json")),
                         json);
    obj = json.object();
    engineUpdater.downloadFile(EngineUpdateFileKind::Add, obj);
    */

    if (argc == 1 && engineUpdater.readDocumentVersion()) {
        engineUpdater.connect(thread, SIGNAL(started()),
                              &engineUpdater, SLOT(downloadEngine()));
        thread->start();
        progress.exec();
        thread->exit();
    }
    else if (argc == 2) {
        if (engineUpdater.check()) {
            QJsonArray tab;
            engineUpdater.getVersions(tab);

            DialogEngineUpdate dialog(tab);
            if (dialog.exec() == QDialog::Accepted) {
                progress.exec();
                engineUpdater.update();
                progress.close();
            }
        }
    }

    return 0;
}
