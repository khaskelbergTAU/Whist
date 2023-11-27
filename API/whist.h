#ifndef _WHIST_H
#define _WHIST_H
#include <stddef.h>
typedef enum suit {
	NONE,
	HEARTS,
	CLUBS,
	DIAMONDS,
	SPADES,
} suit_e;

typedef struct card {
	suit_e suit;
	size_t number;
} card_t, bet_t;

typedef struct round {
	size_t first_player;
	card_t cards[4];
} round_t, bets_t;

const bet_t BET_PASS;

/* to be defined by player: */
bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets);

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]);

card_t play_card(round_t previous_round, round_t current_round);

void game_over(round_t final_round);
#endif /* _WHIST_H */
