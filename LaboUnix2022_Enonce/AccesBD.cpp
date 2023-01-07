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

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);


   MYSQL_RES  *resultat;
   MYSQL_ROW  Tuple;
   MYSQL* connexion;
  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(ACCESBD Erreur de connexion à la base de données...\n");
    exit(1);  
  }

   char requete[200];
   int tempquantite,tempsBD;

 

  MESSAGE m;

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO

    int retour;
    if((retour=read(fdRpipe,&m,sizeof(MESSAGE)))!=sizeof(MESSAGE))
      {
        perror("Erreur du read ACCESBD");
        exit(1);
      }
    

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      // Preparation de la reponse
                      // Envoi de la reponse au bon caddie

                        sprintf(requete,"select * from UNIX_FINAL where id = '%d'",m.data1);//permet de me connecter 
                        if(mysql_query(connexion,requete)==0)
                        {
                          if((resultat = mysql_store_result(connexion))!=NULL)
                          {

                            Tuple = mysql_fetch_row(resultat); // Ce tuple comprend un seul champ correspondant au comptage demandé 
                            
                             /// j'envoie la requete avec toute les info dedans 
                            m.data1=atoi(Tuple[0]);
                            strcpy(m.data2,Tuple[1]);
                            strcpy(m.data3,Tuple[3]);
                            strcpy(m.data4,Tuple[4]);
                            m.data5=atof(Tuple[2]);
                            m.type=m.expediteur;// je renvoi au Caddie
                            
                            if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("Erreur de msgsnd ACCESBD");
                              exit(1);
                            }
                          }

                        }
                        else
                        {
                          printf("Erreur de mysql_query");
                        }
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      
                      // Finalisation et envoi de la reponse
                       sprintf(requete,"select * from UNIX_FINAL where id = '%d'",m.data1);//permet de me connecter 
                        if(mysql_query(connexion,requete)==0)
                        {
                          if((resultat = mysql_store_result(connexion))!=NULL)
                          {

                              Tuple = mysql_fetch_row(resultat); 
                              
                              
                               /// j'envoie la requete avec toute les info dedans 
                              m.data1=atoi(Tuple[0]); // idArticle
                              strcpy(m.data3,m.data2);// quantite 
                              strcpy(m.data2,Tuple[1]); // intitule
                              if((atoi(Tuple[3])<atoi(m.data3)))
                              {
                                strcpy(m.data3,"0");  // quantité ou 0 
                              }
                              
                              sprintf(requete,"update UNIX_FINAL set stock = stock-%d where id=%d",atoi(m.data3),m.data1);
                              if(mysql_query(connexion,requete)==0)
                              {
                                  if((resultat = mysql_store_result(connexion))!=NULL)
                                  {
                                    perror("Erreur de MAJ de la BD\n");
                                    exit(1);
                                  }
                              }
                              else
                              {
                                printf("Impossible de mettre à jours la BD\n");
                              
                              }
                              
                              strcpy(m.data4,Tuple[4]); // image
                              m.data5=atof(Tuple[2]); // prix
                              m.type=m.expediteur;// je renvoi au Caddie
                              
                             

                              if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                perror("Erreur de msgsnd ACCESBD");
                                exit(1);
                              }

                             
                          }
                        }
                        else
                        {
                          printf("Erreur de mysql_query");
                        }

                      break;
                      

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Mise à jour du stock en BD
                      break;

    }
  }
}
