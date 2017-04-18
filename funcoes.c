//funcoes para o servidor

void startServer(){
  int sockfd, *newsock , client, c;
  struct sockaddr_in serv_addr, cli_addr;//estrutura para guardar os edereços de ip do servidor e cliente

  // iniciar socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) { // verificação de estado da socket
    perror("ERRO - Impossivel de abrir socket");
    exit(EXIT_FAILURE);
  }

  //Definir parametros de serv_addr
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY; //definição de ip 
  serv_addr.sin_port = htons( portReader() );//defenição de porta 

  //juntar tudo
  if( bind(sockfd,(struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0){
    perror(ANSI_COLOR_RED"ERRO"ANSI_COLOR_RESET" - Bind falhou");
    exit(EXIT_FAILURE);

  }
  listen(sockfd , 15); //espera por ligação o numero e o maximo de ligaçoes

  //consola de administração do servidor 
  pthread_t menu_thread;
  if( pthread_create( &menu_thread , NULL ,  serveradminpanel , NULL) < 0){
    perror(ANSI_COLOR_RED"ERRO"ANSI_COLOR_RESET" - insucesso a criar thread");
    exit(EXIT_FAILURE);
  }

  puts("Servidor iniciado.");
  puts("A espera de ligação\n");
  //puts("Digite -q para desligar em segurança o servidor.");
  c = sizeof(struct sockaddr_in); 
  while( (client = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&c)) ){
    pthread_t sniffer_thread;
    newsock = malloc(4);
    *newsock = client;

    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) newsock) < 0){ //criar uma thread para cada utilizador
      perror(ANSI_COLOR_RED"ERRO"ANSI_COLOR_RESET" - insucesso a criar thread");
      exit(EXIT_FAILURE);
    }
  }

  if (client < 0){
    perror(ANSI_COLOR_RED"ERRO"ANSI_COLOR_RESET"- falha ao aceitar ligação");
    exit(EXIT_FAILURE);
  }
}


