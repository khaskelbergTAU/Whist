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
#include <errno.h>

#include "engine_api.h"

#define CRITICAL_ERROR(...)\
{\
	fprintf(stderr, "[CRITICAL ERROR in %s, line %d, file %s, ERRNO %d]: ", __FUNCTION__, __LINE__, __FILE__, errno);\
	fprintf(stderr, __VA_ARGS__);\
	exit(-1);\
}

const char INITIAL_BET_OP = 'i';
const char FINAL_BET_OP = 'f';
const char PLAY_CARD_OP = 'p';
const char GAME_OVER_OP = 'o';

const int INITIAL_BET_TIMEOUT_MS = 40;
const int FINAL_BET_TIMEOUT_MS = 24;
const int PLAY_CARD_TIMEOUT_MS = 16;
const int GAME_OVER_TIMEOUT_MS = 10;

const bet_t BET_PASS = {NONE, 0};
const bet_t BET_NOT_PLAYED = {CLUBS, 0};
const card_t EMPTY_CARD = {NONE, 0};
const bet_t INVALID_BET = {(suit_e)-1, (size_t)-1};
const card_t INVALID_CARD = {(suit_e)-1, (size_t)-1};
const size_t INVALID_FINAL_BET = (size_t)-1;

int player_in_fd[4];
int player_out_fd[4];
int player_err_fd[4];

pid_t player_pids[4];

const char *player_execs[5];
const char *player_logfiles[5];


long long time_ms(char *schedstat_name) {
	FILE *schedstat = fopen(schedstat_name, "r");
	if(schedstat == NULL) {
		CRITICAL_ERROR("fopen(schedstat_name, ...) failed\n");
	}
	long long res;
	if(fscanf(schedstat, "%lld", &res) != 1) {
		CRITICAL_ERROR("fscanf(schedstat, ...) failed\n");
	}
	res /= 1000000;
	if(fclose(schedstat) == EOF) {
		CRITICAL_ERROR("fclose(schedstat, ...) failed\n");
	}
	return res;
	/*struct timeval tv;
	  gettimeofday(&tv, NULL);
	  return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);*/
}

int read_with_timeout(int fd, void *res_buf, int read_sz, int timeout, size_t player_id) {
	char schedstat_name[128];
	if(sprintf(schedstat_name, "/proc/%d/schedstat", player_pids[player_id]) < 0) {
		CRITICAL_ERROR("sprintf(schedstat_name, ...) failed\n");
	}
	long long cur_time, end_time;
	struct pollfd pollfd[1];
	int amt_read = 0;
	int poll_res;
	int read_res;
	nfds_t nfds = 1;
	pollfd[0].fd = fd;
	pollfd[0].events = POLLIN;
	cur_time = time_ms(schedstat_name);
	end_time = cur_time + timeout;
	while(amt_read != read_sz) {
		cur_time = time_ms(schedstat_name);
		if(end_time - cur_time < 0) break;
		poll_res = poll(pollfd, nfds, end_time - cur_time);
		if(poll_res < 0) return -1;
		if(poll_res == 0) continue;
		read_res = read(fd, ((char *)res_buf) + amt_read, read_sz - amt_read);
		if(read_res < 0) return -1;
		amt_read += read_res;
	}
	return amt_read;
}

void set_exec(size_t player_id, char *name, const char *logfile)
{
	player_execs[player_id] = name;
	player_logfiles[player_id] = logfile;
}

void replace_with_random(size_t player_id) {
	replace_player(player_id, RANDOM_PLAYER_ID);
}

void clear_player(size_t player_id) {
	if(player_in_fd[player_id] != -1) {
		if(close(player_in_fd[player_id]) != 0) {
			CRITICAL_ERROR("close(player_in_fd[player_id]) failed\n");
		}
		player_in_fd[player_id] = -1;
	}
	if(player_out_fd[player_id] != -1) {
		if(close(player_out_fd[player_id]) != 0) {
			CRITICAL_ERROR("close(player_out_fd[player_id]) failed\n");
		}
		player_out_fd[player_id] = -1;
	}
	if(player_err_fd[player_id] != -1) {
		if(close(player_err_fd[player_id]) != 0) {
			CRITICAL_ERROR("close(player_err_fd[player_id]) failed\n");
		}
		player_err_fd[player_id] = -1;
	}
	kill(player_pids[player_id], SIGKILL);
	waitpid(player_pids[player_id], NULL, 0);
}

