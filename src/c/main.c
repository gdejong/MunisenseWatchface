#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_quiet_time_layer;
static GFont s_time_font;
static GFont s_small_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
  
  if(quiet_time_is_active()) {
    text_layer_set_text(s_quiet_time_layer, "Quiet Time");
  } else{
    text_layer_set_text(s_quiet_time_layer, "");
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Load the fonts.
  s_small_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  s_time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  
  // Create the time layer.
  s_time_layer = text_layer_create( GRect(0, PBL_IF_ROUND_ELSE(58, 58), bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorVividCerulean);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create the Munisense logo layer.
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MUNISENSE_LOGO);
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Create the baterry layer.
  s_battery_layer = text_layer_create(GRect(0, 0, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorVividCerulean);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, s_small_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100% charged");
  battery_state_service_subscribe(handle_battery);
  
  // Create the date layer
  s_date_layer = text_layer_create(GRect(0, 40, 144, 30));
  text_layer_set_text_color(s_date_layer, GColorVividCerulean);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_small_font);
  
  // Create the quiet time layer.
  s_quiet_time_layer = text_layer_create(GRect(0, 110, bounds.size.w, 34));
  text_layer_set_text_color(s_quiet_time_layer, GColorVividCerulean);
  text_layer_set_background_color(s_quiet_time_layer, GColorClear);
  text_layer_set_font(s_quiet_time_layer, s_small_font);
  text_layer_set_text_alignment(s_quiet_time_layer, GTextAlignmentCenter);
  
  // Add the layers to the window.
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_quiet_time_layer));
  
    // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the layers.
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_quiet_time_layer);
  // Unload the fonts. None at this time, system fonts do not need unloading!
  
  // Destroy the bitmaps.
  gbitmap_destroy(s_background_bitmap);
  // Destroy the bitmap layers.
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Set background color.
  window_set_background_color(s_main_window, GColorYellow);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}