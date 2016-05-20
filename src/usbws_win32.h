/*
 * Copyright (C) 2016 Nobuo Iwata
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __USBWS_WIN32_H
#define __USBWS_WIN32_H

#if defined(_WIN32)

#include <Windows.h>

#define inline __inline
#define container_of(ptr, type, member) \
	((type *)((unsigned char *)ptr - (unsigned long)&((type *)0)->member))
#define snprintf _snprintf
#define sleep(sec) \
		Sleep((sec) * 1000)

#define pthread_t HANDLE
#define pthread_create(handle, attr, func, arg) \
		usbws_thread_create(handle, func, arg)
#define pthread_join(handle, ret) \
		WaitForSingleObject((handle), INFINITE)

static inline int usbws_thread_create(HANDLE *handle,
				      void *(*func)(void *),
				      void *arg)
{
	*handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg,
			       0, NULL);
	if (*handle == NULL)
		return -1;
	return 0;
}

#define pthread_mutex_t HANDLE
#define pthread_mutex_init(handle, attr) usbws_mutex_init(handle)
#define pthread_mutex_lock(handle) \
		WaitForSingleObject(*(handle), INFINITE)
#define pthread_mutex_unlock(handle) \
		ReleaseMutex(*(handle))

static inline int usbws_mutex_init(HANDLE *handle)
{
	*handle = CreateMutex(NULL, FALSE, NULL);
	if (*handle)
		return -1;
	return 0;
}

#define usbws_cond_lock_t CRITICAL_SECTION
#define pthread_cond_t CONDITION_VARIABLE
#define usbws_cond_lock_init(lock) \
		InitializeCriticalSection(lock)
#define usbws_cond_lock(lock) \
		EnterCriticalSection(lock)
#define usbws_cond_unlock(lock) \
		LeaveCriticalSection(lock)
#define pthread_cond_init(cond) \
		InitializeConditionVariable(cond)
#define pthread_cond_wait(cond, lock) \
		SleepConditionVariableCS((cond), (lock), INFINITE)
#define pthread_cond_signal(cond) \
		WakeConditionVariable(cond)

#endif /* __WIN32 */

#endif /* !__USBWS_WIN32_H */
