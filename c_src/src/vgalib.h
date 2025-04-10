#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_BLUE 0x01
#define VGA_GREEN 0x02
#define VGA_CYAN 0x03
#define VGA_RED 0x04
#define VGA_PURPLE 0x05
#define VGA_ORANGE 0x06
#define VGA_LIGHT_GREY 0x07
#define VGA_DARK_GREY 0x08
#define VGA_BRIGHT_BLUE 0x09
#define VGA_BRIGHT_GREEN 0x0A
#define VGA_BRIGHT_CYAN 0x0B
#define VGA_MAGENTA 0x0C
#define VGA_BRIGHT_PURPLE 0x0D
#define VGA_YELLOW 0x0E
#define VGA_WHITE 0x0F

#include "freestanding.h"

typedef struct {
  uint8_t character : 8; // 8 bits for the character
  uint8_t fg_color : 4;  // 4 bits for foreground color (0-15)
  uint8_t bg_color : 4;  // 4 bits for background color (0-15)
} __attribute__((packed)) vga_char_t;

typedef struct {
  int x;
  int y;
} position_t;

// globalz
extern position_t VGA_cursor;
extern uint8_t vga_bg_default;
extern uint8_t vga_fg_default;

void VGA_display_char(char c, uint8_t fg, uint8_t bg);
// void VGA_setpos(int x, int y); // global instead
void VGA_clear(void);
