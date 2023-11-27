#include <stdlib.h>

#include "whist.h"

#define ROUNDS 13

void main_bets() {
    
}

void second_bets() {

}

void play_round() {

}

void get_results() {

}

int main(int argc, char * argv[]) {
    int games = atoi(argv[1]);
    for(int game = 0; game < games; i++) {
        main_bets();
        second_bets();
        for(int round = 0; round < ROUNDS; round++) {
            play_round();
        }
        get_results();
    }
}