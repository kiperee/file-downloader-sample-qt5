#ifndef DOWNLOADERWIDGET_H
#define DOWNLOADERWIDGET_H

#include "downloader.h"

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

class DownloaderWidget : public QWidget
{
    Q_OBJECT

private:
    Downloader *downloader;
    QLabel *urlLabel;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QPushButton *cancelButton;
    QGridLayout *layout;

public:
    DownloaderWidget(QWidget *parent = nullptr);
    void download(const QString &, const QString &);
    ~DownloaderWidget();

signals:
    void downloadFinished();
    void downloadAbort();

public slots:
    void downloadStatusUpdate(qint64, qint64, qint64);
    void downloadStatusError(const QString &);
};

#endif // DOWNLOADERWIDGET_H
