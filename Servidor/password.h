#pragma once
#include "stdio.h"
#include "termios.h"

static struct termios old, new;

void initTermios(int echo){
  tcgetattr(0, &old); 
  new = old; 
  new.c_lflag &= ~ICANON; 
  new.c_lflag &= echo ? ECHO : ~ECHO; 
  tcsetattr(0, TCSANOW, &new); 
}

void resetTermios(void){
  tcsetattr(0, TCSANOW, &old);
}

char getch(void) {
  char ch;
  initTermios(0);
  ch = getchar();
  resetTermios();

  return ch;
}
