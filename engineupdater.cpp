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
#include <QMutex>

const QString EngineUpdater::VERSION = "2.3";
const QString EngineUpdater::ELECTRON_VERSION = "1.5.3";
const QString EngineUpdater::jsonFiles = "files";
const QString EngineUpdater::jsonSource = "source";
const QString EngineUpdater::jsonTarget = "target";
const QString EngineUpdater::jsonRepo = "repo";
const QString EngineUpdater::jsonOS = "os";
const QString EngineUpdater::jsonWindows = "w";
const QString EngineUpdater::jsonLinux = "l";
const QString EngineUpdater::jsonMac = "m";
const QString EngineUpdater::jsonOnlyFiles = "onlyFiles";
const QString EngineUpdater::jsonSymLink = "sl";
const QString EngineUpdater::jsonExe = "exe";
const QString EngineUpdater::jsonAdd = "add";
const QString EngineUpdater::jsonRemove = "remove";
const QString EngineUpdater::jsonReplace = "replace";
const QString EngineUpdater::jsonTree = "tree";
const QString EngineUpdater::jsonTranslations = "translations";
const QString EngineUpdater::gitRepoEngine = "RPG-Paper-Maker";
QString EngineUpdater::gitRepoGame = "Game";
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

EngineUpdater::EngineUpdater() :
    m_countFiles(0)
{
    QString path = getVersionJson();
    if (QFile(path).exists()) {
        QJsonDocument doc;
        Common::readOtherJSON(path, doc);
        m_currentVersion = doc.object()["v"].toString();
    }
    m_manager = new QNetworkAccessManager(this);
}

EngineUpdater::~EngineUpdater()
{
 delete m_manager;
}

QString EngineUpdater::messageError() const { return m_messageError; }

bool EngineUpdater::hasUpdaterExpired() const {
    return m_updaterVersion != EngineUpdater::VERSION;
}

QString EngineUpdater::lastVersion() const {
    return m_lastVersion;
}

// -------------------------------------------------------
//
//  INTERMEDIARY FUNCTIONS
//
// -------------------------------------------------------

void EngineUpdater::startEngineProcess() {
    QString path = Common::pathCombine(QDir::currentPath(), "Engine");
    QString exe;
    #ifdef Q_OS_WIN
        exe = "RPG Paper Maker.exe";
    #elif __linux__
        exe = "run.sh";
    #else
        exe = "RPG-Paper-Maker.app/Contents/MacOS/RPG-Paper-Maker";
    #endif
    path = Common::pathCombine(path, exe);
    QStringList arguments;
    arguments.append(path);
    QProcess::startDetached(path, arguments);
}

// -------------------------------------------------------

bool EngineUpdater::isNeedUpdate() {
    QString path = Common::pathCombine(Common::pathCombine(Common::pathCombine(
                   QDir::currentPath(), "Engine"), "Content"),
                   "engineSettings.json");
    QJsonDocument doc;
    Common::readOtherJSON(path, doc);
    QJsonObject obj = doc.object();

    return obj.contains("updater") ? obj["updater"].toBool() : true;
}

// -------------------------------------------------------

void EngineUpdater::writeTrees() {
    QJsonObject objScripts, objGame, objEngineWin, objEngineLinux, objEngineMac,
        objContent, objBR, objEngineExe, objGameExe, obj, objTemp,
        objTranslations;
    writeTree("Content/Datas/Scripts/System", gitRepoGame,
              "Engine/Content/basic/Content/Datas/Scripts/System/",
              objScripts);
    writeTree("Game", gitRepoDependencies, "Engine/Content/", objGame);
    writeTree("Engine/win32", gitRepoDependencies, "Engine/", objEngineWin);
    writeTree("Engine/linux", gitRepoDependencies, "Engine/", objEngineLinux);
    writeTree("Engine/osx", gitRepoDependencies, "Engine/", objEngineMac);
    writeTree("EditorApp/Content", gitRepoEngine, "Engine/Content/", objContent);
    writeTree("Content", gitRepoBR, "Engine/Content/BR/Content", objBR);
    writeTree("EditorApp/Content/translations", gitRepoEngine,
        "Engine/Content/translations", objTranslations);

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
    obj[jsonTranslations] = objTranslations;
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
        QString currentPath = Common::pathCombine(path, name);
        QString currentTarget = Common::pathCombine(targetUrl, name);
        QJsonObject obj;
        bool test = true;
        if (directories.fileInfo().isDir() && !directories.fileInfo().isSymLink())
        {
            getTree(obj, localUrl, currentPath, currentTarget, repo);
        } else {
            if (directories.fileName() == "Electron Framework" && !directories.fileInfo().isSymLink())
            {
                test = false;
            }
            getJSONFile(obj, currentPath, currentTarget, repo, directories
                .fileInfo().isSymLink(), directories.fileInfo().isDir());
        }
        if (test)
        {
            tabFiles.append(obj);
        }
    }

    getJSONDir(objTree, tabFiles, targetUrl);
}

