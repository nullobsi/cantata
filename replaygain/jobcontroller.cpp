/*
 * Cantata
 *
 * Copyright (c) 2011-2022 Craig Drummond <craig.p.drummond@gmail.com>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jobcontroller.h"
#include "support/globalstatic.h"
#include "support/thread.h"

Job::Job()
	: abortRequested(false), finished(false)
{
}

void Job::setFinished(bool f)
{
	finished = f;
	emit done();
}

StandardJob::StandardJob()
	: thread(0)
{
}

void StandardJob::start()
{
	if (!thread) {
		thread = new Thread(metaObject()->className());
		moveToThread(thread);
		thread->start();
		connect(this, SIGNAL(exec()), this, SLOT(run()), Qt::QueuedConnection);
		emit exec();
	}
}

void StandardJob::stop()
{
	requestAbort();
	if (thread) {
		thread->stop();
		thread = 0;
	}
	deleteLater();
}

GLOBAL_STATIC(JobController, instance)

JobController::JobController()
	: maxActive(1)
{
}

void JobController::setMaxActive(int m)
{
	maxActive = m;
}

void JobController::add(Job* job)
{
	jobs.append(job);
	startJobs();
}

void JobController::finishedWith(Job* job)
{
	active.removeAll(job);
	jobs.removeAll(job);
	job->stop();
}

void JobController::startJobs()
{
	while (active.count() < maxActive && !jobs.isEmpty()) {
		Job* job = jobs.takeAt(0);
		active.append(job);
		connect(job, SIGNAL(done()), this, SLOT(jobDone()), Qt::QueuedConnection);
		job->start();
	}
}

void JobController::cancel()
{
	for (Job* j : active) {
		disconnect(j, SIGNAL(done()), this, SLOT(jobDone()));
		j->stop();
	}
	active.clear();
	for (Job* j : jobs) {
		j->deleteLater();
	}
	jobs.clear();
}

void JobController::jobDone()
{
	QObject* s = sender();

	if (s && dynamic_cast<Job*>(s)) {
		active.removeAll(static_cast<Job*>(s));
	}
	startJobs();
}

#include "moc_jobcontroller.cpp"
