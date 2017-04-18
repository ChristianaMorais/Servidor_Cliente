#include <stdio.h>
#include <unistd.h>
#include "cliente_functions.h"

int main(){

  int op;

  printf("**Menu**\n");
  printf("1) Gerir Saldo\n");
  printf("2) Gerir Lista de Compras\n");
  printf("3) Ver Estatísticas\n");
  printf("4) Logout\n");

  scanf("%d", &op);

  while(op<5){

    switch(op){
    
    case 1:
      balance();
      break;
  
    case 2:
      shopping_list();
      break;
      
    case 3:
      show_estatistics();
      break;
      
    case 4:
      logout();
      break;
      
    }

    printf("**Menu**\n");
    printf("1) Gerir Saldo\n");
    printf("2) Gerir Lista de Compras\n");
    printf("3) Ver Estatísticas\n");
    printf("4) Logout\n");
    
    scanf("%d", &op);

  }

  return 0;
}
