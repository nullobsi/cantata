/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 - 2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QMap>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QThreadStorage>

#include "../shared/udevqt.h"

#include "soliddefs_p.h"
#include "udisks2.h"
#include "udisksdevice.h"
#include "udisksopticaldisc.h"

// inspired by http://cgit.freedesktop.org/hal/tree/hald/linux/probing/probe-volume.c
static Solid::OpticalDisc::ContentType advancedDiscDetect(const QByteArray& device_file)
{
	/* the discs block size */
	unsigned short bs;
	/* the path table size */
	unsigned short ts;
	/* the path table location (in blocks) */
	unsigned int tl;
	/* length of the directory name in current path table entry */
	unsigned char len_di = 0;
	/* the number of the parent directory's path table entry */
	unsigned int parent = 0;
	/* filename for the current path table entry */
	char dirname[256];
	/* our position into the path table */
	int pos = 0;
	/* the path table record we're on */
	int curr_record = 1;
	Q_UNUSED(curr_record);

	Solid::OpticalDisc::ContentType result = Solid::OpticalDisc::NoContent;

	int fd = open(device_file.constData(), O_RDONLY);

	/* read the block size */
	lseek(fd, 0x8080, SEEK_CUR);
	if (read(fd, &bs, 2) != 2) {
		qDebug("Advanced probing on %s failed while reading block size", qPrintable(device_file));
		goto out;
	}

	/* read in size of path table */
	lseek(fd, 2, SEEK_CUR);
	if (read(fd, &ts, 2) != 2) {
		qDebug("Advanced probing on %s failed while reading path table size", qPrintable(device_file));
		goto out;
	}

	/* read in which block path table is in */
	lseek(fd, 6, SEEK_CUR);
	if (read(fd, &tl, 4) != 4) {
		qDebug("Advanced probing on %s failed while reading path table block", qPrintable(device_file));
		goto out;
	}

	/* seek to the path table */
	lseek(fd, bs * tl, SEEK_SET);

	/* loop through the path table entries */
	while (pos < ts) {
		/* get the length of the filename of the current entry */
		if (read(fd, &len_di, 1) != 1) {
			qDebug("Advanced probing on %s failed, cannot read more entries", qPrintable(device_file));
			break;
		}

		/* get the record number of this entry's parent
           i'm pretty sure that the 1st entry is always the top directory */
		lseek(fd, 5, SEEK_CUR);
		if (read(fd, &parent, 2) != 2) {
			qDebug("Advanced probing on %s failed, couldn't read parent entry", qPrintable(device_file));
			break;
		}

		/* read the name */
		if (read(fd, dirname, len_di) != len_di) {
			qDebug("Advanced probing on %s failed, couldn't read the entry name", qPrintable(device_file));
			break;
		}
		dirname[len_di] = 0;

		/* if we found a folder that has the root as a parent, and the directory name matches
           one of the special directories then set the properties accordingly */
		if (parent == 1) {
			if (!strcasecmp(dirname, "VIDEO_TS")) {
				qDebug("Disc in %s is a Video DVD", qPrintable(device_file));
				result = Solid::OpticalDisc::VideoDvd;
				break;
			}
			else if (!strcasecmp(dirname, "BDMV")) {
				qDebug("Disc in %s is a Blu-ray video disc", qPrintable(device_file));
				result = Solid::OpticalDisc::VideoBluRay;
				break;
			}
			else if (!strcasecmp(dirname, "VCD")) {
				qDebug("Disc in %s is a Video CD", qPrintable(device_file));
				result = Solid::OpticalDisc::VideoCd;
				break;
			}
			else if (!strcasecmp(dirname, "SVCD")) {
				qDebug("Disc in %s is a Super Video CD", qPrintable(device_file));
				result = Solid::OpticalDisc::SuperVideoCd;
				break;
			}
		}

		/* all path table entries are padded to be even,
           so if this is an odd-length table, seek a byte to fix it */
		if (len_di % 2 == 1) {
			lseek(fd, 1, SEEK_CUR);
			pos++;
		}

		/* update our position */
		pos += 8 + len_di;
		curr_record++;
	}

	close(fd);
	return result;

out:
	/* go back to the start of the file */
	lseek(fd, 0, SEEK_SET);
	close(fd);
	return result;
}

using namespace Solid::Backends::UDisks2;

class ContentTypesCache {
public:
	ContentTypesCache()
		: m_n(0)
	{
	}

