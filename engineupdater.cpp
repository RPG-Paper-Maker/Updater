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

#include "engineupdater.h"
#include "common.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QUrl>
#include <QJsonDocument>
#include <QDirIterator>
#include <QMessageBox>
#include <QThread>

const QString EngineUpdater::VERSION = "2.0";
const QString EngineUpdater::jsonFiles = "files";
const QString EngineUpdater::jsonSource = "source";
const QString EngineUpdater::jsonTarget = "target";
const QString EngineUpdater::jsonOS = "os";
const QString EngineUpdater::jsonWindows = "w";
const QString EngineUpdater::jsonLinux = "l";
const QString EngineUpdater::jsonMac = "m";
const QString EngineUpdater::jsonOnlyFiles = "onlyFiles";
const QString EngineUpdater::jsonAdd = "add";
const QString EngineUpdater::jsonRemove = "remove";
const QString EngineUpdater::jsonReplace = "replace";
const QString EngineUpdater::gitRepoEngine = "RPG-Paper-Maker";
const QString EngineUpdater::gitRepoGame = "Game-Scripts";
const QString EngineUpdater::gitRepoDependencies = "Dependencies";
const QString EngineUpdater::gitRepoBR = "Basic-Ressources";
const QString EngineUpdater::jsonScripts = "scripts";
const QString EngineUpdater::jsonGames = "games";
const QString EngineUpdater::jsonEngineWin = "engineWin";
const QString EngineUpdater::jsonEngineLinux = "engineLinux";
const QString EngineUpdater::jsonEngineMac = "engineOsx";
const QString EngineUpdater::jsonContent = "content";
const QString EngineUpdater::jsonBR = "br";
const QString EngineUpdater::jsonEngineExe = "exeEngine";
const QString EngineUpdater::jsonGameExe = "exeGame";
const QString EngineUpdater::pathGitHub =
        "https://raw.githubusercontent.com/RPG-Paper-Maker/";

// -------------------------------------------------------
//
//  CONSTRUCTOR / DESTRUCTOR / GET / SET
//
// -------------------------------------------------------

EngineUpdater::EngineUpdater(QString engineVersion) :
    m_currentVersion(engineVersion)
{

}

EngineUpdater::~EngineUpdater()
{

}

// -------------------------------------------------------
//
//  INTERMEDIARY FUNCTIONS
//
// -------------------------------------------------------

void EngineUpdater::writeTrees() {
    QJsonObject objScripts, objGame, objEngineWin, objEngineLinux, objEngineMac,
                objContent, objBR, objEngineExe, objGameExe, obj, objTemp;
    writeTree("Content/Datas/Scripts/System", gitRepoGame,
              "Engine/Content/Datas/Scripts/System/", objScripts);
    writeTree("Game", gitRepoDependencies, "Engine/Content/", objGame);
    writeTree("Engine/win32", gitRepoDependencies, "Engine/", objEngineWin);
    writeTree("Engine/linux", gitRepoDependencies, "Engine/", objEngineLinux);
    writeTree("Engine/osx", gitRepoDependencies, "Engine/", objEngineMac);
    writeTree("Content", gitRepoEngine, "Engine/Content/", objContent);
    writeTree("Content", gitRepoBR, "Engine/Content/basic/Content", objBR);

    // Exes
    getJSONExeEngine(objTemp, "win32");
    objEngineExe["win32"] = objTemp;
    getJSONExeEngine(objTemp, "linux");
    objEngineExe["linux"] = objTemp;
    getJSONExeEngine(objTemp, "osx");
    objEngineExe["osx"] = objTemp;
    getJSONExeGame(objTemp, "win32");
    objGameExe["win32"] = objTemp;
    getJSONExeGame(objTemp, "linux");
    objGameExe["linux"] = objTemp;
    getJSONExeGame(objTemp, "osx");
    objGameExe["osx"] = objTemp;

    // All
    obj[jsonScripts] = objScripts;
    obj[jsonGames] = objGame;
    obj[jsonEngineWin] = objEngineWin;
    obj[jsonEngineLinux] = objEngineLinux;
    obj[jsonEngineMac] = objEngineMac;
    obj[jsonContent] = objContent;
    obj[jsonBR] = objBR;
    obj[jsonEngineExe] = objEngineExe;
    obj[jsonGameExe] = objGameExe;
    Common::writeOtherJSON("../RPG-Paper-Maker/trees.json", obj,
                           QJsonDocument::Indented);
}

