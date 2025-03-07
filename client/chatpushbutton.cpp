#include <chatpushbutton.h>

ChatPushButton::ChatPushButton(qlonglong id, QString name, bool group, QWidget *parent) {
    setParent(parent);
    this->id = id;
    this->name = name;
    this->group = group;
}

qlonglong ChatPushButton::getId() {
    return id;
}

QString ChatPushButton::getName() {
    return name;
}

bool ChatPushButton::isGroup() {
    return group;
}
