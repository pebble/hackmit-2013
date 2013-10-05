#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x97, 0x6D, 0xB7, 0x41, 0xF4, 0xD9, 0x42, 0x28, 0x95, 0xF1, 0xC1, 0x7B, 0xB2, 0x06, 0x9D, 0xDD }
PBL_APP_INFO(MY_UUID,
             "ISS Tracker", "Pebble Examples",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer time_text_layer;
static char time_text[10];

TextLayer nextpass_text_layer;
static char nextpass_text[10];
time_t nextpass_time = 1381002096;
BmpContainer background_image;

// Called every second to update the text fields
void handle_tick(AppContextRef app_ctx, PebbleTickEvent *t) {
  string_format_time(time_text, sizeof(time_text), "%H:%M:%S", t->tick_time);
  text_layer_set_text(&time_text_layer, time_text);

  int nextpass = nextpass_time - time(NULL);
  /* next pass is defined and is in the future */
  if (nextpass > 0) {
    int nextpass_hours = nextpass / 3600;
    nextpass %= 3600;
    int nextpass_minutes = nextpass / 60;
    nextpass %= 60;

    snprintf(nextpass_text, sizeof(nextpass_text), "%i:%02i:%02i", nextpass_hours, nextpass_minutes, nextpass);
  }
  /* next pass is in the past - or undefined */
  else {
    snprintf(nextpass_text, sizeof(nextpass_text), "--:--:--");
  }

  text_layer_set_text(&nextpass_text_layer, nextpass_text);
}

void handle_init(AppContextRef ctx) {
  // Create the main window and push it in fullscreen mode
  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  window_set_fullscreen(&window, true);

  // Initialize ressources and load the background image
  resource_init_current_app(&APP_RESOURCES);
  bmp_init_container(RESOURCE_ID_BACKGROUND_IMAGE, &background_image);
  layer_add_child(&window.layer, &background_image.layer.layer);

  // Create a text field to display the time until the next pass
  text_layer_init(&nextpass_text_layer, GRect(0, 126, 144, 34));
  text_layer_set_font(&nextpass_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(&nextpass_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(&nextpass_text_layer, GColorClear);
  layer_add_child(window_get_root_layer(&window), (Layer*)&time_text_layer);

  // Create a text field to display the current time
  text_layer_init(&time_text_layer, GRect(37, 0, 70, 24));
  text_layer_set_font(&time_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(&time_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(&time_text_layer, GColorClear);
  layer_add_child(window_get_root_layer(&window), (Layer*)&nextpass_text_layer);
}

void handle_deinit(AppContextRef ctx) {
  // Frees the memory of the bitmap. This is very important!
  bmp_deinit_container(&background_image);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };

  app_event_loop(params, &handlers);
}
