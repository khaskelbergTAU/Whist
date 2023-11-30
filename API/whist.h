#ifndef _WHIST_H
#define _WHIST_H
#include <stddef.h>

#define LOGF(...) fprintf(stderr, __VA_ARGS__)

typedef enum suit {
	NONE,
	SPADES,
	HEARTS,
	DIAMONDS,
	CLUBS,
} suit_e;

typedef struct card {
	suit_e suit;
	size_t number;
} card_t, bet_t;

typedef struct round {
	card_t cards[4];
} round_t, bets_t;

/* represents a pass */
extern const bet_t BET_PASS;
/* represents that the player has not had a turn to bet yet*/
extern const bet_t BET_NOT_PLAYED;
/* represents a non-existing card in a hand */
extern const card_t EMPTY_CARD;

/* return these values whenever an error occurs */
extern const bet_t INVALID_BET;
extern const card_t INVALID_CARD;
extern const size_t INVALID_FINAL_BET;

/* to be defined by player: */

/* 20 milliseconds timeout */
bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets);

/* 12 milliseconds timeout */
size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]);

/* 8 milliseconds timeout */
card_t play_card(round_t previous_round, round_t current_round);

/* 5 milliseconds timeout */
void game_over(round_t final_round);
#endif /* _WHIST_H */
