#include "fullwrapper.h"

int main(int argc, char *argv[]){


    if (argc != 6){

        //Inserimenti dei dati da terminale. Si inserisce l'id della peer, indirizzo ip, porta di registrazione, nome della peer ed ip del server
        fprintf(stderr, "Uso: %s <ID, Indirizzo, porta registrazione procedure, nome della peer, IP Server", argv[0]);
        exit(1);

    }
    //Salvo le informazioni inserite nei campi della struttura peer.
    mypeer.id = atoi(argv[1]);
    strcpy(mypeer.indirizzo, argv[2]);
    mypeer.porta = atoi(argv[3]);
    strcpy(mypeer.nome, argv[4]);
    strcpy(ipServer, argv[5]);
    int numPort;
    //Utilizzo la funzione che mi consente di comunicare/inviare i dati della peer al server
    Connection(&mypeer.indirizzo, &mypeer.porta, &mypeer.nome);

    //Notifica di avvenuta connesione con il server
    printf("Connessione stabilita!\n");
    printf("Peer con indirizzo IP: %s\n", mypeer.indirizzo);


    pthread_t tid;

    //Si utilizza un thread per l'interfaccia dell'utente
    pthread_create(&tid, NULL, (void*)interface, NULL);

    pthread_join(tid, NULL);


}

//Creazione della funzione Connection per la comunicazione con il server
void Connection(){

    int socketfd;

    char buffAddress[INET_ADDRSTRLEN];
    char readBuffer[1024];
    char *ipaddr;

    struct sockaddr_in servaddr;

    struct hostent *address;


    socketfd = Socket(AF_INET, SOCK_STREAM, 0); //Si crea la socket per la comunicazione con il server

    //Settiamo le variabili per la socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(Portreg);
    servaddr.sin_addr.s_addr = inet_addr(ipServer);


    if (inet_pton(AF_INET, buffAddress, &servaddr.sin_addr) < 0){

        perror("Errore sulla net");
        exit(-1);

    }


    Connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    //Invio le informazioni della peer con una FullWrite.
    FullWrite(socketfd, &mypeer, sizeof(Peer));

    //Chiudo la socket
    close(socketfd);


}

//Creazione della funzione RPCcomm che consente di inviare una richiesta per una procedura da utilizzare, e trovare una peer all'interno del server
void RPCcomm(Request connection, int input){


    int socketfd;
    struct sockaddr_in addr;
    Peer mypeer;
    int choice;
    int result;
    int result3;
    float result2 = 0.0F;
    //Setto la socket
    socketfd = Socket(AF_INET, SOCK_STREAM, 0);
    //Setto le variabili per la socket
    addr.sin_port=htons(2000);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipServer);

    Connect(socketfd, (struct sockaddr*)&addr, sizeof(addr));

    //printf("Sono connesso\n");

    //Invio l'id per la ricerca della peer interessata
    FullWrite(socketfd, &connection.id, sizeof(int));

    //Ricevo i dati della peer registrata
    FullRead(socketfd, &mypeer, sizeof(Peer));

    //Se la peer è presente, mi verrà notificato su che porta è collegata la procedura interessata
    printf("\nPROCEDURA COLLEGATA ALLA PORTA: %d\n", mypeer.porta);

    //Invio degli argomenti per la procedura selezionata
    FullWrite(socketfd, &connection, sizeof(Request));

    //Ricevo il risultato della procedura
    FullRead(socketfd, &result, sizeof(int));
    printf("Risultato: %d\n", result);

    close(socketfd);


}

