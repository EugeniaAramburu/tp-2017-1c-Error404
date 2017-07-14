/******************************************************************************
 *                                                                            *
 *  Colegio Universitario de Cartago                                          *
 *  Programación de Sistemas                                                  *
 *  Curso: Programación III                                                   *
 *  Profesor: Greivin Vásquez Valverde                                        *
 *  Estudiante:                                                               *
 *               Diego Sanabria Valerín                                       *
 *                                                                            *
 ******************************************************************************/

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- S E R V I D O R -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include<ctype.h>

#define MYPORT 3490    // Puerto al que conectarán los usuarios
#define MAXBUFLEN 200  // Maximo de caracteres para el buffer
#define BACKLOG 10     // Conexiones pendientes en cola


#define MAX 100

int longitud;//LONGITUD DE LA CADENA ENTRADA
char salida[MAX];//SALIDA YA CONVERTIDA A LETRAS
char actual[MAX];//VALOR DEL NUMERO ACTUAL
char actualposterior[MAX];//VALOR DEL NUMERO POSTERIOR
char revisada[MAX]; //LA ENTRADA UNA VEZ REVISADA PARA Q EL USUARIO NO ENTRE LETRAS

void num_letra(char revisada[]); //FUNCION Q "TRADUCE" AL CASTELLANO


int sockfd; // El servidor escuchara por sockfd
int newfd; // las transferencias de datos se realizar mediante newfd
char buf[MAXBUFLEN];// buffer para recivir o enviar datos
struct sockaddr_in my_addr; // contendra la direccion IP y el numero de puerto local
struct sockaddr_in their_addr;//Contendra la direccion IP y numero de puerto del cliente
socklen_t sin_size;//Contendra el tamaño de la escructura sockaddr_in

char *token = NULL;
char vector [MAX];
int cont,loop,a;

void crear_socket();
void atributos();
void socket_name();
void escuchar();
void aceptar();
void recibir();
void enviar(char cadena[]);

void crear_socket()//se crea el medio por el cual se va a realizar la comunicación:
{
  //AF_INET = Dominio donde se realiza la conexión.( unidos mediante una red TCP/IP).
  //SOCK_STREAM = Uso del protocolo TCP
  //0 = selecciona el protocolo más apropiado

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)//Crea el socket y verifica si hubo algun error
  {
    perror("socket");
    exit(1);
  }
}

void atributos() // Asignamos valores a la estructura my_addr para luego poder llamar a la funcion bind()
{
  my_addr.sin_family = AF_INET; //Se sa un servidor Stream (Protocolo TCP)

  my_addr.sin_port = htons(MYPORT); //se asigna el puerto por el que se va a escuchar (3490)

  my_addr.sin_addr.s_addr = INADDR_ANY; // se usa la IP local

  bzero(&(my_addr.sin_zero), 8); // rellena con ceros el resto de la estructura 
}

void socket_name()//Le asignamos un nombre al socket, para recivir conexiones
{
  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("bind");
    exit(1);
  }
}

void escuchar()// Habilitamos el socket para recibir conexiones, con una cola de 10 conexiones en espera como maximo (BACKLOG)
{
  if (listen(sockfd, BACKLOG) == -1)
  {
    perror("listen");
    exit(1);
  }  
}

