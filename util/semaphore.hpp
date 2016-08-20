
/*
 * Copyright (C) 2015-2016 Lewis Rhodes Labs, Inc.
 *
 * No Source Code Derivatives of this File
 *
 * Thank you for your effort to port your application to the Lewis
 * Rhodes Labs Neuromorphic Processing Unit (NPU)(TM). This file
 * exclusively contains "NPU Source Code" and is subject to the terms
 * of the Lewis Rhodes Labs NPU(TM) End-User License Agreement (EULA)
 * which you have in your possession; and, by using this file, you
 * accept all of the terms of the EULA. This file exclusively contains
 * Lewis Rhodes Labs Confidential Information protected by the
 * EULA. THE EULA PROHIBITS THE USE OF THIS FILE FOR THE CREATION OR
 * DISTRIBUTION OF THIS SOURCE CODE OR ANY NPU DERIVATIVE WORKS. YOU
 * MAY USE THIS FILE SOLELY AND EXCLUSIVELY FOR THE PURPOSE OF RUNNING
 * YOUR APPLICATIONS AND FOR THE CREATION OF NON-COMMERCIAL
 * APPLICATION DERIVATIVE WORKS.
 *
 * You are under no obligations of confidentiality regarding your
 * Applications and Application Source Code so long as such
 * publication does not compromise Lewis Rhodes Labs rights and
 * reasonable expectations of confidentiality in the NPU(TM), NPU
 * Source Code, Confidential Information, Documentation, and
 * Derivative Works as set forth in the EULA.
 */


_Pragma("once")

#include "util/common.h"
#include <cstdint>
#include <cstddef>
#include <semaphore.h>

struct semaphore {
    sem_t   m_d;
    semaphore(bool pshared = true, uint32_t value = 0)
    {
        if(sem_init(&m_d, static_cast<int>(pshared),value) < 0) {
            throw std::system_error(errno,std::system_category(),"Failed to initialize semaphore.");
        }
    }
    ~semaphore()
    {
        sem_destroy(&m_d);
    }
    void wait()
    {
        while(sem_wait(&m_d) < 0) {
            if(errno != EINTR)
                throw std::system_error(errno,std::system_category(),"Failed to wait on semaphore.");
        }
    }
    bool trywait()
    {
        while(sem_trywait(&m_d) < 0) {
            if(errno == EAGAIN)
                return false;
            else if(errno != EINTR)
                throw std::system_error(errno, std::system_category(),"Failed to try wait on semaphore.");
        }
        return true;
    }
    bool timedwait(const struct timespec &abstime)
    {
        while(sem_timedwait(&m_d,&abstime) < 0) {
            if(errno == ETIMEDOUT)
                return false;
            else if(errno != EINTR)
                throw std::system_error(errno,std::system_category(),"Failed to timed wait on semaphore.");
        }
        return true;
    }
    void post(void)
    {
        if(sem_post(&m_d) < 0) {
            throw std::system_error(errno,std::system_category(),"Failed to post to semaphore.");
        }
    }
    int getvalue(void)
    {
        auto sval = 0;
        while(sem_getvalue(&m_d,&sval) < 0) {
            if(errno != EINTR)
                throw std::system_error(errno,std::system_category(),"Failed to get semaphore value.");
        }
        return sval;
    }
};
