/* Copyright 2013 Ka-Ping Yee

Licensed under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy
of the License at: http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License. */

#ifndef OPC_TYPES_H
#define OPC_TYPES_H

#include <stdint.h>

#ifndef TYPEDEF_U8
#define TYPEDEF_U8
typedef uint8_t u8;
#endif

#ifndef TYPEDEF_S8
#define TYPEDEF_S8
typedef int8_t s8;
#endif

#ifndef TYPEDEF_U16
#define TYPEDEF_U16
typedef uint16_t u16;
#endif

#ifndef TYPEDEF_S16
#define TYPEDEF_S16
typedef int16_t s16;
#endif

#ifndef TYPEDEF_U32
#define TYPEDEF_U32
typedef uint32_t u32;
#endif

#ifndef TYPEDEF_S32
#define TYPEDEF_S32
typedef int32_t s32;
#endif

#ifndef TYPEDEF_PIXEL
#define TYPEDEF_PIXEL
typedef struct { u8 r, g, b; } pixel;
#endif

#endif /* OPC_TYPES_H */
