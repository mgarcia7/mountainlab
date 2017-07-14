#include "tracing/tracing.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <thread>

Tracing* Tracing::m_instance = nullptr;

Tracing::Tracing(const QString& filePath)
    : m_path(filePath)
{
    m_instance = this;
    m_baseTime = high_resolution_clock::now();
    open();
}

Tracing::~Tracing()
{
    m_stream << "]" << endl;
    m_file.close();
}

Tracing *Tracing::instance()
{
    if (!m_instance) {
        new Tracing("/tmp/mountainlab.trace");
    }
    return m_instance;
}

void Tracing::writeCounter(const QString &name, const QString &series, double value, const QStringList &categories)
{
    QMutexLocker locker(&m_mutex);
    qint64 pid = QCoreApplication::applicationPid();
    std::hash<std::thread::id> hasher;
    auto tid = QString::number(hasher(std::this_thread::get_id()));
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - m_baseTime);
    qint64 ts = time_span.count();
    QJsonObject data;
    data.insert("pid", pid);
    data.insert("tid", tid);
    data.insert("ts", ts);
    data.insert("ph", "C");
    data.insert("name", name);
    data.insert("cat", categories.join(','));
    QJsonObject args;
    args.insert(series, value);
    data.insert("args", args);
    write(data);

}

void Tracing::writeBegin(const QString &name, const QVariantMap &argsMap, const QStringList &categories)
{
    QMutexLocker locker(&m_mutex);
    qint64 pid = QCoreApplication::applicationPid();
    std::hash<std::thread::id> hasher;
    auto tid = QString::number(hasher(std::this_thread::get_id()));
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - m_baseTime);
    qint64 ts = time_span.count();
    QJsonObject data;
    data.insert("pid", pid);
    data.insert("tid", tid);
    data.insert("ts", ts);
    data.insert("ph", "B");
    data.insert("name", name);
    data.insert("cat", categories.join(','));
    QJsonObject args = QJsonObject::fromVariantMap(argsMap);
    data.insert("args", args);
    write(data);
}

void Tracing::writeEnd(const QString &name, const QVariantMap &argsMap, const QStringList &categories)
{
    QMutexLocker locker(&m_mutex);
    qint64 pid = QCoreApplication::applicationPid();
    std::hash<std::thread::id> hasher;
    auto tid = QString::number(hasher(std::this_thread::get_id()));
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - m_baseTime);
    qint64 ts = time_span.count();
    QJsonObject data;
    data.insert("pid", pid);
    data.insert("tid", tid);
    data.insert("ts", ts);
    data.insert("ph", "E");
    data.insert("name", name);
    data.insert("cat", categories.join(','));
    QJsonObject args = QJsonObject::fromVariantMap(argsMap);
    data.insert("args", args);
    write(data);
}

void Tracing::open()
{
    QMutexLocker locker(&m_mutex);
    m_file.setFileName(m_path);
    m_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    m_stream.setDevice(&m_file);
    m_stream << "[" << endl;
}

void Tracing::write(const QJsonObject &entry)
{
    m_stream << "  " << QJsonDocument(entry).toJson(QJsonDocument::Compact) << "," << endl;
}
