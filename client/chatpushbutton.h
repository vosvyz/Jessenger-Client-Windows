#pragma once

#include <QDebug>
#include <QPushButton>

// This widget allows storing additional information about the chat - its ID, name, and whether the chat is a group.
class ChatPushButton : public QPushButton
{
Q_OBJECT
public:
    ChatPushButton(qlonglong id, QString name, bool group, QWidget *parent);
    qlonglong getId();
    QString getName();
    bool isGroup();
private:
    qlonglong id;
    QString name;
    bool group;
};
