/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_HAL_STORAGEACCESS_H
#define SOLID_BACKENDS_HAL_STORAGEACCESS_H

#include "haldeviceinterface.h"
#include <solid-lite/ifaces/storageaccess.h>

#include <QDBusError>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QProcess>

namespace Solid {
namespace Backends {
namespace Hal {
class StorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess {
	Q_OBJECT
	Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
	StorageAccess(HalDevice* device);
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
	void ejectDone(Solid::ErrorType error, QVariant errorData, const QString& udi);
	void setupRequested(const QString& udi) override;
	void teardownRequested(const QString& udi) override;

private Q_SLOTS:
	void connectDBusSignals();
	void slotPropertyChanged(const QMap<QString, int>& changes);
	void slotDBusReply(const QDBusMessage& reply);
	void slotDBusError(const QDBusError& error);
	void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void slotSetupRequested();
	void slotTeardownRequested();
	void slotEjectRequested();
	void slotSetupDone(int error, const QString& errorString);
	void slotTeardownDone(int error, const QString& errorString);
	void slotEjectDone(int error, const QString& errorString);

public Q_SLOTS:
	Q_SCRIPTABLE Q_NOREPLY void passphraseReply(const QString& passphrase);

private:
	bool callHalVolumeMount();
	bool callHalVolumeUnmount();
	bool callHalVolumeEject();

	bool callSystemMount();
	bool callSystemUnmount();

	bool requestPassphrase();
	void callCryptoSetup(const QString& passphrase);
	bool callCryptoTeardown();

private:
	bool m_setupInProgress;
	bool m_teardownInProgress;
	bool m_ejectInProgress;
	bool m_passphraseRequested;
	QString m_lastReturnObject;
	QProcess* m_process;
};
}// namespace Hal
}// namespace Backends
}// namespace Solid

#endif// SOLID_BACKENDS_HAL_STORAGEACCESS_H
