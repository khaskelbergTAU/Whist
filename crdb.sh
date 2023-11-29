#!/bin/bash

g++ API/engine_api.c API/grader.cpp -Wall -o grader -g && gcc Bots/min_bot.c API/API.c -o min_bot -Wall && gcc Bots/max_bot.c API/API.c -o max_bot -Wall && gdb --args grader min_bot min_bot min_bot max_bot err0 err1 err2 err3 10
cat err0
cat err1
cat err2
cat err3

