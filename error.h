/**
 * @brief      Error codes
 */
#ifndef TAI_ERROR_HEADER
#define TAI_ERROR_HEADER

#include "taihen_internal.h"

/**
 * @defgroup   error ERROR CODES
 */
/** @{ */

#define TAI_SUCCESS 0
#define TAI_ERROR_SYSTEM 0x90010000
#define TAI_ERROR_MEMORY 0x90010001
#define TAI_ERROR_NOT_FOUND 0x90010002
#define TAI_ERROR_INVALID_ARGS 0x90010003
#define TAI_ERROR_INVALID_KERNEL_ADDR 0x90010004
#define TAI_ERROR_PATCH_EXISTS 0x90010005

/** @} */

#endif // TAI_ERROR_HEADER
