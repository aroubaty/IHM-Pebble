#include <pebble.h>

static Window *main_window;
TextLayer *output_layer;

// Android Communication
#define REQUEST_LOCATION                0
#define REQUEST_FIX_LOCATION            1
#define REQUEST_START_THREADED_LOCATION 2
#define REQUEST_STOP_THREADED_LOCATION  3
#define REQUEST_ELEVATION               4
#define REQUEST_WEATHER_STATUS          5
#define REQUEST_WEATHER_TEMPERATURE     6
#define REQUEST_WEATHER_PRESSURE        7
#define REQUEST_WEATHER_HUMIDITY        8
#define REQUEST_WEATHER_WIND            9
#define REQUEST_WEATHER_SUNRISE        10
#define REQUEST_WEATHER_SUNSET         11
#define REQUEST_TRANSPORT              12
#define SHOW_UP_TIME                   13
#define SHOW_ACTIVE_TIME               14
#define SHOW_BATTERY_STATE             15

#define NUMBER_OF_ITEMS                16


// Pebble KEY
#define PEBBLE_KEY_VALUE        1
// Location API
#define KEY_LATITUDE        100
#define KEY_LONGITUDE       101
#define KEY_DISTANCE        102
#define KEY_DIRECTION       103
// Elevation API
#define KEY_ALTITUDE        200
// Weather API
#define KEY_STATUS          300
#define KEY_DESCRIPTION     301
#define KEY_TEMPERATURE     302
#define KEY_PRESSURE        303
#define KEY_HUMIDITY        304
#define KEY_WIND_SPEED      305
#define KEY_WIND_DIRECTION  306
#define KEY_SUNRISE         307
#define KEY_SUNSET          308
// Transport API
#define KEY_DEPARTURE       400
#define KEY_DEPARTURE_TIME  401
#define KEY_ARRIVAL         402
#define KEY_ARRIVAL_TIME    403


#define MAX_TEXT_SIZE       128
#define NUM_ACCEL_SAMPLES   10
#define GRAVITY             10000 // (1g)² = 10000
#define ACCEL_THRESHOLD     8000  // (1g)² = 10000

//SLOT KEY
#define SLOT1 50
#define SLOT2 51
#define SLOT3 52
#define SLOT4 53
#define SLOT5 54
#define SLOT6 55
#define SLOT7 56
#define SLOT8 57

#define NUMBER_OF_PAGE 4

//MENU 
#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SECOND_MENU_ITEMS 2

static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_first_menu_items[NUM_FIRST_MENU_ITEMS];
static SimpleMenuItem s_second_menu_items[NUM_SECOND_MENU_ITEMS];


//Other
int counter = -1;
char text[MAX_TEXT_SIZE];
unsigned long int up_time = 0;      //in seconds
unsigned long int active_time = 0;  //in seconds/10

char textLayerTop[MAX_TEXT_SIZE];
char textLayerDown[MAX_TEXT_SIZE];
int idTop = -1;
int idBot = -1;
int currentPage = 0;
TextLayer *output_layer_up;
TextLayer *output_layer_down;

void loadSampleData(){
  persist_write_int(SLOT1, 4);
  persist_write_int(SLOT2, 5);
  persist_write_int(SLOT3, 0);
  persist_write_int(SLOT4, 13);
  persist_write_int(SLOT5, 14);
  persist_write_int(SLOT6, 15);
  persist_write_int(SLOT7, 10);
  persist_write_int(SLOT8, 11);
}

void getWaiting(int id){
  switch (id) {
    case REQUEST_LOCATION:
      strcpy(text, "Request for:\nLOCATION\nsent");
      break;
    case REQUEST_FIX_LOCATION:
      strcpy(text, "Request for:\nFIXING TARGET\nsent");
      break;
    case REQUEST_START_THREADED_LOCATION:
      strcpy(text, "Request for:\nSTART THREAD NAVIGATION\nsent");
      break;
    case REQUEST_STOP_THREADED_LOCATION:
      strcpy(text, "Request for:\nSTOP THREAD NAVIGATION\nsent");
      break;
    case REQUEST_ELEVATION:
      strcpy(text, "Request for:\nELEVATION\nsent");
      break;
    case REQUEST_WEATHER_STATUS:
      strcpy(text, "Request for:\nWEATHER_STATUS\nsent");
      break;
    case REQUEST_WEATHER_TEMPERATURE:
      strcpy(text, "Request for:\nTEMPERATURE\nsent");
      break;
    case REQUEST_WEATHER_PRESSURE:
      strcpy(text, "Request for:\nPRESSURE\nsent");
      break;
    case REQUEST_WEATHER_HUMIDITY:
      strcpy(text, "Request for:\nHUMIDITY\nsent");
      break;
    case REQUEST_WEATHER_WIND:
      strcpy(text, "Request for:\nWIND\nsent");
      break;
    case REQUEST_WEATHER_SUNRISE:
      strcpy(text, "Request for:\nSUNRISE\nsent");
      break;
    case REQUEST_WEATHER_SUNSET:
      strcpy(text, "Request for:\nSUNSET\nsent");
      break;
    case REQUEST_TRANSPORT:
      strcpy(text, "Request for:\nTRANSPORT\nsent");
      break;
    case SHOW_UP_TIME:
      strcpy(text, "Mode:\nSHOW_UP_TIME\nset");
      break;
    case SHOW_ACTIVE_TIME:
      strcpy(text, "Mode:\nSHOW_ACTIVE_TIME\nset");
      break;
    case SHOW_BATTERY_STATE:
      strcpy(text, "Mode:\nSHOW_BATTERY_STATE\nset");
      break;
  }
}

