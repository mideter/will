#include "mainwindow.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , bridge_(this)
{
    setWindowTitle(QStringLiteral("Will — чат"));
    resize(720, 520);

    editHost_ = new QLineEdit(QStringLiteral("83.217.202.145"), this);
    editPort_ = new QLineEdit(QStringLiteral("8080"), this);
    editPort_->setMaximumWidth(120);

    btnConnect_ = new QPushButton(QStringLiteral("Подключиться"), this);

    auto* rowHost = new QHBoxLayout();
    rowHost->addWidget(new QLabel(QStringLiteral("Сервер"), this));
    rowHost->addWidget(editHost_, 1);
    rowHost->addWidget(new QLabel(QStringLiteral("Порт"), this));
    rowHost->addWidget(editPort_);
    rowHost->addWidget(btnConnect_);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);

    editMessage_ = new QLineEdit(this);
    editMessage_->setPlaceholderText(QStringLiteral("Сообщение…"));
    btnSend_ = new QPushButton(QStringLiteral("Отправить"), this);
    btnSend_->setEnabled(false);

    auto* rowSend = new QHBoxLayout();
    rowSend->addWidget(editMessage_, 1);
    rowSend->addWidget(btnSend_);

    auto* root = new QVBoxLayout();
    root->addLayout(rowHost);
    root->addWidget(log_, 1);
    root->addLayout(rowSend);

    auto* central = new QWidget(this);
    central->setLayout(root);
    setCentralWidget(central);

    QObject::connect(btnConnect_, &QPushButton::clicked, this, &MainWindow::onToggleConnect);
    QObject::connect(btnSend_, &QPushButton::clicked, this, &MainWindow::onSend);
    QObject::connect(&bridge_, &WillChatBridge::peerMessageReceived, this, &MainWindow::onPeerMessage);
    QObject::connect(&bridge_, &WillChatBridge::errorOccurred, this, &MainWindow::onBridgeError);
    QObject::connect(&bridge_, &WillChatBridge::connectionChanged, this, &MainWindow::onConnectionChanged);

    QObject::connect(editMessage_, &QLineEdit::returnPressed, this, &MainWindow::onSend);
}


void MainWindow::onToggleConnect()
{
    if (bridge_.isConnected()) {
        bridge_.disconnectServer();
        return;
    }

    bool ok = false;
    const int port = editPort_->text().toInt(&ok);
    if (!ok || port < 0 || port > 65535) {
        QMessageBox::warning(this, QStringLiteral("Порт"), QStringLiteral("Укажите порт от 0 до 65535."));
        return;
    }

    const QString host = editHost_->text().trimmed();
    if (host.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Сервер"), QStringLiteral("Укажите IPv4-адрес сервера."));
        return;
    }

    appendLog(QStringLiteral("Подключение к %1:%2…").arg(host).arg(port));
    bridge_.connectToServer(host, port);
}


void MainWindow::onSend()
{
    if (!bridge_.isConnected())
        return;

    const QString text = editMessage_->text();
    if (text.isEmpty())
        return;

    appendLog(QStringLiteral("Вы: %1").arg(text));
    bridge_.sendLine(text);
    editMessage_->clear();
}


void MainWindow::onPeerMessage(const QString& text)
{
    appendLog(QStringLiteral("Собеседник: %1").arg(text));
}


void MainWindow::onBridgeError(const QString& message)
{
    QMessageBox::warning(this, QStringLiteral("Сеть"), message);
    appendLog(QStringLiteral("Ошибка: %1").arg(message));
}


void MainWindow::onConnectionChanged(bool connected)
{
    setConnectedUi(connected);
    if (connected)
        appendLog(QStringLiteral("Соединение установлено."));
    else
        appendLog(QStringLiteral("Отключено."));
}


void MainWindow::setConnectedUi(bool connected)
{
    editHost_->setEnabled(!connected);
    editPort_->setEnabled(!connected);
    btnConnect_->setText(connected ? QStringLiteral("Отключиться") : QStringLiteral("Подключиться"));
    editMessage_->setEnabled(connected);
    btnSend_->setEnabled(connected);
}


void MainWindow::appendLog(const QString& line)
{
    log_->moveCursor(QTextCursor::End);
    log_->insertPlainText(line);
    if (!line.endsWith(QLatin1Char('\n')))
        log_->insertPlainText(QStringLiteral("\n"));
    log_->moveCursor(QTextCursor::End);
}
