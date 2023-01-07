#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;

MYSQL* connexion;

void handlerSIGALRM(int sig);



int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO

  struct sigaction A;

  A.sa_handler = handlerSIGALRM;
  sigemptyset(&A.sa_mask);
  A.sa_flags=0; 

  if(sigaction(SIGUSR1,&A,NULL)==-1)
  {
      perror("Erreur de sigaction");
      exit(1);
  }





  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  /*// Connexion à la base de donnée
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }*/



  MESSAGE m;
  MESSAGE reponse;
  
  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);



  while(1)
  {
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);

                      pidClient=m.expediteur;


                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);

                      mysql_close(connexion);

                      exit(1);
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      m.expediteur=getpid(); // je mets le pid celui du caddie 
                      m.requete=CONSULT;

                      int retour;
                      if((retour=write(fdWpipe,&m,sizeof(MESSAGE)))!=sizeof(MESSAGE))
                      {
                        perror("Erreur du write");
                        exit(1);
                      }

                      
                    
                      if(msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("Erreur de msgrcv de caddie "); // Il recoit le message de AccesBD
                        exit(1);
                      }

                      m.type=pidClient;
                      m.requete=CONSULT;
                      m.expediteur=getpid();//c'est le pid du caddie 

                      //printf("%d\n",m.type);

                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgsnd de CADDIEPPPP");
                        exit(1);
                      }

                      kill(pidClient,SIGUSR1); // je renvoi au client 

                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD

                      m.expediteur=getpid();
                      m.requete=ACHAT;

                      
                      if((retour=write(fdWpipe,&m,sizeof(MESSAGE)))!=sizeof(MESSAGE))
                      {
                        perror("Erreur du write");
                        exit(1);
                      }
                      
                      // on attend la réponse venant de AccesBD
                      if(msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("Erreur de msgrcv de caddie "); // Il recoit le message de AccesBD
                        exit(1);
                      }


                        printf("JESUISAVANTDEREMPLIRBOUCLE\n");
                        if(atoi(m.data3)!=0)
                        {
                          
                            articles[nbArticles].id=m.data1;
                            strcpy(articles[nbArticles].intitule,m.data2);
                            articles[nbArticles].prix=m.data5;
                            articles[nbArticles].stock=atoi(m.data3);
                            strcpy(articles[nbArticles].image,m.data4);
                            nbArticles++;
                            
                            //printf("%d",articles[nbArticles-1].id);
                          
                            // Envoi de la reponse au client
                            printf("Avant l'envoie du msgsnd au client dans caddie\n");
                            m.type=pidClient;
                            m.requete=ACHAT;
                            m.expediteur=getpid();//c'est le pid du caddie

                            if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("Erreur de msgsnd de Achat(CADDIE)");
                              exit(1);
                            }

                            printf("Apres messages caddie\n");
                          }
                        
                        
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      
                          

                          printf("AVANT ENVOI\n");
                          for(int i=0;i<nbArticles;i++)
                          {
                            // j'inverse les champs voir plus haut 
                            m.data1=articles[i].id;
                            strcpy(m.data2,articles[i].intitule);
                            m.data5=articles[i].prix;
                            sprintf(m.data3, "%d", articles[i].stock);
                            //m.data3=atoi(articles[i].stock);
                            strcpy(m.data4,articles[i].image);

                            m.type=pidClient;
                            m.requete=CADDIE;
                            m.expediteur=getpid();
                            if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("Erreur de msgsnd de CADDIE");
                              exit(1);
                            }

                            kill(pidClient,SIGUSR1); // je renvoi au client 
                          }
                          

                      
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD

                      // Suppression de l'aricle du panier
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier

                      // On vide le panier
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      break;

    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier

  // Envoi d'un Time Out au client (s'il existe toujours)
         
  exit(0);
}