	void add(const OpticalDisc::Identity& key, Solid::OpticalDisc::ContentTypes content)
	{
		if (!find(key)) {
			m_n = qMin(m_n + 1, sizeof(m_info) / sizeof(*m_info));
			moveToFront(m_n - 1);
			front().first = key;
		}
		front().second = content;
	}

	bool find(const OpticalDisc::Identity& key)
	{
		for (size_t i = 0; i < m_n; i++) {
			if (m_info[i].first == key) {
				moveToFront(i);
				return true;
			}
		}
		return false;
	}

	QPair<OpticalDisc::Identity, Solid::OpticalDisc::ContentTypes>& front()
	{
		return *m_info;
	}

private:
	void moveToFront(size_t i)
	{
		while (i) {
			qSwap(m_info[i - 1], m_info[i]);
			--i;
		}
	}

	size_t m_n;
	QPair<OpticalDisc::Identity, Solid::OpticalDisc::ContentTypes> m_info[100];
};

class SharedContentTypesCache {
private:
	ContentTypesCache* m_pointer;
	QSystemSemaphore m_semaphore;
	QSharedMemory m_shmem;

	struct Unlocker {
	public:
		Unlocker(QSharedMemory* mem)
			: m_mem(mem)
		{
		}
		~Unlocker()
		{
			m_mem->unlock();
		}
		Unlocker(const Unlocker&) = delete;
		Unlocker& operator=(const Unlocker&) = delete;

	private:
		QSharedMemory* m_mem;
	};

	struct Releaser {
	public:
		Releaser(QSystemSemaphore* sem)
			: m_sem(sem)
		{
		}
		~Releaser()
		{
			m_sem->release();
		}
		Releaser(const Releaser&) = delete;
		Releaser& operator=(const Releaser&) = delete;

	private:
		QSystemSemaphore* m_sem;
	};

	static QString getKey()
	{
		static const QString keyTemplate("solid-disk-info-1-%1-%2");
		static const QString tableSize(QString::number(sizeof(ContentTypesCache)));

		return keyTemplate.arg(tableSize, QString::number(geteuid()));
	}

public:
	SharedContentTypesCache()
		: m_pointer(nullptr), m_semaphore(getKey() + "sem", 1), m_shmem(getKey() + "mem")
	{
		if (!m_semaphore.acquire()) {
			return;
		}
		Releaser releaser(&m_semaphore);

		if (m_shmem.attach()) {
			m_pointer = reinterpret_cast<ContentTypesCache*>(m_shmem.data());
			return;
		}

		if (!m_shmem.create(sizeof(ContentTypesCache))) {
			return;
		}

		if (!m_shmem.lock()) {
			m_shmem.detach();
			return;
		}
		Unlocker unlocker(&m_shmem);

		m_pointer = new (m_shmem.data()) ContentTypesCache;
	}

	Solid::OpticalDisc::ContentTypes getContent(const OpticalDisc::Identity& info, const QByteArray& file)
	{
		if (!m_pointer) {
			return advancedDiscDetect(file);
		}

		if (!m_semaphore.acquire()) {
			return advancedDiscDetect(file);
		}
		Releaser releaser(&m_semaphore);

		if (!m_shmem.lock()) {
			return advancedDiscDetect(file);
		}
		Unlocker unlocker(&m_shmem);

		if (!m_pointer->find(info)) {
			m_pointer->add(info, advancedDiscDetect(file));
		}

		Solid::OpticalDisc::ContentTypes content = m_pointer->front().second;
		return content;
	}

	~SharedContentTypesCache()
	{
		m_semaphore.acquire();
		Releaser releaser(&m_semaphore);
		m_shmem.detach();
	}
};

Q_GLOBAL_STATIC(QThreadStorage<SharedContentTypesCache>, sharedContentTypesCache)

OpticalDisc::Identity::Identity()
	: m_detectTime(0), m_size(0), m_labelHash(0)
{
}

OpticalDisc::Identity::Identity(const Device& device, const Device& drive)
	: m_detectTime(drive.prop("TimeMediaDetected").toLongLong()), m_size(device.prop("Size").toLongLong()), m_labelHash(qHash(device.prop("IdLabel").toString()))
{
}

bool OpticalDisc::Identity::operator==(const OpticalDisc::Identity& b) const
{
	return m_detectTime == b.m_detectTime
			&& m_size == b.m_size
			&& m_labelHash == b.m_labelHash;
}

OpticalDisc::OpticalDisc(Device* dev)
	: StorageVolume(dev)
{
	UdevQt::Client client(this);
	m_udevDevice = client.deviceByDeviceFile(device());
	// qDebug() << "udev device:" << m_udevDevice.name() << "valid:" << m_udevDevice.isValid();
	/*qDebug() << "\tProperties:" << */ m_udevDevice.deviceProperties();// initialize the properties DB so that it doesn't crash further down, #298416

	m_drive = new Device(m_device->drivePath());
}

