#include "whist.h"
#include <stdio.h>

bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets) {
	fprintf(stderr, "place_initial_bet()!\n");
	bet_t bet = {100, 200};
	return bet;
}

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
	return 7;
}

card_t play_card(round_t previous_round, round_t current_round) {
	card_t card = {69, 420};
	return card;
}

void game_over(round_t final_round) {

}
