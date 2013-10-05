#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"

PBL_APP_INFO(HTTP_UUID,
             "ISS Tracker", "Pebble Examples",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

#define KEY_NEXTPASS 0
#define ISS_COOKIE 155

Window window;
TextLayer time_text_layer;
static char time_text[10];

TextLayer nextpass_text_layer;
static char nextpass_text[10];
time_t nextpass_time = 0;
BmpContainer background_image;

int error = 0;

// Starts an HTTP Request to get the time until the next pass of the ISS
void start_http_request() {
  DictionaryIterator *out;
  HTTPResult result = http_out_get("http://io.sarfata.org/iss/", ISS_COOKIE, &out);
  if (result != HTTP_OK) {
    error = result;
    return;
  }
  result = http_out_send();
  if (result != HTTP_OK) {
    error = result;
    return;
  }
}

// Called when the http request is successful. Updates the nextpass_time.
void handle_http_success(int32_t request_id, int http_status, DictionaryIterator* sent, void* context) {
  Tuple *nextpass_tuple = dict_find(sent, KEY_NEXTPASS);
  if (nextpass_tuple) {
    nextpass_time = nextpass_tuple->value->uint32;
  }
  error = 0;
}

// Called when the http request fails. Updates the error variable.
void handle_http_failure(int32_t request_id, int http_status, void* context) {
  error = http_status;
}

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


  // Add background and both text fields to the window

  // Start an HTTP Request
  start_http_request();
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
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 124,
      }
    }
  };
  HTTPCallbacks http_callbacks = {
    .failure = handle_http_failure,
    .success = handle_http_success
  };
  http_register_callbacks(http_callbacks, NULL);

  app_event_loop(params, &handlers);
}
