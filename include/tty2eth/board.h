#pragma once

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define USE_CPU_PERFORMANCE_ZONES 1

#define TEMP_SENSOR_ADDR 0x4A
#define POWER_SENSOR_ADDR 0x40

#define POWER_SENSOR_R_SHUNT 0.002
#define POWER_SENSOR_EXPECTED_CURRENT_MAX 5.0
#define POWER_SENSOR_CURRENT_LSB_MIN    (POWER_SENSOR_EXPECTED_CURRENT_MAX / 32768.0)

#define LED_USR_Pin GPIO_PIN_8
#define LED_USR_GPIO_Port GPIOA
#define PHYTEC_READY_Pin GPIO_PIN_14
#define PHYTEC_READY_GPIO_Port GPIOF
#define DEV_RST_Pin GPIO_PIN_9
#define DEV_RST_GPIO_Port GPIOE
#define PHYTEC_READY_EXTI_IRQn EXTI15_10_IRQn

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern UART_HandleTypeDef huart5;
extern I2C_HandleTypeDef hi2c2;

extern RTC_HandleTypeDef hrtc;

void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MPU_Config(void);
void MX_RTC_Init(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_UART4_Init(void);
void MX_UART5_Init(void);
void MX_I2C2_Init(void);
void MX_HASH_Init(void);
void MX_RNG_Init(void);

void Error_Handler(void);

bool startPhytec();
void restartPhytec();
bool stopPhytec();
bool togglePhytec();

HAL_StatusTypeDef I2CReadReg8(uint32_t dev, uint8_t reg, uint8_t *value);
HAL_StatusTypeDef I2CReadReg16(uint32_t dev, uint8_t reg, uint16_t *value);

HAL_StatusTypeDef I2CWriteReg8(uint32_t dev, uint8_t reg, uint8_t value);
HAL_StatusTypeDef I2CWriteReg16(uint32_t dev, uint8_t reg, uint16_t value);

void cpuHighPerformance();
void cpuLowPerformance();

void initCpuHighPerformanceZone();

void enterCpuHighPerformanceZone();
void exitCpuHighPerformanceZone();