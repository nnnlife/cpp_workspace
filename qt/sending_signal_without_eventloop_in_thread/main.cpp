#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QDebug>

class Argument {
public:
    Argument() {
        str = new char[15];
        ::strcpy(str, "hello world");
    }

    ~Argument() {
        qDebug() << "argument deleted: " << id << "\t" << (int)this;
        delete [] str;
    }

    Argument(const Argument &arg) {
        str = ::strdup(arg.getStr());
        id = arg.getId();
    }

    void setId(int _id) { id = _id; }

    char *getStr() const { return str; }
    int getId() const { return id; }

private:
    char *str;
    int id;
};

class Worker : public QThread {
Q_OBJECT
public:
    Worker() : QThread(0) {
    }

protected:
    void run() {
        int id = 0;
        qDebug() << "Worker Thread ID: " << QThread::currentThreadId();
        forever {
            QThread::msleep(100);
            Argument arg;
            qDebug() << "argument created: " << id;
            arg.setId(id++);
            emit canYou(arg);
        }
    }

signals:
    void canYou(Argument);   // can deliver as a reference type?
};

class Main : public QObject {
Q_OBJECT
public:
    Main() : QObject(0) {

    }

private slots:
    void iCan(Argument arg) {
        qDebug() << arg.getId() << ": " << arg.getStr() << "\t" << QThread::currentThreadId();
    }
};


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Worker worker;
    Main main;
    qRegisterMetaType<Argument>("Argument");
    qDebug() << "Main Thread ID: " << QThread::currentThreadId();

    QObject::connect(&worker, SIGNAL(canYou(Argument)), 
            &main, SLOT(iCan(Argument)));

    worker.start();
    return app.exec();
}

#include "main.moc"
