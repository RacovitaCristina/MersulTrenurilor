#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>


extern int errno; /// codul de eroare returnat de anumite apeluri 

int port;// portul de conectare la server


void Meniu_Client()
{

 printf("                  -----Bun venit!-----                \n");
 printf(" Tabela cu comenzi:                                    \n");
 printf("                 *info_trenuri\n");
 printf("                 *info_tren_ora\n");
 printf("                 *conectare_cont <nume_utilizator> <id_utilizator>\n");
 printf("                 *deconectare\n");
 printf("                 *intarziere <id_tren> <nume_gara>\n");
 printf("                 *actualizare_intarziere <id_tren> <noua_intarziere>\n");
 printf("                 *ora_plecare <id_tren> <nume_gara>\n");
 printf("                 *estimare_sosire <id_tren> <nume_gara>\n");
 printf("                 *ora_sosire <id_tren> <nume_gara>\n");
 printf("                 *gata iesim\n");
 printf(" Incearcati comenzile acum si vedeti cat de usor este!\n");
 printf("\n");


}





int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  
  // mesajul trimis
  int nr=0;
  char buf[20000];

  // exista toate argumentele in linia de comanda?
  ///verificam sa fie destule argumente
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  ///stabilirea portului
  port = atoi (argv[2]);

  /// crearea socketului
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  // umplerea structurii folosite pentru realizarea conexiunii cu serverul + familia socket-ului 
  server.sin_family = AF_INET;
  
  // adresa IP a serverului 
  server.sin_addr.s_addr = inet_addr(argv[1]);
  
  //portul de conectare 
  server.sin_port = htons (port);
  
  //aici ne conectam la server 
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

 Meniu_Client();

 while(1)
 {
  // citirea mesajului 
  printf ("[client]Introduceti un msj: ");
  fflush (stdout);
  bzero(buf,sizeof(buf));
  read (0, buf, sizeof(buf));
  
  if(strncmp(buf,"gata iesim",10)==0)
  {
   printf("Terminam conexiunea de la client\n"); break;
  }
  
  //nr=atoi(buf);
  //scanf("%d",&nr);
  
  printf("[client] Am citit %s\n",buf);

  // trimiterea mesajului catre server 
  if (write (sd,buf,strlen(buf)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      break;
    }
   
   bzero(buf,sizeof(buf));
   
  // citirea raspunsului dat de server (apel blocant pana cand serverul raspunde) 
  if (read (sd, buf,sizeof(buf)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      break;
    }
    
  // afisam mesajul primit 
  printf ("[client]Mesajul primit este: %s\n", buf);

 }
  // inchidem conexiunea, am terminat 
  close (sd);
  return 0;
}