// -------------------------------------------------------

void EngineUpdater::getJSONFile(QJsonObject& obj, QString source,
                                QString target, QString repo, bool link, bool isDir)
{
    obj[jsonSource] = source;
    obj[jsonTarget] = target;
    obj[jsonRepo] = repo;

    QString exe = source.split('/').last();
    QStringList extension = exe.split(".");
    if (!isDir && ((exe == "run.sh" || exe == "RPG-Paper-Maker" ||
        exe == "RPG Paper Maker.exe" ||
        exe == "Electron Framework" || exe == "Game" || exe == "Game.exe") ||
        extension.size() == 1))
    {
        obj[jsonExe] = true;
    }
    if (link) {
        obj[jsonSymLink] = true;
    }
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
        exe = "RPG-Paper-Maker.app/Contents/MacOS/RPG-Paper-Maker";

    getJSONFile(obj, "Engine/" + os + "/" + exe, "Engine/" +
                exe, "Dependencies", false, false);
    obj[jsonExe] = true;
}

// -------------------------------------------------------

void EngineUpdater::getJSONExeGame(QJsonObject& obj, QString os) {
    QString exe;

    if (os == "win32")
        exe = "Game.exe";
    else if (os == "linux")
        exe = "Game";
    else
        exe = "Game.app/Contents/MacOS/Game";

    getJSONFile(obj, "Game/" + os + "/" + exe, "Engine/Content/" + os + "/" +
                exe, "Dependencies", false, false);
    obj[jsonExe] = true;
}

// -------------------------------------------------------

int EngineUpdater::countObjJson(QJsonObject& obj) {
    QJsonArray files = obj[jsonFiles].toArray();
    return countArrayJson(files);
}

// -------------------------------------------------------

int EngineUpdater::countArrayJson(QJsonArray& files) {
    int count = 0, total = files.size();
    QJsonObject obj;

    for (int i = 0; i < total; i++) {
        obj = files.at(i).toObject();
        count += countObjJson(obj);
    }
    if (total == 0)
        count++;

    return count;
}

// -------------------------------------------------------

int EngineUpdater::countObjUpdate(QJsonObject& obj) {
    QJsonArray tabAdd =
          obj.contains(jsonAdd) ? obj[jsonAdd].toArray() : QJsonArray();
    QJsonArray tabRemove =
          obj.contains(jsonRemove) ? obj[jsonRemove].toArray() : QJsonArray();
    QJsonArray tabReplace =
          obj.contains(jsonReplace) ? obj[jsonReplace].toArray() : QJsonArray();

    return countArrayJson(tabAdd) + countArrayJson(tabRemove) +
           countArrayJson(tabReplace);
}

// -------------------------------------------------------


bool EngineUpdater::hasVersion() const {
    return !m_currentVersion.isEmpty();
}

// -------------------------------------------------------

QString EngineUpdater::getVersionJson() const {
    return Common::pathCombine(Common::pathCombine(
           QDir::currentPath(), "Engine"), "version.json");
}

// -------------------------------------------------------

void EngineUpdater::changeNeedUpdate(bool checked) {
    QString path = Common::pathCombine(Common::pathCombine(Common::pathCombine(
                   QDir::currentPath(), "Engine"), "Content"),
                   "engineSettings.json");
    QJsonDocument doc;
    Common::readOtherJSON(path, doc);
    QJsonObject obj = doc.object();
    obj["updater"] = !checked;
    Common::writeOtherJSON(path, obj);
}

// -------------------------------------------------------

void EngineUpdater::start() {
    emit needUpdate();
}

// -------------------------------------------------------

