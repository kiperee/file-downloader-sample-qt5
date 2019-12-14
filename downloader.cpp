
#include "downloader.h"

#include <QDir>
#include <QMessageBox>
#include <QTimer>

Downloader::Downloader(QObject *parent)
    : QObject(parent),
    reply(nullptr),
    file(nullptr),
    cancelled(false),
    resume(false),
    lastReceived(0),
    bytesReceived(0),
    bytesTotal(0)
{
}

QFile *Downloader::openFile(const QString &fileName, bool forceOverwrite)
{
    if (!forceOverwrite && QFile::exists(fileName)) {
        auto answer = QMessageBox::question(
          nullptr,
          QString("File exists"),
          QString("Overwrite existing file %1?").arg(fileName),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No
        );

        if (answer == QMessageBox::No) return nullptr;
    }

    QScopedPointer<QFile> file(new QFile(fileName));

    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(
            nullptr,
            QString("Error"),
            QString("Unable to open file %1: %2.").arg(
                QDir::toNativeSeparators(fileName),
                file->errorString()
            )
        );
        return nullptr;
    }

    return file.take();
}

void Downloader::download(const QString &downloadUrl, const QString &fileName)
{
    if (downloadUrl.isEmpty()) {
        emit downloadAbort();
        return;
    }

    url = QUrl::fromUserInput(downloadUrl);

    if (!url.isValid()) {
        QMessageBox::information(
            nullptr,
            tr("Error"),
            tr("Invalid URL: %1: %2").arg(
              downloadUrl,
              url.errorString()
            )
        );

        emit downloadAbort();
        return;
    }

    if (fileName.isEmpty()) {
        QString urlFileName = url.fileName();
        if (urlFileName.isEmpty()) urlFileName = "index.html";
        file = openFile(urlFileName);
    } else {
        file = openFile(fileName);
    }

    if (!file) {
        emit downloadAbort();
        return;
    }

    resume = false;
    cancelled = false;

    startRequest();
}

void Downloader::startRequest()
{
    if (resume) {
        lastReceived = 0;

        QNetworkRequest request(url);
        request.setRawHeader("Range", "bytes=" + QByteArray::number(file->size()) + "-");
        reply = manager.get(request);
    } else {
        lastReceived = 0;
        bytesReceived = 0;
        bytesTotal = 0;

        reply = manager.get(QNetworkRequest(url));
    }

    connect(reply, &QIODevice::readyRead, this, &Downloader::readyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::downloadProgress);
    connect(reply, &QNetworkReply::finished, this, &Downloader::finished);

    timeCounter.start();
}

void Downloader::readyRead()
{
    if (!file) return;

    if (resume) {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (status_code.toInt() != 206) {
            lastReceived = 0;
            bytesReceived = 0;
            bytesTotal = 0;
            resume = false;
            file->resize(0);
        }
    }

    file->write(reply->readAll());
}

void Downloader::downloadProgress(qint64 received, qint64 total)
{
    if (total <= 0) return;

    if (bytesTotal == 0) bytesTotal = total;
    bytesReceived += received - lastReceived;

    qint64 percents = 100 * bytesReceived / bytesTotal;
    qint64 speed = received * 1000 / timeCounter.elapsed();
    qint64 remaining = (total - received) / speed;

    lastReceived = received;

    emit downloadStatusUpdate(percents, speed, remaining);
}

void Downloader::finished()
{
    if (reply->error() && !cancelled) {
        emit downloadStatusError(reply->errorString());

        reply->deleteLater();
        reply = nullptr;

        resume = true;
        QTimer::singleShot(1000, this, &Downloader::startRequest);
        return;
    }

    QFileInfo fi;
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        delete file;
        file = nullptr;
    }

    if (cancelled) {
        QFile::remove(fi.absoluteFilePath());

        reply->deleteLater();
        reply = nullptr;

        emit downloadFinished();
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    reply->deleteLater();
    reply = nullptr;

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());

        file = openFile(fi.absoluteFilePath(), true);

        if (!file) {
            emit downloadAbort();
            return;
        }

        url = redirectedUrl;

        startRequest();
        return;
    }

    emit downloadFinished();
}

void Downloader::cancel()
{
    cancelled = true;
    if (reply) reply->abort();
}
