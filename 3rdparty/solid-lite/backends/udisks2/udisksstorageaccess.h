/*
    Copyright 2009 Pino Toscano <pino@kde.org>
    Copyright 2009-2012 Lukáš Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UDISKS2STORAGEACCESS_H
#define UDISKS2STORAGEACCESS_H

#include "udisksdeviceinterface.h"
#include <solid-lite/ifaces/storageaccess.h>

#include <QDBusError>
#include <QDBusMessage>

namespace Solid {
namespace Backends {
namespace UDisks2 {
class StorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess {
	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
	StorageAccess(Device* device);
	~StorageAccess() override;

	bool isAccessible() const override;
	QString filePath() const override;
	bool isIgnored() const override;
	bool setup() override;
	bool teardown() override;

Q_SIGNALS:
	void accessibilityChanged(bool accessible, const QString& udi) override;
	void setupDone(Solid::ErrorType error, QVariant errorData, const QString& udi) override;
	void teardownDone(Solid::ErrorType error, QVariant errorData, const QString& udi) override;
	void setupRequested(const QString& udi) override;
	void teardownRequested(const QString& udi) override;

public Q_SLOTS:
	Q_SCRIPTABLE Q_NOREPLY void passphraseReply(const QString& passphrase);

private Q_SLOTS:
	void slotDBusReply(const QDBusMessage& reply);
	void slotDBusError(const QDBusError& error);

	void connectDBusSignals();

	void slotSetupRequested();
	void slotSetupDone(int error, const QString& errorString);
	void slotTeardownRequested();
	void slotTeardownDone(int error, const QString& errorString);

	void checkAccessibility();

private:
	/// @return true if this device is luks and unlocked
	bool isLuksDevice() const;

	void updateCache();

	bool mount();
	bool unmount();

	bool requestPassphrase();
	void callCryptoSetup(const QString& passphrase);
	bool callCryptoTeardown(bool actOnParent = false);

	QString generateReturnObjectPath();
	QString clearTextPath() const;

private:
	bool m_isAccessible;
	bool m_setupInProgress;
	bool m_teardownInProgress;
	bool m_passphraseRequested;
	QString m_lastReturnObject;

	static const int s_unmountTimeout = 0x7fffffff;
};
}// namespace UDisks2
}// namespace Backends
}// namespace Solid

#endif// UDISKS2STORAGEACCESS_H
