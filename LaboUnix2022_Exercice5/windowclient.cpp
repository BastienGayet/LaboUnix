#include "windowclient.h"
#include "ui_windowclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

extern WindowClient *w;

#include "protocole.h" // contient la cle et la structure d'un message

extern char nomClient[40];
int idQ; // identifiant de la file de message
void HandlerSIGUSR1(int);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowClient::WindowClient(QWidget *parent):QMainWindow(parent),ui(new Ui::WindowClient)
{
  ui->setupUi(this);
  setWindowTitle(nomClient);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CLIENT %s %d) Recuperation de l'id de la file de messages\n",nomClient,getpid());
  // TO DO (etape 2)
  

  if((idQ = msgget(CLE,0))==-1)
  {
    perror("Erreur de msgget pour la Recuperation de la cles");
    exit(1);

  }



  // Envoi d'une requete d'identification
  // TO DO (etape 5)
  MESSAGE id;
  id.type= 1; // 1 pour le serveur
  strcpy(id.texte, nomClient);
  id.expediteur = getpid();

  if(msgsnd(idQ,&id,sizeof(MESSAGE)-sizeof(long),0)==-1)
  {
    perror("Erreur de msgsnd pour l'identification ");
    exit(1);
  }




  // Armement du signal SIGUSR1
  // TO DO (etape 4)
  struct sigaction A;

  A.sa_handler = HandlerSIGUSR1;
  sigemptyset(&A.sa_mask);
  A.sa_flags=0; 

  if(sigaction(SIGUSR1,&A,NULL)==-1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }
    
  
}

WindowClient::~WindowClient()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setRecu(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditRecu->clear();
    return;
  }
  ui->lineEditRecu->setText(Text);
}

void WindowClient::setAEnvoyer(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditEnvoyer->clear();
    return;
  }
  ui->lineEditEnvoyer->setText(Text);
}

const char* WindowClient::getAEnvoyer()
{
  if (ui->lineEditEnvoyer->text().size())
  { 
    strcpy(aEnvoyer,ui->lineEditEnvoyer->text().toStdString().c_str());
    return aEnvoyer;
  }
  return NULL;
}

const char* WindowClient::getRecu()
{
  if (ui->lineEditRecu->text().size())
  { 
    strcpy(recu,ui->lineEditRecu->text().toStdString().c_str());
    return recu;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonEnvoyer_clicked()
{
  MESSAGE M;
  M.type= 1; // 1 pour le serveur
  strcpy(M.texte, getAEnvoyer());
  M.expediteur = getpid();


  fprintf(stderr,"Clic sur le bouton Envoyer\n");
  // TO DO (etapes 2, 3, 4)
  if(msgsnd(idQ,&M,sizeof(MESSAGE)-sizeof(long),0)==-1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }


}

void WindowClient::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  exit(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 4)

void HandlerSIGUSR1(int Sig)
{

  MESSAGE recu;

  printf("Entrez handler\n");

  if((msgrcv(idQ,&recu,sizeof(MESSAGE)-sizeof(long),getpid(),0)) == -1)
  {
    perror("Erreur de msgrcv");
    exit(1);
  }
  else
    w->setRecu(recu.texte);



  printf("Sortie handler\n");
}