// -------------------------------------------------------

void EngineUpdater::writeTree(QString path, QString gitRepo, QString targetPath,
                              QJsonObject& objTree)
{
    QString networkUrl = pathGitHub + gitRepo + "/master/";
    QString localUrl = "../" + gitRepo + "/"; // Only for unix

    getTree(objTree, localUrl, networkUrl, path, targetPath);

    if (targetPath == "Engine/")
        objTree[jsonOnlyFiles] = true;
}

// -------------------------------------------------------

void EngineUpdater::getTree(QJsonObject& objTree, QString localUrl,
                            QString networkUrl, QString path, QString targetUrl)
{
    QDirIterator directories(Common::pathCombine(localUrl, path),
                             QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files);
    QJsonArray tabFiles;

    while (directories.hasNext()) {
        directories.next();
        QString name = directories.fileName();
        if (name != "RPG Paper Maker.exe") {
            QString currentPath = Common::pathCombine(path, name);
            QString currentTarget = Common::pathCombine(targetUrl, name);
            QJsonObject obj;
            if (directories.fileInfo().isDir())
                getTree(obj, localUrl, networkUrl, currentPath, currentTarget);
            else {
                getJSONFile(obj, Common::pathCombine(networkUrl, currentPath),
                            currentTarget);
            }
            tabFiles.append(obj);
        }
    }

    getJSONDir(objTree, tabFiles, targetUrl);
}

// -------------------------------------------------------

void EngineUpdater::getJSONFile(QJsonObject& obj, QString source,
                                QString target)
{
    obj[jsonSource] = source;
    obj[jsonTarget] = target;
}

// -------------------------------------------------------

void EngineUpdater::getJSONDir(QJsonObject &obj, QJsonArray& files,
                               QString target)
{
    obj[jsonFiles] = files;
    obj[jsonTarget] = target;
}

// -------------------------------------------------------

void EngineUpdater::getJSONExeEngine(QJsonObject& obj, QString os) {
    QString exe;

    if (os == "win32")
        exe = "RPG Paper Maker.exe";
    else if (os == "linux")
        exe = "RPG-Paper-Maker";
    else
        exe = "RPG-Paper-Maker.app";

    getJSONFile(obj, pathGitHub +
                "RPG-Paper-Maker/master/Engine/Dependencies/" +
                os + "/" + exe, "Engine/" + exe);
}

// -------------------------------------------------------

void EngineUpdater::getJSONExeGame(QJsonObject& obj, QString os) {
    QString exe = "Game";

    if (os == "win32")
        exe += ".exe";
    else if (os == "osx")
        exe += ".app";

    getJSONFile(obj, pathGitHub +
                "RPG-Paper-Maker/master/Engine/Content/" + os + "/" + exe,
                "Content/" + os + "/" + exe);
}

// -------------------------------------------------------

void EngineUpdater::start() {
    emit needUpdate();
}

// -------------------------------------------------------

void EngineUpdater::updateVersion(QJsonObject& obj) {
    QJsonArray tabAdd =
          obj.contains(jsonAdd) ? obj[jsonAdd].toArray() : QJsonArray();
    QJsonArray tabRemove =
          obj.contains(jsonRemove) ? obj[jsonRemove].toArray() : QJsonArray();
    QJsonArray tabReplace =
          obj.contains(jsonReplace) ? obj[jsonReplace].toArray() : QJsonArray();
    QJsonObject objFile;

    for (int i = 0; i < tabAdd.size(); i++) {
        objFile = tabAdd.at(i).toObject();
        download(EngineUpdateFileKind::Add, objFile);
    }
    for (int i = 0; i < tabRemove.size(); i++) {
        objFile = tabRemove.at(i).toObject();
        download(EngineUpdateFileKind::Remove, objFile);
    }
    for (int i = 0; i < tabReplace.size(); i++) {
        objFile = tabReplace.at(i).toObject();
        download(EngineUpdateFileKind::Replace, objFile);
    }
}

// -------------------------------------------------------

