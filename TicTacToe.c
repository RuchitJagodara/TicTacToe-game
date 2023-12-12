#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define MAX_BOARD_SIZE (4)

typedef char player_t; // 'X' or 'O'
typedef char board_t[MAX_BOARD_SIZE][MAX_BOARD_SIZE]; // 'X' or 'O' or '.'

void init_board(board_t board, int size)
{
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            board[row][col] = '.';
        }
    }
}

void print_board(board_t board, int size)
{
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            printf("%3c ", board[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

int is_full(board_t board, int size)
{
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board[row][col] == '.') { return 0; }
        }
    }
    return 1;
}

int has_won(board_t board, player_t player, int size, int consecutive)
{
    // Check rows
    for (int row = 0; row < size; ++row) {
        int count = 0;
        for (int col = 0; col < size; ++col) {
            if (board[row][col] == player) {
                count++;
                if (count == consecutive) {
                    return 1;
                }
            } else {
                count = 0;
            }
        }
    }

    // Check columns
    for (int col = 0; col < size; ++col) {
        int count = 0;
        for (int row = 0; row < size; ++row) {
            if (board[row][col] == player) {
                count++;
                if (count == consecutive) {
                    return 1;
                }
            } else {
                count = 0;
            }
        }
    }

    // Check diagonals
    for (int i = 0; i <= size - consecutive; ++i) {
        for (int j = 0; j <= size - consecutive; ++j) {
            int count_diag1 = 0;
            int count_diag2 = 0;
            for (int k = 0; k < consecutive; ++k) {
                if (board[i + k][j + k] == player) {
                    count_diag1++;
                }
                if (board[i + k][j + consecutive - 1 - k] == player) {
                    count_diag2++;
                }
            }
            if (count_diag1 == consecutive || count_diag2 == consecutive) {
                return 1;
            }
        }
    }

    return 0;
}

player_t other_player(player_t player)
{
    switch (player) {
    case 'X': return 'O';
    case 'O': return 'X';
    default: assert(0);
    }
}

typedef struct {
    int row;
    int col;
    /* -1 for a loss, 0 for a draw, 1 for a win. */
    int score;
} move_t;

#define MAX_ORD (43046720)

uint8_t computed_moves[MAX_ORD+1];

uint8_t encode_move(move_t m)
{
    uint8_t b = 0;

    assert(0 <= m.row && m.row <= 3);
    b |= m.row;

    assert(0 <= m.col && m.col <= 3);
    b |= m.col << 2;

    switch (m.score) {
    case -1: b |= 1 << 6; break;
    case 0: b |= 1 << 5; break;
    case 1: b |= 1 << 4; break;
    }

    return b;
}

move_t decode_move(uint8_t b)
{
    move_t m;

    m.row = b & 0x3;
    m.col = (b & 0xC) >> 2;
    if (b & 0x10) m.score = 1;
    if (b & 0x20) m.score = 0;
    if (b & 0x40) m.score = -1;
    return m;
}

int ord(board_t board, int size)
{
    int p = 1;
    int i = 0;
    int d;

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            switch (board[row][col]) {
            case 'X': d = 1; break;
            case 'O': d = 2; break;
            case '.': d = 0; break;
            }
            i += d * p;
            p *= 3;
        }
    }

    return i;
}

move_t best_move(board_t board, player_t player, int size)
{
    move_t response;
    move_t candidate;
    int no_candidate = 1;

    assert(!is_full(board, size));
    assert(!has_won(board, player, size, size)); // Set consecutive to size for a standard tic-tac-toe
    assert(!has_won(board, other_player(player), size, size));

    int o = ord(board, size);

    if (computed_moves[o]) {
        return decode_move(computed_moves[o]);
    }

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board[row][col] == '.') {
                board[row][col] = player;
                if (has_won(board, player, size, size)) {
                    board[row][col] = '.';
                    computed_moves[o] = encode_move(candidate = (move_t) {
                        .row = row,
                        .col = col,
                        .score = 1
                        });
                    return candidate;
                }
                board[row][col] = '.';
            }
        }
    }

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board[row][col] == '.') {
                board[row][col] = player;
                if (is_full(board, size)) {
                    board[row][col] = '.';
                    computed_moves[o] = encode_move(candidate = (move_t) {
                        .row = row,
                        .col = col,
                        .score = 0
                        });
                    return candidate;
                }
                response = best_move(board, other_player(player), size);
                board[row][col] = '.';
                if (response.score == -1) {
                    computed_moves[o] = encode_move(candidate = (move_t) {
                        .row = row,
                        .col = col,
                        .score = 1
                        });
                    return candidate;
                } else if (response.score == 0) {
                    candidate = (move_t) {
                        .row = row,
                        .col = col,
                        .score = 0
                    };
                    no_candidate = 0;
                } else { /* response.score == +1 */
                    if (no_candidate) {
                        candidate = (move_t) {
                            .row = row,
                            .col = col,
                            .score = -1
                        };
                        no_candidate = 0;
                    }
                }
            }
        }
    }
    computed_moves[o] = encode_move(candidate);
    return candidate;
}

void print_key(int size)
{
    int i = 0;
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            printf("%3d ", i++);
        }
        printf("\n");
    }
    printf("\n");
}

int main()
{
    int move, row, col;
    board_t board;
    move_t response;
    player_t current;
    
    // Add variables to store the user's choices
    int userChoice;
    int consecutive;

    printf("Enter the board size (max %d): ", MAX_BOARD_SIZE);
    int boardSize;
    scanf("%d", &boardSize);

    // Check if the chosen board size is valid
    if (boardSize <= 0 || boardSize > MAX_BOARD_SIZE) {
        printf("Invalid board size. Exiting...\n");
        return 1;
    }

    printf("Enter the number of consecutive symbols needed to win: ");
    scanf("%d", &consecutive);

    // Check if the chosen number of consecutive symbols is valid
    if (consecutive <= 0 || consecutive > boardSize) {
        printf("Invalid number of consecutive symbols. Exiting...\n");
        return 1;
    }

    printf("Choose who goes first:\n");
    printf("1. User (X)\n");
    printf("2. Computer (O)\n");
    scanf("%d", &userChoice);

    if (userChoice == 1) {
        current = 'X';
    } else if (userChoice == 2) {
        current = 'O';
    } else {
        printf("Invalid choice. Exiting...\n");
        return 1;
    }

    init_board(board, boardSize);
    while (1) {
        print_board(board, boardSize);

        if (current == 'X' && userChoice == 1) {
            print_key(boardSize);
            printf("Enter your move: ");
            scanf("%d", &move);
            row = move / boardSize;
            col = move % boardSize;
            assert(board[row][col] == '.');
            board[row][col] = current;
        } else {
            response = best_move(board, current, boardSize);
            board[response.row][response.col] = current;
        }

        if (has_won(board, current, boardSize, consecutive)) {
            print_board(board, boardSize);
            printf("Player %c has won!\n", current);
            break;
        } else if (is_full(board, boardSize)) {
            print_board(board, boardSize);
            printf("Draw.\n");
            break;
        }
        current = other_player(current);
    }

    return 0;
}
