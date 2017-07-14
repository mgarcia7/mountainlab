#ifndef TRACING_H
#define TRACING_H

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include <QThreadStorage>
#include <QVector>
#include <chrono>
#include <sys/file.h>
#include <thread>

using namespace std::chrono;

namespace Trace {


/*
{
  "name": "myName",
  "cat": "category,list",
  "ph": "B",
  "ts": 12345,
  "pid": 123,
  "tid": 456,
  "args": {
    "someArg": 1,
    "anotherArg": {
      "value": "my value"
    }
  }
}
*/

class Event {
public:
    typedef uint32_t Pid;
    typedef uint64_t Tid;
    typedef int64_t Timestamp;
    Event(Timestamp ts, Tid tid = 0, Pid pid = 0) : m_pid(pid), m_tid(tid), m_ts(ts) {
        if (m_pid == 0)
            m_pid = QCoreApplication::applicationPid();
        if (m_tid == 0) {
            static std::hash<std::thread::id> hasher;
            m_tid = hasher(std::this_thread::get_id());
        }
    }
    virtual ~Event(){}
    virtual char eventType() const = 0;
    QString name() const;
    void setName(const QString &name);

    QStringList cat() const;
    void setCat(const QStringList &cat);

    Timestamp ts() const;
    void setTs(const Timestamp &ts);

    Timestamp tts() const;
    void setTts(const Timestamp &tts);

    Pid pid() const;
    void setPid(const Pid &pid);

    Tid tid() const;
    void setTid(const Tid &tid);

    QVariantMap args() const;
    void setArgs(const QVariantMap &args);

    QString cname() const;
    void setCname(const QString &cname);

    virtual void serialize(QJsonObject &doc) const;
protected:
    QVariantMap& argsRef() { return m_args; }
private:
    QString m_name;
    QStringList m_cat;
    Pid m_pid;
    Tid m_tid;
    Timestamp m_ts;
    Timestamp m_tts;
    QVariantMap m_args;
    QString m_cname;
};

class DurationEvent : public Event {
public:
    DurationEvent(Tid tid, Timestamp ts, Pid pid = 0)
        : Event(tid, ts, pid){}
};

class DurationEventB : public DurationEvent {
public:
    DurationEventB(const QString& name, Tid tid, Timestamp ts,  Pid pid = 0)
        : DurationEvent(tid, ts, pid){
        setName(name);
    }
    virtual char eventType() const override { return 'B'; }
};

class DurationEventE : public DurationEvent {
public:
    DurationEventE(Tid tid, Timestamp ts, Pid pid = 0)
        : DurationEvent(tid, ts, pid){}
    virtual char eventType() const override { return 'E'; }
};

class CounterEvent : public Event {
public:
    CounterEvent(const QString& name, Tid tid, Timestamp ts, Pid pid = 0)
        : Event(tid, ts, pid){
        setName(name);
    }
    void setValue(const QString& series, QVariant value) {
        argsRef().insert(series, value);
    }
    QVariant value(const QString& series) const {
        return args().value(series, QVariant());
    }
    void setId(int id) {
        m_id = id;
        m_idSet = true;
    }
    int id() const { return m_id; }

    virtual char eventType() const override { return 'C'; }
    void serialize(QJsonObject &json) const;
private:
    int m_id = 0;
    bool m_idSet = false;
};

class EventManager {
public:
    EventManager() {}
    ~EventManager() {
        flush();
    }
    void append(Event* e) {
        m_events << e;
        if (m_events.size() > 100)
            flush();
    }
    void flush();
protected:
//    void doFlush(const QVector<Event*>& events);

private:
    QVector<Event*> m_events;
};

class TracingSystem {
public:
    TracingSystem() {
        if (m_instance) {
            qWarning("TracingSystem instance already present");
            return;
        }
        m_instance = this;
        init();
    }
    ~TracingSystem() {
        if (m_instance == this) m_instance = nullptr;
    }
    EventManager& eventManager() { return m_manager.localData(); }
    static TracingSystem* instance() { return m_instance; }
protected:
    void init();
    void initMaster();
    void initSlave();
    void doFlush(const QVector<Event*>& events);
    void flush(QVector<Event*> events);

private:
    QString m_traceFilePath;
    static TracingSystem* m_instance;
    QThreadStorage<EventManager> m_manager;
    QMutex m_mutex;
    friend class EventManager;
};

class TraceApi {
public:

};

} // namespace Trace

class LockedFile : public QFile {
public:
    enum LockType {
        SharedLock,
        ExclusiveLock
    };

    void lock(LockType lt) {
        flock(handle(), lt == SharedLock ? LOCK_SH : LOCK_EX);
    }
    void unlock() {
        flock(handle(), LOCK_UN);
    }
};

class Tracing
{
public:
    Tracing(const QString& filePath = "mountainlab.trace");
    ~Tracing();
    static Tracing* instance();
    void writeCounter(const QString& name, const QString& series, double value, const QStringList& categories = QStringList());
    void writeBegin(const QString& name, const QVariantMap& args = QVariantMap(), const QStringList& categories = QStringList());
    void writeEnd(const QString& name, const QVariantMap& args = QVariantMap(), const QStringList& categories = QStringList());
private:
    void open();
    void write(const QJsonObject& entry);
    static Tracing* m_instance;
    QString m_path;
    QMutex m_mutex;
    LockedFile m_file;
    QTextStream m_stream;
    high_resolution_clock::time_point m_baseTime;
};

class TracingScope {
public:
    TracingScope(const QString& name, const QVariantMap& args, const QString& categories = QString())
        : m_name(name), m_args(args), m_cats(categories.split(',')) {
        Tracing::instance()->writeBegin(m_name, m_args, m_cats);
    }
    ~TracingScope() {
        Tracing::instance()->writeEnd(m_name, m_args, m_cats);
    }

private:
    QString m_name;
    QVariantMap m_args;
    QStringList m_cats;
};

#define TRACE_COUNTER1(name, series, value) Tracing::instance()->writeCounter(name, series, value);

#define TRACE_COUNTER(name, series, value) Tracing::instance()->writeCounter(name, series, value);
#define TRACE_BEGIN(name, args, cats) Tracing::instance()->writeBegin(name, args, cats);
#define TRACE_END(name, args, cats) Tracing::instance()->writeEnd(name, args, cats);

#define TRACE_BEGIN0(name, cats) Tracing::instance()->writeBegin(#name, QVariantMap(), cats);
#define TRACE_END0(name, cats) Tracing::instance()->writeEnd(#name, QVariantMap(), cats);


#define TRACE_SCOPE0(name, cat) TracingScope scope_ ## name (#name, QVariantMap(), cat);
#define TRACE_SCOPE1(name, key, val, cat) TracingScope scope_ ## name (#name, { { key, val } }, cat);
#define TRACE_SCOPE2(name, key1, val1, key2, val2, cat) TracingScope scope_ ## name (#name, { { key1, val1 }, {key2, val2} }, cat);
#define TRACE_SCOPE3(name, key1, val1, key2, val2, key3, val3, cat) TracingScope scope_ ## name (#name, { { key1, val1 }, {key2, val2}, {key3, val3} }, cat);
#endif // TRACING_H
