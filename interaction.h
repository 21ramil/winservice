#ifndef UI_INTERACTION_H
#define UI_INTERACTION_H

#include <memory>
#include <utility>
#include <QObject>
#include <QThread>
#include <QString>
#include <QQueue>
#include <QWriteLocker>

/**
 * \brief Взаимодействие с пользовательским интерфейсом
 */
class UIInteraction : public QThread
{
    Q_OBJECT
public:
    //! \brief Конструктор
    UIInteraction ();

    //! \brief Деструктор
    ~UIInteraction () override;

     //! \brief Задать указатель на объект QML
     //! \param [in] obj ненулевой указатель
    void setQMLObject (QObject *obj);

    //! \brief Задать указатель на объект QML
    //! \param [in] obj ненулевой указатель
    void setQueueRequestPointer (std::shared_ptr<QQueue < std::pair< int, QString > > > obj);

    //! \brief Задать указатель на объект QML
    //! \param [in] obj ненулевой указатель
    void setQueueResultPointer (std::shared_ptr<QQueue < std::pair< QString, QString > > > obj);

    //! \brief Зацикливание потока для ожидания очереди
    void runLongCalc ( );

    //! \brief Остановка потока
    void stopLongCalc ( );

Q_SIGNALS:
    //! \brief Сигнал об обновлении очереди запросов
    //! \param [out] count количество запросов в очереди
    void updateQueueRequests (int count);

    //! \brief Сигнал об обновлении очереди результатов
    //! [out] count количество результатов в очереди
    void updateQueueResults (int count);

    //! \brief Сигнал об ошибке
    //! [out] err текст ошибки
    void error(QString err);

public Q_SLOTS:
    /**
     * \brief Слот реагирования на нажатие клавиши равно
     * \param [in] time продолжительность вычисления
     * \param [in] expr выражение, которое необходимо вычислить
     */
    void getExpressionSlot(int time, QString expr);

private:
    //! \brief Входная точка для потока
    void run() override;

private:
    //! \brief Для обеспечения блокировки из разных потоков
    QReadWriteLock lock;

    //! \brief Указатель на объект QML
    QObject *m_qmlObject;

    //! \brief Очередь на выполнение вычислений
    //! содержит пару из продолжительности вычисления и выражения
    std::shared_ptr < QQueue < std::pair< int, QString > > > m_QueueRequests;

    //! \brief Очередь готовых резуьтатов
    std::shared_ptr < QQueue < std::pair< QString, QString > > > m_QueueResults;

    //! \brief Флаг зацикливания потока
    bool isRunLongCalc;
};
#endif // UI_INTERACTION_H
