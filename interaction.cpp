#include "interaction.h"
#include "commom.h"
#include <QVariant>
UIInteraction::UIInteraction(): m_qmlObject (nullptr), isRunLongCalc(false)
{
    setObjectName("UIInteractionObject");
}

UIInteraction::~UIInteraction()
{

}

void UIInteraction::runLongCalc()
{
    isRunLongCalc = true;
}

void UIInteraction::stopLongCalc()
{
    isRunLongCalc = false;
}

void UIInteraction::setQMLObject(QObject *obj)
{
    m_qmlObject = obj;
}

void UIInteraction::setQueueRequestPointer(std::shared_ptr < QQueue < std::pair < int, QString > > > obj)
{
    m_QueueRequests = obj;
}

void UIInteraction::setQueueResultPointer(std::shared_ptr < QQueue < std::pair< QString, QString > > > obj)
{
    m_QueueResults = obj;
}

void UIInteraction::getExpressionSlot(int time, QString expr)
{
    if ( m_QueueRequests)
    {
        lock.lockForWrite();
        m_QueueRequests->enqueue({time, expr});
        lock.unlock();
        Q_EMIT updateQueueRequests( m_QueueRequests->size());
        if ( m_qmlObject )
            QMetaObject::invokeMethod(m_qmlObject, "updateInfoArea", Q_ARG ( QVariant, greenColor + "Запрос: " + expr + endColor));
    }
    else
    {
        Q_EMIT error("Отсутствует очередь запросов");
    }
}

void UIInteraction::run()
{
    while ( isRunLongCalc )
    {
        if ( m_QueueResults )
        {
            if(!m_QueueResults->isEmpty() )
            {
                lock.lockForWrite();
                auto result = m_QueueResults->dequeue();
                lock.unlock();
                if ( m_qmlObject )
                    QMetaObject::invokeMethod(m_qmlObject, "updateInfoArea", Q_ARG(QVariant, blueColor + "Результат: " + result.first + "=" + result.second + endColor));
                Q_EMIT updateQueueResults ( m_QueueResults->size());
            }
        }
        else
            Q_EMIT error("Отсутствует очередь результатов");

        QThread::msleep(200);
    }
}

