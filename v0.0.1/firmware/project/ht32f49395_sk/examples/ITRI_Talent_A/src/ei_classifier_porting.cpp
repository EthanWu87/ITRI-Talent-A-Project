/*
 * Copyright (c) 2023 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "ht32f493x5_int.h"
#include "ei_main.h"
#include "flow_metrics.h"
#include <stddef.h> // for size_t
#include <string.h> // for memse
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum
{
	HT_OK = 0,
	HT_ERROR=1,
	HT_BUSY=2,
	HT_TIMEOUT=3
}HT_StatusTypeDef;

HT_StatusTypeDef HT_UART_Transmit( const uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	if((pData == NULL) || (Size == 0))
	{
		return HT_ERROR;
	}

	/* An inference diagnostic must not become part of the raw image payload. */
	flow_image_uart_wait();
	uint32_t tickstart = SysTick_GetTick();

	while(Size > 0)
	{
		while(usart_flag_get(PRINT_UART, USART_TDBE_FLAG) == RESET)
		{
			if((SysTick_GetTick() - tickstart) > Timeout)
			{
				return HT_TIMEOUT;
			}
		}

		usart_data_transmit(PRINT_UART, *pData++);
		Size--;
	}

	tickstart = SysTick_GetTick();
	while (usart_flag_get(PRINT_UART, USART_TDC_FLAG) == RESET)
	{
		if ((SysTick_GetTick() - tickstart) > Timeout)
		{
				return HT_TIMEOUT;
		}
	}

	return HT_OK;
}

EI_IMPULSE_ERROR ei_run_impulse_check_canceled()
{
	flow_service_background();
	return EI_IMPULSE_OK;
}

__attribute__((weak)) EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
	if(time_ms > 0)
	{
		uint32_t started = SysTick_GetTick();
		while((uint32_t)(SysTick_GetTick() - started) < (uint32_t)time_ms)
			flow_service_background();
	}
	return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms()
{
	return SysTick_GetTimeUs() / 1000ULL;
}

uint64_t ei_read_timer_us()
{
	return SysTick_GetTimeUs();
}

void ei_printf(const char *format, ...)
{
	char buffer[256];
	int length;
	va_list myargs;
	va_start(myargs, format);
	length = vsnprintf(buffer, sizeof(buffer), format, myargs);
	va_end(myargs);

	if(length > 0)
	{
		if(length >= (int)sizeof(buffer)) length = sizeof(buffer) - 1;
		//uart_print(buffer, (uint16_t)length);
		HT_UART_Transmit((uint8_t *)buffer, length, 5000);
	}
}

void ei_printf_float(float f)
{
  /* Bounded conversion also handles NaN/Inf and very large values. */
  if(isnan(f))
    ei_printf("nan");
  else if(isinf(f))
    ei_printf("%s", signbit(f) ? "-inf" : "inf");
  else if(fabsf(f) < 1000000.0f)
    ei_printf("%.5f", (double)f);
  else
    ei_printf("%.5g", (double)f);
}

void *ei_malloc(size_t size)
{
	return malloc(size);
}

void *ei_calloc(size_t nitems, size_t size)
{
	return calloc(nitems, size);
}

void ei_free(void *ptr)
{
	free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif
__attribute__((weak)) void DebugLog(const char* s)
{
	ei_printf("%s", s);
}
