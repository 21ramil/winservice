#ifndef CALCWORKER_H
#define CALCWORKER_H

#include <memory>
#include <utility>
#include <QThread>
#include <QString>
#include <QQueue>
#include <QWriteLocker>

/**
 * \brief Вычисление выражения в отдельном потоке с загрузкой динамической библиотеки
 */
class CalcWorker : public QThread
{
    Q_OBJECT
public:
    //! \brief Конструктор
    CalcWorker ();

    //! \brief Деструктор
    virtual ~CalcWorker () override;

    //! \brief Задать указатель на очередь запросов на вычисление
    //! \param [in] obj ненулевой указатель
    void setQueueRequestPointer ( std::shared_ptr < QQueue < std::pair< int, QString > > > obj );

    //! \brief Задать указатель на объект QML
    //! \param [in] obj ненулевой указатель
    void setQueueResultPointer (std::shared_ptr < QQueue < std::pair< QString, QString > > > obj);

    //! \brief Зацикливание потока для ожидания очереди
    void runLongCalc ( );

    //! \brief Остановка потока
    void stopLongCalc ( );

private:
    //! \brief Входная точка для потока
    void run() override;

    //! \brief Вычисление выражения с заданной задержкой
    //! \param [in] time продолжительность вычисления
    //! \param [in] expr выражение для вычисления
    void doCalc(const int time, const QString expr);

Q_SIGNALS:
    //! \brief Сигнал об ошибке
    //! \param [out] err текст ошибки
    void error(QString err);

    //! \brief Сигнал об обновлении очереди запросов
    //! \param [out] count количество запросов в очереди
    void updateQueueRequests (int count);

    //! \brief Сигнал об обновлении очереди результатов
    //! \param [out] count количество результатов в очереди
    void updateQueueResults (int count);

private:
    //! \brief Для обеспечения блокировки критических участков для разных потоков
    QReadWriteLock lock;

    //! \brief Прототип функции из dll
    using doItPrototype = double (*) (int, double, double, int&);

    //! \brief Указатель на адресс функции в библиотеке
    doItPrototype m_doItFunction;

    //! \brief Cписок возможных операций
    QList< QString > m_operations;

    //! \brief Очередь на вычисление
    //! содержит пару из продолжительности вычисления и выражения
    std::shared_ptr < QQueue < std::pair< int, QString > > > m_request;

    //! \brief Очередь готовых резуьтатов
    std::shared_ptr < QQueue < std::pair< QString, QString > > > m_results;

    //! \brief Флаг зацикливания потока
    bool isRunLongCalc;
};

#endif // CALCWORKER_H
