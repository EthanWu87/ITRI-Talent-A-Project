
/* USART1 TX owns DMA1 Channel1; camera capture uses DMA1 Channel2.
 * Main-context only: do not print to USART1 from an interrupt.
 * The crop buffer is read-only until flow_image_uart_wait() finishes.
 */
#define IMAGE_UART_DMA_TIMEOUT_MS 1000U
static bool image_tx_active;
static bool image_tx_success = true;
static uint32_t image_tx_started;

static void image_uart_dma_init(void)
{
  dma_init_type config;
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dma_reset(DMA1_CHANNEL1);
  dma_flexible_config(DMA1, FLEX_CHANNEL1, DMA_FLEXIBLE_UART1_TX);
  dma_default_para_init(&config);
  config.peripheral_base_addr = (uint32_t)&USART1->dt;
  config.memory_base_addr = (uint32_t)ei_img_crop_buffer;
  config.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  config.buffer_size = EI_BUFFER_SIZE;
  config.peripheral_inc_enable = FALSE;
  config.memory_inc_enable = TRUE;
  config.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  config.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  config.loop_mode_enable = FALSE;
  config.priority = DMA_PRIORITY_LOW;
  dma_init(DMA1_CHANNEL1, &config);
}

static void image_uart_dma_start(void)
{
  flow_image_uart_wait();
  printf("IMG_S");
  image_tx_success = true;
  image_tx_active = true;
  image_tx_started = SysTick_GetTick();

  /* Finish IMG_S before DMA takes ownership of the USART data register. */
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
  {
    if((SysTick_GetTick() - image_tx_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      return;
    }
  }
  dma_flag_clear(DMA1_GL1_FLAG);
  DMA1_CHANNEL1->maddr = (uint32_t)ei_img_crop_buffer;
  dma_data_number_set(DMA1_CHANNEL1, EI_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
  usart_dma_transmitter_enable(USART1, TRUE);
}

uint8_t flow_image_uart_wait(void)
{
  if(!image_tx_active) return image_tx_success ? 1U : 0U;

  /* Called after inference, or before an EI diagnostic needs USART1. */
  while(image_tx_success && dma_flag_get(DMA1_FDT1_FLAG) == RESET)
  {
    if(dma_flag_get(DMA1_DTERR1_FLAG) != RESET ||
       (SysTick_GetTick() - image_tx_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      break;
    }
  }
  usart_dma_transmitter_enable(USART1, FALSE);
  dma_channel_enable(DMA1_CHANNEL1, FALSE);
  dma_flag_clear(DMA1_GL1_FLAG);

  /* DMA completion precedes the last byte leaving the UART shift register. */
  uint32_t drain_started = SysTick_GetTick();
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
  {
    if((SysTick_GetTick() - drain_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      break;
    }
  }
  image_tx_active = false;
  printf("IMG_E\r\n");
  return image_tx_success ? 1U : 0U;
}
