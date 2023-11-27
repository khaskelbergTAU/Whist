#include <stdlib.h>
#include <bits/stdc++.h>

#include "whist.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

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

int compare_cards(card_t card1, card_t card2, suit_e starting_suit, suit_e trump) {
    if(card1.suit == trump && card2.suit != trump) {
        return 1;
    }
    if(card1.suit != trump && card2.suit == trump) {
        return -1;
    }
    if(card1.suit == starting_suit && card2.suit != starting_suit) {
        return 1;
    }
    if(card1.suit != starting_suit && card2.suit == starting_suit) {
        return -1;
    }
    if(card1.number > card2.number) {
        return 1;
    }
    if(card1.number < card2.number) {
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

int hand_contains_card(card_t hand[13], card_t card) {
    for(int i = 0; i < 13; i++) {
        if(compare_bets(hand[i], card) == 0) {
            return 1;
        }
    }
    return 0;
}

int hand_conatins_suit(card_t hand[13], suit_e suit) {
    for(int i = 0; i < 13; i++) {
        if(hand[i].suit == suit) {
            return 1;
        }
    }
    return 0;
}

void remove_card_from_hand(card_t hand[13], card_t card) {
    for(int i = 0; i < 13; i++) {
        if(compare_bets(hand[i], card) == 0) {
            hand[i] = BET_PASS;
            return;
        }
    }
}

int legal_play(card_t hand[13], card_t card, suit_e starting_suit, suit_e trump) {
    if(!hand_contains_card(hand, card)) {
        return 0;
    }
    if(hand_conatins_suit(hand, starting_suit) && card.suit != starting_suit) {
        return 0;
    }
    return 1;
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
    for(int player = 0; player < 4; player++) {
        size_t final_bet = place_final_bet(highest_bet.suit, highest_bidder, final_bets);
        if(final_bet > 13 || (player == 0 && final_bet < highest_bet.number) || (player == 3 && final_bets[0] + final_bets[1] + final_bets[2] + final_bets[3] + final_bet == 13)) {
            // replace player with bot
            final_bet = place_final_bet(highest_bet.suit, highest_bidder, final_bets);
        }
        final_bets[(highest_bidder + player) % 4] = final_bet;
    }
}

round_t play_round(card_t hands[4][13], size_t starting_player, round_t last_round, suit_e trump) {
    round_t current_round;
    for(int i = 0; i < 4; i++) {
        current_round.cards[i] = BET_PASS;
    }
    suit_e starting_suit = NONE;
    for(int player = 0; player < 4; player++) {
        card_t played_card = play_card(last_round, current_round);
        if(player == 0) {
            starting_suit = played_card.suit;
        }
        if(!legal_play(hands[(starting_player + player) % 4], played_card, starting_suit, trump)) {
            // replace player with bot
            played_card = play_card(last_round, current_round);
            if(player == 0) {
                starting_suit = played_card.suit;
            }
        }
        remove_card_from_hand(hands[(starting_player + player) % 4], played_card);
        current_round.cards[(starting_player + player) % 4] = played_card;
    }
    return current_round;
}

size_t get_winner(round_t round, suit_e starting_suit, suit_e trump) {
    size_t winner = 0;
    for(int player = 1; player < 4; player++) {
        if(compare_cards(round.cards[player], round.cards[winner], starting_suit, trump) > 0) {
            winner = player;
        }
    }
    return winner;
}

void update_results(size_t bets[4], size_t takes[4], int total_scores[4]) {
    for(int i = 0; i < 4; i++) {
        if(bets[i] == takes[i]) {
            if(bets[i] == 0) {
                if(bets[0] + bets[1] + bets[2] + bets[3] < 13) {
                    total_scores[i] += 50;
                } else {
                    total_scores[i] += 25;
                }
            } else {
                total_scores[i] += 10 + bets[i] * bets[i];
            }
        } else {
            total_scores[i] -= 10 * ABS(((int) bets[i]) - ((int) takes[i]));
        }
    }
}

int main(int argc, char * argv[]) {
    int games = atoi(argv[1]);
    int total_scores[4] = {0};
    for(int game = 0; game < games; game++) {
        card_t hands[4][13];
        shuffle_cards(hands);
        std::pair<bet_t, size_t> bet_data = main_bets(hands);
        suit_e trump = bet_data.first.suit;
        size_t starting_player = bet_data.second;
        size_t bets[4];
        final_bets(bet_data.first, starting_player, bets);
        round_t last_round;
        for(int i = 0; i < 4; i++) {
            last_round.cards[i] = BET_PASS;
        }
        size_t takes[4] = {0};
        for(int round = 0; round < 13; round++) {
            last_round = play_round(hands, starting_player, last_round, trump);
            starting_player = get_winner(last_round, last_round.cards[starting_player].suit, trump);
            takes[starting_player]++;
        }
        game_over(last_round);
        update_results(bets, takes, total_scores);
    }
}