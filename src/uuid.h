#ifndef BLE_UUID_H
#define BLE_UUID_H

#include <zephyr/bluetooth/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

/* information service/characteristic uuids */
#define EH_INFO_SVC_UUID_VAL BT_UUID_128_ENCODE(0x1a220001, 0xc2ed, 0x4d11, 0xad1e, 0xfc06d8a02d37)
#define EH_INFO_LOG_CHR_UUID_VAL BT_UUID_128_ENCODE(0x1a220002, 0xc2ed, 0x4d11, 0xad1e, 0xfc06d8a02d37)

/* accelerometry service/characteristic uuids */
#define EH_ACCEL_SVC_UUID_VAL BT_UUID_128_ENCODE(0x1a220003, 0xc2ed, 0x4d11, 0xad1e, 0xfc06d8a02d37)
#define EH_ACCEL_CONFIG_CHR_UUID_VAL BT_UUID_128_ENCODE(0x1a220004, 0xc2ed, 0x4d11, 0xad1e, 0xfc06d8a02d37)
#define EH_ACCEL_DATA_CHR_UUID_VAL BT_UUID_128_ENCODE(0x1a220005, 0xc2ed, 0x4d11, 0xad1e, 0xfc06d8a02d37)

#ifdef __cplusplus
}
#endif

#endif
