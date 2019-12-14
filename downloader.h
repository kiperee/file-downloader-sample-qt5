#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class Downloader : public QObject
{
    Q_OBJECT

private:
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QFile *file;
    QUrl url;

    bool cancelled;
    bool resume;

    qint64 lastReceived;
    qint64 bytesReceived, bytesTotal;

    QTime timeCounter;

    QFile *openFile(const QString &fileName, bool forceOverwrite = false);

public:
    Downloader(QObject *parent = nullptr);

signals:
    void downloadStatusUpdate(qint64, qint64, qint64);
    void downloadStatusError(const QString &);
    void downloadFinished();
    void downloadAbort();

private slots:
    void startRequest();
    void readyRead();
    void downloadProgress(qint64, qint64);
    void finished();

public slots:
    void download(const QString &, const QString &);
    void cancel();
};

#endif // DOWNLOADER_H
