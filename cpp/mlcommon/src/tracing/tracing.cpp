#include "tracing/tracing.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <thread>

namespace Trace {
TracingSystem* TracingSystem::m_instance = nullptr;

void EventManager::flush() {
    if (m_events.isEmpty()) return;
    QVector<Event*> eventsToFlush;
    qSwap(eventsToFlush, m_events);
    TracingSystem::instance()->flush(eventsToFlush);
    //        // do flush
    //        // options:
    //        // 1. flush synchronously
    //        // 2. spawn a thread and flush there
    //        // 3. have a global per-pid thread that handles flush
    //        qDeleteAll(eventsToFlush);
}

}

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
//    qgetenv("TRACE")
    m_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    m_stream.setDevice(&m_file);
    m_stream << "[" << endl;
}

void Tracing::write(const QJsonObject &entry)
{
    m_stream << "  " << QJsonDocument(entry).toJson(QJsonDocument::Compact) << "," << endl;
}

namespace Trace {
QString Event::name() const
{
    return m_name;
}

void Event::setName(const QString &name)
{
    m_name = name;
}

QStringList Event::cat() const
{
    return m_cat;
}

void Event::setCat(const QStringList &cat)
{
    m_cat = cat;
}

Event::Timestamp Event::ts() const
{
    return m_ts;
}

void Event::setTs(const Event::Timestamp &ts)
{
    m_ts = ts;
}

Event::Timestamp Event::tts() const
{
    return m_tts;
}

void Event::setTts(const Event::Timestamp &tts)
{
    m_tts = tts;
}

Event::Pid Event::pid() const
{
    return m_pid;
}

void Event::setPid(const Event::Pid &pid)
{
    m_pid = pid;
}

Event::Tid Event::tid() const
{
    return m_tid;
}

void Event::setTid(const Event::Tid &tid)
{
    m_tid = tid;
}

QVariantMap Event::args() const
{
    return m_args;
}

void Event::setArgs(const QVariantMap &args)
{
    m_args = args;
}

QString Event::cname() const
{
    return m_cname;
}

void Event::setCname(const QString &cname)
{
    m_cname = cname;
}

void Event::serialize(QJsonObject &json) const
{
    json.insert("name", name());
    // cats
    if (cat().isEmpty())
        json.insert("cat", cat().join(','));
    json.insert("ph", eventType());
    json.insert("pid", (qint64)pid());
    json.insert("tid", (qint64)tid());
    json.insert("ts", (qint64)ts());
    if (!cname().isEmpty())
        json.insert("cname", cname());
    json.insert("args", QJsonObject::fromVariantMap(args()));
}

void CounterEvent::serialize(QJsonObject &json) const {
    Event::serialize(json);
    if (m_idSet)
        json.insert("id", id());
}

void TracingSystem::init()
{
    QByteArray ba = qgetenv("TRACE_MASTER");
    if (ba.isEmpty()) {
        qputenv("TRACE_MASTER", QByteArray::number(QCoreApplication::applicationPid()));
        initMaster();
    } else {
        initSlave();
    }
}

void TracingSystem::initMaster()
{
    // tell slaves about the file
    const QStringList& args = QCoreApplication::arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args[i].startsWith("--trace-file=")) {
            m_traceFilePath = args[i].mid(13);
            continue;
        }
        if (args[i] == "--trace-file") {
            if (i+1 < args.size()) {
                m_traceFilePath = args[++i];
            }
            continue;
        }
    }
    qputenv("TRACE_FILE", m_traceFilePath.toLocal8Bit());
    LockedFile file;
    file.lock(LockedFile::ExclusiveLock);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
    file.write("[\n");
    file.close();
    file.unlock();
}

void TracingSystem::initSlave()
{
    m_traceFilePath = qgetenv("TRACE_FILE");
}

void TracingSystem::doFlush(const QVector<Event *> &events)
{
    QMutexLocker locker(&m_mutex);
    LockedFile file;
    file.lock(LockedFile::ExclusiveLock);
    file.open(QIODevice::Append|QIODevice::Text);
    QTextStream stream(&file);
    foreach(Event* e, events) {
        QJsonObject obj;
        e->serialize(obj);
        stream << "  " << QJsonDocument(obj).toJson(QJsonDocument::Compact) << ',' << endl;
    }
    file.close();
    file.unlock();
}

void TracingSystem::flush(QVector<Event *> events)
{
    doFlush(events);
    qDeleteAll(events);
}


//void EventManager::doFlush(const QVector<Event *> &events) {
//    foreach(Event* e, events) {
//        QJsonObject obj;
//        e->serialize(obj);
//    }
//}

}

