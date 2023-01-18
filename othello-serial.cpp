#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
using namespace std;

#define BIT 0x1

#define X_BLACK 0
#define O_WHITE 1
#define OTHERCOLOR(c) (1 - (c))

/*
represent game board squares as a 64-bit unsigned integer.
these macros index from a row,column position on the board
to a position and bit in a game board bitvector
*/
#define BOARD_BIT_INDEX(row, col) ((8 - (row)) * 8 + (8 - (col)))
#define BOARD_BIT(row, col) (0x1LL << BOARD_BIT_INDEX(row, col))
#define MOVE_TO_BOARD_BIT(m) BOARD_BIT(m.row, m.col)

/* all of the bits in the row 8 */
#define ROW8                                                                 \
    (BOARD_BIT(8, 1) | BOARD_BIT(8, 2) | BOARD_BIT(8, 3) | BOARD_BIT(8, 4) | \
     BOARD_BIT(8, 5) | BOARD_BIT(8, 6) | BOARD_BIT(8, 7) | BOARD_BIT(8, 8))

/* all of the bits in column 8 */
#define COL8                                                                 \
    (BOARD_BIT(1, 8) | BOARD_BIT(2, 8) | BOARD_BIT(3, 8) | BOARD_BIT(4, 8) | \
     BOARD_BIT(5, 8) | BOARD_BIT(6, 8) | BOARD_BIT(7, 8) | BOARD_BIT(8, 8))

/* all of the bits in column 1 */
#define COL1 (COL8 << 7)

// Check if this move is out of board boundaries
#define IS_MOVE_OFF_BOARD(m) (m.row < 1 || m.row > 8 || m.col < 1 || m.col > 8)

// Never been used
#define IS_DIAGONAL_MOVE(m) (m.row != 0 && m.col != 0)

// Map Move (row, column) to bit format
#define MOVE_OFFSET_TO_BIT_OFFSET(m) (m.row * 8 + m.col)

typedef unsigned long long ull;

/*
game board represented as a pair of bit vectors:
    - one for x_black disks on the board
    - one for o_white disks on the board
*/
typedef struct
{
    ull disks[2];
} Board;

typedef struct
{
    int row;
    int col;
} Move;

typedef struct
{
    int utility;
    bool has_move;
    Move move;
} Action;

Board start = {
    BOARD_BIT(4, 5) | BOARD_BIT(5, 4) /* X_BLACK */,
    BOARD_BIT(4, 4) | BOARD_BIT(5, 5) /* O_WHITE */
};

Move offsets[] = {
    {0, 1} /* right */, {0, -1} /* left */, {-1, 0} /* up */, {1, 0} /* down */, {-1, -1} /* up-left */, {-1, 1} /* up-right */, {1, 1} /* down-right */, {1, -1} /* down-left */
};

int noffsets = sizeof(offsets) / sizeof(Move);
char diskcolor[] = {'.', 'X', 'O', 'I'};

void PrintDisk(int x_black, int o_white)
{
    printf(" %c", diskcolor[x_black + (o_white << 1)]);
}

void PrintBoardRow(int x_black, int o_white, int disks)
{
    if (disks > 1)
    {
        PrintBoardRow(x_black >> 1, o_white >> 1, disks - 1);
    }
    PrintDisk(x_black & BIT, o_white & BIT);
}

void PrintBoardRows(ull x_black, ull o_white, int rowsleft)
{
    if (rowsleft > 1)
    {
        PrintBoardRows(x_black >> 8, o_white >> 8, rowsleft - 1);
    }
    printf("%d", rowsleft);
    PrintBoardRow((int)(x_black & ROW8), (int)(o_white & ROW8), 8);
    printf("\n");
}

void PrintBoard(Board b)
{
    printf("  1 2 3 4 5 6 7 8\n");
    PrintBoardRows(b.disks[X_BLACK], b.disks[O_WHITE], 8);
}

/*
If there is no disk at (row, col) -> place that color of disk. Otherwise, flip it to the other color.
place a disk of color at the position specified by m.row and m,col, flipping the opponents disk there (if any)
*/
void PlaceOrFlip(Move m, Board *b, int color)
{
    ull bit = MOVE_TO_BOARD_BIT(m);
    b->disks[color] |= bit;
    b->disks[OTHERCOLOR(color)] &= ~bit;
}

/*
try to flip disks along a direction specified by a move offset.
the return code is 0 if no flips were done.
the return value is 1 + the number of flips otherwise.
    - verbose = 1: Output Message, domove = 1: Actually change board.
*/
int TryFlips(Move m, Move offset, Board *b, int color, int verbose, int domove)
{
    Move next;
    // offset is one of the elements in Move offsets[]
    next.row = m.row + offset.row;
    next.col = m.col + offset.col;

    if (!IS_MOVE_OFF_BOARD(next))
    {
        ull nextbit = MOVE_TO_BOARD_BIT(next);
        if (nextbit & b->disks[OTHERCOLOR(color)])
        {
            // Flips all the other color's disks among this offset's direction
            int nflips = TryFlips(next, offset, b, color, verbose, domove);
            if (nflips)
            {
                if (verbose)
                    printf("flipping disk at %d,%d\n", next.row, next.col);
                if (domove)
                    PlaceOrFlip(next, b, color);
                return nflips + 1;
            }
        }
        else if (nextbit & b->disks[color])
            return 1;
    }
    return 0;
}

