/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "elog.h"
#include "gpio.h"
#include "task.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// clang-format off
#define TASK1_STACK_SIZE            20
#define TASK2_STACK_SIZE            20
// clang-format on
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
portCHAR flag1;
portCHAR flag2;
/* USER CODE BEGIN PV */
extern List_t pxReadyTasksLists[configMAX_PRIORITIES];
TaskHandle_t Task1_Handle;
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;

TaskHandle_t Task2_Handle;
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;

/* Idle task define */
#define configMINIMAL_STACK_SIZE ((uint16_t)128)
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory(TCB_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer,
    uint32_t* pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &IdleTaskTCB;
    *ppxIdleTaskStackBuffer = IdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void test_elog(void)
{
    /* test log output for all level */
    log_a("Hello EasyLogger!");
    log_e("Hello EasyLogger!");
    log_w("Hello EasyLogger!");
    log_i("Hello EasyLogger!");
    log_d("Hello EasyLogger!");
    log_v("Hello EasyLogger!");
    //    elog_raw("Hello EasyLogger!");
}

void delay(uint32_t count)
{
    for (; count != 0; count--) {
        __asm__ __volatile__("nop");
    }
}

void Task1(void* pvParameters)
{
    while (1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
        vTaskDelay(100);
        // delay(100000);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
        // delay(100000);
        vTaskDelay(100);
        portYIELD();
    }
}

void Task2(void* pvParameters)
{
    while (1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        // delay(100000);
        vTaskDelay(100);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        // delay(100000);
        vTaskDelay(100);
        portYIELD();
    }
}
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
    // SysTick->CTRL = 0;
    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    /* USER CODE BEGIN 2 */
    /* initialize EasyLogger */
    // elog_init();
    // /* set EasyLogger log format */
    // elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    // elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    // elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    // elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    // elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
    // elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
    // /* start EasyLogger */
    // elog_start();

    prvInitialiseTaskLists();
    Task1_Handle = xTaskCreateStatic((TaskFunction_t)Task1,
        (char*)"Task1",
        (uint32_t)TASK1_STACK_SIZE,
        (void*)NULL,
        (StackType_t*)Task1Stack,
        (TCB_t*)&Task1TCB);

    vListInsertEnd(&(pxReadyTasksLists[1]), &(((TCB_t*)(&Task1TCB))->xStateListItem));

    Task2_Handle = xTaskCreateStatic((TaskFunction_t)Task2,
        (char*)"Task2",
        (uint32_t)TASK2_STACK_SIZE,
        (void*)NULL,
        (StackType_t*)Task2Stack,
        (TCB_t*)&Task2TCB);

    vListInsertEnd(&(pxReadyTasksLists[2]), &(((TCB_t*)(&Task2TCB))->xStateListItem));
    vTaskStartScheduler();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */
        // HAL_Delay(1000);
        // test_elog();
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
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
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
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
