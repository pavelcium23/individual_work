#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    int wg;
    char ws[7];
} Game;

typedef struct {
    size_t words, *index;
    char *buffer;
} Buffer;

void display(const Game *game, const char *guess) {
    int wg = game->wg;

    const char *hangmanDrawings[] = {
        "_______\n",
        "  |\n  O\n",
        "  |\n  O\n  |\n",
        "  |\n  O\n /|\n",
        "  |\n  O\n /|\\\n",
        "  |\n  O\n /|\\\n /\n",
        "  |\n  O\n /|\\\n / \\\n"
    };

    printf("\033[H\033[J"); 
    printf("Hangman:\n%s", hangmanDrawings[wg]);

    
    printf("Word: ");
    size_t len = strlen(guess);
    for (size_t i = 0; i < len; ++i) {
        printf("%c ", guess[i]);
    }
    printf("\n");

    
    printf("Wrong guesses %d: %s\n", wg, game->ws);
}

Buffer readWords(const char *fn) {
    char buffer[512];
    char *bp, *wi, *wl;
    size_t size, elements, bytes, wlen = 0, ilen, len;
    FILE *stream, *tstream;
    FILE *fp = fopen(fn, "r");
    stream = open_memstream(&bp, &size);
    do {
        bytes = fread(buffer, sizeof(char), sizeof buffer, fp);
        fwrite(buffer, sizeof(char), bytes, stream);
    } while (bytes == sizeof buffer && !feof(fp));
    fflush(stream);
    fclose(stream);
    stream = open_memstream(&wi, &ilen);
    tstream = open_memstream(&wl, &bytes);
    for (char *tok = strtok(bp, "\n"); tok; tok = strtok(NULL, "\n")) {
        len = strlen(tok);
        if (len < 3 || len > 50)
            continue;
        for (char *p = tok; *p; ++p) {
            if (isalpha(*p))
                *p = tolower(*p);
            else
                goto next;
        }
        fwrite(&wlen, sizeof wlen, 1, stream);
        wlen += fprintf(tstream, "%s", tok);
        fputc(0, tstream);
        wlen++;
    next:;
    }
    fflush(tstream);
    fclose(tstream);
    fflush(stream);
    fclose(stream);
    free(bp);
    return (Buffer){.words = ilen / sizeof ilen, .index = (size_t *)wi, .buffer = wl};
}

void shuffle(Buffer *buf) {
    size_t rnd, tmp;
    for (size_t i = buf->words - 1; i > 0; --i) {
        rnd = rand() % (i + 1);
        tmp = buf->index[rnd];
        buf->index[rnd] = buf->index[i];
        buf->index[i] = tmp;
    }
}

int main(void) {
    Buffer text = readWords("words_file.txt");
    Game game = {0, {[0 ... 6] = 0}};
    srand(time(NULL));  

    int choice;
    do {
        printf("Hangman Game Menu\n");
        printf("1. Single Player\n");
        printf("2. Two Players\n");
        printf("3. Quit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
          printf("Invalid input. Please enter a number.\n");
          return 1;
        }
        getchar(); 


        switch (choice) {
        case 1: {
            size_t word = 0;
            char *guess = NULL;
            int ch;

            do {
                Game game = {0, {[0 ... 6] = 0}};

                shuffle(&text);
                const char *unknown = text.buffer + text.index[word];
                size_t len = strlen(unknown);

                guess = realloc(guess, len + 1);
                for (size_t i = 0; i < len; ++i)
                    guess[i] = '_';
                guess[len] = 0;

                do {
                    display(&game, guess);
                    printf("Enter a character: ");
                     ch = getchar();

                    int c;
                    while ((c = getchar()) != EOF && c != '\n') {
                        if (isalpha(c)) {
                            ch = tolower(c);
                            break;
                        }
                    }

                    if (isalpha(ch)) {
                        if (!strchr(unknown, ch)) {
                            if (!strchr(game.ws, ch)) {
                                game.ws[game.wg++] = ch;
                            }
                        } else {
                            for (size_t i = 0; i < len; ++i) {
                                if (ch == unknown[i]) {
                                    guess[i] = ch;
                                }
                            }
                        }
                    }
                  
                } while (strcmp(unknown, guess) && game.wg < 6);

                display(&game, guess);

                if (strcmp(unknown, guess))
                    printf("You lost. The word was: %s\n", unknown);
                else
                    printf("You won! With %d misses.\n", game.wg);

                printf("Try again (Y/n)? ");
                int response;
                do {
                    if ((response = getchar()) == EOF) {
                        printf("Error reading input.\n");
                        return 1;
                    }
                } while (response != '\n' && isspace(response));

                if (tolower(response) != 'y') {
                    break;
                }

                game.wg = 0;
                for (size_t i = 0; i < 7; ++i)
                    game.ws[i] = 0;
                ++word;
                word %= text.words; 

            } while (1);

            free(guess);
            break;
        }
        case 2: {
            char wordToGuess[50];
            printf("Player 1, enter a word: ");
            scanf("%s", wordToGuess);
            getchar();  

            size_t len = strlen(wordToGuess);

            char *guess = malloc(len + 1);
            for (size_t i = 0; i < len; ++i)
                guess[i] = '_';
            guess[len] = 0;

            int ch;
            do {
                display(&game, guess);
                printf("Enter a character: ");
                ch = getchar();
                getchar();
                 

                if (isalpha(ch)) {
                    ch = tolower(ch);

                    if (!strchr(wordToGuess, ch)) {
                        if (!strchr(game.ws, ch)) {
                            game.ws[game.wg++] = ch;
                        }
                    } else {
                        for (size_t i = 0; i < len; ++i)
                            if (ch == wordToGuess[i])
                                guess[i] = ch;
                    }
                }
            } while (strcmp(wordToGuess, guess) && game.wg < 6);

            display(&game, wordToGuess);

            if (strcmp(wordToGuess, guess))
                printf("Player 1 wins! The word was: %s\n", wordToGuess);
            else
                printf("Player 2 wins! With %d misses.\n", game.wg);

            free(guess);
            break;
        }
        case 3:
            printf("Game Over\n");
            break;
        default:
            printf("Game Over\n");
        }

    } while (choice != 3);


    free(text.index);
    free(text.buffer);

    return 0;
}
