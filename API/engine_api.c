#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "engine_api.h"

const char INITIAL_BET_OP = 'i';
const char FINAL_BET_OP = 'f';
const char PLAY_CARD_OP = 'p';
const char GAME_OVER_OP = 'o';

const bet_t BET_PASS = {NONE, 0};
const bet_t INVALID_BET = {-1, -1};
const card_t INVALID_CARD = {-1, -1};
const size_t INVALID_FINAL_BET = -1;

int player_in_fd[4];
int player_out_fd[4];
int player_err_fd[4];

FILE *player_in_fp[4];
FILE *player_out_fp[4];
FILE *player_err_fp[4];

pid_t player_pids[4];

const char *player_execs[5];

const int RANDOM_PLAYER_ID = 4;

void set_exec_name(size_t player_id, char * name) {
	player_execs[player_id] = name;
}

void replace_with_random(size_t player_id) {
	replace_player(player_id, RANDOM_PLAYER_ID);
}

void clear_player(size_t player_id) {
	kill(player_pids[player_id], SIGKILL);
}

void set_player(size_t player_id, size_t exec_id) {
	int stdin_fd[2]; //1 is write
	int stdout_fd[2];
	assert(pipe(stdin_fd) == 0);
	assert(pipe(stdout_fd) == 0);
	pid_t child_pid = fork();
	if(child_pid == 0) { 
		//src -> dst
		assert(dup2(stdin_fd[0], 0) != -1);
		assert(dup2(stdout_fd[1], 1) != -1);
		assert(close(stdin_fd[0]) == 0);
		assert(close(stdin_fd[1]) == 0);
		assert(close(stdout_fd[0]) == 0);
		assert(close(stdout_fd[1]) == 0);
		//assert(dup2(player_err_fd[player_id], 2) != -1);
		execl(player_execs[exec_id], player_execs[exec_id], (char *)NULL);
		fprintf(stderr, "execl failed\n");
		exit(1);
	}
	else
	{
		player_in_fd[player_id] = stdin_fd[1];
		player_out_fd[player_id] = stdout_fd[0];
		player_pids[player_id] = child_pid;
		assert(close(stdin_fd[0]) == 0);
		assert(close(stdout_fd[1]) == 0);
	}
	assert((player_in_fp[player_id] = fdopen(player_in_fd[player_id], "w")) != NULL);
	assert((player_out_fp[player_id] = fdopen(player_out_fd[player_id], "r")) != NULL);
}

void replace_player(size_t player_id, size_t exec_id) {
	clear_player(player_id);
	set_player(player_id, exec_id);
}

bet_t place_initial_bet(size_t player_id, size_t player_position, card_t my_hand[13], bets_t previous_bets) {
	init_bets_args_t args;
	bet_t res;
	args.player_position = player_position;
	memcpy(&args.my_hand[0], my_hand, sizeof(card_t) * 13);
	args.previous_bets = previous_bets;
	if(write(player_in_fd[player_id], &INITIAL_BET_OP, sizeof(char)) != sizeof(char)) {
		return INVALID_BET;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		return INVALID_BET;
	}
	if(read(player_out_fd[player_id], &res, sizeof(res)) != sizeof(res)) {
		return INVALID_BET;
	}
	return res;
}

size_t place_final_bet(size_t player_id, suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
	final_bets_args_t args;
	size_t res;
	args.trump = trump;
	args.highest_bidder = highest_bidder;
	memcpy(args.final_bets, final_bets, sizeof(size_t) * 4);
	if(write(player_in_fd[player_id], &FINAL_BET_OP, sizeof(char)) != sizeof(char)) {
		return INVALID_FINAL_BET;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		return INVALID_FINAL_BET;
	}
	if(read(player_out_fd[player_id], &res, sizeof(res)) != sizeof(res)) {
		return INVALID_FINAL_BET;
	}
	return res;
}

card_t play_card(size_t player_id, round_t previous_round, round_t current_round) {
	play_card_args_t args;
	card_t res;
	args.previous_round = previous_round;
	args.current_round = current_round;
	if(fwrite(&PLAY_CARD_OP, sizeof(char), 1, player_in_fp[player_id]) != 1) {
		return INVALID_CARD;
	}
	if(fwrite(&args, sizeof(args), 1, player_in_fp[player_id]) != 1) {
		return INVALID_CARD;
	}
	if(fread(&res, sizeof(res), 1, player_out_fp[player_id]) != 1) {
		return INVALID_CARD;
	}
	return res;
}

void game_over(size_t player_id, round_t final_round) {
	game_over_args_t args;
	int res;
	args.final_round = final_round;
	if(fwrite(&GAME_OVER_OP, sizeof(char), 1, player_in_fp[player_id]) != 1) {
		return;
	}
	if(fwrite(&args, sizeof(args), 1, player_in_fp[player_id]) != 1) {
		return;
	}
	if(fread(&res, sizeof(res), 1, player_out_fp[player_id]) != 1) {
		return;
	}
}