void EngineUpdater::updateVersion(QString& version) {
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QEventLoop loop;

    // Get the JSON
    reply = manager.get(QNetworkRequest(QUrl(pathGitHub +
        "RPG-Paper-Maker/master/Versions/" + version + ".json")));

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        return;
    }
    m_document = QJsonDocument::fromJson(reply->readAll()).object();
    /*
    QJsonDocument json;
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/Versions/" + version + ".json"),
                          json);
    m_document = json.object();
    */
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
                                 QJsonObject& obj, QString& version)
{
    QString source = obj[jsonSource].toString();
    QString target = obj[jsonTarget].toString();
    QString repo = obj[jsonRepo].toString();
    bool tree = obj.contains(jsonTree);
    bool exe = obj.contains(jsonExe);
    bool link = obj.contains(jsonSymLink);

    if (tree) {
        QJsonObject objTree = m_document[source].toObject();
        if (!downloadFolder(action, objTree, version))
            return false;
    }
    else {
        if (action == EngineUpdateFileKind::Add)
            return addFile(source, target, repo, version, exe, link);
        else if (action == EngineUpdateFileKind::Remove)
            removeFile(target);
        else if (action == EngineUpdateFileKind::Replace)
            return replaceFile(source, target, repo, version, exe, link);
    }

    return true;
}

// -------------------------------------------------------

bool EngineUpdater::addFile(QString source, QString target, QString repo,
                            QString version, bool exe, bool link)
{
    QString path = Common::pathCombine(QDir::currentPath(), target);
    QUrl url = QUrl(pathGitHub + repo + "/" + version + "/" + source);
    return addFileURL(url, source, target, exe, link, path);
}

// -------------------------------------------------------

bool EngineUpdater::addFileURL(QUrl &url, QString source, QString target,
                                bool exe, bool link, QString path)
{
    QNetworkReply *reply;
    m_countFiles++;
    QNetworkRequest request = QNetworkRequest(url);
    reply = m_manager->get(request);
    reply->setProperty("source", source);
    reply->setProperty("target", target);
    reply->setProperty("path", path);
    reply->setProperty("exe", exe);
    reply->setProperty("link", link);
    QFile *file = new QFile(path);
    QObject::connect(reply, &QNetworkReply::readyRead, this, [this, reply, file]() {
        this->handleReading(reply, file);
    });
    QObject::connect(reply, &QNetworkReply::finished, [this, reply, file](){
        this->handleFinished(reply, file);
    });
    QObject::connect(reply, &QNetworkReply::downloadProgress, [this, source](qint64 a, qint64 b){
        this->onDownloadProgress(source, a, b);
    });

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
                                QString& version, bool exe, bool link)
{
    removeFile(target);
    return addFile(source, target, repo, version, exe, link);
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
    downloadFile(EngineUpdateFileKind::Replace, objGameWin32, m_lastVersion);
    downloadFile(EngineUpdateFileKind::Replace, objGameLinux, m_lastVersion);
    downloadFile(EngineUpdateFileKind::Replace, objGameOsx, m_lastVersion);

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
    downloadFile(EngineUpdateFileKind::Replace, objEngineExe, m_lastVersion);
}

// -------------------------------------------------------

bool EngineUpdater::downloadScripts(QString version) {
    QJsonObject obj = m_document[jsonScripts].toObject();
    bool b = downloadFolder(EngineUpdateFileKind::Add, obj, version);
    if (Common::versionDifferent(version, EngineUpdater::ELECTRON_VERSION) != -1)
    {
        b &= addFile("main.js", "Engine/Content/main.js", gitRepoGame, version, false, false);
        b &= addFile("index.html", "Engine/Content/index.html", gitRepoGame, version, false, false);
        b &= addFile("package.json", "Engine/Content/package.json", gitRepoGame, version, false, false);
    }

    return b;
}

// -------------------------------------------------------

void EngineUpdater::getVersions(QJsonArray& versions) {
    QString version;

    for (int i = m_index; i < m_versions.size(); i++) {
        version = m_versions.at(i).toString();
        updateVersion(version);
        m_document["v"] = version;
        versions.append(m_document);
    }

    m_versionsContent = versions;
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
            if (Common::versionDifferent(m_versions.at(i).toString(),
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
        + version + "/trees.json")));

    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        m_messageError = "Could not read trees.json.";
        return false;
    }
    m_document = QJsonDocument::fromJson(reply->readAll()).object();
    /*
    Common::readOtherJSON(Common::pathCombine(
                             QDir::currentPath(),
                             "../RPG-Paper-Maker/trees.json"),
                          json);
    m_document = json.object();
    */

    return true;
}

