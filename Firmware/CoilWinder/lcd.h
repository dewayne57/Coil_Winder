#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
/* Writes text to a row, padded/truncated to the display width. Skips the write if unchanged. */
void lcd_print_row(unsigned char row, const char *text);

#ifdef __cplusplus
}
#endif

#endif
