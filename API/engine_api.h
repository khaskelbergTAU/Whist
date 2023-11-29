#ifndef _API_ENGINE_API_H
#define _API_ENGINE_API_H
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include <stdio.h>
#include "shared.h"
#define RANDOM_PLAYER_ID 4
#define log_player_err(player_id, ...)\
{\
	dprintf(player_err_fd[ player_id ], "[System Error]: ");\
	dprintf(player_err_fd[ player_id ], __VA_ARGS__);\
}

extern const bet_t INVALID_BET;
extern const card_t INVALID_CARD;
extern const size_t INVALID_FINAL_BET;
extern int player_err_fd[4];

void replace_with_random(size_t player_id);
void clear_player(size_t player_id);
void replace_player(size_t player_id, size_t exec_id);
void set_player(size_t player_id, size_t exec_id);
void set_exec(size_t player_id, char *args, const char *logfile);

bet_t place_initial_bet_api(size_t player_id, size_t player_position, card_t my_hand[13], bets_t previous_bets);

size_t place_final_bet_api(size_t player_id, suit_e trump, size_t highest_bidder, size_t final_bets[4]);

card_t play_card_api(size_t player_id, round_t previous_round, round_t current_round);

void game_over_api(size_t player_id, round_t final_round);
#ifdef __cplusplus
}
#endif
#endif /* _API_ENGINE_API_H */