// -------------------------------------------------------

void EngineUpdater::writeVersion(QString& version) {
    QString path = getVersionJson();
    QJsonObject doc;
    doc["v"] = version;
    Common::writeOtherJSON(path, doc);
}

// -------------------------------------------------------
//
//  SLOTS
//
// -------------------------------------------------------

void EngineUpdater::downloadEngine(bool isLastVersion, QString oldVersion) {
    QJsonObject objContent, objBR, objGames, objEngine, objScripts;
    QDir dir;
    QString version = isLastVersion ? m_lastVersion : oldVersion;

    emit progress(0, "Initializing...");
    readTrees(version);

    // Load obj
    objContent = m_document[jsonContent].toObject();
    objBR = m_document[jsonBR].toObject();
    objGames = m_document[jsonGames].toObject();
    #ifdef Q_OS_WIN
        objEngine = m_document[jsonEngineWin].toObject();
    #elif __linux__
        objEngine = m_document[jsonEngineLinux].toObject();
    #else
        objEngine = m_document[jsonEngineMac].toObject();
    #endif
    objScripts = m_document[jsonScripts].toObject();

    emit progress(100, "Downloading...");
    dir.mkdir("Engine");
    downloadFolder(EngineUpdateFileKind::Add, objContent, version);
    dir.mkdir("Engine/Content/BR");
    downloadFolder(EngineUpdateFileKind::Add, objBR, version);
    dir.mkdir("Engine/Content/basic/Content/Datas/Scripts");
    dir.mkdir("Engine/Content/basic/Content/Datas/Scripts/Plugins");
    QFile include("Engine/Content/basic/Content/Datas/Scripts/Plugins/"
                  "includes.js");
    include.open(QIODevice::WriteOnly);
    include.write("");
    include.close();
    downloadScripts(version);
    downloadFolder(EngineUpdateFileKind::Add, objGames, version);
    downloadFolder(EngineUpdateFileKind::Add, objEngine, version);
    downloadLargeFiles(version);
    writeVersion(version);
    setCurrentCount(m_countFiles);
}

// -------------------------------------------------------

void EngineUpdater::update() {
    QJsonObject obj;
    QString version;
    QJsonArray trees;

    // Updating for each versions
    emit progress(0, "Initializing...");
    if (m_index != m_versions.size()) {
        for (int i = m_index; i < m_versions.size(); i++) {
            version = m_versions.at(i).toString();
            readTrees(version);
            trees.append(m_document);
        }
    }
    emit progress(100, "Downloading version(s)...");
    if (m_index != m_versions.size()) {
        for (int i = m_index; i < m_versions.size(); i++) {
            version = m_versions.at(i).toString();
            obj = m_versionsContent.at(i - m_index).toObject();
            m_document = trees.at(i - m_index).toObject();
            QJsonArray tabRemove =
                  obj.contains(jsonRemove) ? obj[jsonRemove].toArray() : QJsonArray();
            QJsonArray tabAdd =
                  obj.contains(jsonAdd) ? obj[jsonAdd].toArray() : QJsonArray();
            QJsonArray tabReplace =
                  obj.contains(jsonReplace) ? obj[jsonReplace].toArray() : QJsonArray();
            QJsonObject objFile;

            for (int i = 0; i < tabRemove.size(); i++) {
                objFile = tabRemove.at(i).toObject();
                download(EngineUpdateFileKind::Remove, objFile, version);
            }
            for (int i = 0; i < tabAdd.size(); i++) {
                objFile = tabAdd.at(i).toObject();
                download(EngineUpdateFileKind::Add, objFile, version);
            }
            for (int i = 0; i < tabReplace.size(); i++) {
                objFile = tabReplace.at(i).toObject();
                download(EngineUpdateFileKind::Replace, objFile, version);
            }
        }
    }
    downloadScripts(version);
    downloadExecutables();
    downloadTranslations(version);
    writeVersion(version);
    setCurrentCount(m_countFiles);
}

// -------------------------------------------------------

void EngineUpdater::handleReading(QNetworkReply *reply, QFile *file)
{
    QString path = reply->property("path").toString();
    QString source = reply->property("source").toString();

    if (!reply->property("link").toBool())
    {
        if (!file->isOpen())
        {
            file->open(QIODevice::ReadWrite);
        }
        file->write(reply->readAll());
    }
}

// -------------------------------------------------------

