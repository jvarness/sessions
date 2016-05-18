#include <pebble.h>

#define KEY_ACCENT_1 0
#define KEY_ACCENT_2 1
#define KEY_WEATHER_TEMP 2
#define KEY_DISPLAY_FAHRENHEIT 3
#define KEY_WEATHER_TEMP_C 4

static Window *window;
static TextLayer *date_text;
static TextLayer *clock_text;
static TextLayer *steps_text;
static TextLayer *temp_text;
static GFont lobster_font_30;
static GFont lobster_font_18;
static GColor accent_1;
static GColor accent_2;

static void update_time() {
  time_t now = time(NULL);
  struct tm *local = localtime(&now);
  
  static char time_buffer[10];
  static char date_buffer[10];
  
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%k:%M" : "%l:%M %P", local);
  strftime(date_buffer, sizeof(date_buffer), "%b %e", local);
  
  text_layer_set_text(clock_text, time_buffer);
  text_layer_set_text(date_text, date_buffer);
}

static void update_health() {
  HealthValue step_metric = health_service_sum_today(HealthMetricStepCount);
  int step_count = step_metric;
    
  static char buffer[14];
  snprintf(buffer, sizeof(buffer), "Steps: %d", step_count);
  
  text_layer_set_text(steps_text, buffer);
}

static void update_weather() {
  static char buffer[8];
  
  bool display_fah = persist_read_bool(KEY_DISPLAY_FAHRENHEIT);
  
  if(display_fah) {
    persist_read_string(KEY_WEATHER_TEMP, buffer, sizeof(buffer));
  }
  else {
    persist_read_string(KEY_WEATHER_TEMP_C, buffer, sizeof(buffer));
  }
  
  text_layer_set_text(temp_text, buffer);
}

static void trigger_app_message() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void sessions_inbox_handler(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "In handler");
  Tuple *accent_1_t = dict_find(iterator, KEY_ACCENT_1);
  Tuple *accent_2_t = dict_find(iterator, KEY_ACCENT_2);
  Tuple *weather = dict_find(iterator, KEY_WEATHER_TEMP);
  Tuple *weather_display = dict_find(iterator, KEY_DISPLAY_FAHRENHEIT);
  Tuple *weather_c = dict_find(iterator, KEY_WEATHER_TEMP_C);
  
  if (accent_1_t) {
    int color = accent_1_t->value->int32;
    
    persist_write_int(KEY_ACCENT_1, color);
    accent_1 = GColorFromHEX(color);
    
    text_layer_set_text_color(clock_text, accent_1);
    text_layer_set_text_color(temp_text, accent_1);
    
    #if defined(PBL_RECT)
    text_layer_set_background_color(date_text, accent_1);
    text_layer_set_background_color(steps_text, accent_1);
    #elif defined(PBL_ROUND)
    window_set_background_color(window, accent_1);
    #endif
  }
  
  if (accent_2_t) {
    int color = accent_2_t->value->int32;
    
    persist_write_int(KEY_ACCENT_2, color);
    accent_2 = GColorFromHEX(color);
    
    text_layer_set_text_color(date_text, accent_2);
    text_layer_set_text_color(steps_text, accent_2);
    
    #if defined(PBL_RECT)
    window_set_background_color(window, accent_2);
    #elif defined(PBL_ROUND)
    text_layer_set_background_color(clock_text, accent_2);
    text_layer_set_background_color(temp_text, accent_2);
    #endif
  }
  
  if(weather_display) {
    persist_write_bool(KEY_DISPLAY_FAHRENHEIT, weather_display->value->uint8 == 1);
    update_weather();
  }
  
  if(weather || weather_c) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Weather temp: %s", weather->value->cstring);
    persist_write_string(KEY_WEATHER_TEMP, weather->value->cstring);
    persist_write_string(KEY_WEATHER_TEMP_C, weather_c->value->cstring);
    update_weather();
  }
}

