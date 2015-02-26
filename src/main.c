#include <pebble.h>

enum Settings { setting_vibrate = 1, setting_vibrate_start, setting_vibrate_end };

Window *window;
BitmapLayer *background_layer;
RotBitmapLayer *minute_hand_layer, *hour_hand_layer;
AppSync app;
uint8_t buffer[256];
GBitmap *background_image, *minute_hand_image, *hour_hand_image;
int vibrateBool, vibrateStartHour, vibrateEndHour;

void update_watch(struct tm *t){

	long minute_hand_rotation = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
	long hour_hand_rotation = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) + (t->tm_min / 2)) / 360;
	rot_bitmap_layer_set_angle(minute_hand_layer, minute_hand_rotation);
	rot_bitmap_layer_set_angle(hour_hand_layer, hour_hand_rotation);

	layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));
	layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));
	
	if ((vibrateBool == 1)
		&& (t->tm_min == 0) && (t->tm_hour >= vibrateStartHour) && (t->tm_hour <= vibrateEndHour)) {
		vibes_double_pulse();
	}
}

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) {
	switch (key) {
		case setting_vibrate:
			vibrateBool = tuple_new->value->uint16;
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrate boolean: %i", tuple_new->value->uint16);
			break;
		case setting_vibrate_start:
			vibrateStartHour = atoi(tuple_new->value->cstring);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrate start hour: %i", atoi(tuple_new->value->cstring));
			break;
		case setting_vibrate_end:
			vibrateEndHour = atoi(tuple_new->value->cstring);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrate end hour: %i", atoi(tuple_new->value->cstring));
			break;
	}
}

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

// Called once per second
void handle_minute_tick(struct tm *t, TimeUnits units_changed) {
	(void)units_changed;
	update_watch(t);
}


// Handle the start-up of the app
void handle_init() {

	// Create our app's base window
	window = window_create();
	Layer *root_window_layer = window_get_root_layer(window);
	GRect root_window_bounds = layer_get_bounds(root_window_layer);

	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);
	
	vibrateBool = 0;
	vibrateStartHour = 0;
	vibrateEndHour = 23;

	// Set up a layer for the static watch face background
	background_layer = bitmap_layer_create(root_window_bounds);
	background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	bitmap_layer_set_bitmap(background_layer, background_image);
	layer_add_child(root_window_layer, bitmap_layer_get_layer(background_layer));

	// Set up a layer for the minute hand.
	// Compositing tricks take the place of PNG transparency.
	minute_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_TRAIL);
	minute_hand_layer = rot_bitmap_layer_create(minute_hand_image);

	// Default frame for RotBitmapLayers is according to some
	// opaque algorithm. Automatically centre it now.
	GRect minute_frame = layer_get_frame(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));
	GRect new_minute_frame = GRect((144-minute_frame.size.w)/2,
				 (168-minute_frame.size.h)/2,
				 minute_frame.size.w, minute_frame.size.h);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "minute frame: %i, %i, %i, %i",
		minute_frame.origin.x, minute_frame.origin.y,
		minute_frame.size.w, minute_frame.size.h);
	layer_set_frame(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer),
			new_minute_frame);

	rot_bitmap_set_compositing_mode(minute_hand_layer, GCompOpOr);
	layer_add_child(root_window_layer,
			bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));


	// Set up a layer for the hour hand.
	// Compositing tricks take the place of PNG transparency.
	hour_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_TRAIL);
	hour_hand_layer = rot_bitmap_layer_create(hour_hand_image);

	GRect hour_frame = layer_get_frame(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));
	// The current images seem to look best if this frame is offset
	// one more pixel. Still not perfect though.
	GRect new_hour_frame = GRect((144-hour_frame.size.w)/2 + 1,
						 (168-hour_frame.size.h)/2 + 1,
						 hour_frame.size.w, hour_frame.size.h);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "hour frame: %i, %i, %i, %i",
		hour_frame.origin.x, hour_frame.origin.y,
		hour_frame.size.w, hour_frame.size.h);
	layer_set_frame(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer),
			new_hour_frame);
	rot_bitmap_set_compositing_mode(hour_hand_layer, GCompOpOr);
	layer_add_child(root_window_layer,
			bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	
	Tuplet tuples[] = {
		TupletInteger(setting_vibrate, vibrateBool),
		TupletInteger(setting_vibrate_start, vibrateStartHour),
		TupletInteger(setting_vibrate_end, vibrateEndHour)
	};
	app_message_open(160, 160);
	app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples),
								tuple_changed_callback, app_error_callback, NULL);
								
	update_watch(t);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

}

void handle_deinit() {
	tick_timer_service_unsubscribe();
	rot_bitmap_layer_destroy(hour_hand_layer);
	gbitmap_destroy(hour_hand_image);
	rot_bitmap_layer_destroy(minute_hand_layer);
	gbitmap_destroy(minute_hand_image);
	gbitmap_destroy(background_image);
	bitmap_layer_destroy(background_layer);
	window_destroy(window);

}


int main(void) {

	handle_init();
	app_event_loop();
	handle_deinit();

}