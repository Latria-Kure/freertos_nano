/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
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
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */

#include "usart.h"
#include <elog.h>
#include <stdint.h>
#include <stdio.h>

uint32_t log_index = 0;
bool elog_free = false;
/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;
    MX_USART1_UART_Init();
    /* add your code here */

    return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void)
{

    /* add your code here */
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char* log, size_t size)
{

    // printf("%.*s", size, log);
    // dma_to_usart1_send(log, size);
    HAL_UART_Transmit(&huart1, (uint8_t*)log, size, 0xFFFF);
    /* add your code here */
}

/**
 * output lock
 */
void elog_port_output_lock(void)
{
    /* add your code here */
    __disable_irq();
    elog_free = false;
}

/**
 * output unlock
 */
void elog_port_output_unlock(void)
{
    /* add your code here */
    __enable_irq();
    elog_free = true;
}

/**
 * get current time interface
 *
 * @return current time
 */
const char* elog_port_get_time(void)
{

    /* add your code here */
    static char log_index_str[10];
    sprintf(log_index_str, "%d", log_index);
    log_index++;
    return log_index_str;
    // return "";
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char* elog_port_get_p_info(void)
{

    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char* elog_port_get_t_info(void)
{
    // return the log index string
    return "";
}