int FlipDisks(Move m, Board *b, int color, int verbose, int domove)
{
    int i;
    int nflips = 0;

    /* try flipping disks along each of the 8 directions */
    for (i = 0; i < noffsets; i++)
    {
        int flipresult = TryFlips(m, offsets[i], b, color, verbose, domove);
        nflips += (flipresult > 0) ? flipresult - 1 : 0;
    }
    return nflips;
}

void ReadMove(int color, Board *b)
{
    Move m;
    ull movebit;
    for (;;)
    {
        printf("Enter %c's move as 'row,col': ", diskcolor[color + 1]);
        scanf("%d,%d", &m.row, &m.col);

        /* if move is not on the board, move again */
        if (IS_MOVE_OFF_BOARD(m))
        {
            printf("Illegal move: row and column must both be between 1 and 8\n");
            PrintBoard(*b);
            continue;
        }
        movebit = MOVE_TO_BOARD_BIT(m);

        /* if board position occupied, move again */
        if (movebit & (b->disks[X_BLACK] | b->disks[O_WHITE]))
        {
            printf("Illegal move: board position already occupied.\n");
            PrintBoard(*b);
            continue;
        }

        /* if no disks have been flipped */
        {
            int nflips = FlipDisks(m, b, color, 1, 1);
            if (nflips == 0)
            {
                printf("Illegal move: no disks flipped\n");
                PrintBoard(*b);
                continue;
            }
            PlaceOrFlip(m, b, color);
            printf("You flipped %d disks\n", nflips);
            PrintBoard(*b);
        }
        break;
    }
}

/*
Return all possible and valid locations of placing disk for each color (Board):
    return the set of board positions adjacent to an opponent's
    disk that are empty. these represent a candidate set of
    positions for a move by color.
*/
Board NeighborMoves(Board b, int color)
{
    int i;
    Board neighbors = {0, 0};
    for (i = 0; i < noffsets; i++)
    {
        ull colmask = (offsets[i].col != 0) ? ((offsets[i].col > 0) ? COL1 : COL8) : 0;
        int offset = MOVE_OFFSET_TO_BIT_OFFSET(offsets[i]);

        if (offset > 0)
        {
            neighbors.disks[color] |=
                (b.disks[OTHERCOLOR(color)] >> offset) & ~colmask;
        }
        else
        {
            neighbors.disks[color] |=
                (b.disks[OTHERCOLOR(color)] << -offset) & ~colmask;
        }
    }
    neighbors.disks[color] &= ~(b.disks[X_BLACK] | b.disks[O_WHITE]);
    return neighbors;
}

/*
Return the number of valid number (int):
    return the set of board positions that represent legal
    moves for color. this is the set of empty board positions
    that are adjacent to an opponent's disk where placing a
    disk of color will cause one or more of the opponent's
    disks to be flipped.
*/
int EnumerateLegalMoves(Board b, int color, Board *legal_moves)
{
    static Board no_legal_moves = {0, 0};
    Board neighbors = NeighborMoves(b, color);
    ull my_neighbor_moves = neighbors.disks[color];
    int row;
    int col;

    int num_moves = 0;
    *legal_moves = no_legal_moves;

    for (row = 8; row >= 1; row--)
    {
        ull thisrow = my_neighbor_moves & ROW8;
        for (col = 8; thisrow && (col >= 1); col--)
        {
            if (thisrow & COL8)
            {
                Move m = {row, col};
                if (FlipDisks(m, &b, color, 0, 0) > 0)
                {
                    legal_moves->disks[color] |= BOARD_BIT(row, col);
                    num_moves++;
                }
            }
            thisrow >>= 1;
        }
        my_neighbor_moves >>= 8;
    }
    return num_moves;
}

bool HumanTurn(Board *b, int color)
{
    Board legal_moves;
    int num_moves = EnumerateLegalMoves(*b, color, &legal_moves);
    if (num_moves > 0)
    {
        ReadMove(color, b);
        return true;
    }
    else
        return false;
}

// Count how many disks for color
int CountBitsOnBoard(Board *b, int color)
{
    ull bits = b->disks[color];
    int ndisks = 0;
    for (; bits; ndisks++)
    {
        bits &= bits - 1; // clear the least significant bit set
    }
    return ndisks;
}

// Decide who wins when game is ended
void EndGame(Board b)
{
    int o_score = CountBitsOnBoard(&b, O_WHITE);
    int x_score = CountBitsOnBoard(&b, X_BLACK);
    printf("Game over. \n");
    if (o_score == x_score)
    {
        printf("Tie game. Each player has %d disks\n", o_score);
    }
    else
    {
        printf("X has %d disks. O has %d disks. %c wins.\n", x_score, o_score,
               (x_score > o_score ? 'X' : 'O'));
    }
}

