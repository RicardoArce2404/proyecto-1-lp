#include "array.c"
#include "string.c"
#include <ncurses.h>
#include <wchar.h>

#ifndef UI // Ui!
#define UI

typedef struct Cell {
  int row;
  int col;
} Cell;

// Prints a straight line between 2 cells (assumes that they have the same row
// or same column). Only works if the start cell is above and to the left of the
// end cell.
void printLine(Cell start, Cell end) {
  if (start.row > end.row || start.col > end.col) {
    return; // This is to ensure that the cells are passed in the correct order.
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

// Prints a rectangle using the specified cell as up-left corner.
void printRectangle(Cell ulCorner, int width, int height) {
  Cell urCorner = {ulCorner.row, ulCorner.col + width};
  Cell dlCorner = {ulCorner.row + height, ulCorner.col};
  Cell drCorner = {ulCorner.row + height, ulCorner.col + width};
  printLine(ulCorner, urCorner);
  printLine(dlCorner, drCorner);
  printLine(ulCorner, dlCorner);
  printLine(urCorner, drCorner);
}

// Shows a menu and waits for the user to select an option. Returns that
// option's number.
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
  curs_set(0); // Hides the cursor.
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
      selectedOption =
          (selectedOption == 0) ? lastPosition : selectedOption - 1;
    } else if (keyPressed == KEY_DOWN) {
      selectedOption =
          (selectedOption == lastPosition) ? 0 : selectedOption + 1;
    }
  } while (keyPressed != '\n');
  curs_set(1); // Makes cursor visible again.
  clear();     // Clears the screen.
  return selectedOption;
}

typedef enum UiInputType {
  UiTextInput,
  UiNumberInput,
  UiFilenameInput
} UiInputType;

// Shows an input textbox and waits for the user to enter valid text according
// to inputType. Appears at the specified row and gets wider as needed.
String *showInput(UiInputType inputType, int row) {
  int x = inputType;
  x++;
  if (row < 1) {
    return NULL; // In this case there's no room for top border.
  }
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  if (row >= tHeight - 1) {  // -1 to account the bottom border.
    return NULL;
  }
  getmaxyx(stdscr, tHeight, tWidth);
  int MIN_WIDTH = 10;

  int ulCornerRow = row - 1;
  int ulCornerCol = (tWidth - MIN_WIDTH) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};
  printRectangle(ulCorner, MIN_WIDTH, 3);

  echo();
  char input[256] = {0};
  move(row, ulCornerCol + 1);
  refresh();
  // Reads 255 characters (or until \n), stores them in filename, reads one more
  // and reads one more character and stores it in endChar.
  scanw("%255[^\n]", input);
  noecho();
  input[255] = 0;
  String *str = newString(input);
  return str;
}

#endif // Ui!!
// + ¡¿Ui?!
// - Ui.
