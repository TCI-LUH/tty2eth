/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file         stm32h7xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "board.h"

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */
    PWREx_AVDTypeDef sConfigAVD = {0};

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* System interrupt init*/
    /* BusFault_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(BusFault_IRQn, 5, 0);
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    /** AVD Configuration
     */
    sConfigAVD.AVDLevel = PWR_AVDLEVEL_0;
    sConfigAVD.Mode = PWR_AVD_MODE_NORMAL;
    HAL_PWREx_ConfigAVD(&sConfigAVD);

    /** Enable the AVD Output
     */
    HAL_PWREx_EnableAVD();

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hi2c->Instance == I2C2)
    {
        /* USER CODE BEGIN I2C2_MspInit 0 */

        /* USER CODE END I2C2_MspInit 0 */

        __HAL_RCC_GPIOH_CLK_ENABLE();
        /**I2C2 GPIO Configuration
        PH4     ------> I2C2_SCL
        PH5     ------> I2C2_SDA
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();
        /* USER CODE BEGIN I2C2_MspInit 1 */

        /* USER CODE END I2C2_MspInit 1 */
    }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        /* USER CODE BEGIN I2C2_MspDeInit 0 */

        /* USER CODE END I2C2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C2_CLK_DISABLE();

        /**I2C2 GPIO Configuration
        PH4     ------> I2C2_SCL
        PH5     ------> I2C2_SDA
        */
        HAL_GPIO_DeInit(GPIOH, GPIO_PIN_4);

        HAL_GPIO_DeInit(GPIOH, GPIO_PIN_5);

        /* USER CODE BEGIN I2C2_MspDeInit 1 */

        /* USER CODE END I2C2_MspDeInit 1 */
    }
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hrtc->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
        /* USER CODE BEGIN RTC_MspInit 1 */

        /* USER CODE END RTC_MspInit 1 */
    }
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
    if (hrtc->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspDeInit 0 */

        /* USER CODE END RTC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
        /* USER CODE BEGIN RTC_MspDeInit 1 */

        /* USER CODE END RTC_MspDeInit 1 */
    }
}

/**
 * @brief CRYP MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcryp: CRYP handle pointer
 * @retval None
 */
void HAL_CRYP_MspInit(CRYP_HandleTypeDef *hcryp)
{
    if (hcryp->Instance == CRYP)
    {
        /* USER CODE BEGIN CRYP_MspInit 0 */

        /* USER CODE END CRYP_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CRYP_CLK_ENABLE();
        /* USER CODE BEGIN CRYP_MspInit 1 */

        /* USER CODE END CRYP_MspInit 1 */
    }
}

/**
 * @brief CRYP MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcryp: CRYP handle pointer
 * @retval None
 */
void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef *hcryp)
{
    if (hcryp->Instance == CRYP)
    {
        /* USER CODE BEGIN CRYP_MspDeInit 0 */

        /* USER CODE END CRYP_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CRYP_CLK_DISABLE();
        /* USER CODE BEGIN CRYP_MspDeInit 1 */

        /* USER CODE END CRYP_MspDeInit 1 */
    }
}

/**
 * @brief HASH MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hhash: HASH handle pointer
 * @retval None
 */
void HAL_HASH_MspInit(HASH_HandleTypeDef *hhash)
{
    /* USER CODE BEGIN HASH_MspInit 0 */

    /* USER CODE END HASH_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_HASH_CLK_ENABLE();
    /* USER CODE BEGIN HASH_MspInit 1 */

    /* USER CODE END HASH_MspInit 1 */
}

/**
 * @brief HASH MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hhash: HASH handle pointer
 * @retval None
 */
void HAL_HASH_MspDeInit(HASH_HandleTypeDef *hhash)
{
    /* USER CODE BEGIN HASH_MspDeInit 0 */

    /* USER CODE END HASH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_HASH_CLK_DISABLE();
    /* USER CODE BEGIN HASH_MspDeInit 1 */

    /* USER CODE END HASH_MspDeInit 1 */
}

/**
 * @brief RNG MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hrng->Instance == RNG)
    {
        /* USER CODE BEGIN RNG_MspInit 0 */

        /* USER CODE END RNG_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
        PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RNG_CLK_ENABLE();
        /* USER CODE BEGIN RNG_MspInit 1 */

        /* USER CODE END RNG_MspInit 1 */
    }
}

/**
 * @brief RNG MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
    if (hrng->Instance == RNG)
    {
        /* USER CODE BEGIN RNG_MspDeInit 0 */

        /* USER CODE END RNG_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RNG_CLK_DISABLE();
        /* USER CODE BEGIN RNG_MspDeInit 1 */

        /* USER CODE END RNG_MspDeInit 1 */
    }
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (huart->Instance == UART4)
    {
        /* USER CODE BEGIN UART4_MspInit 0 */

        /* USER CODE END UART4_MspInit 0 */

        /* Peripheral clock enable */
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**UART4 GPIO Configuration
        PB8     ------> UART4_RX
        PD1     ------> UART4_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* UART4 DMA Init */
        /* UART4_RX Init */
        hdma_uart4_rx.Instance = DMA2_Stream0;
        hdma_uart4_rx.Init.Request = DMA_REQUEST_UART4_RX;
        hdma_uart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_uart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_uart4_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_uart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_uart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_uart4_rx.Init.Mode = DMA_CIRCULAR;
        hdma_uart4_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
        hdma_uart4_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_uart4_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, hdma_uart4_rx);

        /* UART4 interrupt Init */
        HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);

        /* USER CODE BEGIN UART4_MspInit 1 */

        /* USER CODE END UART4_MspInit 1 */
    }
    else if (huart->Instance == UART5)
    {
        /* USER CODE BEGIN UART5_MspInit 0 */

        /* USER CODE END UART5_MspInit 0 */

        /* Peripheral clock enable */
        __HAL_RCC_UART5_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**UART5 GPIO Configuration
        PB6    ------> UART5_TX
        PD2     ------> UART5_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_UART5;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* USER CODE BEGIN UART5_MspInit 1 */

        /* USER CODE END UART5_MspInit 1 */
    }
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        /* USER CODE BEGIN UART4_MspDeInit 0 */

        /* USER CODE END UART4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART4_CLK_DISABLE();

        /**UART4 GPIO Configuration
        PB8     ------> UART4_RX
        PD1     ------> UART4_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_1);

        /* UART4 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* UART4 interrupt DeInit */
        HAL_NVIC_DisableIRQ(UART4_IRQn);
        /* USER CODE BEGIN UART4_MspDeInit 1 */

        /* USER CODE END UART4_MspDeInit 1 */
    }
    else if (huart->Instance == UART5)
    {
        /* USER CODE BEGIN UART5_MspDeInit 0 */

        /* USER CODE END UART5_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART5_CLK_DISABLE();

        /**UART5 GPIO Configuration
        PD2     ------> UART5_RX
    PB6     ------> UART5_TX
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

        /* USER CODE BEGIN UART5_MspDeInit 1 */

        /* USER CODE END UART5_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
