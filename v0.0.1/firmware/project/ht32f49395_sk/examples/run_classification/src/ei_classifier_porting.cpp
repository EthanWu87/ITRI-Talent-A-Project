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

#include <stddef.h> // for size_t
#include <string.h> // for memse
#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "ht32f493x5_int.h"
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
	uint32_t tickstart = SysTick_GetTick();

	if((pData == NULL) || (Size == 0))
	{
		return HT_ERROR;
	}

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

__attribute__((weak)) EI_IMPULSE_ERROR ei_run_impulse_check_canceled()
{
	return EI_IMPULSE_OK;
}

__attribute__((weak)) EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
	delay_ms(time_ms);
	return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms()
{
	return SysTick_GetTick();
}

uint64_t ei_read_timer_us()
{
	return SysTick_GetTick() * 1000;
}

__attribute__((weak)) void ei_printf(const char *format, ...)
{
	char buffer[1024] = {0};
	int length;
	va_list myargs;
	va_start(myargs, format);
	length = vsnprintf(buffer, sizeof(buffer), format, myargs);
	va_end(myargs);

	if(length > 0)
	{
		//uart_print(buffer, (uint16_t)length);
		HT_UART_Transmit((uint8_t *)buffer, length, 5000);
	}
}

__attribute__((weak)) void ei_printf_float(float f)
{
	float n = f;

	static double PRECISION = 0.00001;
	static int MAX_NUMBER_STRING_SIZE = 32;

	char s[MAX_NUMBER_STRING_SIZE];

	if(n == 0.0)
	{
		strcpy(s, "0");
	}
	else 
	{
		int digit, m;
		char *c = s;
		int neg = (n < 0);
		if(neg)
		{
			n = -n;
		}
		// calculate magnitude
		m = log10(n);
		if(neg)
		{
			*(c++) = '-';
		}
		if(m < 1.0)
		{
			m = 0;
		}
		// convert the number
		while(n > PRECISION || m >= 0)
		{
			double weight = pow(10.0, m);
			if(weight > 0 && !isinf(weight))
			{
				digit = floor(n / weight);
				n -= (digit * weight);
				*(c++) = '0' + digit;
			}
			if(m == 0 && n > 0)
			{
				*(c++) = '.';
			}

			m--;
		}

		*(c) = '\0';
	}


	ei_printf("%s", s);
}

__attribute__((weak)) void *ei_malloc(size_t size)
{
	return malloc(size);
}

__attribute__((weak)) void *ei_calloc(size_t nitems, size_t size)
{
	return calloc(nitems, size);
}

__attribute__((weak)) void ei_free(void *ptr)
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
