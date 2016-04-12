#include <pebble.h>

static Window *window;
static TextLayer *date_text;
static TextLayer *clock_text;
static TextLayer *steps_text;
static GFont lobster_font_30;
static GFont lobster_font_18;

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

static void load_window(Window *win) {
  Layer *win_layer = window_get_root_layer(window);
  GRect dimensions = layer_get_bounds(win_layer);
  
  clock_text = text_layer_create(GRect(0, dimensions.size.h / 2 - 18, dimensions.size.w, 45));
  text_layer_set_text_alignment(clock_text, GTextAlignmentCenter);
  text_layer_set_background_color(clock_text, GColorClear);
  text_layer_set_text_color(clock_text, GColorCyan);
  text_layer_set_font(clock_text, lobster_font_30);
  
  date_text = text_layer_create(GRect(0, 0, dimensions.size.w, 25));
  text_layer_set_background_color(date_text, GColorCyan);
  text_layer_set_text_color(date_text, GColorBlack);
  text_layer_set_font(date_text, lobster_font_18);
  text_layer_set_text_alignment(date_text, GTextAlignmentCenter);
  
  steps_text = text_layer_create(GRect(0, dimensions.size.h - 25, dimensions.size.w, 25));
  text_layer_set_background_color(steps_text, GColorCyan);
  text_layer_set_text_color(steps_text, GColorBlack);
  text_layer_set_font(steps_text, lobster_font_18);
  text_layer_set_text_alignment(steps_text, GTextAlignmentCenter);
  
  layer_add_child(win_layer, text_layer_get_layer(clock_text));
  layer_add_child(win_layer, text_layer_get_layer(steps_text));
  layer_add_child(win_layer, text_layer_get_layer(date_text));
  
  update_time();
  update_health();
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  update_time();
}

static void health_trigger(HealthEventType eventType, void * context) {
  if(eventType == HealthEventMovementUpdate || eventType == HealthEventMovementUpdate) {
    update_health();
  }
}

static void unload_window(Window *win) {
  text_layer_destroy(clock_text);
  text_layer_destroy(steps_text);
  fonts_unload_custom_font(lobster_font_30);
  fonts_unload_custom_font(lobster_font_18);
}

static void sessions_init() {
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = load_window,
    .unload = unload_window
  });
  
  lobster_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOBSTER_30));
  lobster_font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOBSTER_18));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  health_service_events_subscribe(health_trigger, NULL);
  
  window_set_background_color(window, GColorBlack);
  
  window_stack_push(window, false);
}

static void sessions_deinit() {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  health_service_events_unsubscribe();
}

int main(void) {
  sessions_init();
  app_event_loop();
  sessions_deinit();
}