void *connection_handler(void *socket_desc) {

  int sock = *(int*)socket_desc, i,n=0;
  int read_size;
  int user_code=-1;
  int coderr=0; //numero de vezes que a mensagem falhou
  char message[1000] , client_message[2000], menuact[3];
  char login[30],pass[30];

  /*Autenticação do nome do utilizador*/
  //n contador de logins
  while(n<=3){ //verificação de utilizador
    bzero(message, sizeof(message));
    recv(sock , login , 30 , 0);
    user_code=findUser(login); //compara o nome dado com a base de dados
    ++n;
    if (user_code!=-1){
      bzero(message, sizeof(message));
      strcat(message,"2");//codigo para dizer que o login foi aceite
      if (onlineuserChecker(user_code)){ //verificar se o utilizador esta online
	strcat(message,"1"); //envio de codigo para desligar a ligação
	write(sock , message , strlen(message));
	puts("Ligação desligada, o utilizador já se encontrava online.");
	close(sock);
	free(socket_desc);
	return 0;
      }
      else
	strcat(message,"0");
      write(sock , message , strlen(message));
      break;
    }
    else{
      if (n==3){
	bzero(message, sizeof(message));
	strcat(message,"1"); //envio de codigo para desligar a ligação
	write(sock , message , strlen(message));
	puts("Ligação desligada, login sem sucesso.");
	close(sock);
	free(socket_desc);
	return 0;
      }
      else{ //não acertou no utilizador, volta a correr o ciclo
	bzero(message, sizeof(message));
	strcat(message,"0"); //codigo para continuar o ciclo
	write(sock , message , strlen(message));  
      }
    }
  }

  /*Autenticação da passwoard*/
  n=0;//contador de tentaivas
  while(n<=3){
    bzero(pass, sizeof(pass));
    recv(sock , pass , 30 , 0);
    if (strcmp(pass,Dados[user_code].password)==0) //confirma se a passwoard corresponde com o utilizador
      break;
    ++n;
    if (n==3){//significa que exedeu  as tentativas permitidas
      bzero(message, sizeof(message));
      strcat(message,"1");
      write(sock , message , strlen(message));
      puts("Ligação desligada, login sem sucesso.");
      free(socket_desc);
      return 0;
    }
    else{
      bzero(message, sizeof(message));
      strcat(message,"0");//indica que nao acertou e que o ciclo vai continuar
      write(sock , message , strlen(message));
    }
  }


  bzero(message, sizeof(message));
  strcat(message,"2");
  write(sock , message , strlen(message)); //envia o codigo que a passoward foi aceite
  socketSaver(user_code, sock);
  printf(ANSI_COLOR_GREEN"* Utilizador %s  iniciou a sua sessão com sucesso."ANSI_COLOR_RESET"\n",Dados[user_code].login );
  
  recv(sock , message , 30 , 0);//e importante este contacto para que o servidor saiba que o cliente ja se econtra pronto para receber sms's
  offlineRECEIVER(sock,user_code); //Envia mensagens pendente
  
  /*A variavel coderr que vai aparecer várias vezes no próximo switch é contador, que nos permite fehcar a thread automaticamente apos 3 codigos não reconhecidos*/

  /*while(n!=9){
    bzero(menuact, sizeof(menuact));
    if(recv(sock ,menuact , 1 , 0)>0)//verificar se o client enao se desligou
    n=menuact[0]-'0';
    else
    n=9;
    switch(n){
    case 1://Indica quais são os utilizadores online
    onlineusers(sock);
    coderr=0;
    break;
    case 2://Envia uma sms
    i=smssender(user_code,sock);
    coderr=0;
    break;
    case 9:
    break;//o 9 como n ira parar o ciclo
    case 4://Mudança de password
    remoteChanger(user_code,sock);
    coderr=0;
    break;
    default:
    if (coderr>=3){//o o utilizador pode dar ate 3 comunicaçoes desconhecidas seguidas , doutra forma o programa fecha automaticamente
    n=9;
    }
    else{
    perror("Erro na comunicação.");
    ++coderr;
    }
    break; // assim so vai desligar esta socket
    }*/
}
  socketSaver(user_code, -1); //marca utilizador como offline
  logLogoff(user_code); //retira o utilizador dos ficheios onde estão as socket guardadas
  printf(ANSI_COLOR_RED "# %s fez logout" ANSI_COLOR_RESET "\n",Dados[user_code].login );
  message[0]='\0';
  strcat(message,"9"); //confirma que o utilizador será mesmo desligado
  free(socket_desc);
  write(sock , message , strlen(message));
  return 0;
}

int smssender(int user_code,int socksender){
  char argumentos[600],user[30],corpo[500], final[100], send[600]; //user e aquele que queremos enviar
  int n=0, i=0, userSend,p=0, firstcomma;
  bzero(argumentos,sizeof(argumentos));
  bzero(final,sizeof(final));
  recv(socksender,argumentos,1024,0);

  for (i = 0; argumentos[i]!=';'; ++i);//identifica onde é que se encontra o inicio da mensagem na string
  firstcomma=i;

  for ( ++i  ; argumentos[i] != '\0' ; ++i){//isola a mensagem em si
    corpo[n]=argumentos[i];
    ++n;
  }
  corpo[n]='\0';
  
  i=0;
  bzero(final,sizeof(final));
  /*Isola os utilizadores e manda a mesma mensagem para cada*/
  do{
    if (argumentos[i] == ',' || argumentos[i] == ';'){
      user[p]='\0';

      if (strcmp(user,"admin") == 0){
	printf(ANSI_COLOR_CYAN  "\n---Nova mensagem para administrador de %s---\n%s\n\n"  ANSI_COLOR_RESET,Dados[user_code].login,corpo);
	++i;//atualiza os contadores como se tivesse percorrido todo o ciclo
	p=0;
	if (strlen(final)!=0)
	  strcat(final,", ");
	strcat(final,"admin");
	continue;
      }

      userSend=findUser(user);
      if (userSend==-1){//verifica se existe o utilizador
	bzero(send,sizeof(send));
	strcat(send,"admin;O utilizador "); //8
	strcat(send,user);
	strcat(send," não exsite.8");
	write(socksender,send,strlen(send));
	++i;
	p=0;
	continue;    			
      }

      //escolha do metodo de envio
      bzero(send,sizeof(send));
      if (Dados[userSend].sock!=-1){//se estiver online
	strcat(send,Dados[user_code].login);
	strcat(send,";");
	strcat(send,corpo);
	strcat(send,"8");
	write(Dados[userSend].sock,send,strlen(send));
      }
      else { // se estiver offline
	offlineSMS(user_code,userSend,corpo);     
      }
      p=0;
      if (strlen(final)!=0)
	strcat(final,", ");
      strcat(final,Dados[userSend].login);
    }
    else{
      user[p]=argumentos[i];
      ++p;
    }
    ++i;
  }while(i<=firstcomma);
  if (strlen(final)!=0){
    write(socksender,"2",30);//codigo de a mensagem ter sido enviada com sucesso.
    printf(ANSI_COLOR_CYAN"->"ANSI_COLOR_RESET" %s mandou uma mensagem para %s\n",Dados[user_code].login, final);
  }
  return 0;
}

