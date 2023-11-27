#include <stdlib.h>
#include <bits/stdc++.h>

#include "whist.h"

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

void shuffle_cards(card_t cards[4][13]) {
    card_t all_cards[4 * 13];
    for(int i = 0; i < 4 * 13; i++) {
        all_cards[i].suit = (suit_e) ((i / 13) + 1);
        all_cards[i].number = 2 + (i % 13);
    }
    std::shuffle(all_cards, all_cards + 4 * 13, std::default_random_engine(0));
    for(int i = 0; i < 4 * 13; i++) {
        cards[i / 13][i % 13] = all_cards[i];
    }
}

std::pair<bet_t, size_t> main_bets(card_t cards[4][13]) {
    int last_changed = 0;
    int player = 0;
    bet_t best_bet = {CLUBS, 0};
    bets_t bets;
    for(int i = 0; i < 4; i++) {
        bets.cards[i] = {CLUBS, 0};
    }
    while(last_changed < 4) {
        bet_t bet = place_initial_bet(player, cards[player], bets);
        if(compare_bets(bet, BET_PASS) != 0 && (bet.number < 4 || bet.number > 13 || compare_bets(bet, best_bet) <= 0)) {
            // replace player with bot
            bet_t bet = place_initial_bet(player, cards[player], bets);
        }
        if(compare_bets(bet, best_bet) > 0) {
            best_bet = bet;
            last_changed = 0;
        }
        bets.cards[player] = bet;
        last_changed++;
        player = (player + 1) % 4;
    }
    return std::pair<bet_t, size_t> (best_bet, player);
}

void final_bets(bet_t highest_bet, size_t highest_bidder, size_t final_bets[4]) {
    for(int i = 0; i < 4; i++) {
        final_bets[i] = 0;
    }
    for(int i = 0; i < 4; i++) {
        size_t final_bet = place_final_bet(highest_bet.suit, highest_bidder, final_bets);
        if(final_bet > 13 || (i == 0 && final_bet < highest_bet.number) || (i == 3 && final_bets[0] + final_bets[1] + final_bets[2] + final_bets[3] + final_bet == 13)) {
            // replace player with bot
            final_bet = place_final_bet(highest_bet.suit, highest_bidder, final_bets);
        }
        final_bets[(highest_bidder + i) % 4] = final_bet;
    }
}

void play_round() {

}

void get_results() {

}

int main(int argc, char * argv[]) {
    int games = atoi(argv[1]);
    for(int game = 0; game < games; game++) {
        card_t cards[4][13];
        shuffle_cards(cards);
        std::pair<bet_t, size_t> bet_data = main_bets(cards);
        suit_e trump = bet_data.first.suit;
        size_t highest_bidder = bet_data.second;
        size_t bets[4];
        final_bets(bet_data.first, highest_bidder, bets);
        for(int round = 0; round < 13; round++) {
            play_round();
        }
        get_results();
    }
}