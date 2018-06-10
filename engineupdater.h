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

#ifndef ENGINEUPDATER_H
#define ENGINEUPDATER_H

#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include "engineupdatefilekind.h"

// -------------------------------------------------------
//
//  CLASS ProjectUpdater
//
//  Module used for detecting if the engine needs to be updated
//
// -------------------------------------------------------

class EngineUpdater : public QObject
{
    Q_OBJECT
public:
    EngineUpdater();
    virtual ~EngineUpdater();
    QString messageError() const;
    bool hasUpdaterExpired() const;

    static const QString VERSION;
    static const QString jsonFiles;
    static const QString jsonSource;
    static const QString jsonTarget;
    static const QString jsonRepo;
    static const QString jsonOS;
    static const QString jsonWindows;
    static const QString jsonLinux;
    static const QString jsonMac;
    static const QString jsonOnlyFiles;
    static const QString jsonExe;
    static const QString jsonAdd;
    static const QString jsonReplace;
    static const QString jsonRemove;
    static const QString jsonTree;
    static const QString jsonScripts;
    static const QString jsonGames;
    static const QString jsonEngineWin;
    static const QString jsonEngineLinux;
    static const QString jsonEngineMac;
    static const QString jsonContent;
    static const QString jsonBR;
    static const QString jsonEngineExe;
    static const QString jsonGameExe;
    static const QString gitRepoEngine;
    static const QString gitRepoGame;
    static const QString gitRepoDependencies;
    static const QString gitRepoBR;
    static const QString pathGitHub;

    static void startEngineProcess();
    static bool isNeedUpdate();
    static void writeTrees();
    static void writeTree(QString path, QString gitRepo, QString targetPath,
                          QJsonObject &objTree);
    static void getTree(QJsonObject& objTree, QString localUrl, QString path,
                        QString targetUrl, QString repo);
    static void getJSONFile(QJsonObject &obj, QString source, QString target,
                            QString repo);
    static void getJSONDir(QJsonObject &obj, QJsonArray& files, QString target);
    static void getJSONExeEngine(QJsonObject &obj, QString os);
    static void getJSONExeGame(QJsonObject &obj, QString os);
    static int countObjJson(QJsonObject& obj);
    static int countArrayJson(QJsonArray& files);
    static int countObjUpdate(QJsonObject& obj);
    bool hasVersion() const;
    QString getVersionJson() const;
    void start();
    void updateVersion(QJsonObject& obj, QString &version);
    bool download(EngineUpdateFileKind action, QJsonObject& obj,
                  QString& version);
    bool downloadFile(EngineUpdateFileKind action, QJsonObject& obj,
                      QString &version);
    bool addFile(QString& source, QString& target, QString &repo,
                 QString &version, bool exe);
    void removeFile(QString& target);
    bool replaceFile(QString& source, QString& target, QString &repo,
                     QString &version, bool exe);
    bool downloadFolder(EngineUpdateFileKind action, QJsonObject& obj,
                        QString &version, bool onlyFiles = false);
    bool addFolder(QString& target, QJsonArray& files, QString &version,
                   bool onlyFiles = false);
    void removeFolder(QString& target, bool onlyFiles = false);
    bool replaceFolder(QString& target, QJsonArray& files, QString& version,
                       bool onlyFiles = false);
    void downloadExecutables();
    bool downloadScripts();
    void getVersions(QJsonArray& versions) const;
    bool check();
    bool readDocumentVersion();
    bool readTrees(QString& version);
    void writeVersion();

protected:
    QJsonObject m_document;
    QJsonArray m_versions;
    int m_index;
    QString m_currentVersion;
    QString m_lastVersion;
    QString m_updaterVersion;
    QString m_messageError;

public slots:
    void downloadEngine();
    void update();

signals:
    void progress(int, QString);
    void progressDescription(QString);
    void setCount(int);
    void addOne();
    void finishedCheck(bool);
    void needUpdate();
};

#endif // ENGINEUPDATER_H
