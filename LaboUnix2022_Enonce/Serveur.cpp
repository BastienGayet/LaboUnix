#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierClient.h"
#include <setjmp.h>

int idQ,idShm,idSem,cpt=0,status;
int fdPipe[2];
TAB_CONNEXIONS *tab;
pid_t id;

int filsCaddie=0;
 sigjmp_buf contexte;
  
  int retour;

void HandlerSIGINT(int sig);
void handlerSIGCHILD(int sig);

void afficheTab();

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction Sigint;
  
  Sigint.sa_handler = HandlerSIGINT;
  sigemptyset(&Sigint.sa_mask);
  Sigint.sa_flags=0; 

  if(sigaction(SIGINT,&Sigint,NULL)==-1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }

  struct sigaction A;

  A.sa_handler = handlerSIGCHILD;
  sigemptyset(&A.sa_mask);
  A.sa_flags=0; 

  if(sigaction(SIGUSR1,&A,NULL)==-1)
  {
      perror("Erreur de sigaction");
      exit(1);
  }

  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO
  if(pipe(fdPipe)==-1)
  {
    perror("Erreur creation pipe");
    exit(1);
  }
  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO
  int filsPub= fork();
  if(filsPub==0)
  {
    execl("./Publicite","Publicite",NULL);
  }

  int taille = 51;
  if((idShm = shmget(CLE,taille, IPC_CREAT  | 0600))==-1)
  {
    perror("Erreur de shmget");
    exit(1);
  }

  // Creation du processus AccesBD (étape 4)
  // TO DO
  char temp[10];
  int filsAccesBd= fork();
  if(filsAccesBd==0)
  {
    sprintf(temp,"%d",fdPipe[0]);
    execl("./AccesBD","AccesBD",temp,NULL); 

  }

  MESSAGE m;
  MESSAGE reponse;


  
  retour=sigsetjmp(contexte,1);


  
  while(1)
  {
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);

                      for(int i=0; i<6;i++)
                      {
                        if((tab->connexions[i].pidFenetre)==0)
                        {
                          tab->connexions[i].pidFenetre=m.expediteur;
                           i=6;
                        }
                        
                        
                      }

                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);

                      for(int i=0; i<6;i++)
                      {
                        if((tab->connexions[i].pidFenetre)== m.expediteur)
                        {
                          
                          tab->connexions[i].pidFenetre=0;
                          i=6;
                        }

                      }


                      break;
      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                      MESSAGE reponse;
                      
                      int statut;
                      statut = estPresent(m.data2);
                      
                      if(m.data1==1)
                      {
                        if(statut>=1)
                        {
                          reponse.type= m.expediteur;
                          reponse.requete=3;
                          reponse.data1=0;
                          strcpy(reponse.data4, "« Client déjà existant ! »");
                          if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                          {
                            perror("Erreur de msgsnd");
                            exit(1);
                          }

                          kill(m.expediteur,SIGUSR1);
                        }
                        else
                        {
                          ajouteClient(m.data2,m.data3);
                          reponse.type=m.expediteur;
                          reponse.requete=LOGIN;
                          reponse.data1=1;// pour 
                          strcpy(reponse.data4,"« Nouveau client créé : bienvenue ! »" );
                          if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                          {
                            perror("Erreur de msgsnd");
                            exit(1);
                          }

                          if((filsCaddie=fork())==0)
                          {
                            sprintf(temp,"%d",fdPipe[1]);
                            execl("./Caddie","Caddie",temp,NULL);
                          }
                          for(int i=0;i<6;i++)
                          {


                            if(tab->connexions[i].pidFenetre == m.expediteur)
                            {
                              strcpy(tab->connexions[i].nom,m.data2);
                              tab->connexions[i].pidCaddie= filsCaddie;
                              reponse.expediteur=m.expediteur;
                              reponse.type=filsCaddie;
                            
                              if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                              {
                                perror("Erreur de msgsnd de LOGIN avec filsCaddie");
                                exit(1);
                              }

                              break;
                            }
                             
                          }

                          kill(m.expediteur,SIGUSR1);

                        }

                      }
                      
                      else
                      {
                        if(statut ==0)
                        {
                          reponse.type=m.expediteur;
                          reponse.requete=3;
                          reponse.data1=0;

                          strcpy(reponse.data4,"Client inconnu...");

                          if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                          {
                            perror("Erreur de msgsnd");
                            exit(1);
                          }

                          kill(m.expediteur,SIGUSR1);
                        }
                        else
                        {
                          if(verifieMotDePasse(statut,m.data3))
                          {
                            reponse.type=m.expediteur;
                            reponse.requete=3;
                            reponse.data1=1;
                            strcpy(reponse.data4,"Re-bonjour cher client !");
                            if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                            {
                              perror("Erreur de msgsnd");
                              exit(1);
                            }

                            filsCaddie= fork();
                            if(filsCaddie==0)
                            { 
                              sprintf(temp,"%d",fdPipe[1]);
                              execl("./Caddie","Caddie", temp , NULL);
                            }
                          for(int i=0;i<6;i++)
                          {
                            if(tab->connexions[i].pidFenetre == m.expediteur)
                            {
                              strcpy(tab->connexions[i].nom,m.data2);
                              tab->connexions[i].pidCaddie= filsCaddie;

                              reponse.expediteur=m.expediteur;
                              reponse.type=filsCaddie;

                              if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                              {
                                perror("Erreur de msgsnd de LOGIN avec filsCaddie");
                                exit(1);
                              }
                              break;
                            }
                             
                          }
                            
                            kill(m.expediteur,SIGUSR1);
                          }
                            

                          else
                          {
                            reponse.type=m.expediteur;
                            reponse.requete=3;
                            reponse.data1=0;
                            strcpy(reponse.data4,"Mot de passe incorrect...");

                            if(msgsnd(idQ,&reponse, sizeof(reponse)-sizeof(long),0)==-1)
                            {
                              perror("Erreur de msgsnd");
                              exit(1);
                            }

                            kill(m.expediteur,SIGUSR1);
                          } 
                            
                          
                        }
                      }

                      
                      
                      break; 

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      
                      for(int i=0; i<6;i++)
                      {
                        if((tab->connexions[i].pidFenetre)== m.expediteur)
                        {
                          
                          strcpy(tab->connexions[i].nom,"");
                          tab->connexions[i].pidCaddie=0;

                          if(close(fdPipe[0])==-1)
                          {
                            perror("Erreur fermeture de sortie");
                          }
                          if(close(fdPipe[1])==-1)
                          {
                            perror("Erreur fermeture de entre");
                          }
                          i=6;
                        }

                      }

                      break;

      case UPDATE_PUB :  // TO DO
                        for(int i=0;i<6;i++)
                        {
                        if((tab->connexions[i].pidFenetre)!= 0)
                        {
                          kill(tab->connexions[i].pidFenetre,SIGUSR2);
                        }

                        }
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      for(int i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre==m.expediteur)
                        {
                            // ne pas changer le gedpid car le caddie ne le connait pas
                          reponse.data1=m.data1;
                          reponse.requete=CONSULT;
                          reponse.type=tab->connexions[i].pidCaddie;
                          reponse.expediteur=m.expediteur;
                          
                          if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0)==-1)
                          {
                            perror("Erreur de requete CONSULT");
                            exit(1);
                          }
                        }
                      }

                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      for(int i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre==m.expediteur)
                        {
                            // ne pas changer le gedpid car le caddie ne le connait pas
                          reponse.data1=m.data1;
                          strcpy(reponse.data2,m.data2);

                          reponse.requete=ACHAT;
                          reponse.type=tab->connexions[i].pidCaddie;
                          reponse.expediteur=m.expediteur;
                          
                          if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0)==-1)
                          {
                            perror("Erreur de requete ACHAT Serveur");
                            exit(1);
                          }
                        }
                      }

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);


                        printf("AAAAA\n");
                        for(int i=0;i<6;i++)
                        {

                          if(tab->connexions[i].pidFenetre==m.expediteur)
                          {
                            reponse.data1=m.data1;
                            strcpy(reponse.data2,m.data2);
                            strcpy(reponse.data3,m.data3);
                            strcpy(reponse.data4,m.data4);
                            reponse.data5=m.data5;
                            reponse.requete=CADDIE;
                            reponse.type=tab->connexions[i].pidCaddie;
                            reponse.expediteur=m.expediteur;


                            if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0)==-1)
                            {
                              perror("Erreur de requete ACHAT Serveur");
                              exit(1);
                            }

                          }
                        } 
                        
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    
      afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
  fprintf(stderr,"\n");
}


void HandlerSIGINT(int Sig)
{
  printf("Suppresion de la file de messages\n");


  if(msgctl(idQ,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de msgctl");
  
    exit(1);
  }
}

void handlerSIGCHILD(int sig)
{
  while((id=wait(&status))!=-1) // wait retourne l'id qui est fini
  {
    if(WIFEXITED(status))
    {

      for(int i=0 ; i<6;i++)
      {
        if(tab->connexions[i].pidFenetre= id)
        {
           strcpy(tab->connexions[i].nom,"");
           tab->connexions[i].pidCaddie=0;

        }
      }

    }

  }

  siglongjmp(contexte,10);

}