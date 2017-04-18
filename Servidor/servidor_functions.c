#include <stdio.h>
#include <unistd.h>
#include "servidor_functions.h"

Cliente create_user(){

  Cliente new;

  printf("Nome:\n");
  scanf("%s", new.Nome);

  printf("Contacto:\n");
  scanf("%ld", &new.contacto);

  printf("Username:\n");
  scanf("%s", new.username);

  printf("Password:\n");
  scanf("%s", new.password);

  printf("Utilizador criado com sucesso!\n");

  return new;
}

/*stock(){  //ver o que retorna
 
}

show_estatistics(){  //ver o que retorna

}

void logout(){

}
*/
