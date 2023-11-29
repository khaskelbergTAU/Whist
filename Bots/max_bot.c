#include <stdio.h>
#include <string.h>

#include "../API/whist.h"

card_t hand[13];
size_t position;

int cards_equal(card_t card1, card_t card2) {
    return (card1.suit == card2.suit) && (card1.number == card2.number);
}

bet_t place_initial_bet(size_t player_position, card_t my_hand[13], bets_t previous_bets) {
    memcpy(hand, my_hand, sizeof(card_t) * 13);
    position = player_position;
    bet_t bet = {CLUBS, 4};
    for(int i = 0; i < 4; i++) {
        if(!cards_equal(previous_bets.cards[i], BET_PASS) && !cards_equal(previous_bets.cards[i], BET_NOT_PLAYED)) {
            bet = BET_PASS;
        }
    }
    return bet;
}

size_t place_final_bet(suit_e trump, size_t highest_bidder, size_t final_bets[4]) {
    if(highest_bidder == position) {
        return 4;
    }
    if(final_bets[0] + final_bets[1] + final_bets[2] + final_bets[3] == 12) {
        return 2;
    }
    return 1;
}

card_t play_card(round_t previous_round, round_t current_round) {
    suit_e opening_suit = NONE;
    for(int i = 0; i < 4; i++) {
        if(cards_equal(current_round.cards[i], BET_PASS) && !cards_equal(current_round.cards[(i + 1) % 4], BET_PASS)) {
            opening_suit = current_round.cards[(i + 1) % 4].suit;
            break;
        }
    }
    card_t max_card = {NONE, 0};
    int max_card_index = -1;
    for(int i = 0; i < 13; i++) {
        if(hand[i].suit == opening_suit && hand[i].number > max_card.number && hand[i].suit != NONE) {
            max_card = hand[i];
            max_card_index = i;
        }
    }
    if(max_card.suit == NONE) {
        for(int i = 0; i < 13; i++) {
            if(hand[i].number > max_card.number && hand[i].suit != NONE) {
                max_card = hand[i];
                max_card_index = i;
            }
        }
    }
    hand[max_card_index] = EMPTY_CARD;
    fprintf(stderr, "Played card: %s %ld\n", max_card.suit == SPADES ? "spades" : (max_card.suit == HEARTS ? "hearts" : (max_card.suit == DIAMONDS ? "diamonds" : "clubs")), max_card.number);
    return max_card;
}

void game_over(round_t final_round) {
    return;
}