#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "libretro.h"

static char STAT[512];

static uint32_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

void retro_init(void)
{
   frame_buf = calloc(320 * 240, sizeof(uint32_t));
}

void retro_deinit(void)
{
   free(frame_buf);
   frame_buf = NULL;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "TestPointerCore";
   info->library_version  = "v1";
   info->need_fullpath    = false;
   info->valid_extensions = NULL; // Anything is fine, we don't care.
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   float aspect = 4.0f / 3.0f;
   float sampling_rate = 30000.0f;

   info->timing = (struct retro_system_timing) {
      .fps = 60.0,
      .sample_rate = sampling_rate,
   };

   info->geometry = (struct retro_game_geometry) {
      .base_width   = 320,
      .base_height  = 240,
      .max_width    = 320,
      .max_height   = 240,
      .aspect_ratio = aspect,
   };
}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   bool no_content = true;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void)
{
   x_coord = 0;
   y_coord = 0;
}

static int px=160,py=120;

static void update_input(void)
{
   input_poll_cb();
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
   {
      /* stub */
   }

  if (input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_a))
      printf("A key is pressed!\n");
  if (input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_ESCAPE))
      printf("ESC key is pressed!\n");

	int p_x = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
	int p_y = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);
	int p_press = input_state_cb(0, RETRO_DEVICE_POINTER, 0,RETRO_DEVICE_ID_POINTER_PRESSED);

	static int ptrhold=0;

	if(p_press)ptrhold++;
	else ptrhold=0;

        px=(int)((p_x+0x7fff)*320.0/0xffff);
        py=(int)((p_y+0x7fff)*240.0/0xffff);
	//printf("px:%d py:%d (%d,%d) %d\n",p_x,p_y,px,py,p_press);
	sprintf(STAT,"(%d,%d)(%d,%d)%d %d\0",p_x,p_y,px,py,p_press,ptrhold);

}

static const char *cross[] = {
  "X                               ",
  "XX                              ",
  "X.X                             ",
  "X..X                            ",
  "X...X                           ",
  "X....X                          ",
  "X.....X                         ",
  "X......X                        ",
  "X.......X                       ",
  "X........X                      ",
  "X.....XXXXX                     ",
  "X..X..X                         ",
  "X.X X..X                        ",
  "XX  X..X                        ",
  "X    X..X                       ",
  "     X..X                       ",
  "      X..X                      ",
  "      X..X                      ",
  "       XX                       ",
  "                                ",
};

#ifdef M16B
void DrawPointBmp(unsigned short int *buffer,int x, int y, unsigned short int color,int rwidth,int rheight)
#else
void DrawPointBmp(unsigned int *buffer,int x, int y, unsigned int color,int rwidth,int rheight)
#endif

{
   int idx;

   idx=x+y*rwidth;
   if(idx>=0 && idx<rwidth*rheight)
   	buffer[idx]=color;	
}
#ifdef M16B
void draw_cross(unsigned short int *surface,int x,int y)
#else
void draw_cross(unsigned int *surface,int x,int y)
#endif
{

	int i,j,idx;
	int dx=32,dy=20;
#ifdef M16B
	unsigned short int col=0xffff;
#else
	unsigned int col=0xffffffff;
#endif
	int w=320;
	int h=240;

	for(j=y;j<y+dy;j++){
		idx=0;
		for(i=x;i<x+dx;i++){

			if(cross[j-y][idx]=='.')DrawPointBmp(surface,i,j,col,w,h);
			else if(cross[j-y][idx]=='X')DrawPointBmp(surface,i,j,0,w,h);
			idx++;			
		}
	}
}

#include "font2.i"

#ifdef M16B
void Retro_Draw_string(unsigned short *surface, signed short int x, signed short int y, const  char *string,unsigned short maxstrlen,unsigned short xscale, unsigned short yscale, unsigned short fg, unsigned short bg)
#else
void Retro_Draw_string(unsigned *surface, signed short int x, signed short int y, const  char *string,unsigned short maxstrlen,unsigned short xscale, unsigned short yscale, unsigned  fg, unsigned  bg)
#endif
{
    	int k,strlen;
    	unsigned char *linesurf;

    	int col, bit;
    	unsigned char b;

    	int xrepeat, yrepeat;
#ifdef M16B
    	signed short int ypixel;
   	unsigned short *yptr; 

	unsigned short*mbuffer=(unsigned short*)surface;
#else
    	signed  int ypixel;
   	unsigned  *yptr; 

	unsigned *mbuffer=(unsigned*)surface;
#endif


	#define VIRTUAL_WIDTH 320

	#define charWidthLocal 8
	#define charHeightLocal 8

	signed short int  left, right, top, bottom;
	signed short int  x1, y1, x2, y2;

	left = 0;
	x2 = x + charWidthLocal;
	if (x2<left) {
		return;
	} 
	right = 320 - 1;
	x1 = x;
	if (x1>right) {
		return;
	} 
	top = 0;
	y2 = y + charHeightLocal;
	if (y2<top) {
		return;
	} 
	bottom = 240 - 1;
	y1 = y;
	if (y1>bottom) {
		return;
	} 


    	if(string==NULL)return;
    	for(strlen = 0; strlen<maxstrlen && string[strlen]; strlen++) {}


	int surfw=strlen * 7 * xscale;
	int surfh=8 * yscale;

#ifdef M16B	
        linesurf =(unsigned char *)malloc(sizeof(unsigned short)*surfw*surfh );
    	yptr = (unsigned short *)&linesurf[0];

#else
        linesurf =(unsigned char *)malloc(sizeof(unsigned )*surfw*surfh );
    	yptr = (unsigned *)&linesurf[0];

#endif

	for(ypixel = 0; ypixel<8; ypixel++) {

        	for(col=0; col<strlen; col++) {

            		b = font_array[(string[col]^0x80)*8 + ypixel];

            		for(bit=0; bit<7; bit++, yptr++) {              
				*yptr = (b & (1<<(7-bit))) ? fg : bg;
                		for(xrepeat = 1; xrepeat < xscale; xrepeat++, yptr++)
                    			yptr[1] = *yptr;
                        }
        	}

        	for(yrepeat = 1; yrepeat < yscale; yrepeat++) 
            		for(xrepeat = 0; xrepeat<surfw; xrepeat++, yptr++)
                		*yptr = yptr[-surfw];
           
    	}

#ifdef M16B
    	yptr = (unsigned short*)&linesurf[0];
#else
    	yptr = (unsigned *)&linesurf[0];
#endif

    	for(yrepeat = y; yrepeat < y+ surfh; yrepeat++) 
        	for(xrepeat = x; xrepeat< x+surfw; xrepeat++,yptr++)
             		if(*yptr!=0 && (xrepeat+yrepeat*VIRTUAL_WIDTH) < 320*240 )mbuffer[xrepeat+yrepeat*VIRTUAL_WIDTH] = *yptr;

	free(linesurf);

}