OpticalDisc::~OpticalDisc()
{
	delete m_drive;
}

qulonglong OpticalDisc::capacity() const
{
	return m_device->prop("Size").toULongLong();
}

bool OpticalDisc::isRewritable() const
{
	// the hard way, udisks has no notion of a disc "rewritability"
	const QString mediaType = media();
	return mediaType == "optical_cd_rw" || mediaType == "optical_dvd_rw" || mediaType == "optical_dvd_ram" || mediaType == "optical_dvd_plus_rw" || mediaType == "optical_dvd_plus_rw_dl" || mediaType == "optical_bd_re" || mediaType == "optical_hddvd_rw";
}

bool OpticalDisc::isBlank() const
{
	return m_drive->prop("OpticalBlank").toBool();
}

bool OpticalDisc::isAppendable() const
{
	//qDebug() << "appendable prop" << m_udevDevice.deviceProperty("ID_CDROM_MEDIA_STATE");
	return m_udevDevice.deviceProperty("ID_CDROM_MEDIA_STATE").toString() == QLatin1String("appendable");
}

Solid::OpticalDisc::DiscType OpticalDisc::discType() const
{
	QMap<Solid::OpticalDisc::DiscType, QString> map;
	map[Solid::OpticalDisc::CdRom] = "optical_cd";
	map[Solid::OpticalDisc::CdRecordable] = "optical_cd_r";
	map[Solid::OpticalDisc::CdRewritable] = "optical_cd_rw";
	map[Solid::OpticalDisc::DvdRom] = "optical_dvd";
	map[Solid::OpticalDisc::DvdRecordable] = "optical_dvd_r";
	map[Solid::OpticalDisc::DvdRewritable] = "optical_dvd_rw";
	map[Solid::OpticalDisc::DvdRam] = "optical_dvd_ram";
	map[Solid::OpticalDisc::DvdPlusRecordable] = "optical_dvd_plus_r";
	map[Solid::OpticalDisc::DvdPlusRewritable] = "optical_dvd_plus_rw";
	map[Solid::OpticalDisc::DvdPlusRecordableDuallayer] = "optical_dvd_plus_r_dl";
	map[Solid::OpticalDisc::DvdPlusRewritableDuallayer] = "optical_dvd_plus_rw_dl";
	map[Solid::OpticalDisc::BluRayRom] = "optical_bd";
	map[Solid::OpticalDisc::BluRayRecordable] = "optical_bd_r";
	map[Solid::OpticalDisc::BluRayRewritable] = "optical_bd_re";
	map[Solid::OpticalDisc::HdDvdRom] = "optical_hddvd";
	map[Solid::OpticalDisc::HdDvdRecordable] = "optical_hddvd_r";
	map[Solid::OpticalDisc::HdDvdRewritable] = "optical_hddvd_rw";
	// TODO add these to Solid
	//map[Solid::OpticalDisc::MagnetoOptical] ="optical_mo";
	//map[Solid::OpticalDisc::MountRainer] ="optical_mrw";
	//map[Solid::OpticalDisc::MountRainerWritable] ="optical_mrw_w";

	return map.key(media(), Solid::OpticalDisc::UnknownDiscType);// FIXME optimize, lookup by value, not key
}

Solid::OpticalDisc::ContentTypes OpticalDisc::availableContent() const
{
	if (isBlank()) {
		return Solid::OpticalDisc::NoContent;
	}

	Solid::OpticalDisc::ContentTypes content = Solid::OpticalDisc::NoContent;
	const bool hasData = m_drive->prop("OpticalNumDataTracks").toUInt() > 0;
	const bool hasAudio = m_drive->prop("OpticalNumAudioTracks").toUInt() > 0;

	if (hasData) {
		content |= Solid::OpticalDisc::Data;

		Identity newIdentity(*m_device, *m_drive);
		if (!(m_identity == newIdentity)) {
			QByteArray deviceFile(m_device->prop("Device").toByteArray());
			m_cachedContent = sharedContentTypesCache->localData().getContent(newIdentity, deviceFile);
			m_identity = newIdentity;
		}

		content |= m_cachedContent;
	}
	if (hasAudio) {
		content |= Solid::OpticalDisc::Audio;
	}

	return content;
}

QString OpticalDisc::media() const
{
	return m_drive->prop("Media").toString();
}