void set_player(size_t player_id, size_t exec_id) {
	int stdin_fd[2]; //1 is write
	int stdout_fd[2];
	if(pipe(stdin_fd) != 0) {
		CRITICAL_ERROR("pipe(stdin_fd) failed\n");
	}
	if(pipe(stdout_fd) != 0) {
		CRITICAL_ERROR("pipe(stdout_fd) failed\n");
	}
	if ((player_err_fd[player_id] = open(player_logfiles[player_id], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
		CRITICAL_ERROR("open(player_logfiles[player_id], ...) failed\n");
	}
	pid_t child_pid = fork();
	if(child_pid == -1) {
		CRITICAL_ERROR("fork() failed\n");
	}
	if(child_pid == 0) { 
		//src -> dst
		if(dup2(stdin_fd[0], 0) == -1) {
			CRITICAL_ERROR("dup2(stdin_fd[0], ...) failed\n");
		}
		if(dup2(stdout_fd[1], 1) == -1) {
			CRITICAL_ERROR("dup2(stdout_fd[1], ...) failed\n");
		}
		if(close(stdin_fd[0]) != 0) {
			CRITICAL_ERROR("close(stdin_fd[0]) failed\n");
		}
		if(close(stdin_fd[1]) != 0) {
			CRITICAL_ERROR("close(stdin_fd[1]) failed\n");
		}
		if(close(stdout_fd[0]) != 0) {
			CRITICAL_ERROR("close(stdout_fd[0]) failed\n");
		}
		if(close(stdout_fd[1]) != 0) {
			CRITICAL_ERROR("close(stdout_fd[1]) failed\n");
		}
		if(dup2(player_err_fd[player_id], 2) == -1) {
			CRITICAL_ERROR("dup2(player_err_fd[player_id], ...) failed\n");
		}
		if(close(player_err_fd[player_id]) != 0) {
			CRITICAL_ERROR("close(player_err_fd[player_id]) failed\n");
		}
		execl(player_execs[exec_id], player_execs[exec_id], (char *)NULL);
		CRITICAL_ERROR("execl() failed\n");
	}
	else
	{
		player_in_fd[player_id] = stdin_fd[1];
		player_out_fd[player_id] = stdout_fd[0];
		player_pids[player_id] = child_pid;
		if(close(stdin_fd[0]) != 0) {
			CRITICAL_ERROR("close(stdin_fd[0]) failed\n");
		}
		if(close(stdout_fd[1]) != 0) {
			CRITICAL_ERROR("close(stdout_fd[1]) failed\n");
		}
	}
}

void replace_player(size_t player_id, size_t exec_id) {
	clear_player(player_id);
	set_player(player_id, exec_id);
}

bet_t place_initial_bet_api(size_t player_id, size_t player_position, card_t my_hand[13], bets_t previous_bets) {
	//player_perm[player_id];
	init_bets_args_t args;
	bet_t res = {(suit_e) 69, 420};
	args.player_position = player_position;
	memcpy(args.my_hand, my_hand, sizeof(card_t) * 13);
	args.previous_bets = previous_bets;
	if(write(player_in_fd[player_id], &INITIAL_BET_OP, sizeof(char)) != sizeof(char)) {
		fprintf(stderr, "first write failed in initial bet\n");
		dprintf(player_err_fd[player_id], "first write failed in initial bet\n");
		return INVALID_BET;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		fprintf(stderr, "second write failed in initial bet\n");
		dprintf(player_err_fd[player_id], "first write failed in initial bet\n");
		return INVALID_BET;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), INITIAL_BET_TIMEOUT_MS, player_id) != sizeof(res)) {
		fprintf(stderr, "read failed in initial bet\n");
		dprintf(player_err_fd[player_id], "second write failed in initial bet\n");
		return INVALID_BET;
	}
	return res;
}

size_t place_final_bet_api(size_t player_id, suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
	//player_perm[player_id];
	final_bets_args_t args;
	size_t res;
	args.trump = trump;
	args.highest_bidder = highest_bidder;
	memcpy(args.final_bets, final_bets, sizeof(size_t) * 4);
	if(write(player_in_fd[player_id], &FINAL_BET_OP, sizeof(char)) != sizeof(char)) {
		fprintf(stderr, "first write failed in final bet\n");
		dprintf(player_err_fd[player_id], "first write failed in final bet\n");
		return INVALID_FINAL_BET;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		fprintf(stderr, "second write failed in final bet\n");
		dprintf(player_err_fd[player_id], "second write failed in final bet\n");
		return INVALID_FINAL_BET;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), FINAL_BET_TIMEOUT_MS, player_id) != sizeof(res)) {
		fprintf(stderr, "read failed in final bet\n");
		dprintf(player_err_fd[player_id], "read failed in final bet\n");
		return INVALID_FINAL_BET;
	}
	return res;
}

card_t play_card_api(size_t player_id, round_t previous_round, round_t current_round) {
	//player_perm[player_id];
	play_card_args_t args;
	card_t res;
	args.previous_round = previous_round;
	args.current_round = current_round;
	if(write(player_in_fd[player_id], &PLAY_CARD_OP, sizeof(char)) != sizeof(char)) {
		fprintf(stderr, "first write failed in play card\n");
		dprintf(player_err_fd[player_id], "first write failed in play card\n");
		return INVALID_CARD;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		fprintf(stderr, "second write failed in play card\n");
		dprintf(player_err_fd[player_id], "second write failed in play card\n");
		return INVALID_CARD;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), PLAY_CARD_TIMEOUT_MS, player_id) != sizeof(res)) {
		fprintf(stderr, "read failed in play card\n");
		dprintf(player_err_fd[player_id], "read failed in play card\n");
		return INVALID_CARD;
	}
	return res;
}

void game_over_api(size_t player_id, round_t final_round) {
	//player_perm[player_id];
	game_over_args_t args;
	int res;
	args.final_round = final_round;
	if(write(player_in_fd[player_id], &GAME_OVER_OP, sizeof(char)) != 1) {
		fprintf(stderr, "first write failed in game over\n");
		dprintf(player_err_fd[player_id], "first write failed in game over\n");
		return;
	}
	if(write(player_in_fd[player_id], &args, sizeof(args)) != sizeof(args)) {
		fprintf(stderr, "second write failed in game over\n");
		dprintf(player_err_fd[player_id], "second write failed in game over\n");
		return;
	}
	if(read_with_timeout(player_out_fd[player_id], &res, sizeof(res), GAME_OVER_TIMEOUT_MS, player_id) != sizeof(int)) {
		fprintf(stderr, "read failed in game over\n");
		dprintf(player_err_fd[player_id], "read failed in game over\n");
		return;
	}
}
