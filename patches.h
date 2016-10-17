/**
 * @brief      Main patch system
 */
#ifndef TAI_PROC_MAP_HEADER
#define TAI_PROC_MAP_HEADER

#include "taihen_internal.h"

/**
 * @defgroup   patches Patches Interface
 */
/** @{ */

int patches_init(void);
void patches_deinit(void);


SceUID tai_hook_func_abs(tai_hook_ref_t *p_hook, SceUID pid, void *dest_func, const void *hook_func);
int tai_hook_release(tai_hook_t *hook);
SceUID tai_inject_abs(SceUID pid, void *dest, const void *src, size_t size);
int tai_inject_release(tai_inject_t *inject);
int tai_try_cleanup_process(SceUID pid);

/** @} */

#endif // TAI_PROC_MAP_HEADER
