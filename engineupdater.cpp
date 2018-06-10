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
#include <QThread>
#include <QProcess>

const QString EngineUpdater::VERSION = "2.0";
const QString EngineUpdater::jsonFiles = "files";
const QString EngineUpdater::jsonSource = "source";
const QString EngineUpdater::jsonTarget = "target";
const QString EngineUpdater::jsonRepo = "repo";
const QString EngineUpdater::jsonOS = "os";
const QString EngineUpdater::jsonWindows = "w";
const QString EngineUpdater::jsonLinux = "l";
const QString EngineUpdater::jsonMac = "m";
const QString EngineUpdater::jsonOnlyFiles = "onlyFiles";
const QString EngineUpdater::jsonAdd = "add";
const QString EngineUpdater::jsonRemove = "remove";
const QString EngineUpdater::jsonReplace = "replace";
const QString EngineUpdater::jsonTree = "tree";
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

EngineUpdater::EngineUpdater()
{
    QDir bin(QDir::currentPath());
    bin.cdUp();
    QString path = Common::pathCombine(Common::pathCombine(
                   bin.absolutePath(), "Engine"), "version.json");
    if (QFile(path).exists()) {
        QJsonDocument doc;
        Common::readOtherJSON(path, doc);
        m_currentVersion = doc.object()["v"].toString();
    }
}

EngineUpdater::~EngineUpdater()
{

}

QString EngineUpdater::messageError() const { return m_messageError; }

bool EngineUpdater::hasUpdaterExpired() const {
    return m_updaterVersion != EngineUpdater::VERSION;
}

// -------------------------------------------------------
//
//  INTERMEDIARY FUNCTIONS
//
// -------------------------------------------------------

void EngineUpdater::startEngineProcess() {
    QDir bin(QDir::currentPath());
    bin.cdUp();
    QString path = Common::pathCombine(bin.absolutePath(), "Engine");
    QString exe;
    #ifdef Q_OS_WIN
        exe = "RPG Paper Maker.exe";
    #elif __linux__
        exe = "run.sh";
    #else
        exe = "run.sh";
    #endif
    path = Common::pathCombine(path, exe);
    QStringList arguments;
    arguments.append(path);
    QProcess::startDetached(path, arguments);
}

// -------------------------------------------------------

bool EngineUpdater::isNeedUpdate() {
    QDir bin(QDir::currentPath());
    bin.cdUp();
    QString path = Common::pathCombine(Common::pathCombine(Common::pathCombine(
                   bin.absolutePath(), "Engine"), "Content"),
                   "engineSettings.json");
    QJsonDocument doc;
    Common::readOtherJSON(path, doc);
    QJsonObject obj = doc.object();

    return obj.contains("updater") ? obj["updater"].toBool() : true;
}

// -------------------------------------------------------

void EngineUpdater::writeTrees() {
    QJsonObject objScripts, objGame, objEngineWin, objEngineLinux, objEngineMac,
                objContent, objBR, objEngineExe, objGameExe, obj, objTemp;
    writeTree("Content/Datas/Scripts/System", gitRepoGame,
              "../Engine/Content/basic/Content/Datas/Scripts/System/",
              objScripts);
    writeTree("Game", gitRepoDependencies, "../Engine/Content/", objGame);
    writeTree("Engine/win32", gitRepoDependencies, "../Engine/", objEngineWin);
    writeTree("Engine/linux", gitRepoDependencies, "../Engine/", objEngineLinux);
    writeTree("Engine/osx", gitRepoDependencies, "../Engine/", objEngineMac);
    writeTree("Content", gitRepoEngine, "../Engine/Content/", objContent);
    writeTree("Content", gitRepoBR, "../Engine/Content/basic/Content", objBR);

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
    QString localUrl = "../" + gitRepo + "/"; // Only for unix

    getTree(objTree, localUrl, path, targetPath, gitRepo);

    if (targetPath == "Engine/")
        objTree[jsonOnlyFiles] = true;
}

// -------------------------------------------------------

void EngineUpdater::getTree(QJsonObject& objTree, QString localUrl,
                            QString path, QString targetUrl, QString repo)
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
                getTree(obj, localUrl, currentPath, currentTarget, repo);
            else {
                getJSONFile(obj, currentPath, currentTarget, repo);
            }
            tabFiles.append(obj);
        }
    }

    getJSONDir(objTree, tabFiles, targetUrl);
}

// -------------------------------------------------------

void EngineUpdater::getJSONFile(QJsonObject& obj, QString source,
                                QString target, QString repo)
{
    obj[jsonSource] = source;
    obj[jsonTarget] = target;
    obj[jsonRepo] = repo;
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

    getJSONFile(obj, "Engine/" + os + "/" + exe, "../Engine/" +
                exe, "Dependencies");
}

// -------------------------------------------------------

void EngineUpdater::getJSONExeGame(QJsonObject& obj, QString os) {
    QString exe = "Game";

    if (os == "win32")
        exe += ".exe";
    else if (os == "osx")
        exe += ".app";

    getJSONFile(obj, "Game/" + os + "/" + exe, "../Engine/Content/" + os + "/" +
                exe, "Dependencies");
}

