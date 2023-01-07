#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include <string.h>

int idQ;
int pid1,pid2;
void HandlerSIGINT(int);

int main()
{
  

  MESSAGE requete;
  pid_t destinataire;

  // Armement du signal SIGINT
  // TO DO (etape 6)

  struct sigaction Sigint;
  
  Sigint.sa_handler = HandlerSIGINT;
  sigemptyset(&Sigint.sa_mask);
  Sigint.sa_flags=0; 

  if(sigaction(SIGINT,&Sigint,NULL)==-1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }



  // Creation de la file de message
  fprintf(stderr,"(SERVEUR) Creation de la file de messages\n");
  // TO DO (etape 2)
  if((idQ =msgget(CLE,IPC_CREAT | 0600))== -1)
  {
    perror("Erreur de msgget");
    exit(1);
  }

  // Attente de connection de 2 clients
  fprintf(stderr,"(SERVEUR) Attente de connection d'un premier client...\n");

    if((msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),1,0)) == -1)
   {
     perror("Erreur de msgrcv du premier client");
     exit(1);
   }
   //memoriser le pidq de 2 client 

   pid1=requete.expediteur;


  // TO DO (etape 5)
  fprintf(stderr,"(SERVEUR) Attente de connection d'un second client...\n");
  // TO DO (etape 5)

    if((msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),1,0)) == -1)
   {
     perror("Erreur de msgrcv du 2ieme client");
     exit(1);
   }

   pid2=requete.expediteur;

  while(1) 
  {

    // TO DO (etapes 3, 4 et 5)
  	fprintf(stderr,"(SERVEUR) Attente d'une requete...\n");

   if((msgrcv(idQ,&requete,sizeof(requete)-sizeof(long),1,0)) == -1)
   {
     perror("Erreur de msgrcv");
     exit(1);
   }

    fprintf(stderr,"(SERVEUR) Requete recue de %d : --%s--\n",requete.expediteur,requete.texte);
    char tmp[50];

    strcpy(tmp,"(SERVEUR) ");
    strcat(tmp, requete.texte); // je copie le texte dans temps 
    strcpy(requete.texte,tmp); //je renvoi le message 

    if(requete.expediteur== pid1)
    {

      destinataire=pid2 ;
     
    }
    if(requete.expediteur==pid2)
    {
      destinataire=pid1;
      
    }
  requete.type=destinataire; // c'est le destinataire 

    fprintf(stderr,"(SERVEUR) Envoi de la reponse a %d\n",destinataire);

   if(msgsnd(idQ,&requete,sizeof(requete)-sizeof(long),0)==-1)
   {
     perror("Erreur de msgsnd");
     exit(1);
   }

   kill(destinataire, SIGUSR1);
  } 
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 6)

void HandlerSIGINT(int SigNum)
{
  printf("Suppresion de la file de messages\n");


  if(msgctl(idQ,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de msgctl");
  
    exit(1);
  }
}