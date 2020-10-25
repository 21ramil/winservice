#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "commom.h"
#include "interaction.h"
#include "calcworker.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Calculator Create Company");
    app.setOrganizationDomain("ccc.com");
    app.setApplicationName("Calculator");

    // Очередь на выполнение вычислений
    // время и выражение
    using queueReqType = QQueue < std::pair< int, QString > >;
    std::shared_ptr < queueReqType > queueRequests = std::make_shared< queueReqType > ();

    // Очередь готовых результатов
    using queueResType = QQueue < std::pair< QString, QString > >;
    std::shared_ptr < queueResType > queueResults =  std::make_shared< queueResType > ();

    // объект для взаимодействия с UI, подготовки запросов на расчет и их вывод из очереди
    std::shared_ptr< UIInteraction> interact = std::make_shared< UIInteraction> ();
    interact->setQueueRequestPointer(queueRequests);
    interact->setQueueResultPointer(queueResults);
    interact->runLongCalc();
    interact->start();

    // объект для вычисления выражения с загрузкой динамической библиотеки
    std::shared_ptr< CalcWorker> calcWorker = std::make_shared< CalcWorker >();
    calcWorker->setQueueRequestPointer(queueRequests);
    calcWorker->setQueueResultPointer(queueResults);
    calcWorker->runLongCalc();
    calcWorker->start();

    // Остановка потока с ожиданием
    auto stopThread = [] (QThread *ptr)
    {
        if ( ptr )
        {
            QObject::disconnect(ptr,nullptr,nullptr,nullptr);
            ptr->terminate();
            while(!ptr->isFinished())
            {}
        }
    };

    // загрузка ui из qml
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/ui.qml")));

    // проверка существования корневых объектов в ui
    auto allObjects = engine.rootObjects();
    if (allObjects.isEmpty())
    {
        interact->stopLongCalc();
        calcWorker->stopLongCalc();
        stopThread(interact.get());
        stopThread(calcWorker.get());
        return -1;
    }

    QObject *item = allObjects[0];
    if (item)
    {
        // сохраняем указатель на объект для вызова функций из ui
        interact->setQMLObject (item);
        // подписка на сигнал вычисления выражения
        QObject::connect(item, SIGNAL(genericExpression(int, QString)), interact.get(), SLOT(getExpressionSlot(int, QString)));

        // уведомление пользователя об изменении размера очереди запросов
        QObject::connect(calcWorker.get(), &CalcWorker::updateQueueRequests, [=](int queueSize)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, greenColor + "Выражений в очереди на вычисление: " + QString::number(queueSize) + endColor ));
        });
        QObject::connect(interact.get(), &UIInteraction::updateQueueRequests, [=](int queueSize)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, greenColor + "Выражений в очереди на вычисление: " + QString::number(queueSize) + endColor ));
        });

        // уведомление пользователя об изменении размера очереди подготовленных результатов
        QObject::connect(calcWorker.get(), &CalcWorker::updateQueueResults, [=](int queueSize)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, blueColor + "Результатов в очереди на отображение: " + QString::number(queueSize) + endColor ));
        });
        QObject::connect(interact.get(), &UIInteraction::updateQueueResults, [=](int queueSize)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, blueColor + "Результатов в очереди на отображение: " + QString::number(queueSize) + endColor ));
        });

        // уведомление пользователя об ошибках
        QObject::connect(calcWorker.get(), &CalcWorker::error, [=](QString err)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, redColor + err + endColor ));
        });
        QObject::connect(interact.get(), &UIInteraction::error, [=](QString err)
        {
            QMetaObject::invokeMethod(item, "updateInfoArea", Q_ARG(QVariant, redColor + err + endColor ));
        });
    }

    // вход в основной цикл обработки событий
    auto returnCode = app.exec();

    // завершение работы, остановка потоков
    interact->stopLongCalc();
    calcWorker->stopLongCalc();
    stopThread(interact.get());
    stopThread(calcWorker.get());

    return returnCode;
}