void EngineUpdater::handleFinished(QNetworkReply *reply, QFile *file) {
    QString path = reply->property("path").toString();
    QString source = reply->property("source").toString();
    bool exe = reply->property("exe").toBool();
    bool link = reply->property("link").toBool();


    QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!possibleRedirectUrl.isEmpty())
    {
        QString target = reply->property("target").toString();
        removeFile(target);
        m_countFiles--;
        this->addFileURL(possibleRedirectUrl, source, target, exe, link, path);
        return;
    }

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        if (m_messageError.isEmpty())
        {
            m_messageError = "Could not copy from " + source + " to " + path +
                             ": " + reply->errorString();
            emit filesFinished();
        }
        return;
    }
    if (link) {
        QDir dir(path);
        dir.cdUp();
        m_links.append(QPair<QString, QString>(Common::pathCombine(dir
            .absolutePath(), reply->readAll()), path));
    } else {
        if (!file->isOpen())
        {
            file->open(QIODevice::ReadWrite);
        }
        file->write(reply->readAll());

        // If exe, change permissions
        if (exe) {
            file->setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser |
                                QFileDevice::ExeUser | QFileDevice::ReadGroup |
                                QFileDevice::ExeGroup | QFileDevice::ReadOther |
                                QFileDevice::ExeOther);
        }
        file->close();
        delete file;
    }
    m_mutex.lock();
    m_countFiles--;
    emit addOne();
    m_mutex.unlock();

    if (m_countFiles == 0) {
        emit progress(100, "Finishing...");
        emit progressDescription("");
        #ifdef Q_OS_WIN
            // ...
        #else
            while (!m_links.isEmpty()) {
                for (int i = m_links.size() - 1; i >= 0; i--) {
                    QPair<QString, QString> pair = m_links.at(i);
                    if (QFile(pair.first).exists() || QDir(pair.first).exists()) {
                        QFile::link(pair.first, pair.second);
                        m_links.removeAt(i);
                    }
                }
            }
        #endif
        emit filesFinished();
    }
}

// -------------------------------------------------------

void EngineUpdater::onDownloadProgress(QString source, qint64 a, qint64 b)
{
    if (m_focusProgress.isEmpty())
    {
        m_focusProgress = source;
    }
    if (m_focusProgress == source)
    {
        if (b != 0)
        {
            qint64 res = qRound((static_cast<qreal>(a) / b) * 100);
            if (res > 0)
            {
                emit progressDescription("Downloading " + source + " (" + QString
                    ::number(res) + "%) - " + QString::number(static_cast<qreal>(a) / 1000000, 'f', 2) + "MB / " + QString::number(static_cast<qreal>(b) / 1000000, 'f', 2) + "MB");
                if (res == 100)
                {
                    m_focusProgress = "";
                }
            }
        }
    }
}

// -------------------------------------------------------

void EngineUpdater::setCurrentCount(int c) {
    emit setCount(c);
}

// -------------------------------------------------------

void EngineUpdater::fillVersionsComboBox(QComboBox *comboBox) {
    for (int i = m_versions.size() - 2; i >= 0; i--) {
         comboBox->addItem(m_versions.at(i).toString());
    }
}

// -------------------------------------------------------

void EngineUpdater::downloadTranslations(QString version)
{
    QJsonObject objTranslations = m_document[jsonTranslations].toObject();
    downloadFolder(EngineUpdateFileKind::Replace, objTranslations, version);
}

// -------------------------------------------------------

void EngineUpdater::downloadLargeFiles(QString version)
{
    if (Common::versionDifferent(version, EngineUpdater::ELECTRON_VERSION) != -1)
    {
        this->downloadLargeFile(version, "Game.exe", "Engine/Content/win32/Game.exe");
        this->downloadLargeFile(version, "Game", "Engine/Content/linux/Game");
        this->downloadLargeFile(version, "Electron.Framework", "Engine/Content/osx/Game.app/Contents/Frameworks/Electron Framework.framework/Versions/A/Electron Framework");
    }
}

// -------------------------------------------------------

void EngineUpdater::downloadLargeFile(QString version, QString filename, QString
    target)
{
    QUrl url = QUrl("https://github.com/RPG-Paper-Maker/Dependencies/releases/"
        "download/" + version + "/" + filename);
    addFileURL(url, url.toString(), target, true, false, Common::pathCombine(
        QDir::currentPath(), target));
}
