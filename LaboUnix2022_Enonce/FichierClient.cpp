#include "FichierClient.h"




int estPresent(const char* nom)
{
  int pos = 1,fd;
  
  CLIENT client;

  if((fd=open(FICHIER_CLIENTS,O_RDONLY))==-1)
  {
    
    return -1; 
  }
  else
  {
    while(read(fd,&client,sizeof(CLIENT)))
    {
      if(strcmp(nom,client.nom)==0)
      {
        close(fd);
        return pos;
      }

      pos++;
    }

    close(fd);

  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  // TO DO

    int i;
    int hash = 0;

    for (i = 0; motDePasse[i] != '\0'; i++)
    {
        hash += (i + 1)* motDePasse[i];
    }
    return hash % 97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteClient(const char* nom, const char* motDePasse)
{
  // TO DO
  CLIENT ajout;
  int fd;   // crée structure temp pour ajouter dans fichier

  if((fd=open(FICHIER_CLIENTS,O_WRONLY | O_CREAT | O_APPEND,0644))==-1)  // APPEND mets la tete d'ecriture en fin de fichier
  {
    
    perror("Erreur de open()");
  }
  else
  {
    
    strcpy(ajout.nom,nom);
    ajout.hash=hash(motDePasse);
    write(fd,&ajout,sizeof(CLIENT)); // on ecrit dans le fichier ouvert une structure ajout
   
    close(fd);
    
  }

}


////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  // TO DO

  CLIENT mdp;

  int fd;

  int Hash=hash(motDePasse);
  if((fd=open(FICHIER_CLIENTS,O_RDONLY))==-1)
  {
    return -1;
  }
  lseek(fd,(pos-1)*sizeof(CLIENT),SEEK_SET); // je recupere la position et je fais -1
  // car je commence a 1 au lieu de 0 et je fais *sizoef client pour etre au bon endroit 
  
  read(fd,&mdp,sizeof(CLIENT));

  close(fd);
  if(mdp.hash==Hash)
  {
    return 1;
  }
  else
    return 0;

  
}

////////////////////////////////////////////////////////////////////////////////////
int listeClients(CLIENT *vecteur) // le vecteur doit etre suffisamment grand
{
  // TO DO

  int client=0, fd;
  if((fd=open(FICHIER_CLIENTS, O_RDONLY))==-1)
  {
    return -1;
  }

  while((read(fd,&vecteur[client],sizeof(CLIENT)))) // j'envoie l'adresse du vecteur client
  {
    client ++ ;
  }
  close(fd);
  return client;
}