// -------------------------------------------------------


bool EngineUpdater::hasVersion() const {
    return !m_currentVersion.isEmpty();
}

// -------------------------------------------------------

void EngineUpdater::start() {
    emit needUpdate();
}

// -------------------------------------------------------

void EngineUpdater::updateVersion(QJsonObject& obj, QString& version) {
    QJsonArray tabAdd =
          obj.contains(jsonAdd) ? obj[jsonAdd].toArray() : QJsonArray();
    QJsonArray tabRemove =
          obj.contains(jsonRemove) ? obj[jsonRemove].toArray() : QJsonArray();
    QJsonArray tabReplace =
          obj.contains(jsonReplace) ? obj[jsonReplace].toArray() : QJsonArray();
    QJsonObject objFile;
    readTrees(version);

    for (int i = 0; i < tabAdd.size(); i++) {
        objFile = tabAdd.at(i).toObject();
        download(EngineUpdateFileKind::Add, objFile, version);
    }
    for (int i = 0; i < tabRemove.size(); i++) {
        objFile = tabRemove.at(i).toObject();
        download(EngineUpdateFileKind::Remove, objFile, version);
    }
    for (int i = 0; i < tabReplace.size(); i++) {
        objFile = tabReplace.at(i).toObject();
        download(EngineUpdateFileKind::Replace, objFile, version);
    }
}

// -------------------------------------------------------

bool EngineUpdater::download(EngineUpdateFileKind action, QJsonObject& obj,
                             QString& version)
{
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
            return true;
    }

    if (obj.contains(jsonFiles))
        return downloadFolder(action, obj, version);
    else
        return downloadFile(action, obj, version);
}

// -------------------------------------------------------

bool EngineUpdater::downloadFile(EngineUpdateFileKind action,
                                 QJsonObject& obj, QString& version, bool exe)
{
    QString source = obj[jsonSource].toString();
    QString target = obj[jsonTarget].toString();
    QString repo = obj[jsonRepo].toString();
    bool tree = obj.contains(jsonTree);

    if (tree) {
        QJsonObject objTree = m_document[source].toObject();
        if (!downloadFolder(action, objTree, version))
            return false;
    }
    else {
        if (action == EngineUpdateFileKind::Add)
            return addFile(source, target, repo, version, exe);
        else if (action == EngineUpdateFileKind::Remove)
            removeFile(target);
        else if (action == EngineUpdateFileKind::Replace)
            return replaceFile(source, target, repo, version, exe);
    }

    return true;
}

// -------------------------------------------------------

bool EngineUpdater::addFile(QString& source, QString& target, QString& repo,
                            QString& version, bool exe)
{
    QString path = Common::pathCombine(QDir::currentPath(), target);
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QEventLoop loop;
    QString url = pathGitHub + repo + "/" + version + "/" + source;

    emit progressDescription("Downloading " + source);
    reply = manager.get(QNetworkRequest(QUrl(url)));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        m_messageError = "Could not copy from " + source + " to " + path +
                         ": " + reply->errorString();
        return false;
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

    return true;
}

// -------------------------------------------------------

void EngineUpdater::removeFile(QString& target) {
    QString path = Common::pathCombine(QDir::currentPath(), target);
    QFile file(path);
    file.remove();
}

// -------------------------------------------------------

bool EngineUpdater::replaceFile(QString& source, QString& target, QString& repo,
                                QString& version, bool exe)
{
    removeFile(target);
    return addFile(source, target, repo, version, exe);
}

// -------------------------------------------------------

bool EngineUpdater::downloadFolder(EngineUpdateFileKind action,
                                   QJsonObject& obj, QString& version,
                                   bool onlyFiles)
{
    QString target = obj[jsonTarget].toString();
    QJsonArray files = obj[jsonFiles].toArray();
    onlyFiles = obj.contains(jsonOnlyFiles) ? obj[jsonOnlyFiles].toBool()
                                            : onlyFiles;

    // The folder itself
    if (action == EngineUpdateFileKind::Add)
        return addFolder(target, files, version, onlyFiles);
    else if (action == EngineUpdateFileKind::Remove)
        removeFolder(target, onlyFiles);
    else if (action == EngineUpdateFileKind::Replace)
        return replaceFolder(target, files, version, onlyFiles);

    return true;
}

// -------------------------------------------------------

bool EngineUpdater::addFolder(QString& target, QJsonArray& files,
                              QString& version, bool onlyFiles)
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
        if (!download(EngineUpdateFileKind::Add, obj, version))
            return false;
    }

    return true;
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

bool EngineUpdater::replaceFolder(QString& target, QJsonArray &files,
                                  QString &version, bool onlyFiles)
{
    removeFolder(target, onlyFiles);
    return addFolder(target, files, version, onlyFiles);
}

// -------------------------------------------------------

