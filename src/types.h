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