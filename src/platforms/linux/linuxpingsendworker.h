/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXPINGSENDWORKER_H
#define LINUXPINGSENDWORKER_H

#include "pingsendworker.h"

class QSocketNotifier;

class LinuxPingSendWorker final : public PingSendWorker
{
    Q_OBJECT

public slots:
    void sendPing(const QString &destination) override;
    void stopPing() override;

private:
    void releaseObjects();

private:
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socket = 0;
};

#endif // LINUXPINGSENDWORKER_H
