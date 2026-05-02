#pragma once

#include <QMainWindow>

#include "willchatbridge.h"


class QLineEdit;
class QTextEdit;
class QPushButton;


class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onToggleConnect();
    void onSend();
    void onPeerMessage(const QString& text);
    void onBridgeError(const QString& message);
    void onConnectionChanged(bool connected);

private:
    void setConnectedUi(bool connected);
    void appendLog(const QString& line);

    WillChatBridge bridge_;

    QLineEdit* editHost_ = nullptr;
    QLineEdit* editPort_ = nullptr;
    QPushButton* btnConnect_ = nullptr;

    QTextEdit* log_ = nullptr;
    QLineEdit* editMessage_ = nullptr;
    QPushButton* btnSend_ = nullptr;
};