void EngineUpdater::download(EngineUpdateFileKind action, QJsonObject& obj) {
    if (obj.contains(jsonOS)) {
        QString strOS = "";
        #ifdef Q_OS_WIN
            strOS = jsonWindows;
        #elif __linux__
            strOS = jsonLinux;
        #else
            strOS = jsonMac;
        #endif

        if (!obj[jsonOS].toArray().contains(strOS))
            return;
    }

    if (obj.contains(jsonFiles))
        downloadFolder(action, obj);
    else
        downloadFile(action, obj);
}

// -------------------------------------------------------

void EngineUpdater::downloadFile(EngineUpdateFileKind action, QJsonObject& obj,
                                 bool exe)
{
    QString source = obj[jsonSource].toString();
    QString target = obj[jsonTarget].toString();

    if (action == EngineUpdateFileKind::Add)
        addFile(source, target, exe);
    else if (action == EngineUpdateFileKind::Remove)
        removeFile(target);
    else if (action == EngineUpdateFileKind::Replace)
        replaceFile(source, target, exe);
}

// -------------------------------------------------------

void EngineUpdater::addFile(QString& source, QString& target, bool exe) {
    QString path = Common::pathCombine(QDir::currentPath(), target);
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QEventLoop loop;

    reply = manager.get(QNetworkRequest(QUrl(source)));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        QMessageBox::critical(nullptr, "Error",
                              "A network error occured: " +
                              reply->errorString());
    }
    QFile file(path);

    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());

    // If exe, change permissions
    if (exe) {
        file.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser |
                            QFileDevice::ExeUser | QFileDevice::ReadGroup |
                            QFileDevice::ExeGroup | QFileDevice::ReadOther |
                            QFileDevice::ExeOther);
    }
    file.close();
}

// -------------------------------------------------------

void EngineUpdater::removeFile(QString& target) {
    QString path = Common::pathCombine(QDir::currentPath(), target);
    QFile file(path);
    file.remove();
}

// -------------------------------------------------------

void EngineUpdater::replaceFile(QString& source, QString& target, bool exe) {
    removeFile(target);
    addFile(source, target, exe);
}

// -------------------------------------------------------

void EngineUpdater::downloadFolder(EngineUpdateFileKind action,
                                   QJsonObject& obj, bool onlyFiles)
{
    QString target = obj[jsonTarget].toString();
    QJsonArray files = obj[jsonFiles].toArray();
    onlyFiles = obj.contains(jsonOnlyFiles) ? obj[jsonOnlyFiles].toBool()
                                            : onlyFiles;

    // The folder itself
    if (action == EngineUpdateFileKind::Add)
        addFolder(target, files, onlyFiles);
    else if (action == EngineUpdateFileKind::Remove)
        removeFolder(target, onlyFiles);
    else if (action == EngineUpdateFileKind::Replace)
        replaceFolder(target, files, onlyFiles);
}

// -------------------------------------------------------

void EngineUpdater::addFolder(QString& target, QJsonArray& files,
                              bool onlyFiles)
{
    // Create the folder
    if (!onlyFiles) {
        QString path = Common::pathCombine(QDir::currentPath(), target);
        QDir dir(path);
        QString dirName = dir.dirName();
        dir.cdUp();
        dir.mkdir(dirName);
    }

    // Files inside the folder
    QJsonObject obj;
    for (int i = 0; i < files.size(); i++) {
        obj = files.at(i).toObject();
        download(EngineUpdateFileKind::Add, obj);
    }
}

// -------------------------------------------------------

void EngineUpdater::removeFolder(QString& target, bool onlyFiles) {
    QString path = Common::pathCombine(QDir::currentPath(), target);

    if (onlyFiles) {
        QDirIterator files(path, QDir::Files);

        while (files.hasNext()) {
            files.next();
            QFile(files.filePath()).remove();
        }
    }
    else {
        QDir dir(path);
        dir.removeRecursively();
    }
}

// -------------------------------------------------------

void EngineUpdater::replaceFolder(QString& target, QJsonArray &files,
                                  bool onlyFiles)
{
    removeFolder(target, onlyFiles);
    addFolder(target, files, onlyFiles);
}

// -------------------------------------------------------

