
#include "downloaderwidget.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

#include <cstdlib>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;

    parser.setApplicationDescription("Simple file downloader tool");
    QCommandLineOption optionHelp = parser.addHelpOption();
    parser.addPositionalArgument("URL", QCoreApplication::translate("main", "URL to be downloaded"));
    parser.addPositionalArgument("[FILE]", QCoreApplication::translate("main", "output file name"));

    parser.parse(app.arguments());

    if (parser.isSet(optionHelp)) {
        parser.showHelp();
        exit(EXIT_SUCCESS);
    }

    const QStringList args = parser.positionalArguments();

    QString url, fileName;

    if (args.size() > 2) {
        QMessageBox::critical(
            nullptr,
            "Error",
            QString("Too many arguments. Launch %1 --help to learn more.")
                .arg(app.arguments().at(0))
        );
        exit(EXIT_FAILURE);
    }

    if (args.size() >= 1) {
        url = args.at(0);
    } else {
        QMessageBox::critical(
            nullptr,
            "Error",
            QString("Must specify URL. Launch %1 --help to learn more.")
                .arg(app.arguments().at(0))
        );
        exit(EXIT_FAILURE);
    }

    if (args.size() == 2) {
        fileName = args.at(1);
    }

    DownloaderWidget dlWidget;
    QObject::connect(&dlWidget, &DownloaderWidget::downloadFinished, &app, QApplication::quit, Qt::QueuedConnection);
    QObject::connect(&dlWidget, &DownloaderWidget::downloadAbort, &app, QApplication::quit, Qt::QueuedConnection);
    dlWidget.show();
    dlWidget.download(url, fileName);

    return app.exec();
}
