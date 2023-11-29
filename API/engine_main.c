#include "engine_api.h"
int main(int argc, char ** argv) {
	
	set_exec(0, argv[1], "test_log_file.txt");
	set_player(0, 0);
	size_t test_val = 100;
	size_t idk[4] = {1, 2, 3, 4};
	printf("%lu\n", test_val);
	test_val = place_final_bet_api(0, NONE, 0, idk);
	printf("%lu\n", test_val);
	card_t hand[13];
	round_t tmp_round;
	bets_t tmp_bets;
	card_t card_res = play_card_api(0, tmp_round, tmp_round);
	bet_t bet_res = place_initial_bet_api(0, 0, hand, tmp_bets);
	card_t card_res2 = play_card_api(0, tmp_round, tmp_round);
	bet_t bet_res2 = place_initial_bet_api(0, 0, hand, tmp_bets);
	//printf("%d %d %d %d\n", bet_res.suit, bet_res.number, card_res.suit, card_res.number);
	//printf("%d %d %d %d\n", bet_res2.suit, bet_res2.number, card_res2.suit, card_res2.number);
	log_player_err(0, "testing the log\n");
	
}
