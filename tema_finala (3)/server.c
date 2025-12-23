#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define PORT 2908//portul

extern int errno;/// codul de eroare returnat de anumite apeluri 

int n;

typedef struct thData
{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
	int conectat; ///conectare pt fiecare client
}thData;

static void *treat(void *); // functia executata de fiecare thread ce realizeaza comunicarea cu clientii 
void raspunde(void *);

struct informatiiS
{ char numeSt[100];
  char oraSosire[100], oraPlecare[100];
  char estimareSosire[100];
};

struct informatiiT
{ int nr;
  char idTren[100], statiePlecare[100], statieDestinatie[100];
  char oraPlecare[100], oraSosire[100], estimareSosire[100];
  int intarziere;
  struct informatiiS statiiDeMijloc[100];
  int nrStatiiMij;
  
};


struct informatiiT infoTren[100];///////////

//int conectat=0;

void CuratareCaractere(char *sir)
{   size_t lg=strlen(sir);
    while(lg>0 && (sir[lg-1]=='\n' || sir[lg-1]==' ' || sir[lg-1]=='\r' || sir[lg-1]=='\t'))
    {sir[lg-1]='\0';
    lg--;
    }
  
}


int VerificareCont(const char* nume, const char *parola,const char *fisier_baza)
{ char rand[256],nume_baza[256]={0},parola_baza[256]={0};
  char *p;
  FILE *fis=fopen(fisier_baza,"r");
  if(fis==0)
  {
   perror("Eroare, nu se deschide baza de date");
   return errno;
  }
  while(fgets(rand,sizeof(rand),fis))
  { p=strtok(rand,"<> \r\n\t");
    while(p!=0)
    {
     if(strcmp(p,"client")==0)
       { p=strtok(0,"<> \r\n\t");
        if(p!=0) 
          {strncpy(nume_baza,p,sizeof(nume_baza)-1);  
           nume_baza[sizeof(nume_baza)-1]='\0';
           CuratareCaractere(nume_baza);
          }
       }
     else if(strcmp(p,"parola")==0)
     { p=strtok(0,"<> \r\n\t");
       if(p!=0) 
        {strncpy(parola_baza,p,sizeof(parola_baza)-1);  
         parola_baza[sizeof(parola_baza)-1]='\0';
        CuratareCaractere(parola_baza);
        }
       if(strcmp(nume_baza,nume)==0 && strcmp(parola_baza,parola)==0) {fclose(fis);return 1;} ///gasit
     
     }
     
    p=strtok(0,"<> \n\r\t");
   }
  }
  printf("Nume baza: '%s', Nume input: '%s'\n", nume_baza, nume);
printf("Parola baza: '%s', Parola input: '%s'\n", parola_baza, parola);

 fclose(fis);
 return 0;//nu am gasit, poate e numele poate e parola gresita
}




void FctConectareCont(thData *td,char sir[20000])
{ char nume[256]={0},parola[256]={0},*p;
  int gasit;

  p=sir+14; ///conectare_cont -->14
  p=strtok(p," ");
  if(p!=0) strncpy(nume,p,sizeof(nume)-1);
  CuratareCaractere(nume);
  p=strtok(0," ");
  if(p!=0) strncpy(parola,p,sizeof(parola)-1);
  CuratareCaractere(parola);
  
  gasit=VerificareCont(nume,parola,"baza_de_date.xml");
  
  if(td->conectat==1) strcpy(sir,"Client deja logat\n");
  else 
    { if(gasit==1)
      {strcpy(sir,"Conectare reusita\n");
       td->conectat=1;
      }
      else if(gasit==0) strcpy(sir,"Verificati daca nu ati gresit numele de utilizator sau parola!\n");
           else strcpy(sir,"Eroare logare/verificare\n");
    }
  fflush(stdout);
}


void FctDeconectare(thData *td,char sir[20000])
{
  if(td->conectat==1) 
     { strcpy(sir,"Deconectare reusita\n");
       td->conectat=0;
     }
  else strcpy(sir,"Clientul este deja deconectat\n");
  fflush(stdout);
}


