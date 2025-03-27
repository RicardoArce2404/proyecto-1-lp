#define _XOPEN_SOURCE 700
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
      printw("─");
    }
  } else if (start.col == end.col) {
    for (int i = start.row; i <= end.row; i++) {
      move(i, start.col);
      printw("│");
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
  move(ulCorner.row, ulCorner.col);
  printw("╭");
  move(urCorner.row, urCorner.col);
  printw("╮");
  move(dlCorner.row, dlCorner.col);
  printw("╰");
  move(drCorner.row, drCorner.col);
  printw("╯");
  refresh();
}

// Fills a block of the screen with white space.
void clearBlock(Cell ulCorner, int width, int height) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      move(ulCorner.row + i, ulCorner.col + j);
      printw(" ");
    }
  }
  refresh();
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
    if (getVisualLen(line) > width) {
      width = getVisualLen(line);
    }
  }
  // Extra space to correct visual issues.
  width += 4;
  height += 3;

  // ulCornerRow stands for "Up-left corner's row".
  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};
  clear(); // Clears the screen.
  printRectangle(ulCorner, width, height);

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
  return selectedOption;
}

typedef enum UiInputType {
  UiTextInput,
  UiNumberInput,
  UiFilenameInput
} UiInputType;

// Prints a string centered in the given width.
void printCentered(String *str, int width) {
  if (getVisualLen(str) > width) {
    printw("%.*s...", width - 3, str->text);
  } else {
    int blankSpace = width - getVisualLen(str);
    int leftMargin = blankSpace - blankSpace / 2;
    int rightMargin = blankSpace - leftMargin;
    move(getcury(stdscr), getcurx(stdscr) + leftMargin);
    printw("%.*s", str->len, str->text);
    move(getcury(stdscr), getcurx(stdscr) + rightMargin);
  }
  refresh();
}

// Shows an input textbox and waits for the user to enter valid text according
// to inputType. Appears at the specified row and gets wider as needed.
// If isError == 1, the text box appears in red.
String *showInput(String *title, int row, int isError) {
  if (row < 1) {
    return NULL; // In this case there's no room for top border.
  }
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  if (row >= tHeight - 1) { // -1 to account the bottom border.
    return NULL;
  }
  int MIN_WIDTH = 30;

  int ulCornerRow = row - 1;
  int ulCornerCol = (tWidth - MIN_WIDTH) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};

  if (isError) {
    init_pair(1, COLOR_RED, -1);
    attron(COLOR_PAIR(1));
    printRectangle(ulCorner, MIN_WIDTH, 2);
    attroff(COLOR_PAIR(1));
  } else {
    printRectangle(ulCorner, MIN_WIDTH, 2);
  }

  move(ulCornerRow, ulCornerCol);
  printCentered(title, MIN_WIDTH);

  echo();
  curs_set(1);
  char input[256] = {0};
  move(row, ulCornerCol + 1);
  scanw("%255[^\n]", input);
  curs_set(0);
  noecho();
  input[255] = 0;
  if (input[0] == '\0' || input[0] == '\n') {
    return NULL;
  }
  String *str = newString(input);
  clearBlock(ulCorner, MIN_WIDTH + 1, 3);
  return str;
}

// To be used in showScrollableList only.
void printRow(PtrArray *row, IntArray *cellWidths) {
  String *firstCell = (String *)(row->data[0]);
  printCentered(firstCell, cellWidths->data[0]);
  for (int i = 1; i < row->len; i++) {
    printw("│");
    printCentered((String *)(row->data[i]), cellWidths->data[i]);
  }
}

// Shows a scrollable list. Assumes that the headings and each row have the same
// length. columnWidths contains the width of each collumn from left to right.
// Returns numVisibleRows for validation purposes in caller function.
int showScrollableList(String *title, PtrArray *headings, PtrArray *rows,
                       IntArray *columnWidths, int initialRow) {
  if (title == NULL || headings == NULL || rows == NULL) {
    return -1;
  }
  if (headings->len != columnWidths->len) {
    return -2;
  }
  for (int i = 0; i < rows->len; i++) {
    PtrArray *row = rows->data[i];
    if (headings->len != row->len) {
      return -3;
    }
  }
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  // - 4 rows for header
  // - 1 row for bottom border
  // - 3 rows for input textbox
  // - 4 rows for bottom help bar
  // 4 + 1 + 3 + 4 = 12 rows reserved. The rest is for product rows.
  const int MAX_ROWS = tHeight - 12;
  if (MAX_ROWS < 1) {
    return -4;
  }
  int width = 0; // Total width of the list.
  for (int i = 0; i < columnWidths->len; i++) {
    width += columnWidths->data[i];
  }
  // This is to consider the width of vertical column separators and left
  // and right borders.
  width += headings->len + 1;
  int height = 3; // Bottom border, horizontal sep and headings row.
  if (rows->len > MAX_ROWS) {
    height += MAX_ROWS;
  } else {
    height += rows->len;
  }
  int numVisibleRows = height - 3;
  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};
  clear();
  printRectangle(ulCorner, width, height);
  Cell headerLeftCell = {ulCornerRow + 2, ulCornerCol + 1};
  Cell headerRightCell = {ulCornerRow + 2, ulCornerCol + width - 1};
  // Line that separates headings and rows.
  printLine(headerLeftCell, headerRightCell);

  move(ulCornerRow + 1, ulCornerCol + 1);
  printRow(headings, columnWidths);
  /*String *bottomBar1 = newString("Agregar producto: +     |  Eliminar
   * producto: - ");*/
  /*String *bottomBar2 = newString("Subir: <flecha arriba>  |  Bajar: <flecha
   * abajo>");*/
  /*move(tHeight - 2, 0);*/
  /*printCentered(bottomBar1, tWidth);*/
  /*move(tHeight - 1, 0);*/
  /*printCentered(bottomBar2, tWidth);*/
  /*deleteString(bottomBar1);*/
  /*deleteString(bottomBar2);*/

  curs_set(0);
  for (int i = 0; i < numVisibleRows; i++) {
    move(ulCornerRow + 3 + i, ulCornerCol + 1);
    printRow(rows->data[initialRow + i], columnWidths);
  }
  curs_set(1);
  refresh();
  return numVisibleRows;
}

#endif // Ui!!
// + ¡¿Ui?!
// - Ui.
