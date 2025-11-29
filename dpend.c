#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 振子の設定
#define m1 1.0
#define m2 1.0
#define l1 1.0
#define l2 1.0
#define G 9.8

// 画面の設定
#define WIDTH 80 // コンソールの幅
#define HEIGHT 40 // コンソールの高さ
#define SCALE 17 // 1mを何文字分にするか
#define OFFSET_X (WIDTH / 2)
#define OFFSET_Y ((HEIGHT / 2) - 5)

#define RADIUS 2

typedef struct {
  double theta1;
  double theta2;
  double omega1;
  double omega2;
} State;

State func(double t, State s) {
  State dot_s;

  double delta = s.theta1 - s.theta2;
  double M = 2 * m1 + m2;

  double Num1 = - G * M * sin(s.theta1) - m2 * G * sin(s.theta1 - 2 * s.theta2) - 2 * sin(delta) * m2 * (pow(s.omega2, 2) * l2 + pow(s.omega1, 2) * l1 * cos(delta));
  double Den1 = l1 * (M - m2 * cos(2 * delta));
  if (Den1 == 0) {
    printf("Value Error\n");
    exit(1);
  }

  double Num2 = 2 * sin(delta) * (pow(s.omega1, 2) * l1 * (m1 + m2) + G * (m1 + m2) * cos(s.theta1) + pow(s.omega2, 2) * l2 * m2 * cos(delta));
  double Den2 = l2 * (M - m2 * cos(2 * delta));
  if (Den2 == 0) {
    printf("Value Error\n");
    exit(1);
  }

  dot_s.theta1 = s.omega1;
  dot_s.theta2 = s.omega2;
  dot_s.omega1 = Num1 / Den1;
  dot_s.omega2 = Num2 / Den2;

  return dot_s;
}

State rk4_step(double t, State current, double dt) {
  State k1, k2, k3, k4;
  State temp;
  State next_state;

  k1 = func(t, current);

  temp.theta1 = current.theta1 + k1.theta1 * dt * 0.5;
  temp.theta2 = current.theta2 + k1.theta2 * dt * 0.5;
  temp.omega1 = current.omega1 + k1.omega1 * dt * 0.5;
  temp.omega2 = current.omega2 + k1.omega2 * dt * 0.5;
  k2 = func(t + dt * 0.5, temp);
  
  temp.theta1 = current.theta1 + k2.theta1 * dt * 0.5;
  temp.theta2 = current.theta2 + k2.theta2 * dt * 0.5;
  temp.omega1 = current.omega1 + k2.omega1 * dt * 0.5;
  temp.omega2 = current.omega2 + k2.omega2 * dt * 0.5;
  k3 = func(t + dt * 0.5, temp);
  
  temp.theta1 = current.theta1 + k3.theta1 * dt * 1.0;
  temp.theta2 = current.theta2 + k3.theta2 * dt * 1.0;
  temp.omega1 = current.omega1 + k3.omega1 * dt * 1.0;
  temp.omega2 = current.omega2 + k3.omega2 * dt * 1.0;
  k4 = func(t + dt * 1.0, temp);

  next_state.theta1 = current.theta1 + dt * (k1.theta1 + 2 * k3.theta1 + k4.theta1) / 6.0;
  next_state.theta2 = current.theta2 + dt * (k1.theta2 + 2 * k3.theta2 + k4.theta2) / 6.0;
  next_state.omega1 = current.omega1 + dt * (k1.omega1 + 2 * k3.omega1 + k4.omega1) / 6.0;
  next_state.omega2 = current.omega2 + dt * (k1.omega2 + 2 * k3.omega2 + k4.omega2) / 6.0;

  return next_state;
}

// 描画用関数: プレゼンハムのアルゴリズム
void draw_line(char buffer[HEIGHT][WIDTH + 1], int x0, int y0, int x1, int y1, char c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            buffer[y0][x0] = c;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_circle(char buffer[HEIGHT][WIDTH + 1], int cx, int cy, int r, char c) {
  double aspect_ratio = 2.0;

  for (int y = cy - r; y <= cy + r; y++) {
    for (int x = cx - r * 2; x <= cx + r * 2; x++) {
      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        double dx = (double)(x - cx);
        double dy = (double)(y - cy) * aspect_ratio;
        if (dx*dx + dy*dy <= (double)(r*r *4)) {
          buffer[y][x] = c;
        }
      }
    }
  }
}

int main(void) {
  char buffer[HEIGHT][WIDTH + 1];
  double t = 0.0;
  double dt = 0.005;
  double t_max = 30;
  int steps = (int)(t_max / dt);
  
  State s1;
  s1.theta1 = M_PI / 2;
  s1.theta2 = M_PI / 2;
  s1.omega1 = 0.0;
  s1.omega2 = 0.0;
  
  int skip = 6;
  while (1) {
    for (int k = 0; k < skip; k++) {
      s1 = rk4_step(t, s1, dt);
      t += dt;
    }
    
    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        buffer[i][j] = ' ';
      }
      buffer[i][WIDTH] = '\0';
    }  
    
    double x1 = l1 * sin(s1.theta1);
    double y1 = - l1 * cos(s1.theta1);
    double x2 = l1 * sin(s1.theta1) + l2 * sin(s1.theta2);
    double y2 = - l1 * cos(s1.theta1) - l2 * cos(s1.theta2);
    
    int X_or = (int)(0 * SCALE + OFFSET_X);
    int Y_or = (int)(- 0 * SCALE * 0.5 + OFFSET_Y);
    int X_1 = (int)(x1 * SCALE + OFFSET_X);
    int Y_1 = (int)(- y1 * SCALE * 0.5 + OFFSET_Y);
    int X_2 = (int)(x2 * SCALE + OFFSET_X);
    int Y_2 = (int)(- y2 * SCALE * 0.5 + OFFSET_Y);
    
    draw_line(buffer, X_or, Y_or, X_1, Y_1, '#');
    draw_line(buffer, X_1, Y_1, X_2, Y_2,'#');
    // 関節初期案
    /*
    if (X_1 >= 0 && X_1 < WIDTH && Y_1 >= 0 && Y_1 < HEIGHT) {
    buffer[Y_1][X_1] = '@';
    buffer[Y_1 + 1][X_1] = '@';
    buffer[Y_1][X_1 + 1] = '@';
    buffer[Y_1 - 1][X_1] = '@';
    buffer[Y_1][X_1 - 1] = '@';
    }
    if (X_2 >= 0 && X_2 < WIDTH && Y_2 >= 0 && Y_2 < HEIGHT) {
    buffer[Y_2][X_2] = '@';
    buffer[Y_2 + 1][X_2] = '@';
    buffer[Y_2][X_2 + 1] = '@';
    buffer[Y_2 - 1][X_2] = '@';
    buffer[Y_2][X_2 - 1] = '@';
    }
    */
    draw_circle(buffer, X_or, Y_or, 1, '0');
    draw_circle(buffer, X_1, Y_1, 1, '@');
    draw_circle(buffer, X_2, Y_2, RADIUS, '@');
    
    printf("\033[H");
    for (int i = 0; i < HEIGHT; i++) {
      printf("%s\n", buffer[i]);
    }
    usleep(30000);
  }
  return 0;
}







