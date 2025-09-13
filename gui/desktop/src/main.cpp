#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("EtherCAT Simulator GUI (Qt6/QML)");
    parser.addHelpOption();
    QCommandLineOption smokeOpt("smoke", "Run smoke test and exit.");
    parser.addOption(smokeOpt);
    parser.process(app);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    if (parser.isSet(smokeOpt)) {
        // Smoke mode: we exit right away if QML loaded.
        return 0;
    }

    return app.exec();
}

