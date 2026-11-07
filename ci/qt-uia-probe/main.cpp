#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QMainWindow window;
    window.resize(500, 400);
    auto *view = new QTreeView(&window);
    window.setCentralWidget(view);
    auto *model = new QStandardItemModel(0, 2, view);
    for (int i = 0; i < 3; ++i) {
        auto *parent = new QStandardItem(QStringLiteral("Group %1").arg(i));
        model->appendRow({parent, new QStandardItem});
        parent->appendRow({new QStandardItem(QStringLiteral("Option")), new QStandardItem});
    }
    view->setModel(model);
    view->setRootIsDecorated(false);
    view->setHeaderHidden(true);
    window.show();
    view->expandAll();

    const auto log = [](const QString &message) {
        QFile file(qEnvironmentVariable("FC_PROBE_LOG"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << message << Qt::endl;
        }
    };
    log(QStringLiteral("Qt=%1").arg(QString::fromLatin1(qVersion())));
    log(QStringLiteral("READY %1 %2").arg(QCoreApplication::applicationPid())
        .arg(quintptr(window.winId())));

    auto *timer = new QTimer(&window);
    timer->setInterval(100);
    QObject::connect(timer, &QTimer::timeout, &window, [=, count = 0]() mutable {
        const QModelIndex index = model->index(count % 3, 0);
        view->expand(index);
        log(QStringLiteral("BEFORE %1").arg(count));
        view->collapse(index);
        log(QStringLiteral("AFTER %1").arg(count));
        if (++count == 60) {
            timer->stop();
            log(QStringLiteral("DONE"));
        }
    });
    QTimer::singleShot(5000, timer, qOverload<>(&QTimer::start));
    return app.exec();
}
