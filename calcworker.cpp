#include "calcworker.h"
#include <QLibrary>

CalcWorker::CalcWorker(): isRunLongCalc (false)
{
    setObjectName("CalcWorkerObject");

    m_operations = {"÷", "×", "+", "-"};

    try
    {
        QLibrary lib("doIt");
        if (lib.load())
        {

            QFunctionPointer funcPointer = lib.resolve("DoIt");
            m_doItFunction = reinterpret_cast<doItPrototype>(funcPointer);
        }
        else
        {
            Q_EMIT error("Ошибка загрузки библиотеки doIt");
        }
    }
    catch (...)
    {
        Q_EMIT error("Необработанная ошибка при создании помощника калькулятора");
    }
}

CalcWorker::~CalcWorker()
{

}

void CalcWorker::setQueueRequestPointer(std::shared_ptr < QQueue < std::pair< int, QString > > > request)
{
    m_request = request;
}

void CalcWorker::setQueueResultPointer(std::shared_ptr < QQueue < std::pair < QString, QString > > > obj)
{
    m_results = obj;
}

void CalcWorker::runLongCalc()
{
    isRunLongCalc = true;
}

void CalcWorker::stopLongCalc()
{
    isRunLongCalc = false;
}

void CalcWorker::run()
{
    while ( isRunLongCalc )
    {
        if ( !m_request )
            Q_EMIT error("Отсутствует очередь запросов");
        else
        {
            while (!m_request->isEmpty())
            {
                // Берем первый элемент очереды
                auto [time, expr] = m_request->head();
                // Вычисление с заданной задержкой
                doCalc ( time, expr);
                // Удаление из очереди вычисленное выражение
                lock.lockForWrite();
                m_request->dequeue();
                lock.unlock();
                // Уведомление об обновлении очереди
                Q_EMIT updateQueueRequests( m_request->size());
            }
        }
        QThread::msleep(200);
    }
}

void CalcWorker::doCalc(const int time, const QString expr)
{
    try
    {
        // индекс операции
        int reqOper = -1;

        if ( expr.isEmpty())
        {
            Q_EMIT error("Отсутствует выражение для вычисления");
            return;
        }

        // операнды
        QStringList exprParts;

        for ( auto oper : m_operations)
        {
            // определение операции
            if ( exprParts = expr.split(oper, QString::SkipEmptyParts); exprParts.count() < 2 )
                continue;
            else
            {
                // определение соответствия операции его индекса
                if ( oper == "+")
                    reqOper = 0;
                else if ( oper == "-")
                    reqOper = 1;
                else if ( oper == "÷")
                    reqOper = 2;
                else if ( oper == "×")
                    reqOper = 3;
                break;
            }
        }

        if ( exprParts.count() < 2 )
        {
            Q_EMIT error("Отсутствует один из операндов");
            return;
        }

        // результат вычисления
        double result = std::numeric_limits<double>::quiet_NaN();

        // точность вычислений
        int presition = 0;

        // определение точности вычислений
        auto checkPresition = [](QString str)
        {
            auto strlist = str.split(".");
            if ( strlist.count() == 2)
                return strlist[1].count();
            return 0;
        };

        if (m_doItFunction)
        {
            // код ошибки
            int err = -1;

            bool isOk = false;
            double A = exprParts[0].toDouble(&isOk);
            if (!isOk)
            {
                Q_EMIT error("Ошибка конвертации числа " + exprParts[0]);
                return;
            }

            double B = exprParts[1].toDouble(&isOk);
            if (!isOk)
            {
                Q_EMIT error("Ошибка конвертации числа " + exprParts[1]);
                return;
            }

            presition = std::max(checkPresition(exprParts[0]), checkPresition(exprParts[1]));
            presition = presition == 0 ? 15 : presition;
            result = m_doItFunction(reqOper, A, B, err);

            // остановка текущего потока для долгих вычислений
            if ( time > 0)
                QThread::sleep(static_cast<unsigned long>(time));

            if ( std::isnan ( result ) )
                Q_EMIT error("Ошибка вычисления выражения");

            // если код ошибки не нуль, что-то произошло
            if ( err != 0 )
            {
                if ( err == 1 )
                    Q_EMIT error("Задана неподдерживаемая операция");
                else if ( err == 2 )
                    Q_EMIT error("Ошибка деления на нуль");
                else if ( err == 3 )
                    Q_EMIT error("Неопределенная ошибка");
                else
                    Q_EMIT error("Необработанная ошибка при вычислении выражения");
                return;
            }
        }
        else
        {
            Q_EMIT error("Отсутствует функция вычисления выражения");
            return;
        }

        if ( m_results )
        {
            auto resStr = QString::number(result, 'f', presition);
            resStr.remove(QRegExp("0+$"));
            resStr.remove(QRegExp("\\.$"));
            lock.lockForWrite();
            m_results->enqueue({expr, resStr});
            lock.unlock();
            Q_EMIT updateQueueResults( m_results->size());
        }
    }
    catch (...)
    {
        Q_EMIT error("Необработанная ошибка при вычислении выражения doCalc");
    }
}