void ScoatereInformatii(struct informatiiT* info, FILE *fis)
{ char rand[5000],*p;
  ///FILE *fis=fopen(fisier_baza,"r");
  ///if(fis==0)
  ///{
   ///perror("Eroare, nu se deschide baza de date in scoatere informatii");
   ///return errno;
  //} 
  info->nr=0;
  info->nrStatiiMij=0;
  info->idTren[0]='\0';
  info->statiePlecare[0]='\0';
  info->statieDestinatie[0]='\0';
  info->intarziere=0;
  info->oraPlecare[0]='\0';
  info->oraSosire[0]='\0';
  info->estimareSosire[0]='\0';

  for (int i = 0; i < 100; i++) {
    info->statiiDeMijloc[i].numeSt[0] = '\0';
    info->statiiDeMijloc[i].oraSosire[0] = '\0';
    info->statiiDeMijloc[i].oraPlecare[0] = '\0';
}



   while(fgets(rand,sizeof(rand),fis))
  { p=strtok(rand,"<> \r\n\t");
    while(p!=0)
    {if(strcmp(p,"/trenuri")==0) return;  /////////////////////////////////////////////////ATENTIE

     if(strcmp(p,"numar")==0)
        { p=strtok(0,"<> \r\n\t");
          if(p!=0) 
         info->nr=atoi(p);
        }      
     else if(strcmp(p,"idTren")==0)
       { p=strtok(0,"<> \r\n\t");
        if(p!=0) 
          {strncpy(info->idTren,p,sizeof(info->idTren)-1);  
           info->idTren[sizeof(info->idTren)-1]='\0';
           CuratareCaractere(info->idTren);
          }
       }
     else if(strcmp(p,"statie_din_care_pleaca")==0)
     { p=strtok(0,"<> \r\n\t");
       if(p!=0) 
        {strncpy(info->statiePlecare,p,sizeof(info->statiePlecare)-1);  
         info->statiePlecare[sizeof(info->statiePlecare)-1]='\0';
        CuratareCaractere(info->statiePlecare);
        }
     }
           else if(strcmp(p,"statia_destinatie")==0)
           { p=strtok(0,"<> \r\n\t");
             if(p!=0) 
             {strncpy(info->statieDestinatie,p,sizeof(info->statieDestinatie)-1);  
              info->statieDestinatie[sizeof(info->statieDestinatie)-1]='\0';
              CuratareCaractere(info->statieDestinatie);
             }
           }
               else if(strcmp(p,"oraPlecare")==0)
                    { p=strtok(0,"<> \r\n\t");
                      if(p!=0) 
                      {strncpy(info->oraPlecare,p,sizeof(info->oraPlecare)-1);  
                        info->oraPlecare[sizeof(info->oraPlecare)-1]='\0';
                        CuratareCaractere(info->oraPlecare);
                      }
                    }
                    else if(strcmp(p,"oraSosire")==0)
                         { p=strtok(0,"<> \r\n\t");
                           if(p!=0) 
                           {strncpy(info->oraSosire,p,sizeof(info->oraSosire)-1);  
                            info->oraSosire[sizeof(info->oraSosire)-1]='\0';
                             CuratareCaractere(info->oraSosire);
                           }
                         }
                         else if(strcmp(p,"intarziere")==0)
                               { p=strtok(0,"<> \r\n\t");
                                 if(p!=0) 
                                 {///strncpy(info->intarziere,p,sizeof(info->intarziere)-1);  
                                   info->intarziere=atoi(p);
                                 }
                               }
                              else if(strcmp(p,"estimare_sosire")==0)
                             { p=strtok(0,"<> \r\n\t");
                               if(p!=0) 
                               {strncpy(info->estimareSosire,p,sizeof(info->estimareSosire)-1);  
                                info->estimareSosire[sizeof(info->estimareSosire)-1]='\0';
                               CuratareCaractere(info->estimareSosire);
                               }
                             }
                              else if(strcmp(p,"statii_prin_care_trece")==0)
                              { while(fgets(rand,sizeof(rand),fis)!=0)
                                 {p=strtok(rand,"<> \r\n\t");
                                  if(strcmp(p,"statie")==0)
                                 { struct informatiiS statie;

                                   statie.numeSt[0]='\0';
                                   statie.oraPlecare[0]='\0';
                                   statie.oraSosire[0]='\0';
                                   statie.estimareSosire[0]='\0';

                                   while (fgets(rand,sizeof(rand),fis))
                                   {
                                     p=strtok(rand,"<> \r\n\t");
                                     if(strcmp(p,"numeStatie")==0)
                                       { p=strtok(0,"<> \r\n\t");
                                          if(p!=0) 
                                         {strncpy(statie.numeSt,p,sizeof(statie.numeSt)-1);  
                                           statie.numeSt[sizeof(statie.numeSt)-1]='\0';
                                            CuratareCaractere(statie.numeSt);
                                         }
                                       }
                                      else if(strcmp(p,"oraSosire")==0)
                                       { p=strtok(0,"<> \r\n\t");
                                          if(p!=0) 
                                         {strncpy(statie.oraSosire,p,sizeof(statie.oraSosire)-1);  
                                           statie.oraSosire[sizeof(statie.oraSosire)-1]='\0';
                                            CuratareCaractere(statie.oraSosire);
                                         }
                                       }
                                          else if(strcmp(p,"oraPlecare")==0)
                                          { p=strtok(0,"<> \r\n\t");
                                            if(p!=0) 
                                           {strncpy(statie.oraPlecare,p,sizeof(statie.oraPlecare)-1);  
                                           statie.oraPlecare[sizeof(statie.oraPlecare)-1]='\0';
                                            CuratareCaractere(statie.oraPlecare);
                                           }
                                         }
                                          else if(strcmp(p,"estimare_sosire")==0)
                                          {  p=strtok(0,"<> \r\n\t");
                                            if(p!=0) 
                                           {strncpy(statie.estimareSosire,p,sizeof(statie.estimareSosire)-1);  
                                           statie.estimareSosire[sizeof(statie.estimareSosire)-1]='\0';
                                            CuratareCaractere(statie.estimareSosire);
                                           }

                                          }
                                             else if(strcmp(p,"/statie")==0)
                                             {p=strtok(0,"<> \r\n\t"); //////////////////////////////////////////abia am adaugat o grija la BOOM
                                              info->statiiDeMijloc[info->nrStatiiMij] = statie;
                                               info->nrStatiiMij++;
                                               break;
                                             }
                                   }
                                   
                                  }
                                  else if(strcmp(p,"/statii_prin_care_trece")==0)  break;
                              
                                 
                                 }





                              }
                                
   ///////////////  printf("Procesăm linia: %s\n", rand);
/////////////printf("Pointer curent: %s\n", p);

    p=strtok(0,"<> \n\r\t");
   }
  }
   

 ///fclose(fis);                     //////////////////////posibil de aici sa fi fost probleme

}



