#ifndef FLOW_METRICS_H
#define FLOW_METRICS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  volatile uint32_t radar_rx_drops, wifi_rx_drops, uwb_rx_drops;
  volatile uint32_t radar_rx_peak, wifi_rx_peak, uwb_rx_peak;
  uint32_t service_gap_ms, inference_block_ms, dma_wait_ms, text_block_ms;
  uint32_t inference_errors, event_drops;
} flow_metrics_t;

extern flow_metrics_t g_flow_metrics;

#ifdef __cplusplus
}
#endif

#endif
