#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>


#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"



#define PI 3.1415926535897932384626433832795

#define RAZER_VENDOR_ID 0x1532
#define RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID 0x203


#define RAZER_ROW_LENGTH 22
#define RAZER_ROWS_NUM 6

const char *sys_hid_devices_path = "/sys/bus/hid/devices/";

struct razer_pos
{
	int x,y;
};

struct razer_rgb 
{
	unsigned char r,g,b;
};

struct razer_hsl
{
	float h,s,l;
};

struct razer_rgb_row
{
	unsigned char row_index;
	struct razer_rgb column[RAZER_ROW_LENGTH];
};

struct razer_rgb_frame
{
	struct razer_rgb_row rows[RAZER_ROWS_NUM];
	int update_mask;
};

struct razer_keys
{
	struct razer_rgb_row rows[RAZER_ROWS_NUM];
	int update_mask;
	long heatmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];
	int lockmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];//sets to effect id if locked by effect
};

struct razer_keys_locks
{
	int lockmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];
};

struct razer_keys_set
{
	int num;
	unsigned char *keys;/*buffer to keycodes?ascii? */
};

void release_locks(struct razer_keys_locks *locks);
float hue2rgb(float p,float q,float t);
void hsl2rgb(struct razer_hsl *hsl,struct razer_rgb *rgb);
void rgb_from_hue(float percentage,float start_hue,float end_hue,struct razer_rgb *color);
unsigned char rgb_clamp(int v);
void rgb_add(struct razer_rgb *dst,struct razer_rgb *src);
void rgb_mix(struct razer_rgb *dst,struct razer_rgb *src,float factor);



int razer_chroma_open();
int razer_chroma_close();


char *key_filename = NULL;
char *custom_filename = NULL;
char *get_device_path();

void convert_keycode_to_pos(int keycode,struct razer_pos *pos);
void convert_pos_to_keycode(struct razer_pos *pos,int *keycode);
void convert_ascii_to_pos(unsigned char letter,struct razer_pos *pos);
void set_mode_custom(void);
void update_keys(struct razer_keys *keys);
void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void add_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void sub_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void add_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void sub_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color);
void set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);
void clear_all(struct razer_keys *keys);
void sub_heatmap(struct razer_keys *keys,int heatmap_reduction_amount);
void draw_circle(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color);
void draw_ring(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color);


SDL_Texture *sdl_icons[32];


void update_sdl(struct razer_keys *keys,SDL_Renderer *sdl,SDL_Window *window,SDL_Texture *tex);
void load_icons(SDL_Renderer *renderer,char *path,SDL_Texture **icons);


//list of last keystrokes
//time since hit /hitstamps

double deg2rad(double degree);
double rad2deg(double rad);
double pos_angle_radians(struct razer_pos *src,struct razer_pos *dst);


void capture_keys(struct razer_keys *keys,SDL_Renderer *renderer,SDL_Window *window,SDL_Texture *tex);


