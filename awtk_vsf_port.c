/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

// TODO: add support to other color_format and lcd_mode

/*============================ INCLUDES ======================================*/

#define __VSF_DISP_CLASS_INHERIT
// TODO: use dedicated include
#include "vsf.h"

#if VSF_USE_UI == ENABLED && VSF_USE_UI_AWTK == ENABLED

#include "tkc/mem.h"
#include "tkc/fs.h"
#include "lcd/lcd_mem_fragment.h"
#include "base/pixel.h"
#include "base/main_loop.h"

#include "awtk_vsf_port.h"

/*============================ MACROS ========================================*/

#define LCD_FORMAT                  BITMAP_FMT_BGR565
#define pixel_from_rgb(r, g, b)                                                \
  ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define pixel_to_rgba(p)                                                       \
  { (0xff & ((p >> 11) << 3)), (0xff & ((p >> 5) << 2)), (0xff & (p << 3)) }

#define set_window_func
#define write_data_func

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef uint16_t pixel_t;

/*============================ GLOBAL VARIABLES ==============================*/

void * (*malloc_impl)(size_t size);
void * (*realloc_impl)(void *p, size_t size);
void (*free_impl)(void *p);
int (*printf_impl)(const char *format, ...);
int (*strcasecmp_impl)(const char *s1, const char *s2);
int (*vsnprintf_impl)(char *str, size_t size, const char *format, va_list ap);
int (*vsscanf_impl)(const char *str, const char *format, va_list ap);

void (*sleep_ms_impl)(uint32_t ms);

ret_t (*platform_prepare_impl)(void);
main_loop_t* (*main_loop_init_impl)(int w, int h);
uint64_t (*get_time_ms64_impl)(void);
fs_t * (*os_fs_impl)(void);

vsf_eda_t * (*vsf_eda_get_cur_impl)(void);
vsf_err_t (*vsf_eda_post_evt_impl)(vsf_eda_t *pthis, vsf_evt_t evt);
vsf_err_t (*vk_disp_refresh_impl)(vk_disp_t *pthis, vk_disp_area_t *area, void *disp_buff);
void (*vsf_thread_wfe_impl)(vsf_evt_t evt);
vsf_err_t (*vk_disp_init_impl)(vk_disp_t *pthis);

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#include "blend/pixel_ops.inc"
#include "lcd/lcd_mem_fragment.inc"

ret_t lcd_mem_fragment_flush_vsf(lcd_t* lcd)
{
    vk_disp_t *disp = lcd->impl_data;
    lcd_mem_fragment_t* mem = (lcd_mem_fragment_t*)lcd;
    vk_disp_area_t disp_area;

    disp_area.pos.x = mem->x;
    disp_area.pos.y = mem->y;
    disp_area.size.x = mem->fb.w;
    disp_area.size.y = mem->fb.h;
    disp->ui_data = vsf_eda_get_cur_impl();
    vk_disp_refresh_impl(disp, &disp_area, mem->buff);
    vsf_thread_wfe_impl(VSF_EVT_USER);
    return RET_OK;
}

static void __vsf_awtk_disp_on_ready(vk_disp_t *disp)
{
    vsf_eda_t *eda = disp->ui_data;
    disp->ui_data = NULL;
    ASSERT(eda != NULL);
    vsf_eda_post_evt_impl(eda, VSF_EVT_USER);
}

void vsf_awtk_disp_bind(vk_disp_t *disp, lcd_t *lcd)
{
    lcd->impl_data = disp;
    lcd->flush = lcd_mem_fragment_flush_vsf;
    disp->ui_on_ready = __vsf_awtk_disp_on_ready;
    vk_disp_init_impl(disp);
}

void awtk_vsf_init(vsf_awtk_op_t *op)
{
    malloc_impl = op->malloc_impl;
    realloc_impl = op->realloc_impl;
    free_impl = op->free_impl;
    printf_impl = op->printf_impl;
    strcasecmp_impl = op->strcasecmp_impl;
    vsnprintf_impl = op->vsnprintf_impl;
    vsscanf_impl = op->vsscanf_impl;

    sleep_ms_impl = op->sleep_ms_impl;

    platform_prepare_impl = op->platform_prepare_impl;
    main_loop_init_impl = op->main_loop_init_impl;
    get_time_ms64_impl = op->get_time_ms64_impl;
    os_fs_impl = op->os_fs_impl;

    vsf_eda_get_cur_impl = op->vsf_eda_get_cur_impl;
    vsf_eda_post_evt_impl = op->vsf_eda_post_evt_impl;
    vk_disp_refresh_impl = op->vk_disp_refresh_impl;
    vsf_thread_wfe_impl = op->vsf_thread_wfe_impl;
    vk_disp_init_impl = op->vk_disp_init_impl;
}

#endif