/*void globalSMS(int n){//envia uma mensagem para um utilizador ou de forma global para todos , se o n==0 e uma mensagem para um so utilizador
  __fpurge(stdin);
  char message[600], v;
  char user[60];
  bzero(message,sizeof(message));
  int i=6;
  if (n==0){
    printf("Para: ");
    scanf("%s",user);
  }
  __fpurge(stdin);
  strcat(message,"admin;");
  printf("Mensagem: ");
  v=getchar();
  while(v!='\n' && i<592){
    message[i]=v;
    ++i;
    v=getchar();
  }
	
  __fpurge(stdin);
  message[i]='8';
  ++i;
  message[i]='\0';
  printf("\n");
  if (n==0){
    i=findUser(user);
    if (i==-1){
      puts("Utilizador inválido.\nMensagem enviada sem sucesso.");
      return;
    }
    write(Dados[i].sock , message , strlen(message));
    puts("Mensagem enviada com sucesso.");
  }
  else{
    for (i = 0; i < UserNumber(); ++i){
      if (Dados[i].sock!=-1){
	//puts(Dados[i].login);
	write(Dados[i].sock , message , strlen(message));
      }
    }
    puts(ANSI_COLOR_CYAN"->"ANSI_COLOR_RESET"Mensagem global enviada.");
  }
}
*/

