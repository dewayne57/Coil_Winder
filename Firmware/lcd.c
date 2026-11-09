#include <Arduino.h>
#include <string.h>
#include "lcd.h"
#include "config.h"

/* Minimal HD44780 4-bit driver. Avoids the slow clear command so stepping isn't stalled. */

static char shadow[LCD_ROWS][LCD_COLS + 1];
static const uint8_t row_addr[4] = { 0x00, 0x40, 0x14, 0x54 };

static void pulse_enable(void) {
  digitalWrite(LCD_EN_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(LCD_EN_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(LCD_EN_PIN, LOW);
  delayMicroseconds(40);
}

static void write4(uint8_t nibble) {
  digitalWrite(LCD_D4_PIN, nibble & 0x01);
  digitalWrite(LCD_D5_PIN, (nibble >> 1) & 0x01);
  digitalWrite(LCD_D6_PIN, (nibble >> 2) & 0x01);
  digitalWrite(LCD_D7_PIN, (nibble >> 3) & 0x01);
  pulse_enable();
}

static void send(uint8_t value, uint8_t rs) {
  digitalWrite(LCD_RS_PIN, rs);
  write4(value >> 4);
  write4(value & 0x0F);
}

void lcd_init(void) {
  pinMode(LCD_RS_PIN, OUTPUT);
  pinMode(LCD_EN_PIN, OUTPUT);
  pinMode(LCD_D4_PIN, OUTPUT);
  pinMode(LCD_D5_PIN, OUTPUT);
  pinMode(LCD_D6_PIN, OUTPUT);
  pinMode(LCD_D7_PIN, OUTPUT);
  digitalWrite(LCD_RS_PIN, LOW);
  digitalWrite(LCD_EN_PIN, LOW);

  delay(50);            /* power-up wait */
  write4(0x03); delay(5);
  write4(0x03); delay(5);
  write4(0x03); delayMicroseconds(150);
  write4(0x02);         /* switch to 4-bit mode */
  send(0x28, 0);        /* 4-bit, 2+ line mode, 5x8 font */
  send(0x0C, 0);        /* display on, cursor off */
  send(0x06, 0);        /* increment, no shift */
  send(0x01, 0);        /* clear */
  delay(3);

  for (uint8_t r = 0; r < LCD_ROWS; r++) {
    memset(shadow[r], ' ', LCD_COLS);
    shadow[r][LCD_COLS] = '\0';
  }
}

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
