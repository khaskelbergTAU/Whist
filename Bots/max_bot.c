#include <stdio.h>
#include <string.h>

#include "whist.h"

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
    wanted_bet.number = (12 * wanted_bet.number) / 10;
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
		if(cards_equal(current_round.cards[i], BET_PASS) && !cards_equal(current_round.cards[(i + 1) % 4], BET_PASS)) {
			opening_suit = current_round.cards[(i + 1) % 4].suit;
			break;
		}
	}
	card_t max_card = {NONE, 0};
	int max_card_index = -1;
    /* find the strongest card in the opening suit */
	for(int i = 0; i < 13; i++) {
		if(hand[i].suit == opening_suit && hand[i].number > max_card.number && hand[i].suit != NONE) {
			max_card = hand[i];
			max_card_index = i;
		}
	}
    /* if there is no card in the opening suit, find the strongest card in the hand */
	if(max_card.suit == NONE) {
		for(int i = 0; i < 13; i++) {
			if(hand[i].number > max_card.number && hand[i].suit != NONE) {
				max_card = hand[i];
				max_card_index = i;
			}
		}
	}
    /* remove the played card from the hand */
	hand[max_card_index] = EMPTY_CARD;
	fprintf(stderr, "Played card: %s %ld\n", max_card.suit == SPADES ? "spades" : (max_card.suit == HEARTS ? "hearts" : (max_card.suit == DIAMONDS ? "diamonds" : "clubs")), max_card.number);
	return max_card;
}

void game_over(round_t final_round) {
	return;
}
