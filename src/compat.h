/* Copyright 2016 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef _WIN32
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

typedef SOCKET sock_t;
#else
typedef int sock_t;
#define SOCKET_ERROR 0xFFFFFFFF
#define INVALID_SOCKET 0xFFFFFFFF
#endif

#endif // __COMPAT_H__
