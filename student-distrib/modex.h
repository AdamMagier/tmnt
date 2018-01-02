/* modex.h */

#ifndef _MODEX_H
#define _MODEX_H

/*
 * IMAGE   is the whole screen in mode X: 320x200 pixels in our flavor.
 * SCROLL  is the scrolling region of the screen.
 *
 * X_DIM   is a horizontal screen dimension in pixels.
 * X_WIDTH is a horizontal screen dimension in 'natural' units (addresses, characters of text, etc.)
 * Y_DIM   is a vertical screen dimension in pixels.
 */
#define IMAGE_X_DIM     320   /* pixels; must be divisible by 4  */
#define IMAGE_Y_DIM     200   /* pixels                          */
#define IMAGE_X_WIDTH   (IMAGE_X_DIM / 4)   /* addresses (bytes) */
#define SCROLL_X_DIM    IMAGE_X_DIM         /* full image width  */
#define SCROLL_Y_DIM    IMAGE_Y_DIM         /* full image height */
#define SCROLL_X_WIDTH  (IMAGE_X_DIM / 4)   /* addresses (bytes) */

#define TRANSPARENT 0x00

/*
 * NOTES
 *
 * Mode X is an (originally) undocumented variant of mode 13h, the first
 * IBM 256-color graphics mode. Each pixel uses a single byte to specify
 * one of the 256 possible colors, and a palette is used to map each color
 * into an 18-bit space (6-bit red, green, and blue intensities).
 *
 * The map from memory as seen by the host processor is non-linear, and was
 * originally designed to allow high-performance hardware designs. Video
 * memory is in words of 32 bits, and is divided into four planes. In mode
 * X, groups of four pixels are mapped into a single host address and written
 * individually or together by setting a bit mask of planes to be written
 * (a VGA register).
 *
 * each four pixels counts as one (one-byte) address ->
 * 0123012301230123012301230123012301230123012301230123012301230123
 *
 * The mapping is more contorted than with mode 13h, but allows use of
 * graphics tricks such as double-buffering.
 *
 * The design here is strongly influenced by the fact that we are running
 * in a virtual machine in which writes to video memory are exorbitantly
 * expensive. In particular, writing a chunk of 16kB with a single x86
 * instruction (REP MOVSB) is much faster than writing two hundred bytes
 * with many instructions.
 *
 * That said, the design is not unreasonable, and is only slightly different
 * than was (and is) often used today.
 *
 * Double-buffering uses two sections of memory to allow a program to
 * draw the next screen to be displayed without having the partially drawn
 * screen visible on the monitor (which causes flicker). When the drawing
 * is complete, the video adapter is told to display the new screen instead
 * of the old one, and the memory used for the old screen is then used to
 * draw a third screen, the video adapter is switched back, and the process
 * starts again.
 *
 * In our variant of double-buffering, we use non-video memory as the
 * scratch pad, copy the drawn screen as a whole into one of two buffers
 * in video memory, and switch the picture between the two buffers. The
 * cost of the copy is negligible; the cost of writing to video memory
 * instead is quite high (under most virtual machines).
 *
 * In order to reduce drawing time, we reuse most of the screen data between
 * video frames. New data are drawn only when the viewing window moves
 * within a logical space defined by the program. For example, if this
 * window shifts one pixel to the left, only the left border of the screen
 * is drawn. Other data are left untouched in most cases.
 */

/* configure VGA for mode X; initializes logical view to (0, 0) */
extern int set_mode_X(void);

/* return to text mode */
extern void clear_mode_X();

/* set logical view window coordinates */
extern void set_view_window(int scr_x, int scr_y);

/* show the logical view window on the monitor */
extern void show_screen();

/* clear the video memory in mode X */
extern void clear_screens();

/* draw a horizontal line at vertical pixel y within the logical view window */
extern int draw_horiz_line(int y);

/* draw a vertical line at horizontal pixel x within the logical view window */
extern int draw_vert_line(int x);

/* draw the status bar */
extern void draw_status_bar();

void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]);

void fill_vert_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]);

unsigned char font_data[256][16];
#endif /* _MODEX_H */
