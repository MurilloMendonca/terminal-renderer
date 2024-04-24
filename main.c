#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define DESIRE_FPS 30
#define FRAME_TIME (1000 / DESIRE_FPS)

#define SCREEN_HEIGHT 20
#define SCREEN_WIDTH 50
static char BUFFER[SCREEN_WIDTH + 1];
static bool EXIT = false;

void render_line() {
  printf("%.*s\n", SCREEN_WIDTH, BUFFER);
  BUFFER[SCREEN_WIDTH] = 0;
}

void clean_screen() {
  printf("\033[2J");
  printf("\033[H");
}

void render_screen() {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    while (!BUFFER[SCREEN_WIDTH]) {
    }
    render_line();
  }
}

void *render(void *arg) {
  for (;;) {
    if (EXIT)
      return NULL;
    if (BUFFER[SCREEN_WIDTH]) {
      clean_screen();
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
                    int width, int height, char c){
    int x1 = x + width;
    int y1 = y + height;

    for(int i =x;i<x1;i++){
        for(int j=y;j<y1;j++)
            screen[j][i]=c;
    }
}
void send_to_screen(const char screen[SCREEN_HEIGHT][SCREEN_WIDTH]) {
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    for (int j = 0; j < SCREEN_WIDTH; j++) {
      BUFFER[j] = screen[i][j];
    }
    BUFFER[SCREEN_WIDTH] = 1;
    while (BUFFER[SCREEN_WIDTH]) {
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
void *loop(void *arg) {
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
