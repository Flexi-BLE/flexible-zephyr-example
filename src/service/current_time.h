#ifndef BLE_SERVICE_CURRENT_TIME_H_
#define BLE_SERVICE_CURRENT_TIME_H_

#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

void current_time_svc_init();
void current_time_svc_notify();

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_CURRENT_TIME_H_ */