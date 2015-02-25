#include <pebble.h>

#define HOUR_VIBRATION_START 10
#define HOUR_VIBRATION_END 23

Window *window;

BitmapLayer *background_layer;
RotBitmapLayer *minute_hand_layer, *hour_hand_layer;

GBitmap *background_image, *minute_hand_image, *hour_hand_image;

void update_watch(struct tm *t){

	long minute_hand_rotation = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
	long hour_hand_rotation = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) + (t->tm_min / 2)) / 360;
	rot_bitmap_layer_set_angle(minute_hand_layer, minute_hand_rotation);
	rot_bitmap_layer_set_angle(hour_hand_layer, hour_hand_rotation);

	layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));
	layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));
	
	if (t->tm_min == 0 // Make pebble vibrate on the hour at the top of each hour
	 && t->tm_hour >= HOUR_VIBRATION_START
	 && t->tm_hour <= HOUR_VIBRATION_END) {
		vibes_double_pulse();
	}
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
	update_watch(t);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

}

void handle_deinit() {
	
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