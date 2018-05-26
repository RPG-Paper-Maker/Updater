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

#ifndef WIDGETRELEASENOTES_H
#define WIDGETRELEASENOTES_H

#include <QWidget>
#include <QStaticText>
#include <QJsonArray>

class WidgetReleaseNotes : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetReleaseNotes(QWidget *parent = 0);
    ~WidgetReleaseNotes();
    void updateText(QJsonArray& versions);
    void addItems(QJsonArray& list, QString& text);

protected:
    QStaticText m_staticText;

    virtual void paintEvent(QPaintEvent *);
};

#endif // WIDGETRELEASENOTES_H
