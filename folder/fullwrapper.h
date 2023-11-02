#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <errno.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <pthread.h>

//Creazione della struct Peer

typedef struct peer{

    int id; //Campo id della peer
    char nome[30]; //Campo nome della peer
    char indirizzo[30]; //Ip address della peer
    int porta; //Numero porta della peer

}Peer;


//Creazione della struct Request per inviare il pacchetto per la comunicazione RPC

typedef struct request{

    int id; //Richiesta id della peer
    int port; //Numero della porta
    int number1; //Prima variabile integer per la procedura int
    int number2; //Seconda variabile integer per la procedura int
    float numbfloat; //Variabile float per la procedura foat
    char text[20]; //Testo da inserire per la procedura string
    char key[20]; //Chiave da ricercare nella procedura string

}Request;

//Dichiarazioni variabili globali

Peer mypeer;
char ipServer[16];

int Portreg = 1600;

//FUNZIONI WRAPPER
int Socket(int domain,int type,int protocol);
int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
void Listen(int sockfd, int backlog);
void Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *clientaddr, socklen_t *addr_dim);

//------------------------------//

//FUNZIONI PEER PER LA COMUNICAZIONE CON IL SERVER E CREAZIONE RICHIESTA RPC

void interface(void *args);
void Connection();
Request createRPC(int);
void Rpccomm(Request, int);

//--------------------------//

//FUNZIONI SERVER PER IL BIND DELLA PROCEDURE SULLA PORTA SPECIFICATA, REGISTRAZIONE E RICERCA PEER

int integerprocedure(int, int);
int floatprocedure(int, float);
int stringmatching(char[], char[]);
void Register (Peer *mypeer);
void Search (Peer *mypeer);

int scelta = 0;


int Socket(int domain,int type,int protocol){
    int sockfd;
    if ((sockfd=socket(domain,type,protocol))<0)
    {
        printf("Errore apertura Socket\n");
        exit(-1);
    }
    return sockfd;
}

int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    if (connect(sockfd,addr,addrlen)<0)
    {
        printf("Errore in Connect\n");
        return -1;
    }
}

void Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    if (bind(sockfd,addr,addrlen)<0)
    {
        printf("Errore in Bind\n");
        exit(-1);
    }
}

void Listen(int sockfd, int backlog)
{
    if (listen(sockfd,backlog)<0)
    {
        printf("Errore in Listen\n");
        exit(-1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int clientfd;
    if ((clientfd=accept(sockfd,addr,addrlen))<0)
    {
        printf("Errore in Accept\n");
        exit(-1);
    }
    return clientfd;
}




ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    nleft = count;
    while (nleft > 0) {
		if( (nread=read(fd, buf, nleft))<0){//se ce stato errore
			if(errno=EINTR){ continue; }
			else{exit(nread);}
		}else if(nread==0){ break;}//chiuso il canale

		nleft-=nread;
		buf+=nread;
    }
    buf=0;
    return (nleft);
}
//_________________________________________________________________

ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0) {          /* repeat until no left */
        if ( (nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR) {   /* if interrupted by system call */
                continue;           /* repeat the loop */
            } else {
                return(nwritten);   /* otherwise exit with error */
            }
        }
        nleft -= nwritten;          /* set left to write */
        buf +=nwritten;             /* set pointer */
    }
    return (nleft);
}
