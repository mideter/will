#include "willchatbridge.h"

#include "messengerworker.h"

#include <QMetaObject>
#include <QObject>
#include <QThread>

class WorkerRuntime final {
public:
    WorkerRuntime()
        : worker(new MessengerWorker())
    {
        worker->moveToThread(&thread);
        QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
        thread.start();
    }

    ~WorkerRuntime()
    {
        if (worker && thread.isRunning()) {
            QMetaObject::invokeMethod(
                worker,
                [w = worker] { w->disconnectServer(); },
                Qt::BlockingQueuedConnection);
        }
        thread.quit();
        thread.wait();
    }

    QThread thread;
    MessengerWorker* worker = nullptr;
};


WillChatBridge::WillChatBridge(QObject* parent)
    : QObject(parent)
    , worker_runtime_(std::make_unique<WorkerRuntime>())
{
    QObject::connect(worker_runtime_->worker, &MessengerWorker::peerMessageReceived, this, &WillChatBridge::peerMessageReceived);
    QObject::connect(worker_runtime_->worker, &MessengerWorker::serverReceiptConfirmed, this, &WillChatBridge::serverReceiptConfirmed);
    QObject::connect(worker_runtime_->worker, &MessengerWorker::errorOccurred, this, &WillChatBridge::errorOccurred);
    QObject::connect(worker_runtime_->worker, &MessengerWorker::connectionChanged, this, [this](bool connected) {
        connected_.store(connected);
        emit connectionChanged(connected);
    });

    QObject::connect(this, &WillChatBridge::requestConnectDefaultServer, worker_runtime_->worker, &MessengerWorker::connectDefaultServer, Qt::QueuedConnection);
    QObject::connect(this, &WillChatBridge::requestSendLine, worker_runtime_->worker, &MessengerWorker::sendLine, Qt::QueuedConnection);
    QObject::connect(this, &WillChatBridge::requestDisconnectServer, worker_runtime_->worker, &MessengerWorker::disconnectServer, Qt::QueuedConnection);
}


WillChatBridge::~WillChatBridge()
{
    connected_.store(false);
    worker_runtime_.reset();
}


bool WillChatBridge::isConnected() const
{
    return connected_.load();
}


void WillChatBridge::connectDefaultServer()
{
    emit requestConnectDefaultServer();
}


void WillChatBridge::sendLine(const QString& line)
{
    emit requestSendLine(line);
}


void WillChatBridge::disconnectServer()
{
    emit requestDisconnectServer();
}
