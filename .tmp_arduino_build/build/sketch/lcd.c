#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\lcd.c"
/** Coil_Winder Firmware
 * Copyright (c) 2026 Dewayne L. Hafenstein
 *
 * This file is part of the Coil_Winder project.
 *
 * License: Coil_Winder Non-Commercial License
 * Permission is granted to use, copy, modify, and redistribute this
 * firmware for personal, educational, research, and non-commercial
 * purposes only.
 * Commercial use, including sale, manufacturing, sublicensing, or
 * distribution for direct or indirect financial gain, is prohibited
 * without a separate written commercial license from the copyright
 * holder.
 * Any modified or redistributed copies must retain this notice.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE
 * FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <Arduino.h>
#include <string.h>
#include "lcd.h"
#include "config.h"

/**
 * Minimal HD44780 8-bit LCD driver for AVR using PORTE for data and
 * PC0/PC1 for control lines.  Provides functions to initialize the
 * display and print text to specific rows.  Maintains a shadow buffer 
 * to minimize unnecessary writes to the display.
 * Supports basic text output without cursor management or custom
 * characters.  Does not handle scrolling or automatic line wrapping.
 * 
 * Usage:
 * lcd_init();
 * lcd_print_row(row, text);
 * // Example:
 * lcd_print_row(0, "Hello, World!");
 * lcd_print_row(1, "Line 2 text");
 * // Note: LCD_ROWS and LCD_COLS define the dimensions of the display.
 */

static char shadow[LCD_ROWS][LCD_COLS + 1];
static const uint8_t row_addr[4] = { 0x00, 0x40, 0x14, 0x54 };

/**
 * @brief Sets the LCD register-select line.
 *
 * @param rs True selects the data register, false selects the command register.
 */
static inline void lcd_rs_set(uint8_t rs) {
  if (rs) PORTC |= _BV(PC1);
  else    PORTC &= ~_BV(PC1);
}

/**
 * @brief Pulses the LCD enable line to latch the current data byte.
 */
static void pulse_enable(void) {
  PORTC &= ~_BV(PC0);
  delayMicroseconds(1);
  PORTC |= _BV(PC0);
  delayMicroseconds(1);
  PORTC &= ~_BV(PC0);
  delayMicroseconds(40);
}

/**
 * @brief Writes a byte to the selected LCD register.
 *
 * @param value Data or command byte to send.
 * @param rs Register select value: 0 for command, 1 for data.
 */
static void send(uint8_t value, uint8_t rs) {
  lcd_rs_set(rs);
  PORTE = value;
  pulse_enable();
}

/**
 * @brief Initializes the HD44780 LCD and configures the PE/PC port layout.
 * Configures the data and control lines for the LCD and performs the
 * initialization sequence required by the HD44780 controller.
 * @note This function must be called before any other LCD operations.
 * @note The LCD must be powered and connected correctly for initialization to succeed.
 * @note The initialization sequence follows the standard procedure for 8-bit mode with
 *  2-line display and 5x8 font.  This is correct even since we are using a 4-line X 20 
 * display (80 characters, which is the maximum supported by this configuration and is 
 * the same as a standard 2-line display in terms of the initialization sequence).
 * 
 * @note The shadow buffer is initialized to spaces for all rows.
 */
void lcd_init(void) {
  pinMode(LCD_RS_PIN, OUTPUT);
  pinMode(LCD_EN_PIN, OUTPUT);
  DDRC |= _BV(PC0) | _BV(PC1);
  DDRC &= ~(_BV(PC5) | _BV(PC6) | _BV(PC7));
  DDRD; /* keep the compiler happy; no direct effect */
  DDRE = 0xFF;        /* PE0..PE7 = DB0..DB7 */
  PORTE = 0x00;
  PORTC &= ~(_BV(PC0) | _BV(PC1));

  delay(50);            /* power-up wait */
  send(0x30, 0); delay(5);
  send(0x30, 0); delay(5);
  send(0x30, 0); delayMicroseconds(150);
  send(0x38, 0);        /* 8-bit, 2+ line mode, 5x8 font */
  send(0x0C, 0);        /* display on, cursor off */
  send(0x06, 0);        /* increment, no shift */
  send(0x01, 0);        /* clear */
  delay(3);

  for (uint8_t r = 0; r < LCD_ROWS; r++) {
    memset(shadow[r], ' ', LCD_COLS);
    shadow[r][LCD_COLS] = '\0';
  }
}

/**
 * @brief Prints a text string to one LCD row.
 *
 * @param row Display row to update.
 * @param text Null-terminated string to render.
 */
void lcd_print_row(unsigned char row, const char *text) {
  if (row >= LCD_ROWS) return;
  char line[LCD_COLS + 1];
  size_t len = strlen(text);
  for (uint8_t i = 0; i < LCD_COLS; i++) line[i] = (i < len) ? text[i] : ' ';
  line[LCD_COLS] = '\0';

  if (memcmp(line, shadow[row], LCD_COLS) == 0) return;
  memcpy(shadow[row], line, LCD_COLS + 1);

  send(0x80 | row_addr[row], 0);
  for (uint8_t i = 0; i < LCD_COLS; i++) send((uint8_t)line[i], 1);
}
