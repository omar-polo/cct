#include <curses.h>
#include <err.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Even if I find more "natural" to use spatial coordinates in the form of
 * (x, y) ncurses uses by convention (y, x) e.g. see mvprintw for instance.
 * So everywhere here I'll use the ncurses convention.  You have been warned.
 */

/* utility macros */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* enumerate the possible states of a cell */
enum cell_content {
	NOTHING,
	HIDDEN_TREASURE,
	VISIBLE_TREASURE,
};

/* state variables */
int cheats = 0;
int player_x = 0, player_y = 0;
int board[24][80];

/* fill the board with cells that are NOTHING or HIDDEN_TREASURE */
void
generate_board()
{
	int x, y;

	for (x = 0; x < 80; ++x) {
		for (y = 0; y < 24; ++y) {
			/* return 0 or 1, that is NOTHING or
			 * HIDDEN_TREASURE */
			board[y][x] = arc4random_uniform(2);
		}
	}
}

/* draws the screen */
void
draw()
{
	int x, y, curr;

	if (LINES < 24 || COLS < 80) {
		mvprintw(0, 0, "The screen is too small");
		return;
	}

	for (x = 0; x < 80; ++x) {
		for (y = 0; y < 24; ++y) {
			curr = board[y][x];

			if (player_y == y && player_x == x) {
				mvprintw(y, x, "@");
			} else if (curr == VISIBLE_TREASURE) {
				mvprintw(y, x, "%%");
			} else if (cheats && curr == HIDDEN_TREASURE) {
				mvprintw(y, x, "$");
			} else {
				/* curr is NOTHING or HIDDEN_TREASURE */
				mvprintw(y, x, " ");
			}
		}
	}
}

/* search_tile implement the search functionality on the given cell.  There is
 * a 33% percent of probability that if a cell is HIDDEN_TREASURE it will be
 * revealed. */
void
search_tile(int y, int x)
{
	int s;

	s = board[y][x];

	if (s == NOTHING || s == VISIBLE_TREASURE)
		return;

	/* s is HIDDEN_TREASURE */

	/* we have 33% probability to find something.  This is akin to
	 * `randf() < 0.33` in other languages */
	if (arc4random_uniform(3) == 0)
		board[y][x] = VISIBLE_TREASURE;
}

/* search tries to find treasures in all the cells that surround the player,
 * plus the ones he/she is on.  It's probabilistic: that means that it can
 * fail to find a treasure */
void
search()
{
	int x, y;

	/* we have to check 9 cells */

	/* ,--,--,--.
	 * |  |  |  |
	 * |--------|
	 * |  |xx|  |
	 * |--------|
	 * |  |  |  |
	 * `--------'
	 */

	for (x = player_x - 1; x <= player_x + 1; ++x) {
		for (y = player_y - 1; y <= player_y + 1; ++y) {
			if (x >= 0 && x < 80 && y >= 0 && y < 24) {
				search_tile(y, x);
			}
		}
	}
}

/* move the player x cells horizontally and y cells vertically */
void
player_move(int y, int x)
{
	player_x = MAX(MIN(player_x + x, 80), 0);
	player_y = MAX(MIN(player_y + y, 24), 0);
}

int
main(void)
{
	int c, loop;

	generate_board();

	/* the magic spell used to initialize ncurses */
	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);

	/* main loop */

	draw();

	loop = 1;
	while (loop) {
		if ((c = getch()) == ERR)
			break;

		switch (c) {
		case 'h':
			player_move(0, -1);
			break;

		case 'j':
			player_move(+1, 0);
			break;

		case 'k':
			player_move(-1, 0);
			break;

		case 'l':
			player_move(0, +1);
			break;

		case 'c':
			cheats = !cheats;
			break;

		case 's':
			search();
			break;

		case 'q':
			loop = 0;
			break;
		}

		draw();
	}

	/* exit */
	endwin();

	return 0;
}