//menu callback
static void menu_select_callback(int index, void *ctx) {
  s_first_menu_items[index].subtitle = "You've hit select here!";
  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

void send(int key, char *value) {
  int s1, m1, h1;
  BatteryChargeState charge_state;
  DictionaryIterator *iter;
  int boolUpdate = 0;
  
  switch(key){
    case SHOW_UP_TIME:
      s1 = up_time % 60;
      m1 = (up_time % 3600) / 60;
      h1 = up_time / 3600;
      snprintf(text, MAX_TEXT_SIZE, "Uptime:\n%dh %dm %ds", h1, m1, s1);
      
      boolUpdate = 1;
      break;
    
    case SHOW_ACTIVE_TIME:
      break;
    case SHOW_BATTERY_STATE:
      charge_state = battery_state_service_peek();
      if (charge_state.is_charging) {
        snprintf(text, MAX_TEXT_SIZE, "Battery is charging");
      }
      else {
        snprintf(text, MAX_TEXT_SIZE, "Battery is\n%d%% charged", charge_state.charge_percent);
      }
      boolUpdate = 1;
      break;
    default:
      app_message_outbox_begin(&iter);
      dict_write_cstring(iter, key, value);
      app_message_outbox_send();
      break;
  }
  
  if(boolUpdate == 1){
    //update top
    if(key == idTop){
      strcpy(textLayerTop, text);
      text_layer_set_text(output_layer_up, textLayerTop);
      
      //start next
      APP_LOG(APP_LOG_LEVEL_INFO, "send id : %d", idBot);
      send(idBot, "");
    }
    
    //update bot
    if(key == idBot){
      strcpy(textLayerDown, text);
      text_layer_set_text(output_layer_down, textLayerDown);
    }
  }
  
  
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (counter ==  SHOW_UP_TIME) {
    // Get time since launch
    int seconds = up_time % 60;
    int minutes = (up_time % 3600) / 60;
    int hours = up_time / 3600;

    snprintf(text, MAX_TEXT_SIZE, "Uptime:\n%dh %dm %ds", hours, minutes, seconds);
    text_layer_set_text(output_layer, text);
  }
  else if (counter == SHOW_BATTERY_STATE) {
    BatteryChargeState charge_state = battery_state_service_peek();

    if (charge_state.is_charging) {
      snprintf(text, MAX_TEXT_SIZE, "Battery is charging");
    }
    else {
      snprintf(text, MAX_TEXT_SIZE, "Battery is\n%d%% charged", charge_state.charge_percent);
    }
    text_layer_set_text(output_layer, text);
  }  
  
  // Increment uptime
  up_time++;
}

static void data_handler(AccelData *data, uint32_t num_samples) {  // accel from -4000 to 4000, 1g = 1000 cm/s²
  if (idTop == SHOW_ACTIVE_TIME) {
    int i, x, y, z, acc_norm_2;
    for (i = 0; i < NUM_ACCEL_SAMPLES; i++) {
                                             // Divide by 10 to avoid too high values. Now from -400 to 400
      x = data[i].x / 10;                    // accel in dm/s²
      y = data[i].y / 10;                    // accel in dm/s²
      z = data[i].z / 10;                    // accel in dm/s²
                                             // 1g = 100 dm/s²  
      acc_norm_2 = (x*x) + (y*y) + (z*z);    // (1g)² = 10000
      //APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d", x, y, z, acc_norm_2);
      if ( ((acc_norm_2 - GRAVITY) > ACCEL_THRESHOLD) || ((GRAVITY - acc_norm_2) > ACCEL_THRESHOLD) ) {
        active_time++;
      }
    }
    
    int active_time_s = active_time / 10;
    int seconds = active_time_s % 60;
    int minutes = (active_time_s % 3600) / 60;
    int hours = active_time_s / 3600;

    snprintf(text, MAX_TEXT_SIZE, "Active time:\n%dh %dm %ds", hours, minutes, seconds);
    strcpy(textLayerTop, text);
    text_layer_set_text(output_layer_up, textLayerTop);
    
    //start next
    //APP_LOG(APP_LOG_LEVEL_INFO, "send id : %d", idBot);
    send(idBot, "");
    
  }else if (idBot == SHOW_ACTIVE_TIME) {
    int i, x, y, z, acc_norm_2;
    for (i = 0; i < NUM_ACCEL_SAMPLES; i++) {
                                             // Divide by 10 to avoid too high values. Now from -400 to 400
      x = data[i].x / 10;                    // accel in dm/s²
      y = data[i].y / 10;                    // accel in dm/s²
      z = data[i].z / 10;                    // accel in dm/s²
                                             // 1g = 100 dm/s²  
      acc_norm_2 = (x*x) + (y*y) + (z*z);    // (1g)² = 10000
      //APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d", x, y, z, acc_norm_2);
      if ( ((acc_norm_2 - GRAVITY) > ACCEL_THRESHOLD) || ((GRAVITY - acc_norm_2) > ACCEL_THRESHOLD) ) {
        active_time++;
      }
    }
    
    int active_time_s = active_time / 10;
    int seconds = active_time_s % 60;
    int minutes = (active_time_s % 3600) / 60;
    int hours = active_time_s / 3600;

    snprintf(text, MAX_TEXT_SIZE, "Active time:\n%dh %dm %ds", hours, minutes, seconds);
    strcpy(textLayerDown, text);
    text_layer_set_text(output_layer_down, textLayerTop);
  }
}

void received_handler(DictionaryIterator *iter, void *context) {
  Tuple *result_tuple = dict_find(iter, PEBBLE_KEY_VALUE);
  
  switch(result_tuple->value->int32) {
    // Location API
    case REQUEST_LOCATION:
      strcpy(text, "lat : ");
      strcat(text, dict_find(iter, KEY_LATITUDE)->value->cstring);
      strcat(text, "\nlon : ");
      strcat(text, dict_find(iter, KEY_LONGITUDE)->value->cstring);
      break;
    case REQUEST_START_THREADED_LOCATION:
      strcpy(text, "distance : ");
      strcat(text, dict_find(iter, KEY_DISTANCE)->value->cstring);
      strcat(text, "\ndirection : ");
      strcat(text, dict_find(iter, KEY_DIRECTION)->value->cstring);
      break;
    // Elevation API
    case REQUEST_ELEVATION:
      strcpy(text, "altitude : ");
      strcat(text, dict_find(iter, KEY_ALTITUDE)->value->cstring);
      strcat(text, "m");
      break;
    // Weather API
    case REQUEST_WEATHER_STATUS:
      strcpy(text, dict_find(iter, KEY_STATUS)->value->cstring);
      strcat(text, "\n");
      strcat(text, dict_find(iter, KEY_DESCRIPTION)->value->cstring);
      break;
    case REQUEST_WEATHER_TEMPERATURE:
      strcpy(text, dict_find(iter, KEY_TEMPERATURE)->value->cstring);
      strcat(text, "°C");
      break;
    case REQUEST_WEATHER_PRESSURE:
      strcpy(text, "pressure : ");
      strcat(text, dict_find(iter, KEY_PRESSURE)->value->cstring);
      break;
    case REQUEST_WEATHER_HUMIDITY:
      strcpy(text, "humidity : ");
      strcat(text, dict_find(iter, KEY_HUMIDITY)->value->cstring);
      break;
    case REQUEST_WEATHER_WIND:
      strcpy(text, "wind speed : ");
      strcat(text, dict_find(iter, KEY_WIND_SPEED)->value->cstring);
      strcat(text, "km/h\nwind direction : ");
      strcat(text, dict_find(iter, KEY_WIND_DIRECTION)->value->cstring);
      break;
    case REQUEST_WEATHER_SUNRISE:
      strcpy(text, "sunrise : \n");
      strcat(text, dict_find(iter, KEY_SUNRISE)->value->cstring);
      break;
    case REQUEST_WEATHER_SUNSET:
      strcpy(text, "sunset : \n");
      strcat(text, dict_find(iter, KEY_SUNSET)->value->cstring);
      break;
    // Transport API
    case REQUEST_TRANSPORT:
      strcpy(text, dict_find(iter, KEY_DEPARTURE)->value->cstring);
      strcat(text, " : ");
      strcat(text, dict_find(iter, KEY_DEPARTURE_TIME)->value->cstring);
      strcat(text, "\n");
      strcat(text, dict_find(iter, KEY_ARRIVAL)->value->cstring);
      strcat(text, " : ");
      strcat(text, dict_find(iter, KEY_ARRIVAL_TIME)->value->cstring);
      break;
    default:
      strcpy(text, "Error.\nPlease check your dictionary KEYS");
      break;
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "receive: %s", text);
  
  //update top
  if(result_tuple->value->int32 == idTop){
    strcpy(textLayerTop, text);
    text_layer_set_text(output_layer_up, textLayerTop);
    
    //start next
    APP_LOG(APP_LOG_LEVEL_INFO, "send id : %d", idBot);
    send(idBot, "");
  }
  
  //update bot
  if(result_tuple->value->int32 == idBot){
    strcpy(textLayerDown, text);
    text_layer_set_text(output_layer_down, textLayerDown);
  }
}

void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  currentPage = (currentPage + 1) % NUMBER_OF_PAGE;
  APP_LOG(APP_LOG_LEVEL_INFO, "current page : %d", currentPage);
  
  idTop = persist_read_int(SLOT1 + (currentPage * 2));
  APP_LOG(APP_LOG_LEVEL_INFO, "send id : %d", idTop);
  send(idTop, "");
  text_layer_set_text(output_layer_up, "waiting..");
  
  idBot = persist_read_int(SLOT2 + (currentPage * 2));
  text_layer_set_text(output_layer_down, "waiting..");
}

