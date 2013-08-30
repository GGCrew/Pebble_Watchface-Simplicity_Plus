#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define INVERTED false
#define SHOW_WEEK false
#define SHOW_YEAR true


#define MY_UUID {0x47, 0x47, 0x20, 0x43, 0x72, 0x65, 0x77, 0xA6, 0xAA, 0x30, 0xED, 0xBE, 0x01, 0xE3, 0x8A, 0x02}
PBL_APP_INFO(	MY_UUID, 
							"Simplicity Plus", 
							"GG Crew and Pebble Technology", 
							2, 0, /* App version */
							RESOURCE_ID_IMAGE_MENU_ICON, 
							APP_INFO_WATCH_FACE);

Window *window;

TextLayer *text_day_layer;
#if SHOW_WEEK
TextLayer *text_week_layer;
#endif
TextLayer *text_date_layer;
#if SHOW_YEAR
TextLayer *text_year_layer;
#endif
TextLayer *text_time_layer;

Layer *line_layer;

#if INVERTED
InverterLayer *inverter_layer;
#endif

GFont small_font;
GFont large_font;


/**/


/*
void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
*/

void line_layer_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_draw_line(ctx, GPoint(8, 97), GPoint(131, 97));
  graphics_draw_line(ctx, GPoint(8, 98), GPoint(131, 98));
}


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}


void update_display_date(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
#if SHOW_YEAR
	// short month
	static char date_text[] = "Xxx 00";
  static char year_text[] = "0000";
#else
	// full month
  static char date_text[] = "Xxxxxxxxx 00";
#endif

#if SHOW_WEEK
	static char day_text[] = "Xxx";
	static char week_text[] = "w00";
#else
	static char day_text[] = "Xxxxxxxxx";
#endif

#if SHOW_WEEK
	// shortened day_text
  strftime(day_text, sizeof(day_text), "%a", tick_time);
	
	// Too bad sprintf doesn't work with this version of the Pebble SDK...
	// sprintf(week_text, "w%d", week);
	strcpy(week_text, "");

	itoa(week, temp_text);
	strcat(week_text, "P");
	strcat(week_text, temp_text);

	strftime(week_text, sizeof(week_text), "%w", tick_time);
  text_layer_set_text(text_week_layer, week_text);
#else
	// full day_text
  strftime(day_text, sizeof(day_text), "%A", tick_time);
#endif
  text_layer_set_text(text_day_layer, day_text);

#if SHOW_YEAR
	// short month
  strftime(date_text, sizeof(date_text), "%b %e", tick_time);
	
  strftime(year_text, sizeof(year_text), "%Y", tick_time);
  text_layer_set_text(text_year_layer, year_text);
#else
	// full month
	strftime(date_text, sizeof(date_text), "%B %e", tick_time);
#endif
  text_layer_set_text(text_date_layer, date_text);
}


//void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
	if(units_changed & DAY_UNIT) {update_display_date(tick_time);}
}


void handle_init(void) {
//  window_init(&window, "SimplicityPlus");
	window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

//  resource_init_current_app(&APP_RESOURCES);

	small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21));
	large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49));

//  text_layer_init(text_day_layer, window.layer.frame);
	text_day_layer = text_layer_create(GRect(8, 44, 144-8, 168-44));
  text_layer_set_text_color(text_day_layer, GColorWhite);
  text_layer_set_background_color(text_day_layer, GColorClear);
//  layer_set_frame(text_day_layer.layer, GRect(8, 44, 144-8, 168-44));
  text_layer_set_font(text_day_layer, small_font);
  layer_add_child(window_layer, text_layer_get_layer(text_day_layer));


#if SHOW_WEEK
//  text_layer_init(text_week_layer, window.layer.frame);
	text_week_layer = text_layer_create(GRect(-12, 68, 144, 168-68));
  text_layer_set_text_color(text_week_layer, GColorWhite);
  text_layer_set_background_color(text_week_layer, GColorClear);
//  layer_set_frame(text_week_layer.layer, GRect(-12, 68, 144, 168-68));
  text_layer_set_font(text_week_layer, small_font);
	text_layer_set_text_alignment(text_week_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_week_layer));
#endif

//  text_layer_init(text_date_layer, window.layer.frame);
	text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
//  layer_set_frame(text_date_layer.layer, GRect(8, 68, 144-8, 168-68));
  text_layer_set_font(text_date_layer, small_font);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));


#if SHOW_YEAR
//  text_layer_init(text_year_layer, window.layer.frame);
	text_year_layer = text_layer_create(GRect(-12, 68, 144, 168-68));
  text_layer_set_text_color(text_year_layer, GColorWhite);
  text_layer_set_background_color(text_year_layer, GColorClear);
//  layer_set_frame(text_year_layer.layer, GRect(-12, 68, 144, 168-68));
  text_layer_set_font(text_year_layer, small_font);
	text_layer_set_text_alignment(text_year_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_year_layer));
#endif

//  text_layer_init(text_time_layer, window.layer.frame);
	text_time_layer = text_layer_create(GRect(7, 92, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
//  layer_set_frame(text_time_layer.layer, GRect(7, 92, 144-7, 168-92));
  text_layer_set_font(text_time_layer, large_font);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));


//  layer_init(&line_layer, window.layer.frame);
	line_layer = layer_create(layer_get_frame(window_layer));
//  line_layer.update_proc = &line_layer_update_callback;
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);


#if INVERTED
	// The inverter layer (probably) has to be the last layer added to the window
	inverter_layer_init(&inverter_layer, window.layer.frame);
	layer_add_child(window_layer, text_layer_get_layer(inverter_layer));
#endif

	// force the display_updates instead of waiting for second/minute ticks in the main loop
	time_t now;
	struct tm *tick_time;
//	get_time(tick_time);
  now = time(NULL);
  tick_time = localtime(&now);
	update_display_date(tick_time);
	update_display_time(tick_time); 

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


void handle_deinit(void) {
	fonts_unload_custom_font(small_font);
	fonts_unload_custom_font(large_font);
  tick_timer_service_unsubscribe();
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