void EngineUpdater::downloadExecutables() {

    // Games
    QJsonObject objGame = m_document[jsonGameExe].toObject();
    QJsonObject objGameWin32 = objGame["win32"].toObject();
    QJsonObject objGameLinux = objGame["linux"].toObject();
    QJsonObject objGameOsx = objGame["osx"].toObject();
    downloadFile(EngineUpdateFileKind::Replace, objGameWin32, m_lastVersion,
                 true);
    downloadFile(EngineUpdateFileKind::Replace, objGameLinux, m_lastVersion,
                 true);
    downloadFile(EngineUpdateFileKind::Replace, objGameOsx, m_lastVersion,
                 true);

    // Engine
    QJsonObject objEngine = m_document[jsonEngineExe].toObject();
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
    downloadFile(EngineUpdateFileKind::Replace, objEngineExe, m_lastVersion,
                 true);
}

// -------------------------------------------------------

bool EngineUpdater::downloadScripts() {
    QJsonObject obj = m_document[jsonBR].toObject();
    return downloadFolder(EngineUpdateFileKind::Add, obj, m_lastVersion);
}

// -------------------------------------------------------

void EngineUpdater::getVersions(QJsonArray& versions) const {
    for (int i = m_index; i < m_versions.size(); i++)
        versions.append(m_versions.at(i));
}

// -------------------------------------------------------

bool EngineUpdater::check() {
    int dif;

    // Check last version
    if (!readDocumentVersion())
        return false;
    dif = Common::versionDifferent(m_lastVersion, m_currentVersion);

    // Checking versions index
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
    reply = manager.get(QNetworkRequest(
        QUrl(pathGitHub + "RPG-Paper-Maker/master/versions.json")));

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        return false;
    }
    doc = QJsonDocument::fromJson(reply->readAll()).object();
    /*
    QJsonDocument json;
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/versions.json"),
                          json);
    doc = json.object();
    */

    // -----------

    m_lastVersion = doc["lastVersion"].toString();
    m_updaterVersion = doc["uversion"].toString();
    m_versions = doc["versions"].toArray();

    return true;
}

// -------------------------------------------------------

bool EngineUpdater::readTrees(QString& version) {
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QEventLoop loop;
    QJsonObject doc;
    QJsonDocument json;

    reply = manager.get(QNetworkRequest(QUrl(pathGitHub + "RPG-Paper-Maker/"
        + version + "/tree.json")));

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        return false;
    }
    m_document = QJsonDocument::fromJson(reply->readAll()).object();
    /*
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/trees.json"),
                          json);
    m_document = json.object();*/

    return true;
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void EngineUpdater::downloadEngine() {
    QJsonObject obj;
    QDir dir;

    // Executables
    emit progress(0, "Creating content folder...");
    if (!readTrees(m_lastVersion))
        return;
    dir.mkdir("../Engine");
    obj = m_document[jsonContent].toObject();
    if (!downloadFolder(EngineUpdateFileKind::Add, obj, m_lastVersion))
        return;
    emit progress(5, "Downloading Basic Ressources...");
    if (!downloadScripts())
        return;
    emit progress(10, "Downloading System scripts...");
    dir.mkdir("../Engine/Content/basic/Content/Datas/Scripts");
    dir.mkdir("../Engine/Content/basic/Content/Datas/Scripts/Plugins");
    QFile include("../Engine/Content/basic/Content/Datas/Scripts/Plugins/"
                  "includes.js");
    include.open(QIODevice::WriteOnly);
    include.write("");
    include.close();
    obj = m_document[jsonScripts].toObject();
    if (!downloadFolder(EngineUpdateFileKind::Add, obj, m_lastVersion))
        return;
    emit progress(15, "Downloading games dependencies...");
    obj = m_document[jsonGames].toObject();
    if (!downloadFolder(EngineUpdateFileKind::Add, obj, m_lastVersion))
        return;
    emit progress(80, "Downloading engine dependencies...");
    #ifdef Q_OS_WIN
        obj = m_document[jsonEngineWin].toObject();
    #elif __linux__
        obj = m_document[jsonEngineLinux].toObject();
    #else
        obj = m_document[jsonEngineMac].toObject();
    #endif
    if (!downloadFolder(EngineUpdateFileKind::Add, obj, m_lastVersion))
        return;
    QFile("../RPG-Paper-Maker").remove();
    QFile::link("../Engine/RPG-Paper-Maker", "../RPG-Paper-Maker");

    emit progress(100, "Downloading all the system scripts...");
    QThread::sleep(1);
}

// -------------------------------------------------------

void EngineUpdater::update() {
    QJsonObject obj;
    QString version;

    // Updating for each versions
    if (m_index != m_versions.size()) {
        int progressVersion = 80 / (m_versions.size() - m_index);
        for (int i = m_index; i < m_versions.size(); i++) {
            obj = m_versions.at(i).toObject();
            version = obj["v"].toString();
            emit progress(((i - m_index) * progressVersion),
                          "Downloading version " + version + "...");
            updateVersion(obj, version);
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
}
