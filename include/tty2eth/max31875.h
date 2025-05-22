#pragma once
#include <stddef.h>

/* Define registers address */
#define MAX31875_TEMP_REG_ADDR 				0x00			// temperature register address
#define MAX31875_CONF_REG_ADDR 				0x01			// configuration register address

/* Define configuration parameters */						// look on MAX31875 datasheet

/* MAX31875 One-shot */
#define MAX31875_ONESHOT_DISABLE 			((uint8_t)0x00)
#define MAX31875_ONESHOT_ENABLE 			((uint8_t)0x01)

/* MAX31875 Conversion Rate */
#define MAX31875_CONVERSIONRATE_0_25		((uint8_t)0x00)		/* 0.25 conv/sec */
#define MAX31875_CONVERSIONRATE_1			((uint8_t)0x02)		/* 1 	conv/sec */
#define MAX31875_CONVERSIONRATE_4			((uint8_t)0x04)		/* 4	conv/sec */
#define MAX31875_CONVERSIONRATE_8			((uint8_t)0x06)		/* 8	conv/sec */

/* MAX31875 ShutDown */
#define MAX31875_SHUTDOWN_OFF				((uint8_t)0x00)		/* D8 -> 0 : continuous conversion on  */
#define MAX31875_SHUTDOWN_ON				((uint8_t)0x01)		/* D8 -> 1 : continuous conversion off */

/* MAX31875 TimeOut */
#define MAX31875_TIMEOUT_ENABLE				((uint8_t)0x00)
#define MAX31875_TIMEOUT_DISABLE			((uint8_t)0x10)

/* MAX31875 Resolution */
#define MAX31875_RESOLUTION_8				((uint8_t)0x00)		/* 8  bits of resolution */
#define MAX31875_RESOLUTION_9				((uint8_t)0x20)		/* 9  bits of resolution */
#define MAX31875_RESOLUTION_10				((uint8_t)0x40)		/* 10 bits of resolution */
#define MAX31875_RESOLUTION_12				((uint8_t)0x60)		/* 12 bits of resolution */

/* MAX31875 DataFormat */
#define MAX31875_DATAFORMAT_NORMAL			((uint8_t)0x00)		/* Normal mode   */
#define MAX31875_DATAFORMAT_EXTENDED		((uint8_t)0x80)		/* Extended mode */