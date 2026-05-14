#include "mainwindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

namespace {

constexpr QColor kInk(26, 26, 26);
constexpr QColor kMuted(120, 120, 120);
constexpr QColor kSelfBubble(232, 242, 255);
constexpr QColor kPeerBubble(245, 245, 245);
} // namespace


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , bridge_(this)
    , serverOkIcon_(makeServerReceiptIcon())
{
    setWindowTitle(QStringLiteral("Will"));
    resize(720, 520);

    btnConnect_ = new QPushButton(QStringLiteral("Подключиться"), this);
    btnConnect_->setObjectName(QStringLiteral("connectBtn"));
    btnConnect_->setCursor(Qt::PointingHandCursor);
    btnConnect_->setFlat(true);

    chatList_ = new QListWidget(this);
    chatList_->setFrameShape(QFrame::NoFrame);
    chatList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chatList_->setSpacing(0);
    chatList_->setUniformItemSizes(false);
    chatList_->setWordWrap(true);
    chatList_->setTextElideMode(Qt::ElideNone);
    chatList_->setSelectionMode(QAbstractItemView::NoSelection);
    chatList_->setFocusPolicy(Qt::NoFocus);
    chatList_->setIconSize(QSize(12, 12));
    chatList_->viewport()->setCursor(Qt::PointingHandCursor);

    editMessage_ = new QLineEdit(this);
    editMessage_->setObjectName(QStringLiteral("composerField"));
    editMessage_->setPlaceholderText(QStringLiteral("Сообщение…"));

    btnSend_ = new QPushButton(QStringLiteral("→"), this);
    btnSend_->setObjectName(QStringLiteral("sendBtn"));
    btnSend_->setCursor(Qt::PointingHandCursor);
    btnSend_->setFlat(true);
    btnSend_->setFixedWidth(40);
    btnSend_->setToolTip(QStringLiteral("Отправить (Enter)"));
    btnSend_->setEnabled(false);

    auto* rowSend = new QHBoxLayout();
    rowSend->setContentsMargins(16, 10, 16, 14);
    rowSend->setSpacing(8);
    rowSend->addWidget(editMessage_, 1);
    rowSend->addWidget(btnSend_);

    composerWrap_ = new QWidget(this);
    composerWrap_->setObjectName(QStringLiteral("composerWrap"));
    composerWrap_->setLayout(rowSend);
    composerWrap_->setVisible(false);

    auto* rowConnect = new QHBoxLayout();
    rowConnect->setContentsMargins(16, 12, 16, 4);
    rowConnect->addStretch();
    rowConnect->addWidget(btnConnect_);

    auto* root = new QVBoxLayout();
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addLayout(rowConnect);
    root->addWidget(chatList_, 1);
    root->addWidget(composerWrap_, 0);

    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("centralRoot"));
    central->setLayout(root);
    setCentralWidget(central);

    chatList_->viewport()->installEventFilter(this);

    QObject::connect(btnConnect_, &QPushButton::clicked, this, &MainWindow::onToggleConnect);
    QObject::connect(btnSend_, &QPushButton::clicked, this, &MainWindow::onSend);
    QObject::connect(&bridge_, &WillChatBridge::peerMessageReceived, this, &MainWindow::onPeerMessage);
    QObject::connect(&bridge_, &WillChatBridge::serverReceiptConfirmed, this, &MainWindow::onServerReceiptConfirmed);
    QObject::connect(&bridge_, &WillChatBridge::errorOccurred, this, &MainWindow::onBridgeError);
    QObject::connect(&bridge_, &WillChatBridge::connectionChanged, this, &MainWindow::onConnectionChanged);

    QObject::connect(editMessage_, &QLineEdit::returnPressed, this, &MainWindow::onSend);

    applyMinimalStyle();
}


bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == chatList_->viewport() && event->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
            onChatAreaClicked();
    }
    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::onChatAreaClicked()
{
    showComposer();
}


void MainWindow::showComposer()
{
    if (composerWrap_->isVisible())
        return;
    composerWrap_->setVisible(true);
    if (bridge_.isConnected())
        editMessage_->setFocus(Qt::OtherFocusReason);
}


