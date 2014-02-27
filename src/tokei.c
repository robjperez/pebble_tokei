#include <pebble.h>

#define HMSLAYER_SIZE 4
#define DMLAYER_SIZE 6
#define NUMSARRAY_SIZE 11

static Window *window;
static GBitmap *nums_36[NUMSARRAY_SIZE];
static GBitmap *nums_18[NUMSARRAY_SIZE];
static GBitmap *h_36;
static GBitmap *m_36;
static GBitmap *s_18;
static GBitmap *mon_18;
static GBitmap *d_18;
static BitmapLayer *hLayer[HMSLAYER_SIZE];
static BitmapLayer *mLayer[HMSLAYER_SIZE];
static BitmapLayer *sLayer[HMSLAYER_SIZE];
static BitmapLayer *dmLayer[DMLAYER_SIZE];
static int prevMins;
static int prevHour;
static int prevDay;

static int resource36Nums[] = {RESOURCE_ID_0_36_WHITE, RESOURCE_ID_1_36_WHITE, RESOURCE_ID_2_36_WHITE, 
  RESOURCE_ID_3_36_WHITE, RESOURCE_ID_4_36_WHITE, RESOURCE_ID_5_36_WHITE, RESOURCE_ID_6_36_WHITE, 
  RESOURCE_ID_7_36_WHITE, RESOURCE_ID_8_36_WHITE, RESOURCE_ID_9_36_WHITE, RESOURCE_ID_10_36_WHITE };

static int resource18Nums[] = {RESOURCE_ID_0_18_WHITE, RESOURCE_ID_1_18_WHITE, RESOURCE_ID_2_18_WHITE, 
  RESOURCE_ID_3_18_WHITE, RESOURCE_ID_4_18_WHITE, RESOURCE_ID_5_18_WHITE, RESOURCE_ID_6_18_WHITE, 
  RESOURCE_ID_7_18_WHITE, RESOURCE_ID_8_18_WHITE, RESOURCE_ID_9_18_WHITE, RESOURCE_ID_10_18_WHITE };



static void update_layer_array(BitmapLayer *layer[], GBitmap *unit, int value, GBitmap *nums[]) {
  if (value >= 20) {
    bitmap_layer_set_bitmap(layer[0], nums[value / 10]);
    bitmap_layer_set_bitmap(layer[1], nums[10]);
    bitmap_layer_set_bitmap(layer[2], nums[value % 10]);
    bitmap_layer_set_bitmap(layer[3], unit);
  } else if (value > 10) {    
    bitmap_layer_set_bitmap(layer[0], nums[10]);
    bitmap_layer_set_bitmap(layer[1], nums[value % 10]);
    bitmap_layer_set_bitmap(layer[2], unit);    
    bitmap_layer_set_bitmap(layer[3], NULL);
  } else {
    bitmap_layer_set_bitmap(layer[0], nums[value]);    
    bitmap_layer_set_bitmap(layer[1], unit);    
    bitmap_layer_set_bitmap(layer[2], NULL);
    bitmap_layer_set_bitmap(layer[3], NULL);
  }
}

static void update_hour(int hour) {
  if (hour > 12 && !clock_is_24h_style()) {
    hour -= 12;
  }

  update_layer_array(hLayer, h_36, hour, nums_36);  
}

static void update_min(int min) {
  update_layer_array(mLayer, m_36, min, nums_36);
}

static void update_seconds(int sec) {
  update_layer_array(sLayer, s_18, sec, nums_18);
}

static void update_daymonth(int day, int month) {
  int arrayIndex = 0;
  if (month > 10) {
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[10]);
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[month % 10]);    
  } else {
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[month]);    
  }

  bitmap_layer_set_bitmap(dmLayer[arrayIndex++], mon_18);

  if (day >= 20) {
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[day / 10]);
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[10]);  
  } else if (day > 10) {
    bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[10]);        
  }

  bitmap_layer_set_bitmap(dmLayer[arrayIndex++], nums_18[day % 10]);
  bitmap_layer_set_bitmap(dmLayer[arrayIndex++], d_18);

  // Clear remaining layers
  for (int i = arrayIndex; i < 6; i++) {
    bitmap_layer_set_bitmap(dmLayer[i], NULL);
  }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) { 
  update_seconds(tick_time->tm_sec);
  
  if (tick_time->tm_min != prevMins) {
    update_min(tick_time->tm_min);
    prevMins = tick_time->tm_min;
  }

  if (tick_time->tm_hour != prevHour) {
    update_hour(tick_time->tm_hour);
    prevHour = tick_time->tm_hour;
  }

  if (tick_time->tm_mday != prevDay) {
    update_daymonth(tick_time->tm_mday, tick_time->tm_mon+1);
    prevDay = tick_time->tm_mday;
  }
}

