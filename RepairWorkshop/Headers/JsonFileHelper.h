#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QStringList>

class JsonFileHelper {
public:
    static void ensureDirectory(const QString& dirPath);
    static void writeAtomic(const QString& filePath, const QByteArray& content);
    static QJsonDocument readDocument(const QString& filePath, bool expectObject, bool& isEmptyOrMissing);
    static QString copyBackupSnapshot(const QString& sourceDir, const QString& backupRoot, const QStringList& fileNames);
    static void restoreBackupSnapshot(const QString& backupFolder, const QString& targetDir, const QStringList& fileNames);
};

#endif