void EngineUpdater::downloadExecutables() {

    // Games
    QJsonObject objGame = m_document["exeGame"].toObject();
    QJsonObject objGameWin32 = objGame["win32"].toObject();
    QJsonObject objGameLinux = objGame["linux"].toObject();
    QJsonObject objGameOsx = objGame["osx"].toObject();
    downloadFile(EngineUpdateFileKind::Replace, objGameWin32, true);
    downloadFile(EngineUpdateFileKind::Replace, objGameLinux, true);
    downloadFile(EngineUpdateFileKind::Replace, objGameOsx, true);

    // Engine
    QJsonObject objEngine = m_document["exeEngine"].toObject();
    QJsonObject objEngineExe;
    QString strOS = "";
    #ifdef Q_OS_WIN
        strOS = "win32";
    #elif __linux__
        strOS = "linux";
    #else
        strOS = "osx";
    #endif
    objEngineExe = objEngine[strOS].toObject();
    downloadFile(EngineUpdateFileKind::Replace, objEngineExe, true);
}

// -------------------------------------------------------

void EngineUpdater::downloadScripts() {
    //downloadFolder(EngineUpdateFileKind::Replace, objTree);
}

// -------------------------------------------------------

void EngineUpdater::getVersions(QJsonArray& versions) const {
    for (int i = m_index; i < m_versions.size(); i++)
        versions.append(m_versions.at(i));
}

// -------------------------------------------------------

bool EngineUpdater::check() {
    QString lastVersion;
    int dif;

    // Check last version
    if (!readDocumentVersion())
        return false;
    dif = Common::versionDifferent(lastVersion, m_currentVersion);

    // Checking versions index
    m_versions = m_document["versions"].toArray();
    QJsonObject obj;
    m_index = m_versions.size();
    if (m_index != 0) {
        for (int i = 0; i < m_versions.size(); i++) {
            obj = m_versions.at(i).toObject();
            if (Common::versionDifferent(obj["v"].toString(),
                                         m_currentVersion) == 1)
            {
                m_index = i;
                break;
            }
        }
    }

    return dif != 0;
}

// -------------------------------------------------------

bool EngineUpdater::readDocumentVersion() {
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QEventLoop loop;
    QJsonObject doc;

    // Get the JSON
    /*
    reply = manager.get(QNetworkRequest(
        QUrl(pathGitHub + "RPG-Paper-Maker/master/versions.json")));

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        return false;
    }
    m_document = QJsonDocument::fromJson(reply->readAll()).object();
    */
    QJsonDocument json;
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/versions.json"),
                          json);
    doc = json.object();
    m_lastVersion = doc["lastVersion"].toString();
    m_versions = doc["versions"].toArray();
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/trees.json"),
                          json);
    m_document = json.object();

    return true;
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void EngineUpdater::downloadEngine() {
    QThread::sleep(1);

    // Executables
    emit progress(80, "Downloading all the system scripts...");
    //downloadScripts();
    QThread::sleep(1);
    emit progress(100, "Downloading all the system scripts...");

    QThread::sleep(1);
    /*
    // System scripts
    emit progress(90, "Downloading executables for games and engine...");
    downloadExecutables();
    emit progress(100, "Finished!");
    QThread::sleep(1);

    // Remove updater.json
    QFile updater(Common::pathCombine(QDir::currentPath(), "updater.json"));
    if (updater.exists())
        updater.remove();
    */
    emit finished();
}

// -------------------------------------------------------

void EngineUpdater::update() {
    QJsonObject obj;

    // Updating for each versions
    if (m_index != m_versions.size()) {
        int progressVersion = 80 / (m_versions.size() - m_index);
        for (int i = m_index; i < m_versions.size(); i++) {
            obj = m_versions.at(i).toObject();
            emit progress(((i - m_index) * progressVersion),
                          "Downloading version " + obj["v"].toString() + "...");
            updateVersion(obj);
            QThread::sleep(1);
        }
    }

    // Executables
    emit progress(80, "Downloading all the system scripts...");
    downloadScripts();

    // System scripts
    emit progress(90, "Downloading executables for games and engine...");
    downloadExecutables();
    emit progress(100, "Finished!");
    QThread::sleep(1);

    // Remove updater.json
    QFile updater(Common::pathCombine(QDir::currentPath(), "updater.json"));
    if (updater.exists())
        updater.remove();

    emit finished();
}
