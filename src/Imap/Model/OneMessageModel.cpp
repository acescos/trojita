/* Copyright (C) 2006 - 2012 Jan Kundrát <jkt@flaska.net>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "OneMessageModel.h"
#include "FindInterestingPart.h"
#include "ItemRoles.h"
#include "Model.h"
#include "SubtreeModel.h"

namespace Imap
{
namespace Mailbox
{

OneMessageModel::OneMessageModel(Model *model): QObject(model), m_subtree(0)
{
    m_subtree = new SubtreeModel(this);
    m_subtree->setSourceModel(model);
    connect(m_subtree, SIGNAL(modelReset()), this, SIGNAL(envelopeChanged()));
    connect(m_subtree, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(envelopeChanged()));
}

void OneMessageModel::setMessage(const QString &mailbox, const uint uid)
{
    m_mainPartUrl = QUrl(QLatin1String("about:blank"));
    emit mainPartUrlChanged();
    QAbstractItemModel *abstractModel = qobject_cast<QAbstractItemModel*>(QObject::parent());
    Q_ASSERT(abstractModel);
    Model *model = qobject_cast<Model*>(abstractModel);
    Q_ASSERT(model);
    setMessage(model->messageIndexByUid(mailbox, uid));
}

void OneMessageModel::setMessage(const QModelIndex &message)
{
    Q_ASSERT(!message.isValid() || message.model() == QObject::parent());
    m_message = message;
    m_subtree->setRootItem(message);

    // Now try to locate the interesting part of the current message
    QModelIndex idx;
    QString partMessage;
    FindInterestingPart::MainPartReturnCode res = FindInterestingPart::findMainPartOfMessage(m_message, idx, partMessage, 0);
    switch (res) {
    case Imap::Mailbox::FindInterestingPart::MAINPART_FOUND:
    case Imap::Mailbox::FindInterestingPart::MAINPART_PART_LOADING:
        Q_ASSERT(idx.isValid());
        m_mainPartUrl = QLatin1String("trojita-imap://msg") + idx.data(RolePartPathToPart).toString();
        break;
    case Imap::Mailbox::FindInterestingPart::MAINPART_MESSAGE_NOT_LOADED:
        Q_ASSERT(false);
        m_mainPartUrl = QLatin1String("data:text/plain;charset=utf-8;base64,") + QString::fromAscii(QByteArray("").toBase64());
        break;
    case Imap::Mailbox::FindInterestingPart::MAINPART_PART_CANNOT_DETERMINE:
        // FIXME: show a decent message here
        m_mainPartUrl = QLatin1String("data:text/plain;charset=utf-8;base64,") + QString::fromAscii(QByteArray(partMessage.toAscii()).toBase64());
        break;
    }
    qDebug() << m_mainPartUrl;
    emit mainPartUrlChanged();
}

QDateTime OneMessageModel::date() const
{
    return m_message.data(RoleMessageDate).toDateTime();
}

QString OneMessageModel::subject() const
{
    return m_message.data(RoleMessageSubject).toString();
}

QVariantList OneMessageModel::from() const
{
    return m_message.data(RoleMessageFrom).toList();
}

QVariantList OneMessageModel::to() const
{
    return m_message.data(RoleMessageTo).toList();
}

QVariantList OneMessageModel::cc() const
{
    return m_message.data(RoleMessageCc).toList();
}

QVariantList OneMessageModel::bcc() const
{
    return m_message.data(RoleMessageBcc).toList();
}

QVariantList OneMessageModel::sender() const
{
    return m_message.data(RoleMessageSender).toList();
}

QVariantList OneMessageModel::replyTo() const
{
    return m_message.data(RoleMessageReplyTo).toList();
}

QByteArray OneMessageModel::inReplyTo() const
{
    return m_message.data(RoleMessageInReplyTo).toByteArray();
}

QByteArray OneMessageModel::messageId() const
{
    return m_message.data(RoleMessageMessageId).toByteArray();
}

bool OneMessageModel::isMarkedDeleted() const
{
    return m_message.data(RoleMessageIsMarkedDeleted).toBool();
}

bool OneMessageModel::isMarkedRead() const
{
    return m_message.data(RoleMessageIsMarkedRead).toBool();
}

bool OneMessageModel::isMarkedForwarded() const
{
    return m_message.data(RoleMessageIsMarkedForwarded).toBool();
}

bool OneMessageModel::isMarkedReplied() const
{
    return m_message.data(RoleMessageIsMarkedReplied).toBool();
}

bool OneMessageModel::isMarkedRecent() const
{
    return m_message.data(RoleMessageIsMarkedRecent).toBool();
}

QUrl OneMessageModel::mainPartUrl() const
{
    return m_mainPartUrl;
}

}
}
