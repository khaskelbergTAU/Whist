#ifndef _API_ENGINE_API_H
#define _API_ENGINE_API_H
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <stdio.h>

#define RANDOM_PLAYER_ID 4

extern const bet_t INVALID_BET = {-1, -1};
extern const card_t INVALID_CARD = {-1, -1};
extern const size_t INVALID_FINAL_BET = -1;

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
	card_t cards[4];
} round_t, bets_t;

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

void replace_with_random(size_t player_id);
void clear_player(size_t player_id);
void replace_player(size_t player_id, size_t exec_id);
void set_player(size_t player_id, size_t exec_id);
void set_exec(size_t player_id, char *args, char *logfile);
bet_t place_initial_bet(size_t player_id, size_t player_position, card_t my_hand[13], bets_t previous_bets);

size_t place_final_bet(size_t player_id, suit_e trump, size_t highest_bidder, size_t final_bets[4]);

card_t play_card(size_t player_id, round_t previous_round, round_t current_round);

void game_over(size_t player_id, round_t final_round);
#ifdef __cplusplus
}
#endif
#endif /* _API_ENGINE_API_H */
