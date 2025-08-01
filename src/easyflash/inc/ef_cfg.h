/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2015-07-14
 */


#ifndef EF_CFG_H_
#define EF_CFG_H_

#include <stm32f1xx_hal.h>

/* using ENV function, default is NG (Next Generation) mode start from V4.0 */
#define EF_USING_ENV

/* using IAP function */
//#define EF_USING_IAP

/* using save log function */
/* #define EF_USING_LOG */

/* page size for stm32 flash */
#if defined(STM32F10X_LD) || defined(STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
#define PAGE_SIZE     1024
#else
#define PAGE_SIZE     4096
#endif

/* the minimum size of flash erasure */
#define EF_ERASE_MIN_SIZE         PAGE_SIZE              /* it is one page for STM3210x */

/* the flash write granularity, unit: bit
 * only support 1(nor flash)/ 8(stm32f4)/ 32(stm32f1)/ 64(stm32l4) */
#define EF_WRITE_GRAN            1 

/*
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 * @note all area sizes must be aligned with EF_ERASE_MIN_SIZE
 *
 * The EasyFlash add the NG (Next Generation) mode start from V4.0. All old mode before V4.0, called LEGACY mode.
 *
 * - NG (Next Generation) mode is default mode from V4.0. It's easy to settings, only defined the ENV_AREA_SIZE.
 * - The LEGACY mode has been DEPRECATED. It is NOT RECOMMENDED to continue using.
 *   Beacuse it will use ram to buffer the ENV and spend more flash erase times.
 *   If you want use it please using the V3.X version.
 */

/* backup area start address */
// #define EF_START_ADDR                  (FLASH_BASE + 64 * 1024) /* from the chip position: 64KB */
#define EF_START_ADDR                  (0) /* from the chip position: 64KB */
/* ENV area size. It's at least one empty sector for GC. So it's definination must more then or equal 2 flash sector size. */
#define ENV_AREA_SIZE                  (2 * EF_ERASE_MIN_SIZE)      /* 8K */
/* saved log area size */
/* #define LOG_AREA_SIZE             (10 * EF_ERASE_MIN_SIZE)*/      /* 20K */

/* print debug information of flash */
#define PRINT_DEBUG

#endif /* EF_CFG_H_ */