void ScoatereInformatiiToateTrenurile(struct informatiiT info[], const char *fisier_baza,int* n)
{ char rand[5000],*p;
  int ok=0,i;
  FILE *fis=fopen(fisier_baza,"r");
  if(fis==0)
  {
   perror("Eroare, nu se deschide baza de date in scoatere informatii");
   return errno;
  } 
  *n=0;
  struct informatiiT aux;
  memset(rand,0,sizeof(rand)); /////////////////am sch si aici
  
  while(fgets(rand,sizeof(rand),fis))
  {
    p=strtok(rand,"<> \r\n\t");
    
    if(p!=0 && strcmp(p,"trenuri")==0)
      {//printf("Am gasit o sectiune <trenuri>\n");
        ok=1;
        
        aux.nr=0;
        aux.idTren[0]='\0';
        aux.intarziere=0;
        aux.nrStatiiMij=0;
        aux.statiePlecare[0]='\0';
        aux.statieDestinatie[0]='\0';
        aux.estimareSosire[0]='\0';
        aux.oraPlecare[0]='\0';
        aux.oraSosire[0]='\0';
        for(i=0;i<100;i++)
        {
          aux.statiiDeMijloc[i].numeSt[0]='\0';
          aux.statiiDeMijloc[i].oraPlecare[0]='\0';
          aux.statiiDeMijloc[i].oraSosire[0]='\0';
        }
      

        ScoatereInformatii(&aux,fis);
        info[*n]=aux;
        (*n)++;


        /*printf("Tren %d citit:\n", *n);
        printf("Numar: %d\n", aux.nr);
        printf("ID Tren: %s\n", aux.idTren);
        printf("Statie Plecare: %s\n", aux.statiePlecare);
        printf("Statie Destinatie: %s\n", aux.statieDestinatie);
        printf("Ora Plecare: %s\n", aux.oraPlecare);
        printf("Ora Sosire: %s\n", aux.oraSosire);
        printf("Intarziere: %d\n", aux.intarziere);
        printf("Estimare Sosire: %s\n", aux.estimareSosire);
        printf("Numar Statii Intermediare: %d\n\n", aux.nrStatiiMij);
        for (int j = 0; j < aux.nrStatiiMij; j++) {
                printf("Statie %d: %s\n", j + 1, aux.statiiDeMijloc[j].numeSt);
                printf("    Ora Sosire: %s\n", aux.statiiDeMijloc[j].oraSosire);
                printf("    Ora Plecare: %s\n", aux.statiiDeMijloc[j].oraPlecare);
            }
            printf("\n");*/


      }
     /* while (fgets(rand, sizeof(rand), fis)) {
                p = strtok(rand, "<> \r\n\t");
                if (p != NULL && strcmp(p, "/trenuri") == 0) {
                    printf("Am găsit o secțiune </trenuri>\n");
                    break; // es din  tren și continui cu urm
                }
            }*/
/////////////cu break; crapa
      p=strtok(0,"<> \r\n\t");
    
  }
 fclose(fis);
}

int OraAcum()
{
  time_t acum=time(NULL);
  if(acum==-1)
  {
    perror("Nu gasesc timpul"); return;
  }
  struct tm *acum2=localtime(&acum);
  //int ora=acum2->tm_hour;
  //return ora;
  return acum2->tm_hour*60+acum2->tm_min;
}


int TransformareOraInMinute(const char* o)
{ int ora,minute;
  ora=(o[0]-'0')*10+(o[1]-'0');
  minute=(o[3]-'0')*10+(o[4]-'0');
  return ora*60+minute;

}

void FctIntarziere(char sir[20000])
{

    char cod[256]={0},gara[256]={0},*p;
    int i,intarziere=0,j;
    int gasit=0;
  p=sir+10; ///intarziere-->10
  p=strtok(p," ");
  if(p!=0) strncpy(cod,p,sizeof(cod)-1);
  CuratareCaractere(cod);
  p=strtok(0," ");
  
  
  if(p!=0) strncpy(gara,p,sizeof(gara)-1);
   CuratareCaractere(gara);
  /// printf(cod);
    for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) 
    { if(strcmp(gara,infoTren[i].statieDestinatie)==0)
      {intarziere=infoTren[i].intarziere;
      gasit=1;break;
      }
     else 
       { for(j=0;j<infoTren[i].nrStatiiMij;j++)
          if(strcmp(gara,infoTren[i].statiiDeMijloc[j].numeSt)==0)
             {intarziere=infoTren[i].intarziere;
             gasit=1; break;
             }

       }
    }
  /*printf(cod);
  for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) {intarziere=infoTren[i].intarziere;gasit=1;break;}*/
  if(gasit==0) strcpy(sir,"Acest tren nu exista!");
  else if(intarziere>=0) snprintf(sir,20000,"Trenul %s are o intarziere de %d minute pentru gara %s!\n",cod,intarziere,gara);
       else
         { intarziere=intarziere*(-1);
            snprintf(sir,20000,"Trenul %s vine mai devreme cu %d minute pentru gara %s!\n",cod,intarziere,gara);

         }
  fflush(stdout);
}


