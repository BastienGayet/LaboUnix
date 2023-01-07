#include "mainwindowex3.h"
#include "ui_mainwindowex3.h"
#include <errno.h>

int fd;

MainWindowEx3::MainWindowEx3(QWidget *parent):QMainWindow(parent),ui(new Ui::MainWindowEx3)
{
    ui->setupUi(this);

    fd=open("Trace.log",O_CREAT | O_APPEND | O_WRONLY, 0666); // je cree le fichier 

  if(fd==-1)
  {
    printf("erreur de open\n");
  }
}

MainWindowEx3::~MainWindowEx3()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx3::setGroupe1(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe1->clear();
    return;
  }
  ui->lineEditGroupe1->setText(Text);
}

void MainWindowEx3::setGroupe2(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe2->clear();
    return;
  }
  ui->lineEditGroupe2->setText(Text);
}

void MainWindowEx3::setGroupe3(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe3->clear();
    return;
  }
  ui->lineEditGroupe3->setText(Text);
}

void MainWindowEx3::setResultat1(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat1->clear();
    return;
  }
  ui->lineEditResultat1->setText(Text);
}

void MainWindowEx3::setResultat2(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat2->clear();
    return;
  }
  ui->lineEditResultat2->setText(Text);
}

void MainWindowEx3::setResultat3(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat3->clear();
    return;
  }
  ui->lineEditResultat3->setText(Text);
}

bool MainWindowEx3::recherche1Selectionnee()
{
  return ui->checkBoxRecherche1->isChecked();
}

bool MainWindowEx3::recherche2Selectionnee()
{
  return ui->checkBoxRecherche2->isChecked();
}

bool MainWindowEx3::recherche3Selectionnee()
{
  return ui->checkBoxRecherche3->isChecked();
}

const char* MainWindowEx3::getGroupe1()
{
  if (ui->lineEditGroupe1->text().size())
  { 
    strcpy(groupe1,ui->lineEditGroupe1->text().toStdString().c_str());
    return groupe1;
  }
  return NULL;
}

const char* MainWindowEx3::getGroupe2()
{
  if (ui->lineEditGroupe2->text().size())
  { 
    strcpy(groupe2,ui->lineEditGroupe2->text().toStdString().c_str());
    return groupe2;
  }
  return NULL;
}

const char* MainWindowEx3::getGroupe3()
{
  if (ui->lineEditGroupe3->text().size())
  { 
    strcpy(groupe3,ui->lineEditGroupe3->text().toStdString().c_str());
    return groupe3;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx3::on_pushButtonLancerRecherche_clicked()
{
  fprintf(stderr,"Clic sur le bouton Lancer Recherche\n");
  // TO DO

  int idFils1,idFils2,idFils3;
  int status;
  char str[3];

  pid_t id;

  

  if (dup2(fd, fileno(stderr)) == -1) // je duplique le descripteur du fichier avec fileno 
    // et je l'envoie dans stderr  retourne -1 si erreur ou nouveau descripteur si ok 
  {
    printf("Erreur de dup2\n");
    exit(1);
  }


  if (ui->checkBoxRecherche1->isChecked() && getGroupe1() != NULL)
  {
    idFils1=fork();

    if(idFils1==0)
    {
      if(execlp("Lecture","Lecture",getGroupe1(),NULL)==-1)
      {
        perror("Erreur de execlp");
        exit(1);
      }
    }
  }
  
  if (ui->checkBoxRecherche2->isChecked() && getGroupe2() != NULL) 
  {
    idFils2=fork();

    if(idFils2==0)
    {
      if(execlp("Lecture","Lecture",getGroupe2(),NULL)==-1)
      {
        perror("Erreur de execlp");
        exit(1);
      }
    }

  }
  
  if (ui->checkBoxRecherche3->isChecked() && getGroupe3() != NULL)
  {
    idFils3=fork();

    if(idFils3==0)
    {
      if(execlp("Lecture","Lecture",getGroupe3(),NULL)==-1)
      {
        perror("Erreur de execlp");
        exit(1);
      }
    }

  }
  

  while((id=wait(&status))!=-1) // wait est bloquant tant que un fils n'est pas mort 
  {
    if(WIFEXITED(status)) // 1 si se termine pas un exit 0 si pas 
    {
      if(id==idFils1)
      {
        sprintf(str,"%d", WEXITSTATUS(status)); // fourni valeut de l'exit du fils temrine
        ui->lineEditResultat1->setText(str);
      }
      else
      {
        if(id==idFils2)
        {
          sprintf(str,"%d", WEXITSTATUS(status)); // fourni valeut de l'exit du fils temrine
          ui->lineEditResultat2->setText(str);
        }
        else
        {
          if(id==idFils3)
          {
            sprintf(str,"%d", WEXITSTATUS(status)); // fourni valeut de l'exit du fils temrine
            ui->lineEditResultat3->setText(str);
          }
        }
      }
    }
  }


  
}

void MainWindowEx3::on_pushButtonVider_clicked()
{
  fprintf(stderr,"Clic sur le bouton Vider\n");
  // TO DO

  ui->lineEditGroupe1->setText("");
  ui->lineEditGroupe2->setText("");
  ui->lineEditGroupe3->setText("");
  ui->lineEditResultat3->setText("");
  ui->lineEditResultat1->setText("");
  ui->lineEditResultat2->setText("");
}

void MainWindowEx3::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  // TO DO

  ::close(fd);

  exit(0);
}
