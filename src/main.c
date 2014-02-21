#include <pebble.h>
// This is the restore point now

Window *main_window;

void handle_init(void) {
	main_window = window_create();
	window_set_window_handlers(main_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void handle_deinit(void) {
	window_destroy(main_window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
