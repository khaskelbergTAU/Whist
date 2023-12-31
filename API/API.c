#include <stdlib.h>
#include <stdio.h>
#include "shared.h"

const bet_t BET_PASS = {NONE, 0};
const bet_t BET_NOT_PLAYED = {CLUBS, 0};
const card_t EMPTY_CARD = {NONE, 0};
const bet_t INVALID_BET = {(suit_e)-1, (size_t)-1};
const card_t INVALID_CARD = {(suit_e)-1, (size_t)-1};
const size_t INVALID_FINAL_BET = (size_t)-1;

void init_bets_api() {
	init_bets_args_t args;
	bet_t result;
	if(fread(&args, sizeof(args), 1, stdin) != 1) {
		fprintf(stderr, "Error: reading initial bet arguments\n");
		exit(-1);
	}
	result = place_initial_bet(
			args.player_position,
			args.my_hand,
			args.previous_bets);
	if(fwrite(&result, sizeof(result), 1, stdout) != 1) {
		fprintf(stderr, "Error: writing initial bet result\n");
		exit(-1);
	}
	fflush(stdout);
}

void final_bets_api() {
	final_bets_args_t args;
	size_t result;
	if(fread(&args, sizeof(args), 1, stdin) != 1) {
		fprintf(stderr, "Error: reading final bet arguments\n");
		exit(-1);
	}
	result = place_final_bet(
			args.trump,
			args.highest_bidder,
			args.final_bets);
	if(fwrite(&result, sizeof(result), 1, stdout) != 1) {
		fprintf(stderr, "Error: writing final bet result\n");
		exit(-1);
	}
	fflush(stdout);
}

void play_card_api() {
	play_card_args_t args;
	card_t result;
	if(fread(&args, sizeof(args), 1, stdin) != 1) {
		fprintf(stderr, "Error: reading play card arguments\n");
		exit(-1);
	}
	result = play_card(
			args.previous_round,
			args.current_round);
	if(fwrite(&result, sizeof(result), 1, stdout) != 1) {
		fprintf(stderr, "Error: writing play card result\n");
		exit(-1);
	}
	fflush(stdout);
}

void game_over_api() {
	game_over_args_t args;
	if(fread(&args, sizeof(args), 1, stdin) != 1) {
		fprintf(stderr, "Error: reading play card arguments\n");
		exit(-1);
	}
	int result = 1;
	game_over(args.final_round);
	if(fwrite(&result, sizeof(result), 1, stdout) != 1) {
		fprintf(stderr, "Error: writing play card result\n");
		exit(-1);
	}
	fflush(stdout);
}

int main(int argc, char **argv)
{
	char op;
	while(1) {
		if(fread(&op, 1, 1, stdin) != 1) {
			fprintf(stderr, "Error: reading the operation code\n");
			exit(-1);
		}
		switch(op) {
			case 'i':
				init_bets_api();
				break;
			case 'f':
				final_bets_api();
				break;
			case 'p':
				play_card_api();
				break;
			case 'o':
				game_over_api();
				break;
			default:
				fprintf(stderr, "Error: invalid operation code\n");
				exit(-1);
		}
	}
}
