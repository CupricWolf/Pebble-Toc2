#include <pebble.h>

Window *main_window;

void handle_init(void) {
	main_window = window_create();

}

void handle_deinit(void) {
	window_destroy(main_window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}