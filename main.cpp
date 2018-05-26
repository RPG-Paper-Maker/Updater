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
#include <QApplication>
#include "engineupdater.h"
#include "dialogprogress.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc == 1) {

    }
    else if (argc == 2) {
        EngineUpdater engineUpdater(argc == 2 ? argv[1] : "");
        if (engineUpdater.check()) {
            QJsonArray tab;
            engineUpdater.getVersions(tab);

            DialogEngineUpdate dialog(tab);
            if (dialog.exec() == QDialog::Accepted) {
                DialogProgress progress;
                progress.connect(&engineUpdater,
                                  SIGNAL(progress(int, QString)),
                                  &progress, SLOT(setValueLabel(int, QString)));
                progress.connect(&engineUpdater, SIGNAL(needUpdate()),
                                  &engineUpdater, SLOT(update()));
                engineUpdater.start();
                dialog.exec();
            }
            return a.exec();
        }
    }

    return 0;
}
