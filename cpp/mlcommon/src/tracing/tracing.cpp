#include "tracing/tracing.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <thread>
#include <QtDebug>

namespace Trace {
TracingSystem* TracingSystem::m_instance = nullptr;

EventManager::EventManager() {
    qDebug() << Q_FUNC_INFO;
}

EventManager::~EventManager() {
    qDebug() << Q_FUNC_INFO;
    flush();
}

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

void CompleteEvent::serialize(QJsonObject &doc) const
{
    DurationEvent::serialize(doc);
    doc.insert("dur", (qint64)m_dur.count());
}

void InstantEvent::serialize(QJsonObject &doc) const
{
    Event::serialize(doc);
    doc.insert("s", QString((char)m_s));
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

void Event::setArg(const QString &name, const QVariant &value)
{
    m_args.insert(name, value);
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
    json.insert("ph", QString(eventType()));
    json.insert("pid", (qint64)pid());
    json.insert("tid", (qint64)tid());
    if (ts() >=0)
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

void TracingSystem::trace_counter(const QString &name, QVector<QPair<QString, qreal> > series)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2.time_since_epoch());
    qint64 ts = time_span.count();
    auto ctr = new CounterEvent(name, ts);
    for (auto serie: series)
        ctr->setValue(serie.first, serie.second);
    inst->eventManager()->append(ctr);
}

void TracingSystem::trace_begin(const QString &name, QVector<QPair<QString, QVariant>> args)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2.time_since_epoch());
    qint64 ts = time_span.count();
    auto dur = new DurationEventB(name, ts);
    for (auto arg: args) {
        dur->setArg(arg.first, arg.second);
    }
    inst->eventManager()->append(dur);
}

void TracingSystem::trace_end(QVector<QPair<QString, QVariant> > args)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2.time_since_epoch());
    qint64 ts = time_span.count();
    auto dur = new DurationEventE(ts);
    for (auto arg: args) {
        dur->setArg(arg.first, arg.second);
    }
    inst->eventManager()->append(dur);
}

void TracingSystem::trace_end(const QString &name, QVector<QPair<QString, QVariant> > args)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2.time_since_epoch());
    qint64 ts = time_span.count();
    auto dur = new DurationEventE(ts);
    dur->setName(name);
    for (auto arg: args) {
        dur->setArg(arg.first, arg.second);
    }
    inst->eventManager()->append(dur);
}

void TracingSystem::trace_threadname(const QString &name)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;

    auto meta = new MetadataEvent("thread_name");
    meta->setArg("name", name);
    inst->eventManager()->append(meta);
}

void TracingSystem::trace_processname(const QString &name)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;

    auto meta = new MetadataEvent("process_name");
    meta->setArg("name", name);
    inst->eventManager()->append(meta);
}

void TracingSystem::trace_instant(const QString &name, char scope)
{
    TracingSystem* inst = TracingSystem::instance();
    if (!inst) return;
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2.time_since_epoch());
    qint64 ts = time_span.count();
    auto instant = new InstantEvent(name, scope, ts);
    inst->eventManager()->append(instant);
}

void TracingSystem::init()
{
    QByteArray ba = qgetenv("TRACE_MASTER");
    if (ba.isEmpty()) {
        qDebug() << "Init in master mode";
        qputenv("TRACE_MASTER", QByteArray::number(QCoreApplication::applicationPid()));
        initMaster();
    } else {
        qDebug() << "Init in slave mode";
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
    if (m_traceFilePath.isEmpty() && !QCoreApplication::applicationName().isEmpty()) {
        m_traceFilePath = QCoreApplication::applicationName()+".trace";
    }
    if (m_traceFilePath.isEmpty()) {
        m_traceFilePath = "application.trace";
    }
    m_traceFilePath = QFileInfo(m_traceFilePath).absoluteFilePath();
    qputenv("TRACE_FILE", m_traceFilePath.toLocal8Bit());
    qWarning("Trace file: %s", qPrintable(m_traceFilePath));
    LockedFile file(m_traceFilePath);
    file.lock(LockedFile::ExclusiveLock);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) {
        qCritical("Device failed to open");
    } else {
    file.write("[\n");
    file.close();
    }
    file.unlock();
}

void TracingSystem::initSlave()
{
    m_traceFilePath = qgetenv("TRACE_FILE");
}

void TracingSystem::doFlush(const QVector<Event *> &events)
{
    QMutexLocker locker(&m_mutex);
    LockedFile file(m_traceFilePath);
    file.lock(LockedFile::ExclusiveLock);
    if (!file.open(QIODevice::Append|QIODevice::Text)) {
        qCritical("Device failed to open in append mode");
    }
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

}