static void load_window(Window *win) {
  Layer *win_layer = window_get_root_layer(window);
  GRect dimensions = layer_get_bounds(win_layer);
  
  clock_text = text_layer_create(GRect(0, dimensions.size.h / 2 - PBL_IF_ROUND_ELSE(35, 18), dimensions.size.w, 45));
  text_layer_set_text_alignment(clock_text, GTextAlignmentCenter);
  text_layer_set_background_color(clock_text, PBL_IF_ROUND_ELSE(accent_2, GColorClear));
  text_layer_set_text_color(clock_text, accent_1);
  text_layer_set_font(clock_text, lobster_font_30);
  
  date_text = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(20, 0), dimensions.size.w, 25));
  text_layer_set_background_color(date_text, PBL_IF_ROUND_ELSE(GColorClear, accent_1));
  text_layer_set_text_color(date_text, accent_2);
  text_layer_set_font(date_text, lobster_font_18);
  text_layer_set_text_alignment(date_text, GTextAlignmentCenter);
  
  steps_text = text_layer_create(GRect(0, dimensions.size.h - PBL_IF_ROUND_ELSE(45, 25), dimensions.size.w, 25));
  text_layer_set_background_color(steps_text, PBL_IF_ROUND_ELSE(GColorClear, accent_1));
  text_layer_set_text_color(steps_text, accent_2);
  text_layer_set_font(steps_text, lobster_font_18);
  text_layer_set_text_alignment(steps_text, GTextAlignmentCenter);
  
  temp_text = text_layer_create(GRect(0, dimensions.size.h / 2 + PBL_IF_ROUND_ELSE(10, 25), dimensions.size.w, 25));
  text_layer_set_text_alignment(temp_text, GTextAlignmentCenter);
  text_layer_set_background_color(temp_text, PBL_IF_ROUND_ELSE(accent_2, GColorClear));
  text_layer_set_text_color(temp_text, accent_1);
  text_layer_set_font(temp_text, lobster_font_18);
  
  layer_add_child(win_layer, text_layer_get_layer(clock_text));
  layer_add_child(win_layer, text_layer_get_layer(steps_text));
  layer_add_child(win_layer, text_layer_get_layer(date_text));
  layer_add_child(win_layer, text_layer_get_layer(temp_text));
  
  update_time();
  update_health();
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  update_time();
  
  if(changed & HOUR_UNIT) {
    trigger_app_message();
  }
}

static void health_trigger(HealthEventType eventType, void * context) {
  if(eventType == HealthEventMovementUpdate || eventType == HealthEventMovementUpdate) {
    update_health();
  }
}

static void unload_window(Window *win) {
  text_layer_destroy(clock_text);
  text_layer_destroy(steps_text);
  text_layer_destroy(date_text);
  text_layer_destroy(temp_text);
  fonts_unload_custom_font(lobster_font_30);
  fonts_unload_custom_font(lobster_font_18);
}

static void check_defaults() {
  if(!persist_exists(KEY_ACCENT_1)) {
    persist_write_int(KEY_ACCENT_1, 0x00FFFF);
  }
  if(!persist_exists(KEY_ACCENT_2)) {
    persist_write_int(KEY_ACCENT_2, 0x000000);
  }
  if(!persist_exists(KEY_WEATHER_TEMP)) {
    persist_write_string(KEY_WEATHER_TEMP, "--\u00B0F");
  }
  if(!persist_exists(KEY_DISPLAY_FAHRENHEIT)) {
    persist_write_bool(KEY_DISPLAY_FAHRENHEIT, true);
  }
  if(!persist_exists(KEY_WEATHER_TEMP_C)) {
    persist_write_string(KEY_WEATHER_TEMP_C, "--\u00B0C");
  }
}

static void sessions_init() {
  check_defaults();
  
  accent_1 = GColorFromHEX(persist_read_int(KEY_ACCENT_1));
  accent_2 = GColorFromHEX(persist_read_int(KEY_ACCENT_2));
  
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = load_window,
    .unload = unload_window
  });
  
  lobster_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOBSTER_30));
  lobster_font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOBSTER_18));
  
  app_message_register_inbox_received(sessions_inbox_handler);
  app_message_open(128, 128);
  
  tick_timer_service_subscribe(MINUTE_UNIT | HOUR_UNIT, tick_handler);
  health_service_events_subscribe(health_trigger, NULL);
  
  window_set_background_color(window, PBL_IF_ROUND_ELSE(accent_1, accent_2));
  
  window_stack_push(window, false);
}

static void sessions_deinit() {
  tick_timer_service_unsubscribe();
  health_service_events_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(window);
}

int main(void) {
  sessions_init();
  app_event_loop();
  sessions_deinit();
}