///////////////////////////////////////////////////////////////////////
////// Snakes Support
///////////////////////////////////////////////////////////////////////

#include "coop.h"
#include "vga_textbox.h"
#include "vgalib.h"

extern void setup_snakes(int hungry);
extern Process *glbl_thread_current;
#define curr_proc glbl_thread_current

Textbox_t *snakes_textbox; // WARN: not yet allocated

///////////////////////////////////////////////////////////////////////////////
///// Processes
///////////////////////////////////////////////////////////////////////////////
// The struct Process contains a pid field. Each process in the system must have
// a unique ID (use a monotonically incrementing counter). The field type must
// be int.

// WARN:  returns void, or struct process*?
// struct Process *PROC_create_kthread(kproc_t, void*)

extern void yield(void);
// System call as described above that yields control to another process

// A curr_proc global variable that points to the current running process (type
// is assumed to be struct Process)

///////////////////////////////////////////////////////////////////////////////
///// VGA
///////////////////////////////////////////////////////////////////////////////
// WARN: assuming the 0,0 is at top left? IIRC
// TODO: i think these funcs can be made static

int VGA_row_count(void) { return snakes_textbox->height; };

int VGA_col_count(void) { return snakes_textbox->width; };

extern void clear_Textbox(Textbox_t *box);
void VGA_clear(void) { clear_Textbox(snakes_textbox); };

extern position_t VGA_cursor;
extern vga_char_t *VGA_ptr();
void VGA_display_attr_char(int x, int y, char c, int fg, int bg) {
  VGA_cursor.x = snakes_textbox->x_corner + x;
  ASSERT(x < snakes_textbox->width);
  VGA_cursor.y = snakes_textbox->y_corner + y;
  ASSERT(y < snakes_textbox->height);
  VGA_display_char(c, fg, bg);
};

// Writes a single attributed character to the VGA buffer at a specified
// position. X is the buffer column. Y is the buffer row. c is the code point to
// write. fg is the foreground color to use. bg is the background color to use.
// You will need to combine fg and bg to create the attribute for the character.
//
//
//
//

void run_snakes_wrapper(Textbox_t *boxxy) {

  snakes_textbox = boxxy;
  VGA_clear();

  setup_snakes(1);
};
