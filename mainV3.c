#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define DESIRE_FPS 10
#define FRAME_TIME (1000 / DESIRE_FPS)

#define SCREEN_HEIGHT 20
#define SCREEN_WIDTH 50

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  Point start;
  Point end;
} Line;

typedef struct {
  Point center;
  int radius;
} Circle;

typedef struct {
  Point start;
  int width;
  int height;
} Rectangle;

typedef struct {
  int r;
  int g;
  int b;
} Color;

typedef struct {
  Color fontColor;
  Color backgroundColor;
} Style;
typedef struct {
  char c;
  Style style;
} StyledChar;

typedef struct {
  char *text;
  Style style;
} StyledString;

/* SHARED MEMORY */
static bool EXIT = false;
static StyledChar BUFFER[SCREEN_WIDTH + 1];

/* RENDERER ONLY MEMORY */
static StyledChar VIRTUAL_SCREEN[SCREEN_HEIGHT][SCREEN_WIDTH];

void apply_style(StyledChar *styled_char) {
  printf("\033[38;2;%d;%d;%dm", styled_char->style.fontColor.r,
         styled_char->style.fontColor.g, styled_char->style.fontColor.b);
  printf("\033[48;2;%d;%d;%dm", styled_char->style.backgroundColor.r,
         styled_char->style.backgroundColor.g,
         styled_char->style.backgroundColor.b);
}

static int current_line = 0;
bool styled_char_cmp(StyledChar a, StyledChar b) {
  if (a.c != b.c)
    return 0;
  if (a.style.fontColor.r != b.style.fontColor.r)
    return 0;
  if (a.style.fontColor.g != b.style.fontColor.g)
    return 0;
  if (a.style.fontColor.b != b.style.fontColor.b)
    return 0;
  if (a.style.backgroundColor.r != b.style.backgroundColor.r)
    return 0;
  if (a.style.backgroundColor.g != b.style.backgroundColor.g)
    return 0;
  if (a.style.backgroundColor.b != b.style.backgroundColor.b)
    return 0;
  return 1;
}
void render_line() {
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    if (!styled_char_cmp(VIRTUAL_SCREEN[current_line][i], BUFFER[i])) {
      apply_style(&BUFFER[i]);
      printf("%c", BUFFER[i].c);
      VIRTUAL_SCREEN[current_line][i] = BUFFER[i];
    } else {
      printf("\033[1C");
    }
  }
  printf("\033[0m");
  /* Go down one line, without \n */
  /* printf("\033[1E"); */
  printf("\n");
  BUFFER[SCREEN_WIDTH].c = 0;
}

void clean_screen() { printf("\033[2J"); }

void reset_cursor() { printf("\033[H"); }

void render_screen() {
  current_line = 0;
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    while (!BUFFER[SCREEN_WIDTH].c) {
    }
    render_line();
    current_line++;
  }
}

void *render(void *arg) {
  clean_screen();
  for (;;) {
    if (EXIT)
      return NULL;
    if (BUFFER[SCREEN_WIDTH].c) {
      reset_cursor();
      render_screen();
      usleep(FRAME_TIME * 1000);
    }
  }
}

void draw_line(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int x1, int y1, int x2,
               int y2, char c) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
  float x_inc = dx / (float)steps;
  float y_inc = dy / (float)steps;
  float x = x1;
  float y = y1;
  for (int i = 0; i <= steps; i++) {
    screen[(int)y][(int)x] = c;
    x += x_inc;
    y += y_inc;
  }
}

bool is_inside_screen(int x, int y) {
  return x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT;
}
bool is_inside_circle(int x, int y, int xc, int yc, int r) {
  return ((x - xc) * (x - xc) + (y - yc) * (y - yc) <= r * r);
}

bool is_on_circunference(int x, int y, int xc, int yc, int r, int tolerance) {
  return ((x - xc) * (x - xc) + (y - yc) * (y - yc) <= r * r + tolerance) &&
         ((x - xc) * (x - xc) + (y - yc) * (y - yc) >= r * r - tolerance);
}
void draw_circle(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int xc, int yc,
                 int r, char c) {
  int tolerance = 1;

  int x1 = xc - r;
  int x2 = xc + r;
  int y1 = yc - r;
  int y2 = yc + r;

  for (int x = x1; x <= x2; x++) {
    for (int y = y1; y <= y2; y++) {
      if (is_inside_screen(x, y) &&
          is_on_circunference(x, y, xc, yc, r, tolerance)) {
        screen[y][x] = c;
      }
    }
  }
}

void fill_circle(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int xc, int yc,
                 int r, char c) {
  int x1 = xc - r;
  int x2 = xc + r;
  int y1 = yc - r;
  int y2 = yc + r;

  for (int x = x1; x <= x2; x++) {
    for (int y = y1; y <= y2; y++) {
      if (is_inside_screen(x, y) && is_inside_circle(x, y, xc, yc, r)) {
        screen[y][x] = c;
      }
    }
  }
}

void draw_rectangle(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int x, int y,
                    int width, int height, char c) {
  draw_line(screen, x, y, x + width, y, c);
  draw_line(screen, x, y, x, y + height, c);
  draw_line(screen, x + width, y + height, x + width, y, c);
  draw_line(screen, x + width, y + height, x, y + height, c);
}

