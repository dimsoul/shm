
#ifndef __SHARED_MEMORY__
#define __SHARED_MEMORY__

#include <iostream>
#include <optional>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>

template <typename T, bool autoDestroy = true, bool ignoreExist = true>
class shm
{
public:
    shm(key_t key, int flags, size_t size) noexcept(false)
        :m_shmID(-1), m_shmAddr(nullptr)
    {
        if (size != 0) {
            m_size = size;
            if (!ignoreExist)
            {
                flags |= IPC_EXCL;
            }

            m_shmID = shmget(key, m_size, IPC_CREAT | flags);
            if (m_shmID < 0)
            {
                throw std::runtime_error("Failed to create shared memory: " + std::string(strerror(errno)));
            }
        }
        else
        {
            m_shmID = shmget(key, 0, flags);
            if (m_shmID < 0) {
                throw std::runtime_error("Failed to get shared memory: " + std::string(strerror(errno)));
            }

            struct shmid_ds shmbuffer;
            if (shmctl(m_shmID, IPC_STAT, &shmbuffer) == -1) {
                throw std::runtime_error("Failed to get shared memory size: " + std::string(strerror(errno)));
            }

            m_size = shmbuffer.shm_segsz;
        }

        m_shmAddr = reinterpret_cast<T*>(shmat(m_shmID, nullptr, 0));
        if (m_shmAddr == reinterpret_cast<T*>(-1)) {
            throw std::runtime_error("Failed to attach shared memory: " + std::string(strerror(errno)));
        }

        if (size != 0)
        {
            memset(m_shmAddr, 0, sizeof(m_shmAddr));
        }
    }

    shm(const char *path, int flags, int projectID, size_t size) noexcept(false)
    {
        auto key = ftok(path, projectID);
        if (key == -1) {
            throw std::runtime_error("Failed to generate key using ftok: " + std::string(strerror(errno)));
        }

        shm(key, flags, size);
    }

    virtual ~shm()
    {
        if (autoDestroy) {
            delShm();
        }
    }

    int unmapShm()
    {
        if (m_shmAddr != nullptr && m_shmAddr != reinterpret_cast<T*>(-1)) {
            return shmdt(reinterpret_cast<void*>(m_shmAddr));
        }

        return 0;
    }

    int delShm()
    {
        if (m_shmID != -1) 
        {
            return shmctl(m_shmID, IPC_RMID, nullptr);
        }

        return 0;
    }

    int getSize()
    {
        return m_size;
    }

private:
    int m_shmID;
    T *m_shmAddr;
    size_t m_size;
};


#endif