//Creazione del "pacchetto" request per inviare i dati della procedura selezionata prendendo come argomento la scelta selezionata
Request createRPC(int scelta){

    Request info;
    //Se la scelta è uguale a 1 (procedura interi) inserisco gli argomenti integer
    if(scelta == 1){

        printf("Inserisci la porta: ");
        scanf("%d", &info.port);
        printf("Inserisci l'id della peer: ");
        scanf("%d", &info.id);

        printf("Inserisci i due valori: ");
        scanf("%d %d", &info.number1, &info.number2);

        printf("Valore della porta e ID inseriti: %d %d %d %d\n", info.port, info.id, info.number1, info.number2);

    }
    //Se la scelta è uguale a 2 (procedura float) inserisco gli argomenti integer e float
    else if(scelta == 2){

        printf("Inserisci la porta: ");
        scanf(" %d", &info.port);
        printf("Inserisci l'id della peer: ");
        scanf(" %d", &info.id);

        printf("Inserisci i due valori (float): ");
        scanf(" %d %f", &info.number1, &info.numbfloat);

        printf("Valore della porta e ID inseriti: %d %d %d %f", info.port, info.id, info.number1, info.numbfloat);


    }
    //Se la scelta è uguale a 3, utilizzo la procedura integer ed invio in testo e la chiave da ricercare
    else if(scelta == 3){

        printf("Inserisci la porta: ");
        scanf(" %d", &info.port);
        printf("Inserisci l'id della peer: ");
        scanf(" %d", &info.id);

        printf("Inserisci il testo: ");
        scanf(" %s", info.text);
        printf("Inserisci la chiave da ricercare: ");
        scanf(" %s", info.key);

        printf("Valore della porta e ID inseriti: %d %d %s %s", info.port, info.id, info.text, info.key);


    }

    else{

        printf("Input non valido\n");
        exit(0);

    }

    return info;

}


//Creazione dell'interfacciamento con l'utente tramite thread.
void interface(void *args){


    int socketfd, clientfd, option = 1;
    int i = 0;
    int input = 0;
    int idpeer = 0;
    int port = 0;
    Peer peerinfo;
    Peer peerproc;
    Request request;
    //Creazione della socket in caso una peer voglia comunicare con un'altra peer
    struct sockaddr_in client;

    socklen_t size_client = (socklen_t)sizeof(struct sockaddr_in);

    struct sockaddr_in serverAddr;

    socketfd = Socket(AF_INET, SOCK_STREAM, 0);
    //Creazione della socket
    serverAddr.sin_port = htons(mypeer.porta);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &option, sizeof(int));

    Bind(socketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    Listen(socketfd, 1000);

    int fd_disp;
    fd_set readSet; //Creazione della readSet per la gestione dei scrittori
    FD_ZERO(&readSet);

    int max_fd = socketfd+1;

     printf("-----[1] Procedura interi [2] Procedura float [3] Procedura stringa-----\n");
    //Creazione per il menù utente
    while(1){

        //printf("Benvenuto nell'interfaccia utente\n");
        FD_SET(socketfd, &readSet);
        FD_SET(STDIN_FILENO, &readSet);

        //Introduzione della funzione select per la gestione dei file descriptor (readset)
        if((fd_disp = select(socketfd+1, &readSet, NULL, NULL, NULL)) < 0){

            printf("Errore select\n");
            exit(0);

        }


            if (FD_ISSET(0, &readSet)){
                fflush(stdin);
                scanf("%d", &input);

                switch(input){

                    //PROCEDUA INTERI
                    case 1:

                            request = createRPC(input);
                            RPCcomm(request, input);
                            break;

                        break;
                    //PROCEDURA FLOAT
                    case 2:

                            request = createRPC(input);
                            RPCcomm(request, input);

                        break;
                    //PROCEDURA STRING
                    case 3:
                            request = createRPC(input);
                            RPCcomm(request, input);

                        //printf("Prova 3\n");
                        break;

                    //Se non si inserisce nessun input valido, verrà restituito un messaggio di "errore"
                    default:
                        printf("Nessuna scelta valida\n");
                        break;

                }


            }
            //In caso di comunicazione di un'altra peer
             if (FD_ISSET(socketfd, &readSet)){

                clientfd = Accept(socketfd, (struct sockaddr*)&client, &size_client);
                close(clientfd);


            }

        }

}
