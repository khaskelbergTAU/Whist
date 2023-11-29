#ifndef _WHIST_H
#define _WHIST_H
#include <stddef.h>
typedef enum suit {
	NONE,
	HEARTS,
	DIAMONDS,
	CLUBS,
	SPADES,
} suit_e;

typedef struct card {
	suit_e suit;
	size_t number;
} card_t, bet_t;

typedef struct round {
	card_t cards[4];
} round_t, bets_t;

const bet_t BET_PASS = {NONE, 0};
const bet_t BET_NOT_PLAYED = {CLUBS, 0};
const bet_t INVALID_BET = {(suit_e)-1, (size_t)-1};
const card_t INVALID_CARD = {(suit_e)-1, (size_t)-1};
const size_t INVALID_FINAL_BET = -1;

/* to be defined by player: */
bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets);

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]);

card_t play_card(round_t previous_round, round_t current_round);

void game_over(round_t final_round);
#endif /* _WHIST_H */
