#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define INVERTED false
#define SHOW_WEEK false
#define SHOW_YEAR true


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

BitmapLayer *bluetooth_icon_bitmap_layer;
BitmapLayer *battery_icon_bitmap_layer;

GFont small_font;
GFont large_font;

GBitmap *bluetooth_error_image;
GBitmap *battery_full_image;
GBitmap *battery_low_image;
GBitmap *battery_empty_image;
GBitmap *battery_charging_low_image;
GBitmap *battery_charging_half_image;
GBitmap *battery_charging_full_image;


/**/


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


void bluetooth_connection_callback(bool connected) {
	//APP_LOG(APP_LOG_LEVEL_INFO, "bluetooth connected=%d", (int) connected);
	if(connected) {
		// hide missing bluetoot icon
		layer_set_hidden(bitmap_layer_get_layer(bluetooth_icon_bitmap_layer), true);
	} else {
		// show missing bluetooth icon
		layer_set_hidden(bitmap_layer_get_layer(bluetooth_icon_bitmap_layer), false);
		//vibes_double_pulse();
		VibePattern vibration_pattern;
		vibration_pattern.durations = (uint32_t []) {100, 100, 100, 100, 100};
		vibration_pattern.num_segments = 5;
		vibes_enqueue_custom_pattern(vibration_pattern);
	}
}


void battery_state_callback(BatteryChargeState battery_charge_state) {
	if(battery_charge_state.is_charging) {
		if(battery_charge_state.charge_percent <= 20) {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_charging_low_image);}
		else if(battery_charge_state.charge_percent <= 80) {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_charging_half_image);}
		else {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_charging_full_image);}
		layer_set_hidden(bitmap_layer_get_layer(battery_icon_bitmap_layer), false);
	} else {
		if(battery_charge_state.charge_percent <= 5) {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_empty_image);}
		else if(battery_charge_state.charge_percent <= 10) {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_low_image);}
		else {bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_charging_full_image);}
		
		// Hide the battery icon if charge is over 20% (and we're not charging the battery)
		layer_set_hidden(bitmap_layer_get_layer(battery_icon_bitmap_layer), (battery_charge_state.charge_percent > 20));
	}
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
	if(units_changed & DAY_UNIT) {update_display_date(tick_time);}
}


void handle_init(void) {
	window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

	small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21));
	large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49));

	bluetooth_error_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_ERROR_ICON);
	battery_full_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_FULL_ICON);
	battery_low_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_LOW_ICON);
	battery_empty_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_EMPTY_ICON);
	battery_charging_low_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING_LOW_ICON);
	battery_charging_half_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING_HALF_ICON);
	battery_charging_full_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGING_FULL_ICON);
	
	text_day_layer = text_layer_create(GRect(8, 44, 144-8, 168-44));
  text_layer_set_text_color(text_day_layer, GColorWhite);
  text_layer_set_background_color(text_day_layer, GColorClear);
  text_layer_set_font(text_day_layer, small_font);
  layer_add_child(window_layer, text_layer_get_layer(text_day_layer));


#if SHOW_WEEK
	text_week_layer = text_layer_create(GRect(-12, 68, 144, 168-68));
  text_layer_set_text_color(text_week_layer, GColorWhite);
  text_layer_set_background_color(text_week_layer, GColorClear);
  text_layer_set_font(text_week_layer, small_font);
	text_layer_set_text_alignment(text_week_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_week_layer));
#endif

	text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, small_font);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));


#if SHOW_YEAR
	text_year_layer = text_layer_create(GRect(-12, 68, 144, 168-68));
  text_layer_set_text_color(text_year_layer, GColorWhite);
  text_layer_set_background_color(text_year_layer, GColorClear);
  text_layer_set_font(text_year_layer, small_font);
	text_layer_set_text_alignment(text_year_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_year_layer));
#endif

	text_time_layer = text_layer_create(GRect(7, 92, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, large_font);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));


	line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);
	
	bluetooth_icon_bitmap_layer = bitmap_layer_create(GRect(((144 - 3) - 16) - ((16 + 2) * 1), 0, 16, 16));
  bitmap_layer_set_background_color(bluetooth_icon_bitmap_layer, GColorClear);
	bitmap_layer_set_bitmap(bluetooth_icon_bitmap_layer, bluetooth_error_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_icon_bitmap_layer));
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_icon_bitmap_layer), true);
	
	battery_icon_bitmap_layer = bitmap_layer_create(GRect(((144 - 3) - 16) - ((16 + 2) * 0), 0, 16, 16));
  bitmap_layer_set_background_color(battery_icon_bitmap_layer, GColorClear);
	bitmap_layer_set_bitmap(battery_icon_bitmap_layer, battery_full_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_icon_bitmap_layer));
	layer_set_hidden(bitmap_layer_get_layer(battery_icon_bitmap_layer), true);
	


#if INVERTED
	// The inverter layer (probably) has to be the last layer added to the window
	inverter_layer_init(&inverter_layer, window.layer.frame);
	layer_add_child(window_layer, text_layer_get_layer(inverter_layer));
#endif

	// force the display_updates instead of waiting for second/minute ticks in the main loop
	time_t now;
	struct tm *tick_time;
  now = time(NULL);
  tick_time = localtime(&now);
	update_display_date(tick_time);
	update_display_time(tick_time); 
	
	// Checking current bluetooth status
	bluetooth_connection_callback(bluetooth_connection_service_peek());
	
	// TODO: check current battery state
	battery_state_callback(battery_state_service_peek());
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	bluetooth_connection_service_subscribe(bluetooth_connection_callback);
	battery_state_service_subscribe(battery_state_callback);
}


void handle_deinit(void) {
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	tick_timer_service_unsubscribe();
	fonts_unload_custom_font(small_font);
	fonts_unload_custom_font(large_font);
	gbitmap_destroy(bluetooth_error_image);
	gbitmap_destroy(battery_full_image);
	gbitmap_destroy(battery_low_image);
	gbitmap_destroy(battery_empty_image);
	gbitmap_destroy(battery_charging_low_image);
	gbitmap_destroy(battery_charging_half_image);
	gbitmap_destroy(battery_charging_full_image);
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