// Calculate utility score
int utility(Board *b, int color)
{
    return CountBitsOnBoard(b, color) - CountBitsOnBoard(b, OTHERCOLOR(color));
}

vector<Move> get_valid_positions(Board *b, ull move, int color)
{
    vector<Move> valid_positions = {};
    for (auto row = 8; row >= 1; row--)
    {
        ull thisrow = move & ROW8;
        for (auto col = 8; thisrow && (col >= 1); col--)
        {
            if (thisrow & COL8)
            {
                Move m = {row, col};
                if (FlipDisks(m, b, color, 0, 0) > 0)
                    valid_positions.push_back(m);
            }
            thisrow >>= 1;
        }
        move >>= 8;
    }
    return valid_positions;
}

void print_valid_positions(vector<Move> moves, int color)
{
    for (auto i = 0; i < moves.size(); i++)
        printf("Valid position for %c: [%d, %d]\n", diskcolor[color + 1], moves[i].row, moves[i].col);
}

void print_move(Move move, int color)
{
    printf("Try to place %c in [%d, %d]\n", diskcolor[color + 1], move.row, move.col);
}

void place_disk(Board *b, Move move, int color)
{
    FlipDisks(move, b, color, 0, 1);
    PlaceOrFlip(move, b, color);
}

Action alphabeta_negamax(Board b, int color, int depth, int alpha, int beta)
{
    Action best_action;
    if (depth == 0)
    {
        best_action.utility = utility(&b, color);
        return best_action;
    }
    else
    {
        Board legal_moves;
        best_action.utility = INT_MIN;
        EnumerateLegalMoves(b, color, &legal_moves);
        vector<Move> valid_positions = get_valid_positions(&b, legal_moves.disks[color], color);
        int num_of_legal_moves = valid_positions.size();
        best_action.has_move = (num_of_legal_moves > 0) ? true : false;
        for (auto i = 0; i < num_of_legal_moves; i++)
        {
            Board new_board = b;
            place_disk(&new_board, valid_positions[i], color);
            Action current_action = alphabeta_negamax(new_board, OTHERCOLOR(color), depth - 1, -beta, -alpha);
            int current_move_utility = -current_action.utility;
            if (current_move_utility > best_action.utility)
            {
                best_action.move = valid_positions[i];
                best_action.utility = current_move_utility;
            }
            alpha = (best_action.utility > alpha) ? best_action.utility : alpha;
            if (alpha >= beta)
                break;
        }
        if (num_of_legal_moves == 0)
        {
            Board new_board = b;
            if (EnumerateLegalMoves(new_board, OTHERCOLOR(color), &legal_moves) > 0)
            {
                Action current_action = alphabeta_negamax(new_board, OTHERCOLOR(color), depth, -beta, -alpha);
                best_action.move = current_action.move;
                best_action.utility = -current_action.utility;
            }
            else
                best_action.utility = utility(&new_board, color);
        }
    }
    return best_action;
};

// Computer Turn
bool ComputerTurn(Board *b, int color, int depth)
{
    int alpha = -100, beta = 100;
    Action computer_action = alphabeta_negamax(*b, color, depth, alpha, beta);
    Move best_move = computer_action.move;
    int row = best_move.row, column = best_move.col;
    if (computer_action.has_move)
    {
        printf("Computer have placed %c in [row %d, column %d]\n", diskcolor[color + 1], row, column);
        int nflips = FlipDisks(best_move, b, color, 0, 0);
        place_disk(b, best_move, color);
        printf("Computer flipped %d disks\n", nflips);
        PrintBoard(*b);
        return true;
    }
    else
    {
        printf("Computer cannot place %c in current board\n", diskcolor[color + 1]);
        return false;
    }
}

// Handle user input
void handle_input(int id, char &player, int &depth)
{
    printf("Enter h if player%d is a human player, c as a computer player:", id);
    scanf(" %c", &player);
    if (player == 'c')
    {
        printf("Specify the searching depth (integer) between 1 and 60:");
        scanf("%d", &depth);
    }
}

int main(int argc, const char *argv[])
{
    char player1, player2;
    int search_depth1, search_depth2;
    handle_input(1, player1, search_depth1);
    handle_input(2, player2, search_depth2);

    Board gameboard = start;
    bool is_player1_movable, is_player2_movable;
    // PrintBoard(gameboard);
    do
    {
        is_player1_movable = (player1 == 'h') ? HumanTurn(&gameboard, X_BLACK) : ComputerTurn(&gameboard, X_BLACK, search_depth1);
        is_player2_movable = (player2 == 'h') ? HumanTurn(&gameboard, O_WHITE) : ComputerTurn(&gameboard, O_WHITE, search_depth2);
    } while (is_player1_movable || is_player2_movable);

    EndGame(gameboard);

    return 0;
}
