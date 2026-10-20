#ifndef _EI_MAIN_H_
#define _EI_MAIN_H_

#include <stdint.h>
#ifndef FLOW_NODE_ID
#define FLOW_NODE_ID 'A'
#endif
#ifdef __cplusplus
extern "C" {
#endif
int ei_main(void);
uint8_t flow_wifi_send_control(const char *kind, unsigned seq, const char *value);
#ifdef __cplusplus
}
#endif

#endif