void MainWindow::applyMinimalStyle()
{
    QWidget* root = centralWidget();
    if (!root)
        return;

    const QString qss = QStringLiteral(
        "QMainWindow, QWidget#centralRoot { background: #ffffff; color: #1a1a1a; }"
        "QListWidget {"
        "  background: #ffffff;"
        "  color: #1a1a1a;"
        "  border: none;"
        "  outline: none;"
        "  padding: 0 0 8px 0;"
        "}"
        "QListWidget::item {"
        "  border: none;"
        "  border-bottom: 1px solid #ececec;"
        "  padding: 14px 18px 14px 14px;"
        "  margin: 0;"
        "}"
        "QPushButton#connectBtn {"
        "  border: none;"
        "  background: transparent;"
        "  color: #1a1a1a;"
        "  font-size: 13px;"
        "  padding: 4px 2px;"
        "}"
        "QPushButton#connectBtn:hover { color: #3b82f6; }"
        "QWidget#composerWrap {"
        "  background: #fafafa;"
        "  border-top: 1px solid #ececec;"
        "}"
        "QLineEdit#composerField {"
        "  border: none;"
        "  border-bottom: 1px solid #d4d4d4;"
        "  background: transparent;"
        "  padding: 8px 4px;"
        "  font-size: 15px;"
        "  selection-background-color: #dbeafe;"
        "}"
        "QLineEdit#composerField:focus { border-bottom-color: #3b82f6; }"
        "QPushButton#sendBtn {"
        "  border: none;"
        "  background: transparent;"
        "  color: #1a1a1a;"
        "  font-size: 18px;"
        "}"
        "QPushButton#sendBtn:hover { color: #3b82f6; }"
        "QPushButton#sendBtn:disabled { color: #c4c4c4; }");

    root->setStyleSheet(qss);
}


QIcon MainWindow::makeServerReceiptIcon()
{
    QPixmap pm(12, 12);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(148, 163, 184));
    pen.setWidthF(1.15);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    QPainterPath path;
    path.moveTo(2.0, 6.2);
    path.lineTo(4.9, 9.0);
    path.lineTo(10.0, 3.5);
    painter.drawPath(path);
    return QIcon(pm);
}


void MainWindow::appendChatLine(LineKind kind, const QString& text)
{
    auto* item = new QListWidgetItem();
    item->setData(kRoleKind, static_cast<int>(kind));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

    switch (kind) {
    case LineKind::System:
        item->setText(text);
        item->setForeground(kMuted);
        item->setBackground(Qt::transparent);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        break;
    case LineKind::Self:
        item->setText(text);
        item->setForeground(kInk);
        item->setBackground(kSelfBubble);
        item->setTextAlignment(Qt::AlignRight | Qt::AlignTop);
        item->setData(kRoleSelfBody, text);
        item->setData(kRoleServerConfirmed, false);
        break;
    case LineKind::Peer:
        item->setText(text);
        item->setForeground(kInk);
        item->setBackground(kPeerBubble);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        break;
    }

    chatList_->addItem(item);
    chatList_->scrollToItem(item, QAbstractItemView::PositionAtBottom);
}


void MainWindow::onToggleConnect()
{
    if (bridge_.isConnected()) {
        bridge_.disconnectServer();
        return;
    }

    appendChatLine(LineKind::System, QStringLiteral("Подключение к серверу…"));
    bridge_.connectDefaultServer();
}


void MainWindow::onSend()
{
    if (!bridge_.isConnected())
        return;

    const QString text = editMessage_->text();
    if (text.isEmpty())
        return;

    appendChatLine(LineKind::Self, text);
    bridge_.sendLine(text);
    editMessage_->clear();
}


void MainWindow::onPeerMessage(const QString& text)
{
    appendChatLine(LineKind::Peer, text);
}


void MainWindow::onServerReceiptConfirmed()
{
    for (int i = 0; i < chatList_->count(); ++i) {
        QListWidgetItem* item = chatList_->item(i);
        if (!item)
            continue;
        if (item->data(kRoleKind).toInt() != static_cast<int>(LineKind::Self))
            continue;
        if (item->data(kRoleServerConfirmed).toBool())
            continue;

        item->setToolTip(QStringLiteral("Сервер получил сообщение"));
        item->setIcon(serverOkIcon_);
        item->setData(kRoleServerConfirmed, true);
        return;
    }
}


void MainWindow::onBridgeError(const QString& message)
{
    QMessageBox::warning(this, QStringLiteral("Сеть"), message);
    appendChatLine(LineKind::System, QStringLiteral("Ошибка: %1").arg(message));
}


void MainWindow::onConnectionChanged(bool connected)
{
    setConnectedUi(connected);
    if (connected)
        appendChatLine(LineKind::System, QStringLiteral("Соединение установлено."));
    else
        appendChatLine(LineKind::System, QStringLiteral("Отключено."));
}


void MainWindow::setConnectedUi(bool connected)
{
    btnConnect_->setText(connected ? QStringLiteral("Отключиться") : QStringLiteral("Подключиться"));
    editMessage_->setEnabled(connected);
    btnSend_->setEnabled(connected);
}
