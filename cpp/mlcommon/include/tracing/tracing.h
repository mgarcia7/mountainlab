#ifndef TRACING_H
#define TRACING_H

#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include <chrono>
#include <sys/file.h>

using namespace std::chrono;

//class TracingEntry {
//public:
//    TracingEntry(char type)
//};

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