void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int halfHeight = 76;
  
  //Menu part
  s_first_menu_items[0] = (SimpleMenuItem) {
    .title = "Slot 1",
    .callback = menu_select_callback,
  };
  s_first_menu_items[1] = (SimpleMenuItem) {
    .title = "Slot 2",
    .subtitle = "Here's a subtitle",
    .callback = menu_select_callback,
  };
  
  s_second_menu_items[0] = (SimpleMenuItem) {
    .title = "Slot 3",
    .callback = menu_select_callback,
  };
  s_second_menu_items[1] = (SimpleMenuItem) {
    .title = "Slot 4",
    .callback = menu_select_callback,
  };
  
  s_menu_sections[0] = (SimpleMenuSection) {
    .title = "Page 1",
    .num_items = NUM_FIRST_MENU_ITEMS,
    .items = s_first_menu_items,
  };
  s_menu_sections[1] = (SimpleMenuSection) {
    .title = "Page 2",
    .num_items = NUM_SECOND_MENU_ITEMS,
    .items = s_second_menu_items,
  };
  
  s_simple_menu_layer = simple_menu_layer_create(bounds, window, s_menu_sections, NUM_MENU_SECTIONS, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
  
  //layer du haut
  output_layer_up = text_layer_create(GRect(0, 0, bounds.size.w, halfHeight));
  idTop = persist_read_int(SLOT1);
  text_layer_set_text(output_layer_up, "waiting");
  text_layer_set_text_alignment(output_layer_up, GTextAlignmentCenter);
       
  //layer du bas
  output_layer_down = text_layer_create(GRect(0, halfHeight, bounds.size.w, halfHeight));
  idBot = persist_read_int(SLOT2);
  text_layer_set_text(output_layer_down, "waiting");
  text_layer_set_text_alignment(output_layer_down, GTextAlignmentCenter);
  
  //send 
  send(idTop, "");
  
  //on link au root
  //layer_add_child(window_layer, text_layer_get_layer(output_layer_up));
  //layer_add_child(window_layer, text_layer_get_layer(output_layer_down));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(output_layer_up);
  text_layer_destroy(output_layer_down);
  simple_menu_layer_destroy(s_simple_menu_layer);
}

/**
 * Initializes
 */
static void init(void) {
  //load sample
  loadSampleData();
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Subscribe to the accelerometer data service
  accel_data_service_subscribe(NUM_ACCEL_SAMPLES, data_handler);
  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  app_message_register_inbox_received(received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  //window
  main_window = window_create();
  window_set_click_config_provider(main_window, click_config_provider);
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(main_window, true);
}
  
static void deinit(void) {
  window_destroy(main_window);
}

/**
 * Starts the Pebble app.
 */
int main(void) {
  init();
  app_event_loop();
  deinit();
}