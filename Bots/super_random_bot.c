#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "../API/whist.h"

card_t get_random_card(card_t hand[13], suit_e starting_suit) {
	srand(time(0));
	if(rand() % 20 == 0) {
		int rand_res = rand() % 3;
		if(rand_res == 0) {
			card_t res;
			res.suit = (suit_e)rand();
			res.number = (size_t)rand();
			return res;
		} else if(rand_res == 1) {
			card_t res;
			res.suit = rand() % 4 + 1;
			res.number = rand() % 13 + 2;
			return res;
		} else {
			int i;
			do {
				i = rand() % 13;
			} while(hand[i].suit == NONE);
			return hand[i];
		}
	}
	if(starting_suit != NONE) {
		int starting_suit_count = 0;
		for(int i = 0; i < 13; i++) {
			if(hand[i].suit == starting_suit) {
				starting_suit_count++;
			}
		}
		if(starting_suit_count != 0) {
			srand(time(NULL));
			int i = rand() % starting_suit_count;
			int j = 0;
			while(1) {
				while(hand[j].suit != starting_suit) {
					j++;
				}
				if(!i) break;
				i--;
				j++;
			}
			return hand[j];
		}
	}
	int cards_left = 0;
	for(int i = 0; i < 13; i++) {
		if(!(hand[i].suit == EMPTY_CARD.suit && hand[i].number == EMPTY_CARD.number)) {
			cards_left++;
		}
	}
	srand((unsigned)time(0));
	int i = rand() % cards_left;
	int j = 0;
	while(1) {
		while(hand[j].suit == NONE) {
			j++;
		}
		if(!i) break;
		i--;
		j++;
	}
	return hand[j];
}

card_t hand[13];
size_t position;

int cards_equal(card_t card1, card_t card2) {
	return (card1.suit == card2.suit) && (card1.number == card2.number);
}

bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets) {
	memcpy(hand, my_hand, sizeof(card_t) * 13);
	position = player_position;
	bet_t bet = {CLUBS, 4};
	int min_bet = 4;
	for(int i = 0; i < 4; i++) {
		if(previous_bets.cards[i].number > min_bet) min_bet = previous_bets.cards[i].number;
	}
	min_bet++;
	if(min_bet < 13) {
		bet.number = rand() % (13 - min_bet) + min_bet;
	}
	return bet;
}

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
	if(position == 3) {
		size_t val;
		do {
			val = rand() % 14;
		} while(final_bets[0] + final_bets[1] + final_bets[2] + val == 13);
		return val;
	}
	return rand() % 14;
}

card_t play_card(round_t previous_round, round_t current_round) {
	suit_e opening_suit = NONE;
	for(int i = 0; i < 4; i++) {
		if(cards_equal(current_round.cards[i], EMPTY_CARD) && !cards_equal(current_round.cards[(i + 1) % 4], EMPTY_CARD)) {
			opening_suit = current_round.cards[(i + 1) % 4].suit;
			break;
		}
	}
	card_t random_card = get_random_card(hand, opening_suit);
	for(int i = 0; i < 13; i++) {
		if(hand[i].suit == random_card.suit && hand[i].number == random_card.number) {
			hand[i] = EMPTY_CARD;
			break;
		}
	}
	fprintf(stderr, "Played card: %s %ld\n", random_card.suit == SPADES ? "spades" : (random_card.suit == HEARTS ? "hearts" : (random_card.suit == DIAMONDS ? "diamonds" : "clubs")), random_card.number);
	return random_card;
}

void game_over(round_t final_round) {
	return;
}