static void set_current_time() {
  time_t currentTime = time(NULL);
  struct tm* tmTime = localtime(&currentTime); 
  update_hour(tmTime->tm_hour);
  update_min(tmTime->tm_min);
  update_seconds(tmTime->tm_sec);
  update_daymonth(tmTime->tm_mday, tmTime->tm_mon+1);
  prevMins = tmTime->tm_min;
  prevHour = tmTime->tm_hour;
  prevDay = tmTime->tm_mday;
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);  

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);  

  for (int i = 0; i < HMSLAYER_SIZE; i++) {    
    layer_add_child(window_layer, bitmap_layer_get_layer(hLayer[i]));    
    layer_add_child(window_layer, bitmap_layer_get_layer(mLayer[i]));
    layer_add_child(window_layer, bitmap_layer_get_layer(sLayer[i]));    
  }

  for (int i = 0; i < DMLAYER_SIZE; i++) {
    layer_add_child(window_layer, bitmap_layer_get_layer(dmLayer[i]));    
  }

  set_current_time();
}

static void create_bitmaps() {
  for (int i = 0; i < NUMSARRAY_SIZE; i++) {
    nums_36[i] = gbitmap_create_with_resource(resource36Nums[i]);
    nums_18[i] = gbitmap_create_with_resource(resource18Nums[i]);
  }

  h_36 = gbitmap_create_with_resource(RESOURCE_ID_h_36_WHITE);
  m_36 = gbitmap_create_with_resource(RESOURCE_ID_m_36_WHITE);
  s_18 = gbitmap_create_with_resource(RESOURCE_ID_s_18_WHITE);
  d_18 = gbitmap_create_with_resource(RESOURCE_ID_d_18_WHITE);
  mon_18 = gbitmap_create_with_resource(RESOURCE_ID_mon_18_WHITE);
}

static void create_layers() {
  static int nums36LayerBoundingSize = 40;
  static int nums18LayerBoundingSize = 20;
  static int nums36LayerSize = 36;
  static int nums18LayerSize = 18;
  static int startingLayerX = 80;
  static int startingLayerY = 10;
  static int startingDateAndSecondsLayerY = 15;
  static int extraPaddingSecondsLayer = 7;

  for (int i = 0; i < HMSLAYER_SIZE; i++) {
    hLayer[i] = bitmap_layer_create( (GRect) {
      .origin = { 
        .x = startingLayerX,
        .y = startingLayerY +(i*nums36LayerBoundingSize)
      },
      .size = { 
        .h = nums36LayerSize,
        .w = nums36LayerSize
      }
    });

    mLayer[i] = bitmap_layer_create( (GRect) {
      .origin = { 
        .x = startingLayerX - nums36LayerBoundingSize,
        .y = startingLayerY +(i*nums36LayerBoundingSize)
      },
      .size = { 
        .h = nums36LayerSize,
        .w = nums36LayerSize
      }
    });

    sLayer[i] = bitmap_layer_create( (GRect) {
      .origin = { 
        .x = startingLayerX - nums36LayerBoundingSize - nums18LayerBoundingSize - extraPaddingSecondsLayer,
        .y = startingDateAndSecondsLayerY +(i*nums18LayerBoundingSize)
      },
      .size = { 
        .h = nums18LayerSize,
        .w = nums18LayerSize
      }
    });
  }

  for (int i = 0; i < DMLAYER_SIZE; i++) {
    dmLayer[i] = bitmap_layer_create( (GRect) {
      .origin = { 
        .x = startingLayerX + nums36LayerBoundingSize,
        .y = startingDateAndSecondsLayerY +(i*nums18LayerBoundingSize)
      },
      .size = { 
        .h = nums18LayerSize,
        .w = nums18LayerSize
      }
    });
  }
}

static void init(void) {
  window = window_create();
 
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,    
  });
  window_set_background_color(window, GColorBlack);

  create_bitmaps();
  create_layers();

  const bool animated = true;
  window_stack_push(window, animated);
}

static void free_bitmaps() {
  for (int i = 0; i < NUMSARRAY_SIZE; i++) {
    free(nums_36[i]);
    free(nums_18[i]);
  }

  free(h_36);
  free(m_36);
  free(s_18);
  free(mon_18);
  free(d_18);
}

static void destroy_layers() {
  for (int i = 0; i < HMSLAYER_SIZE; i++) {
    bitmap_layer_destroy(hLayer[i]);
    bitmap_layer_destroy(mLayer[i]);
    bitmap_layer_destroy(sLayer[i]);
  }  

  for (int i = 0; i < DMLAYER_SIZE; i++) {
    bitmap_layer_destroy(dmLayer[i]);  
  }
}

static void deinit(void) { 
  free_bitmaps();
  destroy_layers();

  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
