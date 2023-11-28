#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <poll.h>

#include "engine_api.h"

const char INITIAL_BET_OP = 'i';
const char FINAL_BET_OP = 'f';
const char PLAY_CARD_OP = 'p';
const char GAME_OVER_OP = 'o';

const bet_t BET_PASS = {NONE, 0};
const bet_t INVALID_BET = {-1, -1};
const card_t INVALID_CARD = {-1, -1};
const size_t INVALID_FINAL_BET = -1;

const int INITIAL_BET_TIMEOUT = 3500;
const int FINAL_BET_TIMEOUT = 1200;
const int PLAY_CARD_TIMEOUT = 1000;
const int GAME_OVER_TIMEOUT = 5000;

int player_in_fd[4];
int player_out_fd[4];
int player_err_fd[4];

FILE *player_in_fp[4];
FILE *player_out_fp[4];
FILE *player_err_fp[4];

pid_t player_pids[4];

const char *player_execs[5];
const char *player_logfiles[5];

long long time_ms() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int read_with_timeout(int fd, void *res_buf, int read_sz, int timeout) {
	long long cur_time, end_time;
	struct pollfd pollfd[1];
	int amt_read = 0;
	int poll_res;
	int read_res;
	nfds_t nfds = 1;
	pollfd[0].fd = fd;
	pollfd[0].events = POLLIN;
	cur_time = time_ms();
	end_time = cur_time + timeout;
	while(amt_read != read_sz) {
		cur_time = time_ms();
		//printf("%ld %ld %ld\n", end_time, cur_time, end_time - cur_time);
		if(end_time - cur_time < 0) break;
		poll_res = poll(pollfd, nfds, end_time - cur_time);
		if(poll_res < 0) return -1;
		//printf("arrived\n");
		if(poll_res == 0) continue;
		read_res = read(fd, ((char *)res_buf) + amt_read, read_sz - amt_read);
		if(read_res < 0) return -1;
		amt_read += read_res;
	}
	return amt_read;
}

void set_exec(size_t player_id, char *name, char *logfile)
{
	player_execs[player_id] = name;
	player_logfiles[player_id] = logfile;
}

void replace_with_random(size_t player_id) {
	replace_player(player_id, RANDOM_PLAYER_ID);
}

void clear_player(size_t player_id) {
	kill(player_pids[player_id], SIGKILL);
	waitpid(player_pids[player_id], NULL, 0);
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
		if ((player_err_fd[player_id] = open(player_logfiles[player_id], O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
		{
			perror("open logfile");
			exit(1);
		}
		assert(dup2(player_err_fd[player_id], 2) != -1);
		assert(close(player_err_fd[player_id]) == 0);
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
	memcpy(args.my_hand, my_hand, sizeof(card_t) * 13);
	args.previous_bets = previous_bets;
	if(write(player_in_fd[player_id], &INITIAL_BET_OP, sizeof(char)) != sizeof(char)) {
		return INVALID_BET;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		return INVALID_BET;
	}
	int idk;
	if((idk = read_with_timeout(player_out_fd[player_id], &res, sizeof(res), INITIAL_BET_TIMEOUT)) != sizeof(res)) {
		printf("%d", idk);
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
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), FINAL_BET_TIMEOUT) != sizeof(res)) {
		return INVALID_FINAL_BET;
	}
	return res;
}

card_t play_card(size_t player_id, round_t previous_round, round_t current_round) {
	play_card_args_t args;
	card_t res;
	args.previous_round = previous_round;
	args.current_round = current_round;
	if(write(player_in_fd[player_id], &PLAY_CARD_OP, sizeof(char)) != sizeof(char)) {
		return INVALID_CARD;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		return INVALID_CARD;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), PLAY_CARD_TIMEOUT) != sizeof(res)) {
		return INVALID_CARD;
	}
	return res;
}

void game_over(size_t player_id, round_t final_round) {
	game_over_args_t args;
	int res;
	args.final_round = final_round;
	if(write(player_in_fd[player_id], &GAME_OVER_OP, sizeof(char)) != 1) {
		return;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != 1) {
		return;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), GAME_OVER_TIMEOUT) != 1) {
		return;
	}
}
