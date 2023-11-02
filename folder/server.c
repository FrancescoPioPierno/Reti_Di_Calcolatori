#include "fullwrapper.h"

int main(int argc, char *argv[]){

    //Inizializzazione delle socket e file descriptors per la select

    int listenfd, searchlisten, option1, option2, option3,maxfd, clientfd, rpclisten;
    struct sockaddr_in server_add, clientAddr, searchAddr, rpcAddr;
    int i;
    int input;
    option1 = 1;
    option2 = 1;
    int result;
    float result2;
    int result3;

    Peer peer;
    Request info;
    fd_set readSet;
    fd_set writeSet;

    //Creo la socket
    socklen_t sizeClient=sizeof(clientAddr);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    //Setto le variabili per la socket
    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(Portreg);
    server_add.sin_addr.s_addr=htonl(INADDR_ANY);
    //la funzione setsockopt permette di andare a controllare il comportamento della socket
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &option1, sizeof(int));

    Bind(listenfd, (struct sockaddr*)&server_add, sizeof(server_add));

    Listen(listenfd, 1000);


    //Setto la socket per il controllo di ricerca della peer all'interno del server
    searchlisten = Socket(AF_INET, SOCK_STREAM, 0);
    searchAddr.sin_family = AF_INET;
    searchAddr.sin_port = htons(2000);
    searchAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    setsockopt(searchlisten, SOL_SOCKET, SO_REUSEPORT, &option2, sizeof(int));
    Bind(searchlisten, (struct sockaddr*)&searchAddr, sizeof(searchAddr));
    Listen(searchlisten, 1000);

    if (listenfd > searchlisten)
        maxfd = listenfd;
    else
        maxfd = searchlisten;



    FD_ZERO(&readSet);
    //FD_ZERO(&writeSet);




    while(1){


        FD_SET(listenfd, &readSet);
        FD_SET(searchlisten, &readSet);



        fflush(stdin);

        printf("\nServer in ascolto\n\n");



        if(select(maxfd+1, &readSet, NULL, NULL, NULL) < 0)
        {

            perror("errore select\n");
            exit(-1);

        }

        if(FD_ISSET(listenfd, &readSet)){ //In caso in cui una peer si connette

            printf("Una nuova peer si è connessa\n");


            clientfd = Accept(listenfd, (struct sockaddr*)&clientAddr, &sizeClient);


        if (fork() == 0){ //Creo un processo figlio per la registrazione della peer sul server

                close(listenfd);

                FullRead(clientfd, &peer, sizeof(Peer)); //Ricevo i dati

                printf("Peer con IP %s, porta = %d, nome = %s  connesso al server\n", peer.indirizzo, peer.porta, peer.nome);
                Register(&peer);

                close(clientfd);
                exit(0);

                }
            close(clientfd);

           }

        if (FD_ISSET(searchlisten, &readSet)){ //In caso venga richiesta la ricerca di una peer se è registrata nel server

            //printf("Mi chiedono info\n");

            clientfd = Accept(searchlisten, NULL, NULL);

            if (fork() == 0){

                    close(searchlisten);

                    FullRead(clientfd, &peer.id, sizeof(int)); //Ricevo i dati
                    Search(&peer);

                    printf("Invio informazioni della peer IP= %s porta= %d nome = %s\n", peer.indirizzo, peer.porta, peer.nome);
                    FullWrite(clientfd, &peer, sizeof(Peer)); //Invio i dati della peer interessata


                    FullRead(clientfd, &info, sizeof(Request));
                    //Se i dati che ricevo corrispondono ai corrispondenti controllo, verrà attivata la procedura interessata
                    if (peer.porta != -1 && info.port == 2500){

                        printf("Ho ricevuto i valori %d %d\n", info.number1, info.number2);
                        result = integerprocedure(info.number1, info.number2);
                        FullWrite(clientfd, &result, sizeof(int));


                    }

                    else if(peer.porta != -1 && info.port == 2501){

                        printf("Ho ricevuto i valori %d %f\n", info.number1, info.numbfloat);
                        result = floatprocedure(info.number1, info.numbfloat);
                        FullWrite(clientfd, &result, sizeof(int));


                    }

                    else if(peer.porta != -1 && info.port == 2502){

                        printf("Ho ricevuto %s %s\n", info.text, info.key);
                        result3 = stringmatching(info.text, info.key);
                        FullWrite(clientfd, &result3, sizeof(int));

                    }
                    else
                        printf("Error");



                    close(clientfd);
                    exit(0);

                }

                close(clientfd);


            }


    }

}

