// DOM-IGNORE-BEGIN
/*******************************************************************************
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _EINK_BUNDLE_H_
#define _EINK_BUNDLE_H_

#include <stdbool.h>
#include <stdint.h>




#define BLE_DATA_LINE                     02u
#define BLE_DATA_SIZE                     03u
#define BLE_DATA                          04u

#define BLE_TEXT_LINE1                    01u
#define BLE_TEXT_LINE2                    02u
#define DISP_DATA_OFFSET                  03u
#define BLE_TEXT_STATUS_SIZE              12u
#define TXT_STR1_MAX_LEN                  20u
#define TXT_STR2_MAX_LEN                  10
typedef enum APP_E_PAPER_T
{
    APP_E_PAPER_INIT,
    APP_E_PAPER_PRINT,
    APP_E_PAPER_STACK_END
} APP_E_PAPER_T;


/* Commands for SSD16xx controller */
#define EPAPER_DRIVER_OUTPUT_CONTROL               0x01
#define EPAPER_GATE_DRIVING_VOLTAGE_CONTROL        0x03
#define EPAPER_SOURCE_DRIVING_VOLTAGE_CONTROL      0x04
#define EPAPER_SOFT_START_CONTROL                  0x0C
#define EPAPER_DEEP_SLEEP_MODE                     0x10
#define EPAPER_DATA_ENTRY_MODE                     0x11
#define EPAPER_SW_RESET                            0x12
#define EPAPER_MASTER_ACTIVATION                   0x20    
#define EPAPER_DISPLAY_UPDATE_1                    0x21
#define EPAPER_DISPLAY_UPDATE_2                    0x22    
#define EPAPER_WRITE_RAM                           0x24
#define EPAPER_WRITE_RAM_BW                        0x24
#define EPAPER_WRITE_RAM_RED                       0x26
#define EPAPER_AVCOM_SETTING                       0x2B
#define EPAPER_WRITE_VCOM_REGISTER                 0x2C
#define EPAPER_WRITE_LUT_REGISTER                  0x32
#define EPAPER_SET_DUMMY_LINE_PERIOD               0x3A
#define EPAPER_SET_GATE_LINE_WIDTH                 0x3B
#define EPAPER_BORDER_WAVEFORM                     0x3C    
#define EPAPER_SET_RAM_X                           0x44    
#define EPAPER_SET_RAM_Y                           0x45    
#define EPAPER_SET_RAM_X_ADDRESS_COUNTER           0x4E
#define EPAPER_SET_RAM_Y_ADDRESS_COUNTER           0x4F    
#define EPAPER_SET_ANALOG_BLOCK_CONTROL            0x74
#define EPAPER_SET_DIGITAL_BLOCK_CONTROL           0x7E
#define EPAPER_BOOSTER_FEEDBACK_SELECTION          0xF0
#define EPAPER_NO_OPERATION                        0xFF


/* E-Paper display 2,13" 122x250 dots
 * Display datasheet: 
 * https://download.mikroe.com/documents/datasheets/e-paper-display-2%2C13-122x250-n.pdf
 * SSD1675 Controller datasheet:
 * https://cdn-learn.adafruit.com/assets/assets/000/092/748/original/SSD1675_0.pdf?1593792604
 */
// Specific to ePAPER Display
#define EINK213_DISPLAY_WIDTH                       128 //normally 122 (0-121)
#define EINK213_DISPLAY_HEIGHT                      250 //0-254
#define EINK213_DISPLAY_RESOLUTIONS                 4000//3812//122x250/8

#define ANALOG_BLOCK_CONTROL_VALUE                  0x54
#define DIGITAL_BLOCK_CONTROL_VALUE                 0x3B
#define DUMMY_LINE_PERIOD_VALUE                     0x06
#define GATE_LINE_WDITH_VALUE                       0x0B
#define BORDER_WAVEFORM_VALUE                       0x33
#define VCOM_REGISTER_VALUE                         0x4B
#define DISPLAY_UPDATE_SEQUENCE_MODE_1              0xC7
#define DISPLAY_UPDATE_SEQUENCE_MODE_2              0x0C
#define LUT_DATA_SIZE                               70



/**
 *
 * @name                 Display Colors
 ******************************************************************************/
///@{
#define EINK_COLOR_WHITE                    0xFF
#define EINK_COLOR_LIGHT_GREY               0xAA
#define EINK_COLOR_DARK_GREY                0x55
#define EINK_COLOR_BLACK                    0x00

/**
 * @name                 Font Direction
 ******************************************************************************/
///@{
#define FO_HORIZONTAL           0x00
#define FO_VERTICAL             0x01
#define FO_VERTICAL_COLUMN      0x02
///@}

