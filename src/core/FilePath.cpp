/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FilePath.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibrary>

#include "config-keepassx.h"

FilePath* FilePath::m_instance(Q_NULLPTR);

QString FilePath::dataPath(const QString& name)
{
    if (name.isEmpty() || name.startsWith('/')) {
        return m_dataPath + name;
    }
    else {
        return m_dataPath + "/" + name;
    }
}

QString FilePath::pluginPath(const QString& name)
{
    QStringList pluginPaths;

    QDir buildDir(QCoreApplication::applicationDirPath() + "/autotype");
    Q_FOREACH (const QString& dir, buildDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        pluginPaths << QCoreApplication::applicationDirPath() + "/autotype/" + dir;
    }

    // for TestAutoType
    pluginPaths << QCoreApplication::applicationDirPath() + "/../src/autotype/test";

    pluginPaths << QCoreApplication::applicationDirPath();

    QString configuredPluginDir = KEEPASSX_PLUGIN_DIR;
    if (configuredPluginDir != ".") {
        if (QDir(configuredPluginDir).isAbsolute()) {
            pluginPaths << configuredPluginDir;
        }
        else {
            QString relativePluginDir = QString("%1/../%2")
                    .arg(QCoreApplication::applicationDirPath(), configuredPluginDir);
            pluginPaths << QDir(relativePluginDir).canonicalPath();

            QString absolutePluginDir = QString("%1/%2")
                    .arg(KEEPASSX_PREFIX_DIR, configuredPluginDir);
            pluginPaths << QDir(absolutePluginDir).canonicalPath();
        }
    }

    QStringList dirFilter;
    dirFilter << QString("*%1*").arg(name);

    Q_FOREACH (const QString& path, pluginPaths) {
        QStringList fileCandidates = QDir(path).entryList(dirFilter, QDir::Files);

        Q_FOREACH (const QString& file, fileCandidates) {
            QString filePath = path + "/" + file;

            if (QLibrary::isLibrary(filePath)) {
                return filePath;
            }
        }
    }

    return QString();
}

QIcon FilePath::applicationIcon()
{
    return icon("apps", "keepassx");
}

QIcon FilePath::icon(const QString& category, const QString& name, bool fromTheme)
{
    QString combinedName = category + "/" + name;

    QIcon icon = m_iconCache.value(combinedName);

    if (!icon.isNull()) {
        return icon;
    }

    if (fromTheme) {
        icon = QIcon::fromTheme(name);
    }

    if (icon.isNull()) {
        QList<int> pngSizes;
        pngSizes << 16 << 22 << 24 << 32 << 48 << 64 << 128;
        QString filename;
        Q_FOREACH (int size, pngSizes) {
            filename = QString("%1/icons/application/%2x%2/%3.png").arg(m_dataPath, QString::number(size),
                                                                        combinedName);
            if (QFile::exists(filename)) {
                icon.addFile(filename, QSize(size, size));
            }
        }
        filename = QString("%1/icons/application/scalable/%3.svgz").arg(m_dataPath, combinedName);
        if (QFile::exists(filename)) {
            icon.addFile(filename);
        }
    }

    m_iconCache.insert(combinedName, icon);

    return icon;
}

QIcon FilePath::onOffIcon(const QString& category, const QString& name)
{
    QString combinedName = category + "/" + name;
    QString cacheName = "onoff/" + combinedName;

    QIcon icon = m_iconCache.value(cacheName);

    if (!icon.isNull()) {
        return icon;
    }

    for (int i = 0; i < 2; i++) {
        QIcon::State state;
        QString stateName;

        if (i == 0) {
            state = QIcon::Off;
            stateName = "off";
        }
        else {
            state = QIcon::On;
            stateName = "on";
        }

        QList<int> pngSizes;
        pngSizes << 16 << 22 << 24 << 32 << 48 << 64 << 128;
        QString filename;
        Q_FOREACH (int size, pngSizes) {
            filename = QString("%1/icons/application/%2x%2/%3-%4.png").arg(m_dataPath, QString::number(size),
                                                                           combinedName, stateName);
            if (QFile::exists(filename)) {
                icon.addFile(filename, QSize(size, size), QIcon::Normal, state);
            }
        }
        filename = QString("%1/icons/application/scalable/%3-%4.svgz").arg(m_dataPath, combinedName, stateName);
        if (QFile::exists(filename)) {
            icon.addFile(filename, QSize(), QIcon::Normal, state);
        }
    }

    m_iconCache.insert(cacheName, icon);

    return icon;
}

FilePath::FilePath()
{
    const QString appDirPath = QCoreApplication::applicationDirPath();
    bool isDataDirAbsolute = QDir::isAbsolutePath(KEEPASSX_DATA_DIR);

    if (false) {
    }
#ifdef QT_DEBUG
    else if (testSetDir(QString(KEEPASSX_SOURCE_DIR) + "/share")) {
    }
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    else if (isDataDirAbsolute && testSetDir(KEEPASSX_DATA_DIR)) {
    }
    else if (!isDataDirAbsolute && testSetDir(QString("%1/../%2").arg(appDirPath, KEEPASSX_DATA_DIR))) {
    }
    else if (!isDataDirAbsolute && testSetDir(QString("%1/%2").arg(KEEPASSX_PREFIX_DIR, KEEPASSX_DATA_DIR))) {
    }
#endif
#ifdef Q_OS_MAC
    else if (testSetDir(appDirPath + "/../Resources")) {
    }
#endif
#ifdef Q_OS_WIN
    else if (testSetDir(appDirPath + "/share")) {
    }
#endif

    if (m_dataPath.isEmpty()) {
        qWarning("FilePath::DataPath: can't find data dir");
    }
    else {
        m_dataPath = QDir::cleanPath(m_dataPath);
    }
}

bool FilePath::testSetDir(const QString& dir)
{
    if (QFile::exists(dir + "/icons/database/C00_Password.png")) {
        m_dataPath = dir;
        return true;
    }
    else {
        return false;
    }
}

FilePath* FilePath::instance()
{
    if (!m_instance) {
        m_instance = new FilePath();
    }

    return m_instance;
}
