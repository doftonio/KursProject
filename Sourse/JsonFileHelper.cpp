#include "../Headers/JsonFileHelper.h"
#include "../Exeptions/FileStorageException.h"
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QDateTime>

void JsonFileHelper::ensureDirectory(const QString& dirPath) {
    if (QDir dir; !dir.exists(dirPath) && !dir.mkpath(dirPath)) {
        throw FileStorageException("Cannot create directory: " + QFileInfo(dirPath).absoluteFilePath().toStdString());
    }
}

void JsonFileHelper::writeAtomic(const QString& filePath, const QByteArray& content) {
    if (QSaveFile saveFile(filePath); saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (saveFile.write(content) != -1 && saveFile.commit()) {
            return;
        }
    }

    QFile directFile(filePath);
    if (!directFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        throw FileStorageException(
            "Cannot write file: " + QFileInfo(filePath).absoluteFilePath().toStdString() +
            " (" + directFile.errorString().toStdString() + ")"
        );
    }
    if (directFile.write(content) == -1) {
        throw FileStorageException("Failed to write data to file: " + filePath.toStdString());
    }
    directFile.flush();
    directFile.close();
}

QJsonDocument JsonFileHelper::readDocument(const QString& filePath, bool expectObject, bool& isEmptyOrMissing) {
    isEmptyOrMissing = false;
    QFile file(filePath);
    if (!file.exists()) {
        isEmptyOrMissing = true;
        return QJsonDocument();
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw FileStorageException("Cannot open file: " + filePath.toStdString() + " (" + file.errorString().toStdString() + ")");
    }
    QByteArray raw = file.readAll().trimmed();
    if (raw.isEmpty()) {
        isEmptyOrMissing = true;
        return QJsonDocument();
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError) {
        throw FileStorageException("Corrupted JSON in " + filePath.toStdString() + ": " + err.errorString().toStdString());
    }
    if (expectObject && !doc.isObject()) {
        throw FileStorageException("Expected JSON object in " + filePath.toStdString());
    }
    if (!expectObject && !doc.isArray()) {
        throw FileStorageException("Expected JSON array in " + filePath.toStdString());
    }
    return doc;
}

QString JsonFileHelper::copyBackupSnapshot(const QString& sourceDir, const QString& backupRoot, const QStringList& fileNames) {
    QString stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString targetFolder = backupRoot + "/backup_" + stamp;
    ensureDirectory(targetFolder);

    for (const auto& fileName : fileNames) {
        QString src = sourceDir + "/" + fileName;
        QString dst = targetFolder + "/" + fileName;
        if (QFile::exists(src)) {
            if (QFile::exists(dst)) {
                QFile::remove(dst);
            }
            if (!QFile::copy(src, dst)) {
                throw FileStorageException("Failed to copy file to backup: " + fileName.toStdString());
            }
        }
    }
    return targetFolder;
}

void JsonFileHelper::restoreBackupSnapshot(const QString& backupFolder, const QString& targetDir, const QStringList& fileNames) {
    if (QDir dir(backupFolder); !dir.exists()) {
        throw FileStorageException("Backup directory does not exist: " + backupFolder.toStdString());
    }
    for (const auto& fileName : fileNames) {
        QString src = backupFolder + "/" + fileName;
        if (!QFile::exists(src)) {
            throw FileStorageException("Missing file in backup archive: " + fileName.toStdString());
        }
        bool dummy = false;
        readDocument(src, fileName == "app_state.json", dummy);
    }
    for (const auto& fileName : fileNames) {
        QString src = backupFolder + "/" + fileName;
        QString dst = targetDir + "/" + fileName;
        if (QFile::exists(dst)) {
            QFile::remove(dst);
        }
        if (!QFile::copy(src, dst)) {
            throw FileStorageException("Failed to restore file: " + fileName.toStdString());
        }
    }
}
