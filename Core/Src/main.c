/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define ENC28J60_FRAME_DATA_MAX                                 1024
typedef enum
{
  READ_CONTROL_REG,
  READ_BUFFER_MEM,
  WRITE_CONTROL_REG,
  WRITE_BUFFER_MEM,
  BIT_FIELD_SET,
  BIT_FIELD_CLEAR,
  SYSTEM_RESET,
  COMMANDS_NUM,
} ENC28J60_Command;
typedef enum
{
  CS_LOW = 0,
  CS_HIGH = 1,
} ENC28J60_CS_State;
typedef enum
{
  BANK_0,
  BANK_1,
  BANK_2,
  BANK_3,
} ENC28J60_RegBank;
typedef enum
{
  ETH_REG,
  MAC_MII_REG,
} ENC28J60_RegType;
typedef struct ENC28J60_Frame
{
  uint16_t nextPtr;
  uint16_t length;
  uint16_t status;
  uint8_t data[ENC28J60_FRAME_DATA_MAX];
  uint32_t checkSum;
} ENC28J60_Frame;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAC_ADDRESS_BYTES_NUM                                   6
#define IP_ADDRESS_BYTES_NUM                                    4
#define ENC28J60_SPI_TIMEOUT                                    10
#define ENC28J60_OP_CODE_OFFSET                                 5
#define ENC28J60_REG_BANK_OFFSET                                5
#define ENC28J60_REG_TYPE_OFFSET                                7
#define ENC28J60_TX_BUF_START                                   0x0000
#define ENC28J60_RX_BUF_START                                   0x0600
#define ENC28J60_RX_BUF_END                                     0x1FFF
#define ENC28J60_FRAME_RX_OK_MASK                               0x80
#define ENC28J60_REG_BANK_MASK                                  0x60
#define ENC28J60_REG_TYPE_MASK                                  0x80
#define ENC28J60_REG_ADDR_MASK                                  0x1F
#define ENC28J60_BUF_COMMAND_ARG                                0x1A
#define ENC28J60_RESET_COMMAND_ARG                              0x1F
#define ENC28J60_BB_PACKET_GAP                                  0x15
#define ENC28J60_NBB_PACKET_GAP                                 0x0C12
#define ENC28J60_BANK_0_BITS                                    (BANK_0 << ENC28J60_REG_BANK_OFFSET)
#define ENC28J60_BANK_1_BITS                                    (BANK_1 << ENC28J60_REG_BANK_OFFSET)
#define ENC28J60_BANK_2_BITS                                    (BANK_2 << ENC28J60_REG_BANK_OFFSET)
#define ENC28J60_BANK_3_BITS                                    (BANK_3 << ENC28J60_REG_BANK_OFFSET)
#define ENC28J60_BANK_COMMON_BITS                               (BANK_0 << ENC28J60_REG_BANK_OFFSET)
#define ENC28J60_COMMON_REGS_ADDR                               0x1B
#define ENC28J60_ETH_REG_BIT                                    (ETH_REG << ENC28J60_REG_TYPE_OFFSET)
#define ENC28J60_MAC_MII_REG_BIT                                (MAC_MII_REG << ENC28J60_REG_TYPE_OFFSET)
// Common bank registers
#define EIE                                                     (0x1B | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_COMMON_BITS)
#define EIR                                                     (0x1C | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_COMMON_BITS)
#define ESTAT                                                   (0x1D | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_COMMON_BITS)
#define ECON2                                                   (0x1E | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_COMMON_BITS)
#define ECON1                                                   (0x1F | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_COMMON_BITS)
#define EIR_PKTIF_BIT                                           (1 << 6)
#define EIR_DMAIF_BIT                                           (1 << 5)
#define EIR_LINKIF_BIT                                          (1 << 4)
#define EIR_TXIF_BIT                                            (1 << 3)
#define EIR_TXERIF_BIT                                          (1 << 1)
#define EIR_RXERIF_BIT                                          (1 << 0)
#define ECON2_AUTOINC_BIT                                       (1 << 7)
#define ECON2_PKTDEC_BIT                                        (1 << 6)
#define ECON2_PWRSV_BIT                                         (1 << 5)
#define ECON2_VRPS_BIT                                          (1 << 3)
#define ECON1_TXRST_BIT                                         (1 << 7)
#define ECON1_RXRST_BIT                                         (1 << 6)
#define ECON1_DMAST_BIT                                         (1 << 5)
#define ECON1_CSUMEN_BIT                                        (1 << 4)
#define ECON1_TXRTS_BIT                                         (1 << 3)
#define ECON1_RXEN_BIT                                          (1 << 2)
#define ECON1_BSEL1_BIT                                         (1 << 1)
#define ECON1_BSEL0_BIT                                         (1 << 0)
// Bank 0 registers
#define ERDPTL                                                  (0x00 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERDPTH                                                  (0x01 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EWRPTL                                                  (0x02 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EWRPTH                                                  (0x03 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ETXSTL                                                  (0x04 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ETXSTH                                                  (0x05 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ETXNDL                                                  (0x06 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ETXNDH                                                  (0x07 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXSTL                                                  (0x08 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXSTH                                                  (0x09 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXNDL                                                  (0x0A | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXNDH                                                  (0x0B | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXRDPTL                                                (0x0C | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXRDPTH                                                (0x0D | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXWRPTL                                                (0x0E | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define ERXWRPTH                                                (0x0F | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMASTL                                                 (0x10 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMASTH                                                 (0x11 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMANDL                                                 (0x12 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMANDH                                                 (0x13 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMADSTL                                                (0x14 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMADSTH                                                (0x15 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMACSL                                                 (0x16 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
#define EDMACSH                                                 (0x17 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_0_BITS)
// Bank 1 registers
#define EHT0                                                    (0x00 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT1                                                    (0x01 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT2                                                    (0x02 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT3                                                    (0x03 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT4                                                    (0x04 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT5                                                    (0x05 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT6                                                    (0x06 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EHT7                                                    (0x07 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM0                                                   (0x08 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM1                                                   (0x09 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM2                                                   (0x0A | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM3                                                   (0x0B | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM4                                                   (0x0C | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM5                                                   (0x0D | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM6                                                   (0x0E | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMM7                                                   (0x0F | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMCSL                                                  (0x10 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMCSH                                                  (0x11 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMOL                                                   (0x14 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define EPMOH                                                   (0x15 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define ERXFCON                                                 (0x18 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
#define ERXFCON_UCEN_BIT                                        (1 << 7)
#define ERXFCON_ANDOR_BIT                                       (1 << 6)
#define ERXFCON_CRCEN_BIT                                       (1 << 5)
#define ERXFCON_PMEN_BIT                                        (1 << 4)
#define ERXFCON_MPEN_BIT                                        (1 << 3)
#define ERXFCON_HTEN_BIT                                        (1 << 2)
#define ERXFCON_MCEN_BIT                                        (1 << 1)
#define ERXFCON_BCEN_BIT                                        (1 << 0)
#define EPKTCNT                                                 (0x19 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_1_BITS)
// Bank 2 registers
#define MACON1                                                  (0x00 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MACON1_TXPAUS_BIT                                       (1 << 3)
#define MACON1_RXPAUS_BIT                                       (1 << 2)
#define MACON1_PASSALL_BIT                                      (1 << 1)
#define MACON1_MARXEN_BIT                                       (1 << 0)
#define MACON3                                                  (0x02 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MACON3_PADCFG2_BIT                                      (1 << 7)
#define MACON3_PADCFG1_BIT                                      (1 << 6)
#define MACON3_PADCFG0_BIT                                      (1 << 5)
#define MACON3_TXCRCEN_BIT                                      (1 << 4)
#define MACON3_PHDRLEN_BIT                                      (1 << 3)
#define MACON3_HFRMEN_BIT                                       (1 << 2)
#define MACON3_FRMLNEN_BIT                                      (1 << 1)
#define MACON3_FULDPX_BIT                                       (1 << 0)
#define MACON4                                                  (0x03 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MABBIPG                                                 (0x04 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MAIPGL                                                  (0x06 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MAIPGH                                                  (0x07 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MACLCON1                                                (0x08 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MACLCON2                                                (0x09 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MAMXFLL                                                 (0x0A | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MAMXFLH                                                 (0x0B | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MICMD                                                   (0x12 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MICMD_MIIRD_BIT                                         (1 << 0)
#define MIREGADR                                                (0x14 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MIWRL                                                   (0x16 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MIWRH                                                   (0x17 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MIRDL                                                   (0x18 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
#define MIRDH                                                   (0x19 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_2_BITS)
// Bank 3 registers
#define MAADR5                                                  (0x00 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MAADR6                                                  (0x01 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MAADR3                                                  (0x02 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MAADR4                                                  (0x03 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MAADR1                                                  (0x04 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MAADR2                                                  (0x05 | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define EBSTSD                                                  (0x06 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EBSTCON                                                 (0x07 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EBSTCSL                                                 (0x08 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EBSTCSH                                                 (0x09 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define MISTAT                                                  (0x0A | ENC28J60_MAC_MII_REG_BIT | ENC28J60_BANK_3_BITS)
#define MISTAT_BUSY_BIT                                         (1 << 0)
#define EREVID                                                  (0x12 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define ECOCON                                                  (0x15 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EFLOCON                                                 (0x17 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EPAUSL                                                  (0x18 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
#define EPAUSH                                                  (0x19 | ENC28J60_ETH_REG_BIT | ENC28J60_BANK_3_BITS)
// PHY registers
#define PHCON1                                                  (0x00)
#define PHSTAT1                                                 (0x01)
#define PHID1                                                   (0x02)
#define PHID2                                                   (0x03)
#define PHCON2                                                  (0x10)
#define PHCON2_FRCLNK_BIT                                       (1 << 14)
#define PHCON2_TXIS_BIT                                         (1 << 13)
#define PHCON2_JABBER_BIT                                       (1 << 10)
#define PHCON2_HDLDIS_BIT                                       (1 << 8)
#define PHSTAT2                                                 (0x11)
#define PHIE                                                    (0x12)
#define PHIR                                                    (0x13)
#define PHLCON                                                  (0x14)
#define PHLCON_LACFG3_BIT                                       (1 << 11)
#define PHLCON_LACFG2_BIT                                       (1 << 10)
#define PHLCON_LACFG1_BIT                                       (1 << 9)
#define PHLCON_LACFG0_BIT                                       (1 << 8)
#define PHLCON_LBCFG3_BIT                                       (1 << 7)
#define PHLCON_LBCFG2_BIT                                       (1 << 6)
#define PHLCON_LBCFG1_BIT                                       (1 << 5)
#define PHLCON_LBCFG0_BIT                                       (1 << 4)
#define PHLCON_LFRQ1_BIT                                        (1 << 3)
#define PHLCON_LFRQ0_BIT                                        (1 << 2)
#define PHLCON_STRCH_BIT                                        (1 << 1)
#define MISTAT_BUSY_BIT                                         (1 << 0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI5_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */