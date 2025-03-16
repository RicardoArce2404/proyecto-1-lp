#include <ncurses.h>
#include "array.c"
#include "string.c"

#ifndef UI  // Ui!
#define UI

typedef struct Cell {
  int row;
  int col;
} Cell;

// Prints a straight line between 2 cells (assumes that they have the same row or same column).
// Only works if the start cell is above and to the left of the end cell.
void printLine(Cell start, Cell end) {
  if (start.row > end.row || start.col > end.col) {
    return;  // This is to ensure that the cells are passed in the correct order.
  }
  if (start.row == end.row) {
    for (int i = start.col; i <= end.col; i++) {
      move(start.row, i);
      printw("█");
    }
  } else if (start.col == end.col) {
    for (int i = start.row; i <= end.row; i++) {
      move(i, start.col);
      printw("█");
    }
  } else {
    return;
  }
}

// Shows a menu and wait for the user to select an option. Returns that option's number.
int showMenu(PtrArray *options) {
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  int width = 0;
  int height = options->len;
  for (int i = 0; i < options->len; i++) {
    String *line = options->data[i];
    if (line->len > width) {
      width = line->len;
    }
  }
  // Extra space to correct visual issues.
  width += 4;
  height += 3;

  // ulCornerRow stands for "Up-left corner's row".
  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};
  Cell urCorner = {ulCornerRow, ulCornerCol + width};
  Cell dlCorner = {ulCornerRow + height, ulCornerCol};
  Cell drCorner = {ulCornerRow + height, ulCornerCol + width};
  printLine(ulCorner, urCorner);
  printLine(dlCorner, drCorner);
  printLine(ulCorner, dlCorner);
  printLine(urCorner, drCorner);

  int selectedOption = 0;
  int keyPressed = 0;
  curs_set(0);  // Hides the cursor.
  do {
    for (int i = 0; i < options->len; i++) {
      move(ulCornerRow + i + 2, ulCornerCol + 2);
      String *line = options->data[i];
      if (i == selectedOption) {
        printw(">%.*s", line->len, line->text);
      } else {
        printw(" %.*s", line->len, line->text);
      }
    }
    refresh();

    keyPressed = getch();
    int lastPosition = options->len - 1;
    if (keyPressed == KEY_UP) {
      selectedOption = (selectedOption == 0) ? lastPosition : selectedOption - 1;
    } else if (keyPressed == KEY_DOWN) {
      selectedOption = (selectedOption == lastPosition) ? 0 : selectedOption + 1;
    }
  } while (keyPressed != '\n');
  curs_set(1);  // Makes cursor visible again.
  return selectedOption;
}

#endif  // Ui!!
// + ¡¿Ui?!
// - Ui.