#ifdef M16B
void Retro_Draw_char(unsigned short *surface, signed short int x, signed short int y,  char string,unsigned short xscale, unsigned short yscale, unsigned short fg, unsigned short bg)
#else
void Retro_Draw_char(unsigned *surface, signed short int x, signed short int y,  char string,unsigned short xscale, unsigned short yscale, unsigned  fg, unsigned  bg)
#endif
{
    	int k,strlen;
    	unsigned char *linesurf;
    	int col, bit;
    	unsigned char b;

    	int xrepeat, yrepeat;

#ifdef M16B
    	signed short int ypixel;
   	unsigned short *yptr; 

	unsigned short*mbuffer=(unsigned short*)surface;
#else
    	signed  int ypixel;
   	unsigned  *yptr; 

	unsigned *mbuffer=(unsigned*)surface;
#endif

	#define VIRTUAL_WIDTH 320

	#define charWidthLocal2 7*xscale
	#define charHeightLocal2 8*yscale

	signed short int  left, right, top, bottom;
	signed short int  x1, y1, x2, y2;

	left = 0;
	x2 = x + charWidthLocal2;
	if (x2<left) {
		return;
	} 
	right = 320 - 1;
	x1 = x;
	if (x1>right) {
		return;
	} 
	top = 0;
	y2 = y + charHeightLocal2;
	if (y2<top) {
		return;
	} 
	bottom = 240 - 1;
	y1 = y;
	if (y1>bottom) {
		return;
	} 

        strlen = 1;

	int surfw=strlen * 7 * xscale;
	int surfh=8 * yscale;

#ifdef M16B	
        linesurf =(unsigned char *)malloc(sizeof(unsigned short)*surfw*surfh );
    	yptr = (unsigned short *)&linesurf[0];

#else
        linesurf =(unsigned char *)malloc(sizeof(unsigned )*surfw*surfh );
    	yptr = (unsigned *)&linesurf[0];

#endif

	for(ypixel = 0; ypixel<8; ypixel++) {

            		b = font_array[(string^0x80)*8 + ypixel];

            		for(bit=0; bit<7; bit++, yptr++) {              
				*yptr = (b & (1<<(7-bit))) ? fg : bg;
                		for(xrepeat = 1; xrepeat < xscale; xrepeat++, yptr++)
                    			yptr[1] = *yptr;
                        }

        	for(yrepeat = 1; yrepeat < yscale; yrepeat++) 
            		for(xrepeat = 0; xrepeat<surfw; xrepeat++, yptr++)
                		*yptr = yptr[-surfw];
           
    	}


#ifdef M16B
    	yptr = (unsigned short*)&linesurf[0];
#else
    	yptr = (unsigned *)&linesurf[0];
#endif

    	for(yrepeat = y; yrepeat < y+ surfh; yrepeat++) 
        	for(xrepeat = x; xrepeat< x+surfw; xrepeat++,yptr++)
             		if(*yptr!=0 && (xrepeat+yrepeat*VIRTUAL_WIDTH) < 320*240 )mbuffer[xrepeat+yrepeat*VIRTUAL_WIDTH] = *yptr;

	free(linesurf);

}


static void render_screen(void)
{
   uint32_t *buf    = frame_buf;
   unsigned stride  = 320;

   uint32_t color_r = 0xff << 16;
   uint32_t color_g = 0xff <<  8;
   uint32_t *line   = buf;

   memset(frame_buf,0,320*240*4);

   draw_cross(buf,px,py);

   Retro_Draw_string(buf,10,10,STAT,40,1,1,0xFFFFFFFF,0x0);

   video_cb(buf, 320, 240, stride << 2);
}

static void check_variables(void)
{
}

static void audio_callback(void)
{
   int i;
   for(i=0;i<500;i++)audio_cb(0, 0);
}

void retro_run(void)
{
   update_input();
   render_screen();
   audio_callback();

   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();
}

bool retro_load_game(const struct retro_game_info *info)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
      return false;
   }

   check_variables();

   (void)info;
   return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   if (type != 0x200)
      return false;
   if (num != 2)
      return false;
   return retro_load_game(NULL);
}

size_t retro_serialize_size(void)
{
   return 2;
}

bool retro_serialize(void *data_, size_t size)
{
   if (size < 2)
      return false;

   uint8_t *data = data_;
   data[0] = x_coord;
   data[1] = y_coord;
   return true;
}

bool retro_unserialize(const void *data_, size_t size)
{
   if (size < 2)
      return false;

   const uint8_t *data = data_;
   x_coord = data[0] & 31;
   y_coord = data[1] & 31;
   return true;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

