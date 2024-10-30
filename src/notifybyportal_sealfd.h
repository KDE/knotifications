// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

extern "C" {
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
}

#include <QByteArray>
#include <QLoggingCategory>
#include <QScopeGuard>

Q_DECLARE_LOGGING_CATEGORY(PORTAL)
Q_LOGGING_CATEGORY(PORTAL, "kf.notifications.portal", QtInfoMsg)

class SealableHandle
{
    static char *safe_strerror(int error)
    {
        constexpr auto maxBufferSize = 1024;
        thread_local std::array<char, maxBufferSize> buffer;
        buffer[0] = '\0'; // in case strerror_r fails, terminate the string
        strerror_r(error, buffer.data(), buffer.size());
        return buffer.data();
    }

public:
    SealableHandle(const QByteArray &data)
    {
        if (data.isEmpty()) {
            return;
        }

        auto fd = memfd_create("knotifications-sealed-fd", MFD_ALLOW_SEALING | MFD_CLOEXEC);
        if (fd == -1) {
            const auto error = errno;
            qCWarning(PORTAL) << "memfd_create failed:" << safe_strerror(error);
            return;
        }
        auto fdClose = qScopeGuard([fd] {
            if (fd != -1) {
                close(fd);
            }
        });

        // Resize the bugger so it can fit our data.
        if (ftruncate(fd, data.size()) == -1) {
            const auto error = errno;
            qCWarning(PORTAL) << "ftruncate failed:" << safe_strerror(error);
            return;
        }

        auto shm = mmap(nullptr, data.size(), PROT_WRITE, MAP_SHARED, fd, 0 /* offset */);
        if (shm == MAP_FAILED) {
            const auto error = errno;
            qCWarning(PORTAL) << "mmap failed:" << safe_strerror(error);
            return;
        }
        auto deferUnmap = qScopeGuard([shm, size = data.size()] {
            if (munmap(shm, size) == -1) {
                const auto error = errno;
                qCWarning(PORTAL) << "munmap failed:" << safe_strerror(error);
                return;
            }
        });

        memcpy(shm, data.data(), data.size());

        // The documentation isn't quite clear on it: we do not have to seal the fd.
        // Indeed we shouldn't because we don't know which seals XDP wants.

        fdClose.dismiss();
        m_fd = fd;
    }

    ~SealableHandle()
    {
        if (m_fd != -1) {
            close(m_fd);
        }
    }

    Q_DISABLE_COPY_MOVE(SealableHandle)

    [[nodiscard]] bool isValid() const
    {
        return m_fd != -1;
    }

    [[nodiscard]] int fd() const
    {
        return m_fd;
    }

private:
    int m_fd = -1;
};
