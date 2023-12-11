#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_SIZE 10

typedef char player_t; // 'X' or 'O'
typedef char **board_t; // 'X' or 'O' or '.'

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
            if (board[row][col] == '.') {
                return 0;
            }
        }
    }
    return 1;
}

int has_won(board_t board, player_t player, int size)
{
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board[row][col] != player) {
                goto next_row;
            }
        }
        return 1;
    next_row:;
    }

    for (int col = 0; col < size; ++col) {
        for (int row = 0; row < size; ++row) {
            if (board[row][col] != player) {
                goto next_col;
            }
        }
        return 1;
    next_col:;
    }

    for (int i = 0; i < size; ++i) {
        if (board[i][i] != player) goto next_diagonal;
    }
    return 1;

next_diagonal:;
    for (int i = 0; i < size; ++i) {
        if (board[i][size - 1 - i] != player) return 0;
    }
    return 1;
}

player_t other_player(player_t player)
{
    switch (player) {
    case 'X':
        return 'O';
    case 'O':
        return 'X';
    default:
        assert(0);
    }
}

typedef struct {
    int row;
    int col;
    /* -1 for a loss, 0 for a draw, 1 for a win. */
    int score;
} move_t;

#define MAX_ORD (43046720)

uint8_t computed_moves[MAX_ORD + 1];

uint8_t encode_move(move_t m)
{
    uint8_t b = 0;

    assert(0 <= m.row && m.row <= 9); // Assuming maximum board size of 10
    b |= m.row;

    assert(0 <= m.col && m.col <= 9);
    b |= m.col << 4;

    switch (m.score) {
    case -1:
        b |= 1 << 6;
        break;
    case 0:
        b |= 1 << 5;
        break;
    case 1:
        b |= 1 << 4;
        break;
    }

    return b;
}

move_t decode_move(uint8_t b)
{
    move_t m;

    m.row = b & 0xF;
    m.col = (b & 0xF0) >> 4;
    if (b & 0x10)
        m.score = 1;
    if (b & 0x20)
        m.score = 0;
    if (b & 0x40)
        m.score = -1;
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
            case 'X':
                d = 1;
                break;
            case 'O':
                d = 2;
                break;
            case '.':
                d = 0;
                break;
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
    assert(!has_won(board, player, size));
    assert(!has_won(board, other_player(player), size));

    int o = ord(board, size);

    if (computed_moves[o]) {
        return decode_move(computed_moves[o]);
    }

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board[row][col] == '.') {
                board[row][col] = player;
                if (has_won(board, player, size)) {
                    board[row][col] = '.';
                    computed_moves[o] = encode_move(candidate = (move_t){
                        .row = row,
                        .col = col,
                        .score = 1,
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
                    computed_moves[o] = encode_move(candidate = (move_t){
                        .row = row,
                        .col = col,
                        .score = 0,
                    });
                    return candidate;
                }
                response = best_move(board, other_player(player), size);
                board[row][col] = '.';
                if (response.score == -1) {
                    computed_moves[o] = encode_move(candidate = (move_t){
                        .row = row,
                        .col = col,
                        .score = 1,
                    });
                    return candidate;
                } else if (response.score == 0) {
                    candidate = (move_t){
                        .row = row,
                        .col = col,
                        .score = 0,
                    };
                    no_candidate = 0;
                } else { /* response.score == +1 */
                    if (no_candidate) {
                        candidate = (move_t){
                            .row = row,
                            .col = col,
                            .score = -1,
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
    int size, goFirst;
    printf("Enter the size of the Tic Tac Toe board (max %d): ", MAX_SIZE);
    scanf("%d", &size);

    if (size > MAX_SIZE || size < 3) {
        printf("Invalid board size. Exiting...\n");
        return 1;
    }

    printf("Choose who goes first (1 for user, 2 for computer): ");
    scanf("%d", &goFirst);

    // Allocate memory for the board dynamically
    board_t board = (char **)malloc(size * sizeof(char *));
    for (int i = 0; i < size; ++i) {
        board[i] = (char *)malloc(size * sizeof(char));
    }

    init_board(board, size);

    int move, row, col;
    move_t response;
    player_t userPlayer, computerPlayer, currentPlayer;

    if (goFirst == 1) {
        userPlayer = 'X';
        computerPlayer = 'O';
        currentPlayer = userPlayer;
    } else {
        userPlayer = 'O';
        computerPlayer = 'X';
        currentPlayer = computerPlayer;
    }

    while (1) {
        print_board(board, size);

        if (currentPlayer == userPlayer) {
            print_key(size);
            printf("Enter your move: ");
            scanf("%d", &move);
            row = move / size;
            col = move % size;
            assert(board[row][col] == '.');
            board[row][col] = currentPlayer;
        } else {
            response = best_move(board, currentPlayer, size);
            board[response.row][response.col] = currentPlayer;
        }

        if (has_won(board, currentPlayer, size)) {
            print_board(board, size);
            printf("Player %c has won!\n", currentPlayer);
            break;
        } else if (is_full(board, size)) {
            print_board(board, size);
            printf("Draw.\n");
            break;
        }

        currentPlayer = other_player(currentPlayer);
    }

    // Free dynamically allocated memory
    for (int i = 0; i < size; ++i) {
        free(board[i]);
    }
    free(board);

    return 0;
}
