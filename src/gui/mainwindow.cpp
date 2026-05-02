#include "mainwindow.h"

#include <QHBoxLayout>
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
    setWindowTitle("Will");
    resize(720, 520);

    btnConnect_ = new QPushButton("Подключиться", this);

    auto* rowConnect = new QHBoxLayout();
    rowConnect->addStretch();
    rowConnect->addWidget(btnConnect_);

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);

    editMessage_ = new QLineEdit(this);
    editMessage_->setPlaceholderText("Сообщение…");
    btnSend_ = new QPushButton("Отправить", this);
    btnSend_->setEnabled(false);

    auto* rowSend = new QHBoxLayout();
    rowSend->addWidget(editMessage_, 1);
    rowSend->addWidget(btnSend_);

    auto* root = new QVBoxLayout();
    root->addLayout(rowConnect);
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

    appendLog("Подключение к серверу…");
    bridge_.connectDefaultServer();
}


void MainWindow::onSend()
{
    if (!bridge_.isConnected())
        return;

    const QString text = editMessage_->text();
    if (text.isEmpty())
        return;

    appendLog(QString("Вы: %1").arg(text));
    bridge_.sendLine(text);
    editMessage_->clear();
}


void MainWindow::onPeerMessage(const QString& text)
{
    appendLog(QString("Собеседник: %1").arg(text));
}


void MainWindow::onBridgeError(const QString& message)
{
    QMessageBox::warning(this, "Сеть", message);
    appendLog(QString("Ошибка: %1").arg(message));
}


void MainWindow::onConnectionChanged(bool connected)
{
    setConnectedUi(connected);
    if (connected)
        appendLog("Соединение установлено.");
    else
        appendLog("Отключено.");
}


void MainWindow::setConnectedUi(bool connected)
{
    btnConnect_->setText(connected ? "Отключиться" : "Подключиться");
    editMessage_->setEnabled(connected);
    btnSend_->setEnabled(connected);
}


void MainWindow::appendLog(const QString& line)
{
    log_->moveCursor(QTextCursor::End);
    log_->insertPlainText(line);
    if (!line.endsWith(QLatin1Char('\n')))
        log_->insertPlainText("\n");
    log_->moveCursor(QTextCursor::End);
}
