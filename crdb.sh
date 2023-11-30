#!/bin/bash

g++ API/engine_api.c API/grader.cpp -Wall -o grader -g $2 && gcc Bots/min_bot.c API/API.c -I API -o min_bot -Wall && gcc Bots/a_9.c API/API.c -IAPI -o a_9 && gcc Bots/max_bot.c API/API.c -I API -o max_bot -Wall && gcc Bots/random_bot.c API/API.c -I API -Wall -o random_bot && gcc Bots/super_random_bot.c API/API.c -I API -o super_random_bot -Wall && gdb --args grader a_9 a_9 a_9 a_9 err0 err1 err2 err3 $1 summary
