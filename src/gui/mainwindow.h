#pragma once

#include <QMainWindow>

#include "willchatbridge.h"


class QEvent;
class QObject;
class QLineEdit;
class QListWidget;
class QPushButton;
class QIcon;


class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onToggleConnect();
    void onSend();
    void onPeerMessage(const QString& text);
    void onServerReceiptConfirmed();
    void onBridgeError(const QString& message);
    void onConnectionChanged(bool connected);
    void onChatAreaClicked();

private:
    enum class LineKind { System, Self, Peer };

    void setConnectedUi(bool connected);
    void appendChatLine(LineKind kind, const QString& text);
    void showComposer();
    void applyMinimalStyle();
    static QIcon makeServerReceiptIcon();

    static constexpr int RoleKind = 256; // Qt::UserRole
    static constexpr int RoleServerConfirmed = 257;
    static constexpr int RoleSelfBody = 258;

    WillChatBridge bridge_;

    QPushButton* btnConnect_ = nullptr;

    QListWidget* chatList_ = nullptr;
    QWidget* composerWrap_ = nullptr;
    QLineEdit* editMessage_ = nullptr;
    QPushButton* btnSend_ = nullptr;

    QIcon serverOkIcon_;
};