void fill_rectangle(char screen[SCREEN_HEIGHT][SCREEN_WIDTH], int x, int y,
                    int width, int height, char c) {
  int x1 = x + width;
  int y1 = y + height;

  for (int i = x; i < x1; i++) {
    for (int j = y; j < y1; j++)
      screen[j][i] = c;
  }
}
void styled_fill_rectangle(StyledChar screen[SCREEN_HEIGHT][SCREEN_WIDTH],
                           Rectangle rect, StyledChar c) {
  int x1 = rect.start.x + rect.width;
  int y1 = rect.start.y + rect.height;

  for (int i = rect.start.x; i < x1; i++) {
    for (int j = rect.start.y; j < y1; j++)
      screen[j][i] = c;
  }
}
void send_to_screen(const char screen[SCREEN_HEIGHT][SCREEN_WIDTH]) {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      BUFFER[j].c = screen[i][j];
      BUFFER[j].style.fontColor.r = 255;
      BUFFER[j].style.fontColor.g = 0;
      BUFFER[j].style.fontColor.b = 0;
      BUFFER[j].style.backgroundColor.r = 0;
      BUFFER[j].style.backgroundColor.g = 0;
      BUFFER[j].style.backgroundColor.b = 0;
    }
    BUFFER[SCREEN_WIDTH].c = 1;
    while (BUFFER[SCREEN_WIDTH].c) {
    }
  }
}

void styled_send_to_screen(
    const StyledChar screen[SCREEN_HEIGHT][SCREEN_WIDTH]) {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      BUFFER[j] = screen[i][j];
    }
    BUFFER[SCREEN_WIDTH].c = 1;
    while (BUFFER[SCREEN_WIDTH].c) {
    }
  }
}
void draw_limits(char screen[SCREEN_HEIGHT][SCREEN_WIDTH]) {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    screen[i][0] = '|';
    screen[i][SCREEN_WIDTH - 1] = '|';
  }
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    screen[0][i] = '-';
    screen[SCREEN_HEIGHT - 1][i] = '-';
  }
  screen[0][0] = '+';
  screen[0][SCREEN_WIDTH - 1] = '+';
  screen[SCREEN_HEIGHT - 1][0] = '+';
  screen[SCREEN_HEIGHT - 1][SCREEN_WIDTH - 1] = '+';
}

/* LOOP ONLY MEMORY */
const static StyledChar EMPTY_CHAR = {' ', {{0, 0, 0}, {0, 0, 0}}};
const static Color RED = {255, 0, 0};
const static Color GREEN = {0, 255, 0};
const static Color BLUE = {0, 0, 255};
const static Color BLACK = {0, 0, 0};
const static Color WHITE = {255, 255, 255};
const static Color YELLOW = {255, 255, 0};
const static Color COLORS[] = {RED, GREEN, BLUE, WHITE, YELLOW};
const static int COLORS_SIZE = sizeof(COLORS) / sizeof(Color);
void *loop(void *arg) {
  StyledChar screen[SCREEN_HEIGHT][SCREEN_WIDTH];
  StyledChar c = {'#', {{255, 0, 0}, {255, 0, 0}}};
  const int frames = 300;
  Rectangle rect = {{0, 0}, 5, 3};
  int color_index = 1;
  int x_speed = 1;
  int y_speed = 1;
  for (int i = 0; i < frames; i++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        screen[y][x] = EMPTY_CHAR;
      }
    }
    rect.start.x += x_speed;
    rect.start.y += y_speed;

    if (rect.start.x + rect.width >= SCREEN_WIDTH || rect.start.x <= 0) {
      x_speed *= -1;
      c.style.fontColor = COLORS[color_index % COLORS_SIZE];
      c.style.backgroundColor = COLORS[color_index % COLORS_SIZE];
      color_index++;
    }
    if (rect.start.y + rect.height >= SCREEN_HEIGHT || rect.start.y <= 0) {
      y_speed *= -1;
      c.style.fontColor = COLORS[color_index % COLORS_SIZE];
      c.style.backgroundColor = COLORS[color_index % COLORS_SIZE];
      color_index++;
    }

    styled_fill_rectangle(screen, rect, c);
    styled_send_to_screen(screen);
  }
  EXIT = true;
  return NULL;
}
void *loop_old(void *arg) {
  char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
  const int frames = 30;
  int line_end = 0;
  int line_start = 0;
  for (int i = 0; i < frames; i++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        screen[y][x] = ' ';
      }
    }
    if (i < frames / 2)
      line_end++;
    else
      line_start++;
    draw_line(screen, line_start, line_start, line_end, line_end, '*');
    fill_circle(screen, line_end, line_end, 2, 'O');
    /* fill_rectangle(screen, line_end, line_end, 5, 3, '#'); */
    draw_limits(screen);
    send_to_screen(screen);
  }
  EXIT = true;
  return NULL;
}

int main() {
  pthread_t loop_thread, render_thread;
  pthread_create(&loop_thread, NULL, loop, NULL);
  pthread_create(&render_thread, NULL, render, NULL);
  pthread_join(loop_thread, NULL);
  pthread_join(render_thread, NULL);
  return 0;
}
