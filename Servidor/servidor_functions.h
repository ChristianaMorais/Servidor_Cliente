#include <stdio.h>
#include <unistd.h>

typedef struct cliente{
  char Nome[30];
  long contacto;
  char username[30];
  char password[30];

}Cliente;

Cliente create_user();

void stock(); //ver o que retorna

  /*
show_estatistics();  //ver o que retorna

void logout();
*/