void FctSchInBazaIntarzierea(const char*baza,const char*cod,int nouaint)
{ char rand[5000],gasit=0,a[5000]={0},*p;
  char estim[100]={0},orasos[100]={0},orasos2[100]={0};

  FILE* fis1=fopen(baza,"r");
  if(fis1==0)
  {
    perror("Eroare nu se deschide fis1");
    return;
  }
  FILE *fis2=fopen("aux.xml","w");
  if(fis2==0)
  { fclose(fis1);
    perror("Eroare la deschiderea fis2");
    return;

  }
  while(fgets(rand,sizeof(rand),fis1))
  { strcpy(a,rand);
    p=strtok(a,"<> \r\n\t");
    if(p!=0 && strcmp(p,"idTren")==0)
        { p=strtok(0,"<> \r\n\t");
          if(p!=0 && strcmp(p,cod)==0) gasit=1;

        }
    if(gasit==1 && p!=0 && strcmp(p,"oraSosire")==0)
    { fprintf(fis2,"%s",rand);
      p=strtok(0,"<> \r\n\t");
      if(p!=0) strcpy(orasos,p);

    }
    else if(gasit==1 && p!=0 && strcmp(p,"intarziere")==0)
       { fprintf(fis2,"   <intarziere>%d</intarziere>\n",nouaint);
         fflush(fis2);
         gasit=2;
       }
    else if(gasit==2 && p!=0 && strcmp(p,"estimare_sosire")==0)
    { p=strtok(0,"<> \r\n\t");
      if(p!=0)
      { strcpy(estim,p);
        int ora,minut;
        sscanf(orasos,"%d:%d",&ora,&minut);
        int M=ora*60+minut+nouaint;
        int oranoua=(M/60)%24;
        int minnou=M%60;
        fprintf(fis2,"   <estimare_sosire>%02d:%02d</estimare_sosire>\n",oranoua,minnou);
        fflush(fis2);
      }

    }     
    else if(gasit==2 && p!=0 && strcmp(p,"statii_prin_care_trece")==0)
    {
      fprintf(fis2,"%s",rand);
      while(fgets(rand,sizeof(rand),fis1))
      { strcpy(a,rand);
        p=strtok(a,"<> \r\n\t");
        if(p!=0 && strcmp(p,"oraSosire")==0)
        {fprintf(fis2,"%s",rand);
         p=strtok(0,"<> \r\n\t");
         if(p!=0) strcpy(orasos2,p);
        }
        else if(p!=0 && strcmp(p,"estimare_sosire")==0)
        {
          p=strtok(0,"<> \r\n\t");
          if(p!=0)
          {
            ///strcpy(estim,p);
            int ora,minut;
            sscanf(orasos2,"%d:%d",&ora,&minut);
            int M=ora*60+minut+nouaint;
            int oranoua=(M/60)%24;
            int minnou=M%60;
            fprintf(fis2,"         <estimare_sosire>%02d:%02d</estimare_sosire>\n",oranoua,minnou);
            fflush(fis2);
          }
        }
       else fprintf(fis2,"%s",rand);
       if(strstr(rand,"</statii_prin_care_trece>")!=0) 
         {///fprintf(fis2,"%s",rand);
          break;

         }
      }
    }
    else 
     {fprintf(fis2,"%s",rand);}
    if(gasit==2 && strstr(rand,"</trenuri>")!=0) gasit=0;

  }
  fclose(fis1); fclose(fis2);

  if(remove(baza)==0)
   {if(rename("aux.xml",baza)!=0) perror("ERR nu se redenumeste fis aux");}
  else perror("ERR la stergerea fis baza");



}


/*
void FctSchInBazaIntarzierea(const char*baza,const char*cod,int nouaint)
{ char rand[5000],gasit=0,a[5000]={0},*p;
  
  FILE* fis1=fopen(baza,"r");
  if(fis1==0)
  {
    perror("Eroare nu se deschide fis1");
    return;
  }
  FILE *fis2=fopen("aux.xml","w");
  if(fis2==0)
  { fclose(fis1);
    perror("Eroare la deschiderea fis2");
    return;

  }
  while(fgets(rand,sizeof(rand),fis1))
  { strcpy(a,rand);
    p=strtok(a,"<> \r\n\t");
    if(p!=0 && strcmp(p,"idTren")==0)
        { p=strtok(0,"<> \r\n\t");
          if(p!=0 && strcmp(p,cod)==0) gasit=1;

        }
    
    if(gasit==1 && p!=0 && strcmp(p,"intarziere")==0)
       { fprintf(fis2,"   <intarziere>%d</intarziere>\n",nouaint);
         fflush(fis2);
         gasit=0;
       }
     
    else 
     fprintf(fis2,"%s",rand);


  }
  fclose(fis1); fclose(fis2);

  if(remove(baza)==0)
   {if(rename("aux.xml",baza)!=0) perror("ERR nu se redenumeste fis aux");}
  else perror("ERR la stergerea fis baza");



}*/

