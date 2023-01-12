// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "app.h"
#include "definitions.h"
#include "app_zigbee/app_zigbee.h"
#include <z3device/common/include/z3Device.h>
#include <z3device/stack_interface/zgb_api.h>
#include <z3device/stack_interface/bdb/include/bdb_api.h>
#include <osal/osal_freertos.h>
#include <stdio.h>
#include "app_timer/app_timer.h"
#include <zcl/include/zclThermostatCluster.h>
#include "click_routines/eink_bundle/eink_bundle.h"
#include "zcl/include/zclThermostatCluster.h"
#include <z3device/thermostat/include/thThermostatCluster.h>
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern void process_UART_evt(char* cmdBuf);
extern void APP_UartInit(void);
extern void APP_UartHandler(void);
extern void process_ZB_evt(void);
extern void ZB_ZCL_CallBack(ZB_AppGenericCallbackParam_t* cb);
extern ZDO_CALLBACK_ptr ZB_ZDO_CallBack[];
extern void setOccupiedCoolingSp(int16_t temp);
int16_t thermoCool_Temp = 2500,max_temp=20;
#define TOUCH_COOL_INC          "Cool +\r\n"
#define TOUCH_COOL_DEC          "Cool -\r\n"
#define TOUCH_DISPLAY_ON        "Dis_ON\r\n"
#define TOUCH_DISPLAY_OFF       "DisOFF\r\n"
#define TOUCH_RESET_TO_FN       "ResetF\r\n"
#define TOUCH_DISPLAY_TOGGLE    "DisTog\r\n"
#define THERMOSTAT_LOCAL_TEMPERATURE_SCALE 100

