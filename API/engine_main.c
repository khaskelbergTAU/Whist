#include "engine_api.h"
int main(int argc, char ** argv) {
	set_exec(0, argv[1], "idkman");
	set_player(0, 0);
	size_t test_val = 100;
	size_t idk[4] = {1, 2, 3, 4};
	printf("%lu\n", test_val);
	test_val = place_final_bet(0, 0, 0, idk);
	printf("%lu\n", test_val);
}
