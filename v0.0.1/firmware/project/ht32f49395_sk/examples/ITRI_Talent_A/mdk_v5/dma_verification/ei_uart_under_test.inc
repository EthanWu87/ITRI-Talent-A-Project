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

