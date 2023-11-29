#ifndef SHARED_H
#define SHARED_H
#include "whist.h"
typedef struct {
	size_t player_position;
	card_t my_hand[13];
	bets_t previous_bets;
} init_bets_args_t;

typedef struct {
	suit_e trump;
	size_t highest_bidder;
	size_t final_bets[4];
} final_bets_args_t;

typedef struct {
	round_t previous_round, current_round;
} play_card_args_t;

typedef struct {
	round_t final_round;
} game_over_args_t;

#endif
