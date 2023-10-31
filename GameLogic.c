#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

enum commands {TwistIt, PourIt, RipIt};

int main() {
    // seed random numbers
    srand(time(NULL));

    // initialize input time and score
    float inputTime = 2;
    int score = 0;

    // loop for the game
    bool isRunning = true;
    if (!isRunning) // game has not started (ie. button needs to be pressed)
        printf("game is not running");
    else { // game is running
        while (true) {
            int command = rand() % 3;
            printf("Score: %d\n", score);

            // twist it
            if (command == TwistIt) {
                printf("Twist It!\n");

                // poll for user input
                Sleep(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime *= 0.95;
                score++;
            }

            // pour it
            else if (command == PourIt) {
                printf("Pour It!\n");

                // poll for user input
                Sleep(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime *= 0.95;
                score++;
            }

            // rip it
            else if (command == RipIt) {
                printf("Rip It!\n");

                // poll for user input
                Sleep(inputTime*1000);

                // adjust inputTime to be faster for next instruction
                inputTime *= 0.95;
                score++;
            }
        }
    }

    return 0;
}