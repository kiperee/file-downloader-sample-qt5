
#include "downloaderwidget.h"

#include <QGridLayout>

DownloaderWidget::DownloaderWidget(QWidget *parent)
    : QWidget(parent),
    downloader(new Downloader(this)),
    urlLabel(new QLabel(this)),
    progressBar(new QProgressBar(this)),
    statusLabel(new QLabel(this)),
    cancelButton(new QPushButton(this)),
    layout(new QGridLayout(this))
{
    cancelButton->setText(tr("Cancel"));

    connect(cancelButton, &QPushButton::clicked, downloader, &Downloader::cancel);
    connect(downloader, &Downloader::downloadStatusUpdate, this, &DownloaderWidget::downloadStatusUpdate);
    connect(downloader, &Downloader::downloadStatusError, this, &DownloaderWidget::downloadStatusError);
    connect(downloader, &Downloader::downloadFinished, this, &DownloaderWidget::downloadFinished);
    connect(downloader, &Downloader::downloadAbort, this, &DownloaderWidget::downloadAbort);

    layout->addWidget(urlLabel);
    layout->addWidget(progressBar);
    layout->addWidget(statusLabel);
    layout->addWidget(cancelButton);

    setLayout(layout);
}

void DownloaderWidget::downloadStatusUpdate(qint64 percents, qint64 speed, qint64 remaining)
{
    progressBar->setValue(int(percents));

    QString unit;

    if (speed > 1024 * 1024) {
        unit = "MB/s";
        speed /= 1024 * 1024;
    } else if (speed > 1024) {
        unit = "KB/s";
        speed /= 1024;
    } else {
        unit = "bytes/s";
    }

    QString remainingStr;

    if (remaining >= 60 * 60 * 24) {
        remainingStr += QString("%1 days ").arg(remaining / (60 * 60 * 24));
        remaining %= 60 * 60 * 24;
    }

    if (remaining >= 60 * 60) {
        remainingStr += QString("%1 hours ").arg(remaining / (60 * 60));
        remaining %= 60 * 60;
    }

    if (remaining >= 60) {
        remainingStr += QString("%1 minutes ").arg(remaining / 60);
        remaining %= 60;
    }

    if (remaining >= 0) {
        remainingStr += QString("%1 seconds ").arg(remaining);
    }

    remainingStr += "remaining";

    statusLabel->setText(tr("%1 (%2 %3)").arg(remainingStr).arg(speed).arg(unit));
}

void DownloaderWidget::downloadStatusError(const QString &errorString)
{
    statusLabel->setText(errorString);
}

void DownloaderWidget::download(const QString &downloadUrl, const QString &fileName)
{
    urlLabel->setText(QString("Downloading: %1").arg(downloadUrl));
    downloader->download(downloadUrl, fileName);
}

DownloaderWidget::~DownloaderWidget()
{

}