void *serveradminpanel(){
  char op[50], user[60], decisao,v,pass1[30],pass2[30];
  int i, b, codeuser, flag=0,p=0;
  while(1){
    i=0;
    bzero(op,sizeof(op));
    v=getchar();
    while(v!='\n'&&i<48){
      op[i]=v;
      ++i;
      v=getchar();
    }
    op[i]='\0';
    __fpurge(stdin); //limpar o buffer metodo especifico para c em linux
		
    if(op[0]=='-'){
      switch(op[1]){
      case 'p'://alterar a passwoard
	i=3;
	b=0;
	bzero(user,sizeof(user));
	while(op[i]!='\0'){
	  user[b]=op[i];
	  ++i;
	  ++b;
	}
	user[b]='\0';
	codeuser=findUser(user);
	if (codeuser!=-1){
	  i=0;
	  passwordConfirm(user);
	  passwordChanger(codeuser,user);
	}
	else
	  puts("Utilizador inexistente.");
	__fpurge(stdin);
	break;

      case 'q':
	printf("\nTem a certeza que pretende terminar o servidor?\nEstão neste momento %d utilizadores online...\nConfirme por favor (s/n) ? ", onlineuserscounter());
	if (!(confirma())){
	  puts("\n!!!!! A enviar mensagem de fecho de sessão a todos os utilizadores");
	  for (i = 0; i <UserNumber(); ++i){
	    if (Dados[i].sock!=-1)
	      write(Dados[i].sock,"7",2);
	  }
	  puts("!!!!! Servidor terminado !!!!!\n\n");
	  exit(0);
	}else
	  puts("Cancelado!");
	break;

      case 'h':
	printf("Manual de opções da consola do servidor:\n -a : adicionar um novo utilizador;\n -m : Enviar mensagem global;\n -p [Utilizador] : mudar a passwoard de um utilizador;\n -s :mandar sms;\n -h : mostra a ajuda à consola;\n -r: eliminar utilizador ou lista de utilizadores;\n     -a: opção que apaga todos os utilizadores;\n     -v: apagar de forma verbal;\n -:[port] : altera a porta do servdior;\n -q : desliga o servidor;\n");
	break;

      case ':':
	i=2;
	b=0;
	while(op[i]!='\0'){
	  if (isdigit(op[i])==0){
	    puts("Porta inválida!");
	    break;
	  }
	  b=b*10+(op[i]-'0');//converte string para int
	  ++i;
        }
	if (b < 1 || b > 65536){ // As porta so podem ser de 1 a 65536
	  puts("Porta inválida!");
	  break;
	}
	portChanger(b);//muda a porta no ficheiro d econfigurações
	printf("A porta do servidor foi alterada para a porta %d.\nPara que as alterações façam efeito reinicie o servidor.\n",b);
	break;

      case 's': //mensagem para um utilizador
	globalSMS(0); 
	break;
      case 'm': //mensagem global
	globalSMS(1);   	
	break;

      case 'a':
	printf("Novo utilizador: ");
	scanf("%s",user);
	__fpurge(stdin);
	addUser(user);
	DBreader();    			
	break;

      case 'r':
	i=3;
	if (op[3]=='-'){
	  if (op[4]=='a'){ //eliminar todos os utilizadores
	    FILE *fx;
	    fx=fopen(FX,"w");
	    fclose(fx);
	    DBreader();
	    break;
	  }
	  else if (op[4]=='v'){ //ver o nome dos utilizadores
	    flag=1;
	    i=6;
	  }
	  else
	    continue; 
	}
    		
	bzero(user,sizeof(user));
	b=0;

	while(op[i]!='\0'){
	  if (op[i]!=' '){
	    user[b]=op[i];
	    ++b;
	    ++i;
	  }
	  if(op[i]==' '||op[i]=='\0'){
	    user[b]!='\0';

	    codeuser=findUser(user);//procura o codigo do utilizador
	    if (codeuser!=-1){
	      if (flag){ //se o modo verboso estiver ativado existe uma confirmaçao antes d eliminar o utilizador
		printf("Deseja mesmo eliminar o utilizador %s?(s/n)",user);
		if(confirma())//simples função que gere um sim ou um não
		  break;
		printf("\n");
	      }

	      p=0;// ira servir como o contador que nos permitira saber em que utilizador vamos (codigo do utilizador)
	      FILE *fp = fopen(FX, "a+");
	      FILE *fpc= fopen(FOC,"w");
	      while(fgets(user,60,fp)){
		if (codeuser!=p){
		  fputs(user,fpc);
		}  
		++p;
	      }
	      fclose(fp);
	      fclose(fpc);
	      remove(FX);
	      rename(FOC,FX);
	      DBreader();  
	      puts("Utilizador eliminado.");
	      ++i;
	      b=0;  							
            }
            else
	      puts("Utilizador inexistente.");

	  }                
	}
	flag=0;
	printf("\n");
	break;

      default:
	printf("Opção não reconhecida.\n");
	break;
      }
    }
    else
      puts("As opções devem começar com - ex: -h");
  }
}

int confirma(){ //esta função linda com inputs do tipo sim e nao
  char c;
  c=getchar();
  __fpurge(stdin);
  if (c=='s'){
    return 0;
  }
  else
    return 1;
}