SYS_CONSOLE_HANDLE touchDevConsoleHandle;
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/
void local_temp_update(int16_t temp_value)
{
    char printData[30];
    if(max_temp!=(temp_value/100))
    {
        max_temp=temp_value/100;
        sprintf(printData, "Curr: %d.%dC\n", (int)(temp_value/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(temp_value%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
        LCD_PRINT(0,3,printData);
        APP_TIMER_SetTimer(APP_TIMER_ID_0, APP_TIMER_30S , false);
    }
}



void SetCoolTemperature(uint8_t *coolTemp)
{
    ZCL_WriteAttributeValue(APP_ENDPOINT_THERMOSTAT,THERMOSTAT_CLUSTER_ID,
                                    ZCL_SERVER_CLUSTER_TYPE,ZCL_THERMOSTAT_CLUSTER_OCCUPIED_COOLING_SETPOINT_SERVER_ATTRIBUTE_ID,
                                    ZCL_S16BIT_DATA_TYPE_ID,coolTemp);

}

void update_written_setValue(int16_t temp)
{
    if(thermoCool_Temp!=temp)
    {
    char printData[30];
    thermoCool_Temp=temp;
    sprintf(printData, "Set: %d.%dC\n", (int)(thermoCool_Temp/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(thermoCool_Temp%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
    LCD_PRINT(0,5,printData);
    }
}

void HVAC_Control(void)
{
    if(thThermostatClusterServerAttributes.localTemperature.value > thThermostatClusterServerAttributes.occupiedCoolingSetpoint.value)
    {
        printf("AC On\r\n");
        LCD_PRINT(0,7,"AC ON");
        RED_LED_Set();
    }
    else
    {
        printf("AC Off\r\n");
        LCD_PRINT(0,7,"AC OFF");
        RED_LED_Clear();
    }
}

static void touch_read_data(void)                                   //Function to read the touch data transmitted by ATtiny3217+T10
{
    ssize_t nUnreadBytes = 0;
    APP_Msg_T appMsg;
    nUnreadBytes = SYS_CONSOLE_ReadCountGet(touchDevConsoleHandle);
    if(nUnreadBytes > 7)
    {
        appMsg.msgId = APP_TOUCH_USART_READ_MSG;
        appMsg.msgData[0] = (uint8_t)nUnreadBytes;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
    else
    {
        appMsg.msgId = APP_TOUCH_USART_GET_COUNT_MSG;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
}

void APP_Touch_Handler(char *tDataReadBuf)
{
    char printData[30];
    if( strncmp(tDataReadBuf, TOUCH_COOL_INC, strlen(TOUCH_COOL_INC)) == 0)
    {
        if(thermoCool_Temp < 8000)
        {
            thermoCool_Temp += 50;
            sprintf(printData, "Set: %d.%dC\n", (int)(thermoCool_Temp/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(thermoCool_Temp%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
            SetCoolTemperature( (uint8_t *)&thermoCool_Temp );
            LCD_PRINT(0,5,printData);
            printf("Set temp: %d\n",thermoCool_Temp);
        }
        else
        {
            sprintf(printData, "%d.%dC Max", (int)(thermoCool_Temp/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(thermoCool_Temp%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
            LCD_PRINT(0,5,printData);
        }
    }
    else if(strncmp(tDataReadBuf, TOUCH_COOL_DEC, strlen(TOUCH_COOL_DEC)) == 0)
    {
        if(thermoCool_Temp >= 2100)
        {
            thermoCool_Temp -= 50;
            sprintf(printData, "Set: %d.%dC\n", (int)(thermoCool_Temp/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(thermoCool_Temp%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
            SetCoolTemperature( (uint8_t *)&thermoCool_Temp );
            LCD_PRINT(0,5,printData);
            printf("Set temp: %d\n",thermoCool_Temp);
        }
        else
        {
            sprintf(printData, "%d.%dC Min", (int)(thermoCool_Temp/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(thermoCool_Temp%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
            LCD_PRINT(0,5,printData);
        }  
    }
    else if(strncmp(tDataReadBuf, TOUCH_RESET_TO_FN, strlen(TOUCH_RESET_TO_FN)) == 0)
    {
        ZB_BDB_ResetToFactoryNew(true);
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;


    appData.appQueue = xQueueCreate( 64, sizeof(APP_Msg_T) );
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    APP_UartInit();

    APP_ZigbeeStackInit();
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_Msg_T    appMsg[1];
    APP_Msg_T   *p_appMsg;
    p_appMsg=appMsg;
    char touchDataReadBuffer[15];
    ZB_AppGenericCallbackParam_t cb;
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            touchDevConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);
            touch_read_data();
            if (appInitialized)
            {
                //appData.state = APP_STATE_E_PAPER_INIT;
                appData.state = APP_STATE_SERVICE_TASKS; 
            }
            break;
        }
        case APP_STATE_SERVICE_TASKS:
        {
            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, OSAL_WAIT_FOREVER))
            {
                if (p_appMsg->msgId == APP_MSG_ZB_STACK_CB)
                {
                    // Pass Zigbee Stack Callback Event Message to User Application for handling
                    uint32_t *paramPtr = NULL;
                    memcpy(&paramPtr,p_appMsg->msgData,sizeof(paramPtr));
                    memcpy(&cb, paramPtr, sizeof(cb));
                    switch (cb.eModuleID)
                {
                      case ZIGBEE_BDB:
                        ZB_BDB_CallBack(&cb);
                      break;

                      case ZIGBEE_ZDO:
                        ZB_ZDO_CallBack[cb.uCallBackID]((void *)cb.parameters);
                      break;

                      case ZIGBEE_ZCL:
                          ZB_ZCL_CallBack(&cb);
                      break;

                      default:
                        //appSnprintf("[APP CB]  Default case\r\n");
                      break;
                }
                    void *ptr = NULL;
                    memcpy(&ptr, p_appMsg->msgData,sizeof(ptr));
                    OSAL_Free(ptr);
                    OSAL_Free(cb.parameters);
                    
                }
                else if(p_appMsg->msgId==APP_MSG_ZB_STACK_EVT)
                {
                    // Pass Zigbee Stack Event Message to User Application for handling
                    process_ZB_evt();
                }
                else if( p_appMsg->msgId == APP_MSG_UART_CMD_READY)
                {                   
                    process_UART_evt((char*)(p_appMsg->msgData));
                }
                else if( p_appMsg->msgId == APP_MSG_E_PAPER_EVT)
                {
                    APP_E_PAPER_Handler((uint8_t *)(p_appMsg->msgData));
                }
                else if(p_appMsg->msgId== APP_TOUCH_USART_READ_MSG)
                {
                    memset(touchDataReadBuffer, 0, sizeof(touchDataReadBuffer));
                    SYS_CONSOLE_Read( touchDevConsoleHandle, touchDataReadBuffer, p_appMsg->msgData[0] );
                    APP_Touch_Handler(touchDataReadBuffer);
                    p_appMsg->msgId = APP_TOUCH_USART_GET_COUNT_MSG;
                    OSAL_QUEUE_Send(&appData.appQueue, p_appMsg, 0);
                }
                else if(p_appMsg->msgId== APP_TOUCH_USART_GET_COUNT_MSG)
                {
                    touch_read_data();
                }
                else if(p_appMsg->msgId == APP_IDLE_TASK_EVT)
                {
                    extern void app_idle_task( void );
                    app_idle_task();
                }
            }
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