//Funzione di registrazion della peer all'interno del server
void Register (Peer *mypeer){


    FILE *ptr;
    char *buff, *cleanbuff;
    buff = (char *)malloc(50);

    size_t sizeBuffer = sizeof(buff);
    cleanbuff =(char *)calloc(70,1);

    sprintf(cleanbuff, "%d %s %d %s", mypeer->id, mypeer->indirizzo, mypeer->porta, mypeer->nome);
    int size = strlen(cleanbuff);

    for (int i = 0;i<40-size;i++){

        strcat(cleanbuff, " ");
    strcat(cleanbuff, "\n");

    }

    ptr = fopen("peer.txt", "r+");
    fseek(ptr, 0, SEEK_SET); //Bisogna posizionarsi all'inizio del file per il corretto inserimento dei dati

    while(getline(&buff, &sizeBuffer, ptr) != EOF){


        if(strcmp(buff, cleanbuff) == 0){

            printf("Linee uguali\n");
            fclose(ptr);
            free(buff);
            free(cleanbuff);
            return;


        }



    else if(atoi(buff) == mypeer->id){ //Se la peer cambia informazioni (porta)

        int pos = ftell(ptr);
        fseek(ptr, pos-strlen(buff), SEEK_SET);
        printf("Sto per scrivere questo: %s\n", cleanbuff);

        fprintf(ptr, "%s", cleanbuff);

        fclose(ptr);
        free(buff);
        free(cleanbuff);
        return;

        }


    }

    fseek(ptr, 0, SEEK_END);
    fprintf(ptr, "%s",cleanbuff);
    fclose(ptr);
    free(buff);
    free(cleanbuff);


}
//Funzione di ricerca di una peer
void Search (Peer *mypeer){


    FILE *file;


    char *buffer, *token, del[2] = " ";
    size_t sizeBUffer = sizeof(buffer);
    buffer = (char *)malloc(60);
    file = fopen("peer.txt", "r");

    fseek(file, 0, SEEK_SET); //Mi metto sempre all'inizio del file

    while(getline(&buffer, &sizeBUffer, file) != EOF){

        if(atoi(buffer) == mypeer->id){ //Se l'id inserito corrisponde, comincio ad inserire i dati nei vari campi

            printf("Trovata la peer!\n");
            token = strtok(buffer, del);

            for (int i = 0; token!=NULL; i++){

            if (i == 0){

                    mypeer->id = atoi(token);

                }

            else if(i == 1)

                strcpy(mypeer->indirizzo, token);

            else if (i == 2)

                mypeer->porta = atoi(token);

            else if(i == 3)

                strcpy(mypeer->nome, token);

            token = strtok(NULL, del);


        }
        fclose(file);
        free(buffer);
        return;
        }


    }

    mypeer->porta = -1; //Verrà restituito peer = -1 se la peer interessata non è presente
    fclose(file);
    free(buffer);
    return;

}
//Procedura RPC integer
int integerprocedure(int numb1, int numb2){

    int result = 0;
    result = numb1 + numb2;
    return result;


}
//Procedura RPC float
int floatprocedure(int numb1, float numb2){

    float result = 0;
    result = numb1*numb2;
    return result;

}
//Procedura RPC stringmatching
int stringmatching(char testo[], char chiave[]){


    int n, i, m, contchiave;
    n = strlen(chiave);
    m = strlen(testo);
    contchiave = 0;
    for (i = 0;i <= m-n; i++){

        if (strncmp(&testo[i], chiave, n) == 0)
            contchiave++;

    }

    return contchiave;
}