void FctActualizareIntarziere(thData *td,char sir[20000])
{ //////////////// sa nu uit sa ii las doar pe cei logati sa faca asta !!!
  char cod[256]={0},*p;
  int nouaint=0,gasit=0;
  p=sir+22;///actualizare_intarziere-->22
  p=strtok(p," ");
  if(p!=0) strncpy(cod,p,sizeof(cod)-1);
 CuratareCaractere(cod);
  p=strtok(0," ");
  if(p!=0) nouaint=atoi(p);
  
  for(int i=0;i<n;i++)
    if(strcmp(cod,infoTren[i].idTren)==0) gasit=1;
  if(gasit==0) strcpy(sir,"Acest tren nu exista!");

  if(td->conectat==1 && gasit==1)
  {
 FctSchInBazaIntarzierea("testtt2.xml",cod,nouaint);

 snprintf(sir,20000,"Intarzierea pentru trenul %s a fost actualizata si acum e de %d minute!\n",cod,nouaint);
  }
  else if(gasit==1) strcpy(sir,"Nu sunteti conectat,nu sunteti autorizat sa schimbati datele intarzierii");
 fflush(stdout);
}


void FctOraPlecare(char sir[20000])
{ 
  
  
  char cod[256]={0},gara[256]={0},*p;
    int i,j;
    char hpl[100]={0};
    int gasit=0,okgara=0;
    /*p=sir+11; ///ora_plecare-->11
    p=strtok(p," ");
    if(p!=0) strncpy(cod,p,sizeof(cod)-1);
    CuratareCaractere(cod);
    p=strtok(0," ");
    printf(cod);
    for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) {strcpy(hpl,infoTren[i].oraPlecare);gasit=1;break;}
  if(gasit==0) strcpy(sir,"Acest tren nu exista!");
  else snprintf(sir,100,"Trenul %s pleaca la ora %s\n",cod,hpl);
  fflush(stdout);*/
  
   p=sir+11; ///ora_plecare-->11
    p=strtok(p," ");
    if(p!=0) strncpy(cod,p,sizeof(cod)-1);
    CuratareCaractere(cod);
    p=strtok(0," ");
  
   if(p!=0) strncpy(gara,p,sizeof(gara)-1);
   CuratareCaractere(gara);
   printf(cod);
    for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) 
    { if(strcmp(gara,infoTren[i].statiePlecare)==0)
      {strcpy(hpl,infoTren[i].oraPlecare);
      gasit=1;break;
      }
     else 
       { for(j=0;j<infoTren[i].nrStatiiMij;j++)
          if(strcmp(gara,infoTren[i].statiiDeMijloc[j].numeSt)==0)
             {strcpy(hpl,infoTren[i].statiiDeMijloc[j].oraPlecare);
             gasit=1; okgara=1;break;
             }

       }
    }
  if(gasit==0) strcpy(sir,"Acest tren nu exista!");
  else snprintf(sir,20000,"Trenul %s pleaca la ora %s din statia %s\n",cod,hpl,gara);
  fflush(stdout);


}

void FctOraSosire(char sir[20000])
{  char cod[256]={0},*p,gara[256]={0};
    int i,j;
    char hsos[100]={0};
    int gasit=0;
    p=sir+10; ///ora_sosire-->10
    p=strtok(p," ");
    if(p!=0) strncpy(cod,p,sizeof(cod)-1);
    CuratareCaractere(cod);
    p=strtok(0," ");
    
    if(p!=0) strncpy(gara,p,sizeof(gara)-1);
   CuratareCaractere(gara);
   printf(cod);
    for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) 
    { if(strcmp(gara,infoTren[i].statieDestinatie)==0)
      {strcpy(hsos,infoTren[i].oraSosire);
      gasit=1;break;
      }
     else 
       { for(j=0;j<infoTren[i].nrStatiiMij;j++)
          if(strcmp(gara,infoTren[i].statiiDeMijloc[j].numeSt)==0)
             {strcpy(hsos,infoTren[i].statiiDeMijloc[j].oraSosire);
             gasit=1; break;
             }

       }
    }

   if(gasit==0) strcpy(sir,"Acest tren nu exista!");
   else snprintf(sir,20000,"Trenul %s soseste la ora %s in gara %s\n",cod,hsos,gara);
   fflush(stdout);
}

void FctEstimareSosire(char sir[20000])
{ char cod[256]={0},*p,gara[256]={0};
    int i,j;
    char estim[100]={0};
    int gasit=0;
    p=sir+15; ///estimare_sosire-->15
    p=strtok(p," ");
    if(p!=0) strncpy(cod,p,sizeof(cod)-1);
    CuratareCaractere(cod);
    p=strtok(0," ");
    
    if(p!=0) strncpy(gara,p,sizeof(gara)-1);
   CuratareCaractere(gara);
   printf(cod);
    for(i=0;i<n;i++)
   if(strcmp(infoTren[i].idTren,cod)==0) 
    { if(strcmp(gara,infoTren[i].statieDestinatie)==0)
      {strcpy(estim,infoTren[i].estimareSosire);
      gasit=1;break;
      }
     else 
       { for(j=0;j<infoTren[i].nrStatiiMij;j++)
          if(strcmp(gara,infoTren[i].statiiDeMijloc[j].numeSt)==0)
             {strcpy(estim,infoTren[i].statiiDeMijloc[j].estimareSosire);
             gasit=1; break;
             }

       }
    }

   if(gasit==0) strcpy(sir,"Acest tren nu exista!");
   else snprintf(sir,20000,"Trenul %s are estimarea sosirii la ora %s in gara %s\n",cod,estim,gara);
   fflush(stdout);


}


