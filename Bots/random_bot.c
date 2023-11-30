#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "whist.h"

/* return a random card
if there are any cards of suit starting_suit then one of them is returned
otherwise another random card is returned */
card_t get_random_card(card_t hand[13], suit_e starting_suit) {
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
size_t wanted_bet_number;

/* return 1 if bet1 is stronger than bet2, -1 if bet2 is stronger than bet1, and 0 if they are equal */
int compare_bets(bet_t bet1, bet_t bet2) {
    if(bet1.number > bet2.number) {
        return 1;
    }
    if(bet1.number < bet2.number) {
        return -1;
    }
    if(bet1.suit < bet2.suit) {
        return 1;
    }
    if(bet1.suit > bet2.suit) {
        return -1;
    }
    return 0;
}

/* check if two cards are equal */
int cards_equal(card_t card1, card_t card2) {
	return (card1.suit == card2.suit) && (card1.number == card2.number);
}

bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets) {
	/* initialize global fields */
	memcpy(hand, my_hand, sizeof(card_t) * 13);
	position = player_position;
    bet_t wanted_bet = {NONE, 0};
    size_t suit_count[4] = {0};
	/* get a score for the hand */
    for(int i = 0; i < 13; i++) {
        suit_count[hand[i].suit - 1]++;
        wanted_bet.number += (hand[i].number > 4 ? hand[i].number - 4 : 0);
    }
    wanted_bet.number /= 10;
    wanted_bet_number = wanted_bet.number;
    if(wanted_bet.number < 4) {
        return BET_PASS;
    }
	/* find the suit with the most cards in hand and take it as trump */
    wanted_bet.suit = 1;
    for(int i = 2; i < 5; i++) {
        if(suit_count[i - 1] > suit_count[wanted_bet.suit - 1]) {
            wanted_bet.suit = i;
        }
    }
	/* check if the bet is better than the current highest bet */
    for(int i = 0; i < 4; i++) {
        if(compare_bets(wanted_bet, previous_bets.cards[i]) <= 0) {
            wanted_bet = BET_PASS;
        }
    }
    return wanted_bet;
}

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
	/* bet the wanted amount, if the sum becomes 13 then bet one more */
	if(final_bets[0] + final_bets[1] + final_bets[2] + final_bets[3] == 13 - wanted_bet_number) {
        return wanted_bet_number + 1;
    }
    return wanted_bet_number;
}

card_t play_card(round_t previous_round, round_t current_round) {
	suit_e opening_suit = NONE;
	/* find the opening suit */
	for(int i = 0; i < 4; i++) {
		if(cards_equal(current_round.cards[i], EMPTY_CARD) && !cards_equal(current_round.cards[(i + 1) % 4], EMPTY_CARD)) {
			opening_suit = current_round.cards[(i + 1) % 4].suit;
			break;
		}
	}
	/* get a random card to play */
	card_t random_card = get_random_card(hand, opening_suit);
	/* remove the card from the hand */
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
