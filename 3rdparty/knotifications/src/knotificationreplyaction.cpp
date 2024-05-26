/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knotificationreplyaction.h"

#include <QString>

class KNotificationReplyActionPrivate
{
public:
    QString label;
    QString placeholderText;
    QString submitButtonText;
    QString submitButtonIconName;
    KNotificationReplyAction::FallbackBehavior fallbackBehavior = KNotificationReplyAction::FallbackBehavior::HideAction;
};

KNotificationReplyAction::KNotificationReplyAction(const QString &label)
    : QObject()
    , d(new KNotificationReplyActionPrivate)
{
    d->label = label;
}

KNotificationReplyAction::~KNotificationReplyAction() = default;

QString KNotificationReplyAction::label() const
{
    return d->label;
}

void KNotificationReplyAction::setLabel(const QString &label)
{
    if (d->label != label) {
        d->label = label;
        Q_EMIT labelChanged();
    }
}

QString KNotificationReplyAction::placeholderText() const
{
    return d->placeholderText;
}

void KNotificationReplyAction::setPlaceholderText(const QString &placeholderText)
{
    if (d->placeholderText != placeholderText) {
        d->placeholderText = placeholderText;
        Q_EMIT placeholderTextChanged();
    }
}

QString KNotificationReplyAction::submitButtonText() const
{
    return d->submitButtonText;
}

void KNotificationReplyAction::setSubmitButtonText(const QString &submitButtonText)
{
    if (d->submitButtonText != submitButtonText) {
        d->submitButtonText = submitButtonText;
        Q_EMIT submitButtonTextChanged();
    }
}

QString KNotificationReplyAction::submitButtonIconName() const
{
    return d->submitButtonIconName;
}

void KNotificationReplyAction::setSubmitButtonIconName(const QString &submitButtonIconName)
{
    if (d->submitButtonIconName != submitButtonIconName) {
        d->submitButtonIconName = submitButtonIconName;
        Q_EMIT submitButtonIconNameChanged();
    }
}

KNotificationReplyAction::FallbackBehavior KNotificationReplyAction::fallbackBehavior() const
{
    return d->fallbackBehavior;
}

void KNotificationReplyAction::setFallbackBehavior(FallbackBehavior fallbackBehavior)
{
    if (d->fallbackBehavior != fallbackBehavior) {
        d->fallbackBehavior = fallbackBehavior;
        Q_EMIT fallbackBehaviorChanged();
    }
}

#include "moc_knotificationreplyaction.cpp"