void ComandaInvalida(char sir[20000])
{
 strcpy(sir,"Ati gresit ceva in scrierea comenzii,incercati din nou!");
 fflush(stdout);

}


int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit ce trebuie trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
  int i=0;
  

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) //crearea socket-ului
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
    
    
  //utilizarea optiunii SO_REUSEADDR 
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));  
  
  //pregatim structurile de date 
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  

  ///umplerea structurii folosite de server + stabilirea familiei de socket-uri
    server.sin_family = AF_INET;
    
    
  ///acceptam orice adresa 
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    
    
  /// utilizam un port utilizator 
    server.sin_port = htons (PORT);
    
  
  /// atasarea socketului 
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }


  /// in aceasta parte punem serverul sa asculte daca vin clienti sa se conecteze 
  if (listen (sd, 6) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
    
    
  ///servim in mod concurent clientii folosind thread-uri 
  while (1)
    {
      int client;
      thData * td; //parametru pentru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      ///acceptam un client (in stare blocanta pana la realizarea conexiunii) 
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
      /// s-a realizat conexiunea, se astepta mesajul 
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;
	td->conectat=0;
        
        pthread_t thr;                        /////////////////////////////////////////////
	pthread_create(&thr, NULL, &treat, td); ///////////////////////////////////
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		// am terminat cu acest client, inchidem conexiunea 
		/////////////////////close ((intptr_t)arg);
		close(tdL.cl);
		free(arg);   ////////////////////////////////////////////////
		return 0;	
  		
};


void raspunde(void *arg)
{
        ////int nr, i=0;
        char sir[20000];
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	char buf[100];//va pastra pt mesajul de la client
	
      while(1)
      { bzero(sir,sizeof(sir));      
       
	if (read (tdL.cl, sir,sizeof(sir)) <= 0)
	{
	 printf("[Thread %d]\n",tdL.idThread);
         perror ("Clientul a întrerupt conexiunea sau exista o eroare la read() de la client.\n");
          close(tdL.cl); /////////////////////////////////////////////////////////////////////////////////////////////////////
          break; ////
  	}


    ///////////////////////////////////////////////////////////////// DE SCOS///////////////////////////////////////////////////////////!!!!!!!!da eroare sa vad de ce
     //struct informatiiT infoTren[100];
     n=0;
     int i;
     ScoatereInformatiiToateTrenurile(infoTren, "testtt2.xml",&n); /////aici aveam &infoTren
 
    // Afișează informațiile pentru a verifica rezultatul
    /*for(i=0;i<n;i++)
    {printf("Numar Tren:%d\n",infoTren[i].nr);
    printf("ID Tren: %s\n", infoTren[i].idTren);
    printf("Statie Plecare: %s\n", infoTren[i].statiePlecare);
    printf("Statie Destinatie: %s\n", infoTren[i].statieDestinatie);
    printf("Ora Plecare: %s\n", infoTren[i].oraPlecare);
    printf("Ora Sosire: %s\n", infoTren[i].oraSosire);
    printf("Intarziere: %d minute\n", infoTren[i].intarziere);
    printf("Estimare Sosire: %s\n", infoTren[i].estimareSosire);
    printf("Numar Statii Intermediare: %d\n", infoTren[i].nrStatiiMij);

    for (int j = 0; j < infoTren[i].nrStatiiMij; j++) 
    {
        printf("Statie %d: %s\n", j + 1, infoTren[i].statiiDeMijloc[j].numeSt);
        printf("    Ora Sosire: %s\n", infoTren[i].statiiDeMijloc[j].oraSosire);
        printf("    Ora Plecare: %s\n", infoTren[i].statiiDeMijloc[j].oraPlecare);
    }
    printf("\n");
   ///////////////////////////////////////////////////////////// de scos de mai sus//////////////////////////!!!!!!!!!!!!!11

   /*for(i=0;i<n;i++)
    {printf("Numar Tren:%d\n",infoTren[i].nr);
    printf("ID Tren: %s\n", infoTren[i].idTren);
    printf("Statie Plecare: %s\n", infoTren[i].statiePlecare);
    printf("Statie Destinatie: %s\n", infoTren[i].statieDestinatie);
    printf("Ora Plecare: %s\n", infoTren[i].oraPlecare);
    printf("Ora Sosire: %s\n", infoTren[i].oraSosire);
    printf("Intarziere: %d minute\n", infoTren[i].intarziere);
    printf("Estimare Sosire: %s\n", infoTren[i].estimareSosire);
    printf("Numar Statii Intermediare: %d\n", infoTren[i].nrStatiiMij);

    for (int j = 0; j < infoTren[i].nrStatiiMij; j++) {
        printf("Statie %d: %s\n", j + 1, infoTren[i].statiiDeMijloc[j].numeSt);
        printf("    Ora Sosire: %s\n", infoTren[i].statiiDeMijloc[j].oraSosire);
        printf("    Ora Plecare: %s\n", infoTren[i].statiiDeMijloc[j].oraPlecare);
    }
    printf("\n");
   ///////////////////////////////////////////////////////////// de scos de mai sus//////////////////////////!!!!!!!!!!!!!11


    }*/
    //}

   
   /////////////int ora=OraAcum();
    ////////////////////////printf("e ora:%d\n",ora);

	
	if(strncmp(sir,"gata iesim",10)==0)
          {
           printf("Terminam conexiunea cu clientul-din server la cererea clientului\n");
           close(tdL.cl);
           break;
           
          }
          
        else if(strncmp(sir,"conectare_cont",14)==0)
        { 
           FctConectareCont(&tdL,sir);
           //break;
        
        }
        
        
        else if(strncmp(sir,"deconectare",11)==0)
        { 
           FctDeconectare(&tdL,sir);
           //break;
        }
        
        
        else if(strncmp(sir,"intarziere",10)==0)
        { 
           FctIntarziere(sir);
           //break;
        }
        
        else if(strncmp(sir,"actualizare_intarziere",22)==0)
        { 
           FctActualizareIntarziere(&tdL,sir);
          // break;
        }
      
        else if(strncmp(sir,"ora_plecare",11)==0)
        { 
           FctOraPlecare(sir);
          // break;
        }
        
        else if(strncmp(sir,"ora_sosire",10)==0)
        { 
           FctOraSosire(sir);
           //break;
        }
        else if(strncmp(sir,"info_trenuri",12)==0)
        {  
          bzero(sir, sizeof(sir)); 
          for (i = 0; i < n; i++) {
           char temp[500];  
             snprintf(temp, sizeof(temp),
                 "Numar Tren: %d\nID Tren: %s\nStatie Plecare: %s\nStatie Destinatie: %s\nOra Plecare: %s\nOra Sosire: %s\nIntarziere: %d minute\nEstimare Sosire: %s\n",
                 infoTren[i].nr, infoTren[i].idTren, infoTren[i].statiePlecare, infoTren[i].statieDestinatie,
                 infoTren[i].oraPlecare, infoTren[i].oraSosire, infoTren[i].intarziere, infoTren[i].estimareSosire);
            strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);

       
        for (int j = 0; j < infoTren[i].nrStatiiMij; j++) {
            snprintf(temp, sizeof(temp),
                     "    Statie Intermediara %d:\n"
                     "        Nume Statie: %s\n"
                     "        Ora Sosire: %s\n"
                     "        Ora Plecare: %s\n"
                     "        Estimare Sosire: %s\n",
                     j + 1, infoTren[i].statiiDeMijloc[j].numeSt,
                     infoTren[i].statiiDeMijloc[j].oraSosire,
                     infoTren[i].statiiDeMijloc[j].oraPlecare,
                     infoTren[i].statiiDeMijloc[j].estimareSosire);
            strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);
        }

        
        strncat(sir, "\n", sizeof(sir) - strlen(sir) - 1);
    }
          
          
          /*for(i=0;i<n;i++)
          {
             snprintf(sir,20000,"Numar Tren: %d\nID Tren: %s\nStatie Plecare: %s\nStatie Destinatie: %s\nOra Plecare: %s\nOra Sosire: %s\nIntarziere: %d minute\nEstimare Sosire: %s\n",
                 infoTren[i].nr, infoTren[i].idTren, infoTren[i].statiePlecare, infoTren[i].statieDestinatie,
                 infoTren[i].oraPlecare, infoTren[i].oraSosire, infoTren[i].intarziere, infoTren[i].estimareSosire);
                 
            
            for (int j = 0; j < infoTren[i].nrStatiiMij; j++) 
                snprintf(sir,20000,"    Statie Intermediara %d:\n"
                     "        Nume Statie: %s\n"
                     "        Ora Sosire: %s\n"
                     "        Ora Plecare: %s\n"
                     "        Estimare Sosire: %s\n",
                     j + 1, infoTren[i].statiiDeMijloc[j].numeSt,
                     infoTren[i].statiiDeMijloc[j].oraSosire,
                     infoTren[i].statiiDeMijloc[j].oraPlecare,
                     infoTren[i].statiiDeMijloc[j].estimareSosire);*/

            /*printf("Numar Tren:%d\n",infoTren[i].nr);
            printf("ID Tren: %s\n", infoTren[i].idTren);
            printf("Statie Plecare: %s\n", infoTren[i].statiePlecare);
            printf("Statie Destinatie: %s\n", infoTren[i].statieDestinatie);
            printf("Ora Plecare: %s\n", infoTren[i].oraPlecare);
            printf("Ora Sosire: %s\n", infoTren[i].oraSosire);
            printf("Intarziere: %d minute\n", infoTren[i].intarziere);
            printf("Estimare Sosire: %s\n", infoTren[i].estimareSosire);
            printf("Numar Statii Intermediare: %d\n", infoTren[i].nrStatiiMij);

            for (int j = 0; j < infoTren[i].nrStatiiMij; j++) 
            {
                printf("Statie %d: %s\n", j + 1, infoTren[i].statiiDeMijloc[j].numeSt);
                printf("    Ora Sosire: %s\n", infoTren[i].statiiDeMijloc[j].oraSosire);
                printf("    Ora Plecare: %s\n", infoTren[i].statiiDeMijloc[j].oraPlecare);
                printf("    Estimare Sosire:%s\n",infoTren[i].statiiDeMijloc[j].estimareSosire);
            }
            printf("\n");*/
          
          //}


        }
          else if(strncmp(sir,"info_tren_ora",13)==0)
          { int ora=OraAcum(),oraminute,maxim,i,j;
            char temp[500];
            int p1,p2,s1,s2,pmij,smij,ctp=0,cts=0;   ////p plecari, s sosiri
            printf("Acum e ora:%02d:%02d\n",ora/60,ora%60);
            ///oraminute=TransformareOraInMinute(ora);
            //ora=629;
            maxim=ora+60;

            for(i=0;i<n;i++)
            {p1=TransformareOraInMinute(infoTren[i].oraPlecare)+infoTren[i].intarziere;
             if(p1>=ora && p1<=maxim)
               { ctp++;
                 if(ctp==1) {snprintf(temp, sizeof(temp), "In urmatoarea ora pleaca trenurile:\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}
                  snprintf(temp, sizeof(temp), "Trenul cu ID-ul %s pleaca din %s spre %s la ora estimata %02d:%02d\n",infoTren[i].idTren, infoTren[i].statiePlecare, infoTren[i].statieDestinatie, p1 / 60, p1 % 60);
                  strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);////////////////////////////////////////////////// !!!!!!DE CONTINUATTTTT
               }
              for(j=0;j<infoTren[i].nrStatiiMij;j++)
               {
                p2=TransformareOraInMinute(infoTren[i].statiiDeMijloc[j].oraPlecare)+infoTren[i].intarziere;
                 if(p2>=ora && p2<=maxim)
                { ctp++;
                 if(ctp==1) {snprintf(temp, sizeof(temp), "In urmatoarea ora pleaca trenurile:\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}
                  snprintf(temp, sizeof(temp),"Trenul cu ID-ul %s pleaca din %s (statie intermediara) spre %s la ora estimata %02d:%02d\n", infoTren[i].idTren, infoTren[i].statiiDeMijloc[j].numeSt, infoTren[i].statieDestinatie, p2 / 60, p2 % 60);
                strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);////////////////////////////////////////////////// !!!!!!DE CONTINUATTTTT
                }
 

               }

            }
            //printf("\n");
            if(ctp==0) {snprintf(temp, sizeof(temp), "In urmatoarea ora nu exista trenuri care pleaca\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}  ///////////////////////////PARTEA ASTA NU PREA AFISEAZA CUM VREAU

           for(i=0;i<n;i++)
            {s1=TransformareOraInMinute(infoTren[i].estimareSosire);
             if(s1>=ora && s1<=maxim)
               { cts++;
                 if(cts==1) { snprintf(temp, sizeof(temp), "In urmatoarea ora sosesc trenurile:\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}
                 snprintf(temp, sizeof(temp), "Trenul cu ID-ul %s soseste in %s la ora estimata %02d:%02d\n", infoTren[i].idTren, infoTren[i].statieDestinatie, s1 / 60, s1 % 60);
             strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);////////////////////////////////////////////////// !!!!!!DE CONTINUATTTTT
               }
              for(j=0;j<infoTren[i].nrStatiiMij;j++)
               {
                s2=TransformareOraInMinute(infoTren[i].statiiDeMijloc[j].estimareSosire);
                 if(s2>=ora && s2<=maxim)
                { cts++;
                 if(cts==1) {snprintf(temp, sizeof(temp), "In urmatoarea ora sosesc trenurile:\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}
                  snprintf(temp, sizeof(temp), "Trenul cu ID-ul %s soseste in %s (statie intermediara) la ora estimata %02d:%02d\n", infoTren[i].idTren, infoTren[i].statiiDeMijloc[j].numeSt, s2 / 60, s2 % 60);
                strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);////////////////////////////////////////////////// !!!!!!DE CONTINUATTTTT
                }
 

               }
            

             }  
           
            if(cts==0) { snprintf(temp, sizeof(temp), "In urmatoarea ora nu exista trenuri care sosesc\n");strncat(sir, temp, sizeof(sir) - strlen(sir) - 1);}

            
           }
         // }
        
          else if(strncmp(sir,"estimare_sosire",15)==0)
            {
              FctEstimareSosire(sir);
            }
        else ComandaInvalida(sir);
         
           
          
	
	//printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, sir);
		      
        //pregatim mesajul pentru raspuns 
        //nr++;      
	printf("[Thread %d]Trimitem mesajul inapoi...%s\n",tdL.idThread, sir);
		      
		      
	//returnam mesajul clientului
	if (write (tdL.cl, sir, strlen(sir)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		 close(tdL.cl);
		 break;
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
    }//while
 //close(tdL.cl);
}