void aceptar()//accept = Se utiliza en el servidor, con un socket habilitado para recibir conexiones
{
  sin_size = sizeof(struct sockaddr_in);
    
  //Se espera por conexiones
    
  if ((newfd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1)
  {
    perror("accept");
    exit(1); // Si se produce un error se finaliza el programa
  }
  printf("server: conexion desde: %s\n", inet_ntoa(their_addr.sin_addr));  
}

void recibir()
{
  if ((recv(newfd, buf, MAXBUFLEN, 0)) == -1)// buf = tipo de funcion a realizar
  {
    perror("recv");
    exit(1);
  }
}

void enviar(char cadena[])
{
  if (send(newfd, cadena, MAXBUFLEN, 0) == -1)//envia el resultado de la conversion de numero a letras
  perror("send");
  exit(1);
}

int main()//metodo principal
{ 
  crear_socket();
  
  atributos();
  
  socket_name();

  escuchar();
 
  while(1) // ciclo para llamar accept
  {
    cont=0;
    // Llamamos a fork() para crear un proceso hijo que atendera a la conexion recien establecida
    aceptar();
    if (!fork())
    { 
      //comienza el proceso hijo y enviamos los datos mediante newfd
      
      recibir();//el servidor recive que tipo de funcion tiene que realizar, las opciones son:
      // 1 = InitCap ... 2 = UpCap ... 3 = LowCap ... 4 = Numero -> Letras
      int a;
      if (strcmp (buf,"1") == 0)//1 = InitCap
      {
	cont=0;//cuenta las lineas que va recibiendo el servidor
	send(newfd, "", 1, 0);//+++++++++
	recibir();// se recibe la cantidad de lineas que tiene el archivo que abrió el cliente
	printf("lineas a procesar: %s\n",buf);
	loop = atoi(buf);//loop tiene la cantidad de lineas que tiene el archivo que abrió el cliente
	send(newfd, "", 1, 0);//+++++++++
	while(cont < loop)//el ciclo se repite el numero de lineas que tenga el archivo enviado por el cliente
	{
	  cont++;//aumenta el contador de lineas
	  recibir();//se reciben las lineas
	  
	  token = strtok(buf, " ");//se hace el primer token de la linea en curso
	  if(((int)token[0]) < 123 && ((int)token[0]) > 96)//se revisa que la primera letra de cada palabra esté entre la a y la z
	  token[0]=((char)((int)token[0]-32));//se condigue el codigo ASCII de la primera letra y se le resta 32 para convertirla en su mayuscula correspondiente
	  strcat(vector,token);//se agrega la nueva palabra con la primera letra en mayuscula, al const char vector
	  strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	  while(token != NULL)//ciclo para continuar dividiendo la linea
	  {
	    token = strtok(NULL, " ");//se continua dividiendo la linea
	    if(token!=NULL)
	    {
	      if(((int)token[0]) < 123 && ((int)token[0]) > 96)//se revisa que la primera letra de cada palabra esté entre la a y la z
	      token[0]=((char)((int)token[0]-32));//se condigue el codigo ASCII de la primera letra y se le resta 32 para convertirla en su mayuscula correspondiente
	      strcat(vector,token);//se agrega la nueva palabra con la primera letra en mayuscula, al const char vector
	      strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	    }
	  }

	  send(newfd, vector, MAX, 0);//se envia cada linea ya modificada al cliente
	  memset(vector,0,MAX);//se borra el contenido del const char vector
	  memset(buf,0,MAXBUFLEN);//se borra el contenido del const char buf
	}
	printf("Listo!!! se han cambiado todas las primer letras a Mayuscula\n\n");
      }
      
      if (strcmp (buf,"2") == 0)//2 = UpCap
      {
	cont=0;//cuenta las lineas que va recibiendo el servidor
	send(newfd, "", 1, 0);//+++++++++
	recibir();// se recibe la cantidad de lineas que tiene el archivo que abrió el cliente
	printf("lineas a procesar: %s\n",buf);
	loop = atoi(buf);//loop tiene la cantidad de lineas que tiene el archivo que abrió el cliente
	send(newfd, "", 1, 0);//+++++++++
	while(cont < loop)//el ciclo se repite el numero de lineas que tenga el archivo enviado por el cliente
	{
	  cont++;//aumenta el contador de lineas
	  recibir();//se reciben las lineas
	  
	  token = strtok(buf, " ");//se hace el primer token de la linea en curso
	  for(a=0;a<strlen(token);a++)
	  {
	    if(((int)token[a]) < 123 && ((int)token[a]) > 96)//se revisa que cada letra de cada palabra esté entre la a y la z
	      token[a]=((char)((int)token[a]-32));//se condigue el codigo ASCII de cada letra y se le resta 32 para convertirla en su mayuscula correspondiente
	  }
	  strcat(vector,token);//se agrega la nueva palabra con todas las letras en mayuscula, al const char vector
	  strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	  while(token != NULL)//ciclo para continuar dividiendo la linea
	  {
	    token = strtok(NULL, " ");//se continua dividiendo la linea
	    if(token!=NULL)
	    {
	      for(a=0;a<strlen(token);a++)
	      {
		if(((int)token[a]) < 123 && ((int)token[a]) > 96)//se revisa que cada letra de cada palabra esté entre la a y la z
		  token[a]=((char)((int)token[a]-32));//se condigue el codigo ASCII de cada letra y se le resta 32 para convertirla en su mayuscula correspondiente
	      }
	      strcat(vector,token);//se agrega la nueva palabra con cada letra en mayuscula, al const char vector
	      strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	    }
	  }

	  send(newfd, vector, MAX, 0);//se envia cada linea ya modificada al cliente
	  memset(vector,0,MAX);//se borra el contenido del const char vector
	  memset(buf,0,MAXBUFLEN);//se borra el contenido del const char buf
	}
	printf("Listo!!! se han cambiado todas las letras a MAYUSCULA\n\n");	
      }
      
      if (strcmp (buf,"3") == 0)//3 = LowCap
      {
	cont=0;//cuenta las lineas que va recibiendo el servidor
	send(newfd, "", 1, 0);//+++++++++
	recibir();// se recibe la cantidad de lineas que tiene el archivo que abrió el cliente
	printf("lineas a procesar: %s\n",buf);
	loop = atoi(buf);//loop tiene la cantidad de lineas que tiene el archivo que abrió el cliente
	send(newfd, "", 1, 0);//+++++++++
	while(cont < loop)//el ciclo se repite el numero de lineas que tenga el archivo enviado por el cliente
	{
	  cont++;//aumenta el contador de lineas
	  recibir();//se reciben las lineas
	  
	  token = strtok(buf, " ");//se hace el primer token de la linea en curso
	  for(a=0;a<strlen(token);a++)
	  {
	    if(((int)token[a]) < 91 && ((int)token[a]) > 64)//se revisa que cada letra de cada palabra esté entre la A y la Z
	      token[a]=((char)((int)token[a]+32));//se condigue el codigo ASCII de cada letra y se le suma 32 para convertirla en su minuscula correspondiente
	  }
	  strcat(vector,token);//se agrega la nueva palabra con todas las letras en minuscula, al const char vector
	  strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	  while(token != NULL)//ciclo para continuar dividiendo la linea
	  {
	    token = strtok(NULL, " ");//se continua dividiendo la linea
	    if(token!=NULL)
	    {
	      for(a=0;a<strlen(token);a++)
	      {
		if(((int)token[a]) < 91 && ((int)token[a]) > 64)//se revisa que cada letra de cada palabra esté entre la A y la Z
		  token[a]=((char)((int)token[a]+32));//se condigue el codigo ASCII de cada letra y se le suma 32 para convertirla en su mayuscula correspondiente
	      }
	      strcat(vector,token);//se agrega la nueva palabra con cada letra en minuscula, al const char vector
	      strcat(vector," ");//se agrega un espacio, ya que el token se lo quita
	    }
	  }

	  send(newfd, vector, MAX, 0);//se envia cada linea ya modificada al cliente
	  memset(vector,0,MAX);//se borra el contenido del const char vector
	  memset(buf,0,MAXBUFLEN);//se borra el contenido del const char buf
	}
	printf("Listo!!! se han cambiado todas las letras a minuscula\n\n");		
      }      
      
      if (strcmp (buf,"4") == 0)//4 = Numero -> Letras
      {
	if (send(newfd, "", 1, 0) == -1)// hago esto pq cuando hacia 2 send en el cliente me los recivia 1 recv, y necesitaba que estuvieran separados
	perror("send");
	
	recibir();//recibo el numero del cliente
	
	num_letra(buf);//combierto el numero del cliente en letras

	enviar(salida);//envia el resultado de la conversion del numero a letras
      }
      close(newfd);
      exit(0);
    }
    close(newfd);
  }
}

void num_letra(char revisada[])
{
	char unidades[MAX][MAX]={"uno ","dos ","tres ","cuatro ","cinco ","seis ","siete ","ocho ","nueve "};
	char decenas[MAX][MAX]={"diez ","veinte ","treinta ","cuarenta ","cincuenta ","sesenta ","setenta ","ochenta ","noventa "};
	char centenas[MAX][MAX]={"ciento ","doscientos ","trescientos ","cuatrocientos ","quinientos ","seiscientos ","setecientos ","ochocientos ","novecientos "};
	char decenasespeciales[MAX][MAX]={"diez ","once ","doce ","trece ","catorce ","quince ","dieciseis ","diecisiete ","dieciocho ","diecinueve "};

	int numerito;
	int numeritoposterior;
	int i;
	int bandera;
	int posicionactual;

	longitud = strlen(revisada);

	for (i=longitud;i >= 1 ;i--){
		bandera = longitud - i;
		posicionactual = longitud - bandera;

		switch(posicionactual){
			case 1:case 4:case 7: //unidades

				actual[0] = revisada[bandera];actual[1] = '\0';
				numerito = atoi(actual);
				if (numerito != 1){
					strcat(salida,unidades[numerito-1]);
				}
				else{
					if (longitud == 4 && posicionactual == 4){
					}
					else if(longitud == 7 && posicionactual == 7){
						strcat(salida,"un ");
					}else{
						strcat(salida,unidades[numerito-1]);
					}
				}
				break;

			case 2:case 5:case 8: //decenas

				actual[0] = revisada[bandera];actual[1] = '\0';
				numerito = atoi(actual);
				actualposterior[0] = revisada[bandera+1];actualposterior[1] = '\0';
				numeritoposterior = atoi(actualposterior);
				if (numerito == 1){
					if (numeritoposterior != 0){
						strcat(salida,decenasespeciales[numeritoposterior]);
						i--;
					}else{
						strcat(salida,decenas[numerito-1]);
					}
				}
				else{
					strcat(salida,decenas[numerito-1]);
					if (numeritoposterior !=0 && numerito != 0) strcat(salida,"y ");
				}

				break;

			case 3:case 6: //centenas

				actual[0] = revisada[bandera];actual[1] = '\0';
				numerito = atoi(actual);
				if (posicionactual == 6 && longitud > 6){
					if (longitud == 7 && revisada[bandera-1] == '1'){
						strcat(salida,"millon ");
					}else{
					strcat(salida,"millones ");
					}
				}
				else if (posicionactual == 3 && longitud > 3){
					if(revisada[bandera-1] =='0' && revisada[bandera-2] == '0' && revisada[bandera-3] == '0'){
					}else{
					strcat(salida,"mil ");}
				}
				if (numerito == 1 && revisada[bandera+1] == '0' && revisada[bandera+2] == '0'){
					strcat(salida,"cien ");
				}else{
					strcat(salida,centenas[numerito - 1]);
				}
				break;

		}//fin del switch

	}//fin del for
	if (strcmp (salida,"") == 0)// si salida esta vacio significa que es el cero
	  strcat(salida,"cero");

} //fin de funcion