#define FULL			0
#define PART			1

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief eINK Initialization
     *
     * @param[in] mode          partial update mode or full update mode
     * 
     * Initializes display.
     */
    void eink_init(char Mode);

    /**
     * @brief eINK Set Font
     *
     * @param[in] font          Pointer to font definition
     * @param[in] color         Eink Display color
     * @param[in] orientation   Font direction
     *
     * Function must be called before text write function.
     */
    void eink_set_font(const uint8_t *font, uint16_t color, uint8_t orientation);

    /**
     * @brief Fills Screen Provided Color with partial update
     *
     * @param[in] color         Eink Display color
     */
    void eink_fill_screen_part(uint8_t color);
    
        /**
     * @brief Fills Screen Provided Color with full update
     *
     * @param[in] color         Eink Display color
     */
    void eink_fill_screen_full(uint8_t color);

    /**
     * @brief Fills Screen Provided Color
     *
     * @param[in] bgcolor         Eink Background color
     */
    void eink_frame_bg(uint8_t bgcolor);
    
    /**
     * @brief Write Image with partial update
     *
     *
     * @param[in] img - pointer to the image
     * @param[in] img_x_start - image x start
     * @param[in] img_x_end - image x end
     * @param[in] img_y_start - image y start
     * @param[in] img_y_end - image y end
     * 
     */
    void eink_display_image_part(const uint8_t* img, uint16_t img_x_start, uint16_t img_x_end, uint16_t img_y_start, uint16_t img_y_end);

    /**
     * @brief Write Image with full update
     *
     *
     * @param[in] img - pointer to the image
     * @param[in] img_x_start - image x start
     * @param[in] img_x_end - image x end
     * @param[in] img_y_start - image y start
     * @param[in] img_y_end - image y end
     * 
     */
    void eink_display_image_full(const uint8_t* img, uint16_t img_x_start, uint16_t img_x_end, uint16_t img_y_start, uint16_t img_y_end);

    
    /**
     * @brief Write Text with partial update
     *
     * @param[in] text  input text
     * @param[in] n     Number of Character
     * @param[in] x     horizontal offset
     * @param[in] y     vertical offset
     *
     * Writes text on eINK display.
     */
    void eink_text_part(char *text, uint8_t n, uint16_t text_x_start, uint16_t text_x_end, uint16_t text_y_start, uint16_t text_y_end);

     /**
     * @brief Write Text with full update
     *
     * @param[in] text  input text
     * @param[in] n     Number of Character
     * @param[in] x     horizontal offset
     * @param[in] y     vertical offset
     *
     * Writes text on eINK display.
     */
    void eink_text_full(char *text, uint8_t n, uint16_t text_x_start, uint16_t text_x_end, uint16_t text_y_start, uint16_t text_y_end);

    
    /**
     * @brief Send Command
     *
     * @param[in] cmd  Command Address
     *
     * Writes command to eINK display.
     */
    void eink_sendCmd(uint8_t cmd);

    /**
     * @brief Send Data
     *
     * @param[in] data  Data Address
     *
     * Writes Data to eINK display.
     */
    void eink_sendData(uint8_t data);

    /**
     * @brief Write Text
     *
     * @param[in] x     display width
     * @param[in] y     display height
     *
     * Writes RAM Address to eINK display.
     */
    void eink_setRAMaddress(uint16_t x, uint16_t y);

    /**
     * @brief eINK reset counter
     *
     * Reset the counter.
     */
    void eink_resetCounter(void);

    /**
     * @brief eINK power up.
     *
     * Wakeup from sleep mode and Initializes display with default settings.
     */
    void eink_powerup(void);

    /**
     * @brief eINK power down.
     *
     * Puts display to sleep mode.
     */
    void eink_powerDown(void);

    /**
     * @brief eINK Update Full.
     *
     * Update Full display.
     */
    void eink_updateFull(void);

    /**
     * @brief eINK Update Partial.
     *
     * Update Partial display.
     */
    void eink_updatePartial(void);

    /**
     * @brief eINK example application.
     *
     * Update Partial display.
     */
    void eink_example(void);

    /**
     * @brief eINK eINK Initialize.
     *
     * Update Partial display.
     */
    void e_paper_initialize(void);
    
    /**
     * @brief eINK print.
     *
     * Update Partial display.
     */
    void e_paper_print(uint8_t disp, uint8_t line, uint8_t *data);
    
    /**
     * @brief eINK Handler.
     *
     * Update Partial display.
     */
    void APP_E_PAPER_Handler(uint8_t *msgData);
    
    
#ifdef __cplusplus
} // extern "C"
#endif

#define LCD_INIT()          e_paper_initialize()
#define LCD_PRINT(...)      e_paper_print(__VA_ARGS__)

#endif
///@}
/**
 * @}                                                           End of File
 ******************************************************************************/
