#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
 
/*
 * uint16_t field (from high to low bits):
 * bit index: 15 14 13        | 12 11 10  9  8  7  6  5  4 |  3  2  1  0
 *            __ __ __        | _9 _8 _7 _6 _5 _4 _3 _2 _1 | _8 _4 _2 _1
 * descr:     nothing in here | possible digits            | set number
 *                              (0 if can be this digit,
 *                               1 if cannot)
 */

// @return Number set in field
int get_digit(uint16_t field)
{
    // get lowest 4 bits
    return field & 0xf;
}

// Sets possible digits in rows or columns
void set_possible_rc(uint16_t arr[9][9], bool is_row)
{
    for (int i = 0; i < 9; i++) {
        uint16_t possible = 0;

        for (int j = 0; j < 9; j++) {
            int digit;
            if (is_row) {
                digit = get_digit(arr[i][j]);
            } else {
                digit = get_digit(arr[j][i]);
            }

            if (digit == 0) {
                continue;
            }

            possible |= (8 << digit);
        }

        for (int j = 0; j < 9; j++) {
            if (is_row) {
                arr[i][j] |= possible;
            } else {
                arr[j][i] |= possible;
            }
        }
    }
}

// Sets possible digits in a square with top-left coords [i][j]
// @return Number of fields with at least 1 possible digit
int set_possible_square(uint16_t arr[9][9], int i, int j)
{
    uint16_t possible_digits = 0;

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int digit = get_digit(arr[i + x][j + y]);
            if (digit == 0) {
                continue;
            }

            possible_digits |= (8 << digit);
        }
    }

    int possible_fields = 9;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            arr[i + x][j + y] |= possible_digits;

            // all possible digits are set to 1, none is possible
            if ((arr[i + x][j + y] & 0x1ff0) == 0x1ff0) {
                possible_fields--;
            }
        }
    }

    return possible_fields;
}

int set_possible_squares(uint16_t arr[9][9])
{
    int possible_fields = 0;

    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            possible_fields = set_possible_square(arr, i, j);
        }
    }

    return possible_fields;
}

// Sets all field's possible digits
// @return Number of fields with at least 1 possible digit
int set_possible_digits(uint16_t arr[9][9])
{
    set_possible_rc(arr, true);
    set_possible_rc(arr, false);
    return set_possible_squares(arr);
}

// Get number of possible digits
int get_possible(uint16_t field)
{
    int possible_digits = 0;
    field >>= 4;

    for (int i = 0; i < 9; i++) {
        if ((field & 1) == 0) {
            possible_digits++;
        }
        field >>= 1;
    }

    return possible_digits;
}

// Finds field with least possible digits,
// sets x and y to its coords
// @return Possible digits
int find_solid(uint16_t arr[9][9], int *x, int *y)
{
    int min = 10;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (get_digit(arr[i][j]) != 0) {
                continue;
            }

            int possible = get_possible(arr[i][j]);
            if (possible < min) {
                min = possible;
                *x = i;
                *y = j;
            }
        }
    }

    return min;
}

void set_digit(uint16_t *field, int i)
{
    uint16_t copy = *field >> 3;
    for (int j = 1; j < 10; j++) {
        copy >>= 1;

        if (copy & 1) {
            continue;
        }

        if (i == 0) {
            *field |= j;
            return;
        }

        i--;
    }
}

bool solve(uint16_t arr[9][9])
{
    if (set_possible_digits(arr) == 0) {
        return true;
    }

    int x, y;
    int min_possible_digits = find_solid(arr, &x, &y);

    uint16_t copy_arr[9][9];
    memcpy(copy_arr, arr, 81 * sizeof(uint16_t));

    for (int i = 0; i < min_possible_digits; i++) {
        set_digit(&copy_arr[x][y], i);

        if (solve(copy_arr)) {
            memcpy(arr, copy_arr, 81 * sizeof(uint16_t));
            return true;
        }

        memcpy(copy_arr, arr, 81 * sizeof(uint16_t));
    }

    return false;
}

void print_sudoku(uint16_t arr[9][9])
{
    for (int i = 0; i < 9; i++) {
        const char *pre = "";
        for (int j = 0; j < 9; j++) {
            printf("%s%d", pre, get_digit(arr[i][j]));
            if ((j + 1) % 3 == 0) {
                pre = " || ";
            } else {
                pre = " | ";
            }
        }

        putchar('\n');

        if (i == 8) {
            break;
        }
        
        if ((i + 1) % 3 == 0) {
            printf("==========||===========||==========\n");
        } else {
            printf("----------||-----------||----------\n");
        }
    }
}

int load_sudoku(FILE *in, uint16_t arr[9][9])
{
   for (int i = 0; i < 9; i++) {
       for (int j = 0; j < 9; j++) {
	        int digit;

            if (fscanf(in ," %d ", &digit) != 1) {
                fprintf(stderr, "invalid format\n");
                return 1;
            }

            if (digit < 0 || digit > 9) {
                fprintf(stderr, "%d - invalid number\n", digit);
                return 1;
            }

            arr[i][j] = digit;
        }
    }

   return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s INPUT_FILE\n", argv[0]);
        return 1;
    }

    uint16_t arr[9][9] = {{0}};

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "error - open %s\n", argv[1]);
        return 1;
    }

    if (load_sudoku(input_file, arr) != 0) {
        fclose(input_file);
        fprintf(stderr, "error - could not load sudoku\n");
        return 1;
    }

    fclose(input_file);

    if (solve(arr)) {
        print_sudoku(arr);
    } else {
        printf("No solution found\n");
    }

    return 0;
}
