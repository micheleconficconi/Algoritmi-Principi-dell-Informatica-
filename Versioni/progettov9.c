//differenze dalla precedente: rimuovo i nomi dalle ricette e le sostituisco con un numero unico

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//1024, 2048, 4096, 8192, 16384, 32768, 65536
#define DIM_RIC_BASE 8192
#define DIM_MAG_BASE 2048
#define HASH_CONSTANT_R ((double) (sqrt(5)-1)/2)
#define HASH_CONSTANT_M ((double) (sqrt(5)-1)/2)
#define HEAP_SIZE_O 16384
#define HEAP_SIZE_M 30;
#define DIM_MAX_VETT 51; //dimesione dell'array di supporto per copiare le ricette, con del testing nessuna ricetta superava i 50 ingredienti

//opzioni di compilazione: gcc -Wall -Werror -std=gnu11 -g3 -O2 -lm primastesura.c -o primastesura

//elemento del vettore del minheap magazzino
typedef struct {
    int quantita;
    int scadenza;
}   Lotto;
//minheap magazzino
typedef struct {
    int dim;
    int numero_elem;
    Lotto * vett;
} Heap_M;
typedef Heap_M * minHeap_M_P;

//MinHeap_struct nel magazzino, in una cella, con un puntatore a next per gestire le collisioni
typedef struct MH{
    int ingrediente_n; //è un numero unico per ogni ingrediente, dato da hash+somma delle ultime 3 lettere
    int qt_tot;
    minHeap_M_P minHeap_M;
    struct MH * next;
} MHm;
//puntatori che escono fuori dal magazzino->vett[i]
typedef MHm * MHmagazzino;

//hash table magazzino
typedef struct {
    int dim;
    MHmagazzino * vett; //vettore contenete le testate degli alberi dei lotti;
} MAGAZZINO;
typedef MAGAZZINO* MAGAZZINO_P;

//ingrediente in una ricetta:
typedef struct {
    int ingrediente_n;
    int quantita;
    int hash;
} Ingrediente;
//elemento della tabella hash RICETTARIO, con prox per gestire le collisioni e next che punta all'ingrediente
typedef struct RIC {
    int ricetta_n; //numero unico per la ricetta, dato da Hash + somma delle ultime 3 lettere
    int peso;
    int n_ingredienti;
    Ingrediente * vett;
    struct RIC *prox;
} Ricetta;
//puntatore che esce fuori da Ricettario->vett[i]
typedef Ricetta * RICETTA_p;

//tabella hash per il ricettario:
typedef struct {
    int dim;
    RICETTA_p * vett; //vettore di ricette
} RICETTARIO;
typedef RICETTARIO * RICETTARIO_P;

//lista Ordini in Attesa (semplice coda FIFO)
typedef struct OA {
    char * ordine;
    int quantita;
    int data;
    RICETTA_p Ricetta; //tengo un puntatore alla ricetta
    struct OA * next;
} Ordine_in_attesa;
typedef Ordine_in_attesa * Lista_Ordini_Attesa;

//cella del vettore che verrà dato a mergesort per stampare gli ordini
typedef struct {
    char * ordine;
    int data;
    int quantita;
    int peso;
} Ordine_Stampabile;

//lista in cui metto il minimo estratto dallo heap mano a mano che procedo, che poi verrà trasformata in vettore
typedef struct OS_L{
    char * ordine;
    int data;
    int quantita;
    int peso;
    struct OS_L * next;
} Nodo_Lista_OS;
typedef Nodo_Lista_OS * Lista_OS;

//Lista degli ordini stampabili, supporto per creare poi il vettore che MergeSort ordinerà per peso
typedef struct{
    Lista_OS Primo_ordine;
    Lista_OS Ultimo_ordine;
} Lista_Ordini_Stampabili;

//typedef per lo heap
typedef struct {
    int dim;
    int numero_elem;
    Ordine_Stampabile * V;
    Lista_Ordini_Stampabili * LS;
} Heap;
typedef Heap * minHeap_P;

//struct che punta alle due liste per fare in modo che anche le altre funzioni possano modificarle
typedef struct LO{ 
    Lista_Ordini_Attesa  OA;
    Lista_Ordini_Attesa OA_ult;
    minHeap_P  OP;
} Lista_Ordini;

typedef Lista_Ordini * LO_Poynter; //puntatore che permette di modificare le due liste OA e OP

//testate delle funzioni:
//gestione del ricettario:
void Aggiungi_Ricetta(RICETTARIO_P, MAGAZZINO_P);
int Funzione_hash_R(int, char *, size_t);
void Rimuovi_Ricetta(RICETTARIO_P, LO_Poynter);
int Check_Attesa(Lista_Ordini_Attesa, char *); //controlla se c'è un ordine in attesa per quella ricetta
int Check_Ordini_Pronti(minHeap_P, char *); //controlla se c'è un ordine pronto per quella ricetta

//gestione del magazzino:
void Rifornimento (MAGAZZINO_P, RICETTARIO_P, LO_Poynter);
int Funzione_hash_M(int, char*, size_t);
void Inserisci_M (minHeap_M_P, char *, int, int);
void MinheapIfy_M(minHeap_M_P, int);
int Estrai_Min_M(minHeap_M_P);
void Espandi_MinHeap(minHeap_M_P);
void Controlla_Scadenze(MHmagazzino);
void Controlla_Ordini_Attesa(LO_Poynter, MAGAZZINO_P, RICETTARIO_P); //controlla per le ricette in attesa se ora ci sono gli ingredienti 

void Ordine(RICETTARIO_P, MAGAZZINO_P, LO_Poynter);
int ControllaEPrepara(RICETTA_p, MAGAZZINO_P, LO_Poynter, int, char *, int);
void Enqueue_OA(LO_Poynter, char *, int, RICETTA_p);
void Enqueue_OP(minHeap_P ,char *, int, int, int);
int Estrai_Min(minHeap_P);
void Coda_OS(Lista_Ordini_Stampabili * OS, char *, int, int, int);
void MinHeapIfy(minHeap_P, int);
void Riempi_vett(Ordine_Stampabile *, minHeap_P);

void Corriere(LO_Poynter, minHeap_P);
void MergeSort(Ordine_Stampabile *, int, int);
void Merge(Ordine_Stampabile *, int, int, int);

int capienza=0, periodicita=0, tempo=0; //3 variabili globali per comodità
char primo_OA=1; //per resettare l'inserimento nella lista degli OA;
char primo_OS=1; //uguale a primo_OA ma per gli ordini stampabili;
int main( ){
    //per prima cosa devo inizializzare le strutture dati: 
    //alloco e inizializzo Ricettario:
    int i=0;
    RICETTARIO_P Ricettario;
    Ricettario=(RICETTARIO*)malloc(sizeof(RICETTARIO));
    Ricettario->vett=(RICETTA_p*)malloc(sizeof(Ricetta)*DIM_RIC_BASE); //vettore di ricette
    for(i=0; i<DIM_RIC_BASE; i++){
        Ricettario->vett[i]=NULL;
    }
    Ricettario->dim=DIM_RIC_BASE;


    //alloco e inizializzo Magazzino:
    MAGAZZINO_P Magazzino;
    Magazzino=(MAGAZZINO_P)malloc(sizeof(MAGAZZINO));
    Magazzino->vett=(MHmagazzino *)malloc(sizeof(MHmagazzino)*DIM_MAG_BASE); //vettore di puntatori a testate di alberi
    for(i=0; i<DIM_MAG_BASE; i++){
        Magazzino->vett[i]=NULL;
    }
    Magazzino->dim=DIM_MAG_BASE;  

    //alloco e inizializzo minHeap:
    minHeap_P minHeap;
    minHeap=(minHeap_P)malloc(sizeof(Heap));
    minHeap->V=(Ordine_Stampabile *)calloc(HEAP_SIZE_O, sizeof(Ordine_Stampabile));
    minHeap->dim=HEAP_SIZE_O;
    minHeap->numero_elem=0;
    minHeap->LS=(Lista_Ordini_Stampabili *)malloc(sizeof(Lista_Ordini_Stampabili));
    minHeap->LS->Primo_ordine=NULL;
    minHeap->LS->Ultimo_ordine=NULL;


    //alloco e inizializzno lista_ordini
    LO_Poynter Lista_Or;
    Lista_Or=(LO_Poynter)malloc(sizeof(Lista_Ordini));    
    Lista_Or->OP=minHeap;
    Lista_Or->OA=NULL;
    Lista_Or->OA_ult=NULL;
    //ora la scanf che salva periodicità e capacità del furgoncino
    if(scanf("%d %d", &periodicita, &capienza)){
        //ora inizio con la parte che smista gli ordini
        char s[17];
        int check_periodicita=0;
        while((scanf("%s", s))!=EOF){
            if(check_periodicita==periodicita){
                Corriere(Lista_Or, Lista_Or->OP);
                check_periodicita=0;
            }
            if(strcmp(s, "aggiungi_ricetta")==0){
                Aggiungi_Ricetta(Ricettario, Magazzino);
            }
            else if(strcmp(s, "rimuovi_ricetta")==0){
                Rimuovi_Ricetta(Ricettario, Lista_Or);
            }
            else if(strcmp(s, "rifornimento")==0){
                Rifornimento(Magazzino, Ricettario, Lista_Or);            
            }
            else if(strcmp(s, "ordine")==0){
                Ordine(Ricettario, Magazzino, Lista_Or);
            }
            check_periodicita++;
            tempo++;
        }
        if(check_periodicita==periodicita) { //per evitare di lasciare ordini nel furgoncino se si devono stampare al tempo di chiusura del programma
            Controlla_Ordini_Attesa(Lista_Or, Magazzino, Ricettario);
            Corriere(Lista_Or, Lista_Or->OP);
        }
    }
    return 0;
}

//aggiunge una ricetta al Ricettario, gestendo le collisioni 
void Aggiungi_Ricetta(RICETTARIO_P Ricettario, MAGAZZINO_P Magazzino){
    char r[256], c='a';
    int k=0,Hk=0, peso=0, n_ingredienti=0, dim=DIM_MAX_VETT;
    int i=0, ricetta_n;
    size_t lunghezza; //conterra il risultato della strlen
    Ingrediente vett[dim]; //serve per allocare gli ingredienti della ricetta
    Ricetta * punt=NULL, *prev=NULL; //scorrono la lista nel bucket, tra le varie ricette (che collidono)
    if(scanf("%s", r)>0){ //copio il nome della ricetta
        lunghezza=strlen(r);
        Hk=Funzione_hash_R(Ricettario->dim, r, lunghezza);
        ricetta_n=Hk+r[lunghezza-1]+r[lunghezza-2]*r[lunghezza-3]*r[lunghezza-4]; //numero unico per le ricette, come per gli ingredienti
        if(Ricettario->vett[Hk]==NULL){ //inserimento su cella vuota 
            Ricettario->vett[Hk]=(RICETTA_p)malloc(sizeof(Ricetta));
            // Ricettario->vett[Hk]->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
            // strcpy(Ricettario->vett[Hk]->nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
            Ricettario->vett[Hk]->ricetta_n=ricetta_n;
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita>
                    vett[i].quantita=k; //salvo tutto nell'array di supporto per poi copiare in un array della dimensione giusta
                    peso+=k;
                    lunghezza=strlen(r);
                    vett[i].hash=Funzione_hash_M(Magazzino->dim, r, lunghezza);
                    vett[i].ingrediente_n=vett[i].hash +r[lunghezza-1]+r[lunghezza-2]+r[lunghezza-3];
                    i++;
                    n_ingredienti++;
                }
            }
            //uscito dal while alloco un vettore della dim giusta per tenere gli ingredienti
            Ricettario->vett[Hk]->vett=(Ingrediente *)malloc(sizeof(Ingrediente)*n_ingredienti);
            for(i=0; i<n_ingredienti; i++){ //copio dal vettore grosso in quello di dim giusta
                Ricettario->vett[Hk]->vett[i].quantita=vett[i].quantita;
                Ricettario->vett[Hk]->vett[i].hash=vett[i].hash;
                Ricettario->vett[Hk]->vett[i].ingrediente_n=vett[i].ingrediente_n;
            }
            Ricettario->vett[Hk]->peso=peso;
            Ricettario->vett[Hk]->n_ingredienti=n_ingredienti;
            Ricettario->vett[Hk]->prox=NULL; //prox nel bucket
            printf("aggiunta\n");
            return;
        } 
        else if(Ricettario->vett[Hk]->ricetta_n!=ricetta_n){ //caso in cui è già presente una ricetta ma con nome diverso =>cerco da Ricettario->vett[Hk]->prox
            if(Ricettario->vett[Hk]->prox==NULL){ //inserisco direttamente su quella dopo, nel bucket c'e' solo una 
                Ricettario->vett[Hk]->prox=(RICETTA_p)malloc(sizeof(Ricetta));
                // Ricettario->vett[Hk]->prox->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
                // strcpy(Ricettario->vett[Hk]->prox->nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
                Ricettario->vett[Hk]->prox->ricetta_n=ricetta_n;
                while(c!='\n'){
                    if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita>
                        vett[i].quantita=k;
                        peso+=k;
                        lunghezza=strlen(r);
                        vett[i].hash=Funzione_hash_M(Magazzino->dim, r, lunghezza);
                        vett[i].ingrediente_n=vett[i].hash +r[lunghezza-1]+r[lunghezza-2]+r[lunghezza-3];
                        i++;
                        n_ingredienti++;
                    }
                }
                //uscito dal while alloco un vettore della dim giusta per tenere gli ingredienti
                Ricettario->vett[Hk]->prox->vett=(Ingrediente *)malloc(sizeof(Ingrediente)*n_ingredienti);
                for(i=0; i<n_ingredienti; i++){ //copio dal vettore grosso in quello di dim giusta
                    Ricettario->vett[Hk]->prox->vett[i].quantita=vett[i].quantita;
                    Ricettario->vett[Hk]->prox->vett[i].hash=vett[i].hash;
                    Ricettario->vett[Hk]->prox->vett[i].ingrediente_n=vett[i].ingrediente_n;
                }
                Ricettario->vett[Hk]->prox->peso=peso;
                Ricettario->vett[Hk]->prox->n_ingredienti=n_ingredienti;
                Ricettario->vett[Hk]->prox->prox=NULL; //prox nel bucket
                printf("aggiunta\n");
                return;
            }
            else{ //caso in cui la seconda e' !=NULL, scorro finche' non arrivo alla fine o trovo che c'e' gia' una ricetta con quel nome
                punt=Ricettario->vett[Hk]->prox;
                while((punt!=NULL)&&(punt->ricetta_n!=ricetta_n)){
                    prev=punt; //tengo un riferimento a prev per inserire il nuovo nodo piu' facilmente
                    punt=punt->prox;
                }
                if(punt==NULL){ //se c'è spazio vuoto aggiungo
                    prev->prox=(RICETTA_p)malloc(sizeof(Ricetta));
                    // prev->prox->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
                    // strcpy(prev->prox->nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
                    prev->prox->ricetta_n=ricetta_n;
                    while(c!='\n'){
                        if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita>
                            vett[i].quantita=k;
                            peso+=k;
                            lunghezza=strlen(r);
                            vett[i].hash=Funzione_hash_M(Magazzino->dim, r, lunghezza);
                            vett[i].ingrediente_n=vett[i].hash +r[lunghezza-1]+r[lunghezza-2]+r[lunghezza-3];
                            i++;
                            n_ingredienti++;
                        }
                    }
                    //uscito dal while alloco un vettore della dim giusta per tenere gli ingredienti
                    prev->prox->vett=(Ingrediente *)malloc(sizeof(Ingrediente)*n_ingredienti);
                    for(i=0; i<n_ingredienti; i++){ //copio dal vettore grosso in quello di dim giusta
                        prev->prox->vett[i].quantita=vett[i].quantita;
                        prev->prox->vett[i].hash=vett[i].hash;
                        prev->prox->vett[i].ingrediente_n=vett[i].ingrediente_n;
                    }
                    prev->prox->peso=peso;
                    prev->prox->n_ingredienti=n_ingredienti;
                    prev->prox->prox=NULL; //prox nel bucket
                    printf("aggiunta\n");
                    return;
                }
                else{ //vuol dire che c'è già una ricetta con quel nome, sono uscito dal while per la strcmp
                    while(c!='\n'){
                        if(scanf("%s %d%c", r, &k, &c)>0); //consumo tutta la riga
                    }
                    printf("ignorato\n");
                    return;
                }
            }
        }
        else if(Ricettario->vett[Hk]->ricetta_n==ricetta_n) {  //se la ricetta è già presente nel primo nodo
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0); //consumo tutta la riga
            }
            printf("ignorato\n");
            return;
        }
    }
}

//calcola l'indice della tabella hash_Ric partendo dal nome della ricetta
int Funzione_hash_R(int m, char * r, size_t lunghezza){  
    int i=0, z=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        z=z*3+r[i];
    }
    z=(int)floor(((double) m)*(((double) z)*HASH_CONSTANT_M-floor(((double) z)*HASH_CONSTANT_M)));
    return z;
}

//rimuove la ricetta, se presente e se non è in attesa/pronto
void Rimuovi_Ricetta(RICETTARIO_P Ricettario, LO_Poynter Lista_Ordini){
    char r[256];
    int Hk, ricetta_n;             
    size_t lunghezza;
    Ricetta * punt=NULL, * prev=NULL;
    if(scanf("%s", r)>0){ //salvo il nome della ricetta
        lunghezza=strlen(r);
        Hk=Funzione_hash_R(Ricettario->dim, r, lunghezza);
        punt=Ricettario->vett[Hk];
        ricetta_n=Hk+r[lunghezza-1]+r[lunghezza-2]*r[lunghezza-3]*r[lunghezza-4]; //numero unico per le ricette, come per gli ingredienti
        while((punt!=NULL)&&(punt->ricetta_n!=ricetta_n)){ //scorro il bucket finche o trovo null o la ricetta
            prev=punt;
            punt=punt->prox;
        }
        if(punt!=NULL){ //in questo momento se entro nell'if, punt punta alla ricetta da rimuovere
            if((Check_Attesa(Lista_Ordini->OA, r))+(Check_Ordini_Pronti(Lista_Ordini->OP, r))){ //controllo che non ci siano ordini in attesa/pronto con quella ricetta
                printf("ordini in sospeso\n");
                return;
            }
            else{//in questo caso la ricetta va rimossa
                free(punt->vett); //dealloco il vettore degli ingredienti
                //orda distinguo 3 casi, a seconda che punt sia un nodo iniziale, centrale o finale della lista nel bucket
                if(punt==Ricettario->vett[Hk]){ //sono nel primo nodo del bucket
                    Ricettario->vett[Hk]=punt->prox;  //aggancio il vettore al secondo elemento e elimino punt (va bene sia che sia null che no)
                    free(punt);
                    printf("rimossa\n");
                    // punt=Ricettario->vett[Hk];
                    return;
                }
                else if(punt->prox==NULL){ //se è l'ultimo nodo della lista
                    prev->prox=NULL;
                    free(punt);
                    printf("rimossa\n");
                    // punt=Ricettario->vett[Hk];
                    return;
                }
                else{//è un nodo centrale
                    prev->prox=punt->prox;
                    free(punt);
                    printf("rimossa\n");
                    // punt=Ricettario->vett[Hk];
                    return;
                }
            }
        }
        else {
            printf("non presente\n");
            return;
        }
    }
}

//controlla se ci sono ordini nella coda di attesa con nome r
int Check_Attesa(Lista_Ordini_Attesa Attese, char * r){
    Lista_Ordini_Attesa ptr=NULL;
    if(Attese!=NULL){
        ptr=Attese;
        while(ptr!=NULL){
            if(strcmp((ptr->ordine), r)==0)
                return 1;
            ptr=ptr->next;
        }
    }
    return 0;  
}

//controlla se ci sono ordini nello heap con nome r
int Check_Ordini_Pronti(minHeap_P minHeap, char *r){
    int i=0;
    while(i<minHeap->numero_elem){
        if(strcmp(minHeap->V[i].ordine, r)==0)
            return 1;
        i++;
    }
    return 0;
}

//aggiunge gli ingredienti al magazzino e controlla se può preparare nuove ricette
void Rifornimento(MAGAZZINO_P Magazzino, RICETTARIO_P Ricettario, LO_Poynter Lista_Ordini){
    char r[256], c='a';
    int ingrediente_n, quantita, scadenza, Hk, k=HEAP_SIZE_M;
    size_t lunghezza;
    MHmagazzino ptr=NULL;
    while(c!='\n'){
        if(scanf("%s %d %d%c", r, &quantita, &scadenza, &c)>0){ //<ingrediente> <qt> <scadenza> ...
            if(scadenza>tempo){
                lunghezza=strlen(r);
                Hk=Funzione_hash_M(Magazzino->dim, r, lunghezza); //uscito da qui ho 3 casi: 1)puntatore a null =>inserisco
                ingrediente_n=Hk+r[lunghezza-1]+r[lunghezza-2]+r[lunghezza-3];
                if(Magazzino->vett[Hk]==NULL){ //caso 1)
                    Magazzino->vett[Hk]=(MHmagazzino)malloc(sizeof(MHm)); //alloco e inizializzo
                    Magazzino->vett[Hk]->qt_tot=quantita;
                    Magazzino->vett[Hk]->next=NULL;
                    // Magazzino->vett[Hk]->ingrediente=(char*)malloc(sizeof(char)*(strlen(r)+1));
                    // strcpy(Magazzino->vett[Hk]->ingrediente, r);
                    Magazzino->vett[Hk]->ingrediente_n=ingrediente_n;
                    //una volta allocato e inizializzato il nodo della lista per le collisioni alloco e inizializzo lo heap per quell'ingrediente
                    Magazzino->vett[Hk]->minHeap_M=(minHeap_M_P)malloc(sizeof(Heap_M));
                    Magazzino->vett[Hk]->minHeap_M->dim=HEAP_SIZE_M;
                    Magazzino->vett[Hk]->minHeap_M->numero_elem=0;
                    Magazzino->vett[Hk]->minHeap_M->vett=(Lotto*)malloc(sizeof(Lotto)*k);
                    Inserisci_M(Magazzino->vett[Hk]->minHeap_M, r, quantita, scadenza);
                }
                else if(Magazzino->vett[Hk]->ingrediente_n==ingrediente_n){ //caso 2) ho un nodo al primo posto già inizializzato per quell'ingrediente
                    Magazzino->vett[Hk]->qt_tot+=quantita;
                    if(Magazzino->vett[Hk]->minHeap_M->numero_elem==Magazzino->vett[Hk]->minHeap_M->dim-1)
                        Espandi_MinHeap(Magazzino->vett[Hk]->minHeap_M);
                    Inserisci_M(Magazzino->vett[Hk]->minHeap_M, r, quantita, scadenza);

                }
                else{ //caso 3)collisione, due sottocasi a)/b)
                    ptr=Magazzino->vett[Hk];
                    while((ptr->next!=NULL)&&(ptr->next->ingrediente_n!=ingrediente_n))
                        ptr=ptr->next;
                    if(ptr->next==NULL){ //a)il nodo non c'è =>ne alloco uno nuovo
                        ptr->next=(MHmagazzino)malloc(sizeof(MHm));
                        ptr->next->qt_tot=quantita;
                        ptr->next->next=NULL;
                        // ptr->next->ingrediente=(char*)malloc(sizeof(char)*(strlen(r)+1));
                        // strcpy(ptr->next->ingrediente, r);
                        ptr->next->ingrediente_n=ingrediente_n;
                        ptr->next->minHeap_M=(minHeap_M_P)malloc(sizeof(Heap_M));
                        ptr->next->minHeap_M->dim=HEAP_SIZE_M;
                        ptr->next->minHeap_M->numero_elem=0;
                        ptr->next->minHeap_M->vett=(Lotto*)malloc(sizeof(Lotto)*k);
                        Inserisci_M(ptr->next->minHeap_M, r, quantita, scadenza);
                    }
                    else{ //b) il nodo c'è già
                        ptr->next->qt_tot+=quantita;
                        if(ptr->next->minHeap_M->numero_elem==ptr->next->minHeap_M->dim-1)
                            Espandi_MinHeap(ptr->next->minHeap_M);
                        Inserisci_M(ptr->next->minHeap_M, r, quantita, scadenza);
                        // Controlla_Scadenze(ptr->next, ptr->next->RBT); posso evitare di fare le controlla_scadenze perche' le faccio in ctrleprep
                    }
                }
            }
        }
    }
    Controlla_Ordini_Attesa(Lista_Ordini, Magazzino, Ricettario); //voglio che controlli per ogni ricetta in lista d'attesa di avere gli ingredienti e se si la prepari
    printf("rifornito\n");
    return;
}

//calcola l'hash nel magazzino partendo dal nome dell'ingrediente
int Funzione_hash_M(int m, char* r, size_t lunghezza){
    int i, z, k=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        k=k*3+r[i];
    }
    z=(int)floor(((double) m)*(((double) k)*HASH_CONSTANT_M-floor(((double) k)*HASH_CONSTANT_M)));
    return z;
}

//inserisce un nuovo lotto nel minheap
void Inserisci_M (minHeap_M_P minHeap, char * r, int quantita, int scadenza){
    int i=0, k=0;
    Lotto tmp;
    minHeap->numero_elem++;
    minHeap->vett[minHeap->numero_elem-1].quantita=quantita;
    minHeap->vett[minHeap->numero_elem-1].scadenza=scadenza;
    i=minHeap->numero_elem-1;
    k=(i-1)/2;

    while((i>0)&&(minHeap->vett[k].scadenza>minHeap->vett[i].scadenza)){
        //swap: V[parent(i)], V[i]: copio in tmp il parent
        tmp.quantita=minHeap->vett[k].quantita;
        tmp.scadenza=minHeap->vett[k].scadenza;
        //copio i dati di V[i] in V[k]
        minHeap->vett[k].quantita=minHeap->vett[i].quantita;
        minHeap->vett[k].scadenza=minHeap->vett[i].scadenza;
        //ora metto tmp in V[i]
        minHeap->vett[i].scadenza=tmp.scadenza;
        minHeap->vett[i].quantita=tmp.quantita;
        i=(i-1)/2;
        k=(i-1)/2;
    }
    return;
}

//rimette a posto il minheap magazzino
void MinheapIfy_M(minHeap_M_P minHeap, int n){
    int l=2*n+1, r=2*n+2, posmax;
    Lotto tmp;
    if((l<minHeap->numero_elem)&&(minHeap->vett[l].scadenza<minHeap->vett[n].scadenza))
        posmax=l;
    else posmax=n;
    if((r<minHeap->numero_elem)&&(minHeap->vett[r].scadenza<minHeap->vett[posmax].scadenza))
        posmax=r;
    if(posmax!=n){
        //swap: V[parent(i)], V[i]: copio in tmp il parent
        tmp.quantita=minHeap->vett[n].quantita;
        tmp.scadenza=minHeap->vett[n].scadenza;
        //copio i dati di V[i] in V[k]
        minHeap->vett[n].quantita=minHeap->vett[posmax].quantita;
        minHeap->vett[n].scadenza=minHeap->vett[posmax].scadenza;
        //ora metto tmp in V[i]
        minHeap->vett[posmax].scadenza=tmp.scadenza;
        minHeap->vett[posmax].quantita=tmp.quantita;
        // free(tmp.ingrediente);
        MinheapIfy_M(minHeap, posmax);
    }
    return;
}

//cancella l'ingrediente a scadenza più prossima e restituisce il peso 
int Estrai_Min_M(minHeap_M_P minHeap){
    int peso=0;
    if(minHeap->numero_elem==0){
        return 0;
    }
    //salvo la quantita del lotto che sto eliminando
    if(minHeap->numero_elem==1){
        // free(minHeap->vett[0].ingrediente);
        minHeap->numero_elem--;
        return minHeap->vett[0].quantita;
    }
    peso=minHeap->vett[0].quantita;
    minHeap->vett[0].quantita=minHeap->vett[minHeap->numero_elem-1].quantita;
    minHeap->vett[0].scadenza=minHeap->vett[minHeap->numero_elem-1].scadenza;
    minHeap->numero_elem--;
    MinheapIfy_M(minHeap, 0);
    return peso;
}

//gli passo il minheap vecchio e lo ingrandisce
void Espandi_MinHeap(minHeap_M_P minHeap){
    Lotto * vett_new=NULL;
    int i=0, k=minHeap->numero_elem;
    minHeap->dim=minHeap->dim*2; //raddoppio la dim
    vett_new=(Lotto*)malloc(sizeof(Lotto)*(minHeap->dim));
    //copio tutti gli elementi del vecchio vettore in quello nuovo
    while(i<k){
        vett_new[i].quantita=minHeap->vett[i].quantita;
        vett_new[i].scadenza=minHeap->vett[i].scadenza;
        i++;
    }
    free(minHeap->vett); //dealloco il vecchio vettore e punto a quello nuovo
    minHeap->vett=vett_new;
    return;
}

// cancella lotti scaduti
void Controlla_Scadenze(MHmagazzino MHmag){ //ptr scorre i nodi dell'albero
    int peso; //forse ha più senso scorrere tutto il vettore, eliminare gli elemtni scaduti e chiamare la minheapify solo una volta, pero boh
    if(MHmag->minHeap_M->vett[0].scadenza>tempo)
        return;
    else{
        while((MHmag->minHeap_M->numero_elem>0)&&(MHmag->minHeap_M->vett[0].scadenza<=tempo)){ //elimina le scadenze finche' ci sono
            peso=Estrai_Min_M(MHmag->minHeap_M);
            MHmag->qt_tot-=peso;
        }
    }
    return;
}

// void Controlla_Scadenze(MHmagazzino MHmag){
//     int num=0, peso_tolto=0, i, k=MHmag->minHeap_M->numero_elem;
//     Lotto *vett;
//     vett=(Lotto *)malloc(sizeof(Lotto)*MHmag->minHeap_M->dim);
//     for(i=0; i<k; i++){
//         if(MHmag->minHeap_M->vett[i].scadenza>tempo){ //copio solo gli elementi non scaduti
//             vett[num].scadenza=MHmag->minHeap_M->vett[i].scadenza;
//             vett[num].quantita=MHmag->minHeap_M->vett[i].quantita;
//             num++;
//         }
//         else peso_tolto=MHmag->minHeap_M->vett[i].quantita;
//     }
//     free(MHmag->minHeap_M->vett);
//     MHmag->minHeap_M->vett=vett;
//     MHmag->qt_tot-=peso_tolto;
//     MHmag->minHeap_M->numero_elem=num+1;
//     k=(MHmag->minHeap_M->dim-1)/2;
//     for(i=k-1; i>=0; i--){
//         MinheapIfy_M(MHmag->minHeap_M, i);
//     }
// }

//scorre gli ordini in attesa e vede se sono preparabili
void Controlla_Ordini_Attesa(LO_Poynter Lista_Ordini, MAGAZZINO_P Magazzino, RICETTARIO_P Ricettario){ 
    Lista_Ordini_Attesa ptr=Lista_Ordini->OA, prev=NULL;
    int peso_ordine;                                                                    
    while(ptr!=NULL){             
        //non devo controllare se la ricetta c'è o no perché non può essere rimossa se è in attesa
        peso_ordine=ControllaEPrepara(ptr->Ricetta, Magazzino, Lista_Ordini, ptr->quantita, ptr->ordine, 1);
        if(peso_ordine){ //se è maggiore di 0 devo metterlo nella lista degli ordini pronti e eliminarlo da quella di attesa
            Enqueue_OP(Lista_Ordini->OP, ptr->ordine, ptr->quantita, peso_ordine, ptr->data);
            if(prev!=NULL){ //se è un nodo centrale nella lista o l'ultimo
                prev->next=ptr->next;
                free(ptr->ordine);
                free(ptr);
                ptr=prev;
                if(ptr->next==NULL)
                    Lista_Ordini->OA_ult=ptr;
            }
            else { //vuol dire che ptr è il primo nodo della lista
                Lista_Ordini->OA=ptr->next;
                free(ptr->ordine);
                free(ptr);
                ptr=Lista_Ordini->OA;
                if((ptr!=NULL)&&(ptr->next==NULL))
                    Lista_Ordini->OA_ult=ptr;
            }
        }
        else {
            prev=ptr;
            ptr=ptr->next;
        }
    }
    if(Lista_Ordini->OA==NULL)
        primo_OA=1;
    return;
}

//riceve l'ordine e verifica di avere la ricetta, e se e' preparabile lo aggiunge nello heap
void Ordine(RICETTARIO_P Ricettario, MAGAZZINO_P Magazzino, LO_Poynter Lista_Ordini){
    char r[256];
    int num, Rk, peso_ordine, ricetta_n;
    size_t lunghezza;
    RICETTA_p punt=NULL;
    if(scanf("%s %d", r, &num)>0){
        lunghezza=strlen(r);
        Rk=Funzione_hash_R(Ricettario->dim, r, lunghezza); //verifico di avere la ricetta
        punt=Ricettario->vett[Rk];
        ricetta_n=Rk+r[lunghezza-1]+r[lunghezza-2]*r[lunghezza-3]*r[lunghezza-4]; //numero unico per le ricette, come per gli ingredienti

        while((punt!=NULL)&&(punt->ricetta_n!=ricetta_n))
            punt=punt->prox;
        if((punt==NULL)){
            printf("rifiutato\n");
            return;
        }
        else{
            //Per rendere più facile controlla_ordini_attesa voglio scrivere una funzione separata che,
            //data una ricetta, controlli di avere gli ingredienti, li tiene con una lista e se li ho tutti prepara la ricetta
            peso_ordine=ControllaEPrepara(punt, Magazzino, Lista_Ordini, num, r, 0);
            if(peso_ordine){
                printf("accettato\n");
                Enqueue_OP(Lista_Ordini->OP, r, num, peso_ordine, tempo);
                }
        }
    }
    return;
}

//verifica di avere gli ingredienti e se li ha li rimuove preparando l'ordine
int ControllaEPrepara(RICETTA_p Ricetta, MAGAZZINO_P Magazzino, LO_Poynter Lista_Ordini, int num, char * r, int flag){
    // if(flag)
    //     printf("arrivo da controlla_ordini_attesa\n");
    // else 
    //     printf("arrivo da ordine\n");
    int i, n, peso_tolto, k=Ricetta->n_ingredienti, ingrediente_n=0; 
    MHmagazzino punt=NULL, Check[k];
    for(i=0; i<Ricetta->n_ingredienti; i++){
        n=Ricetta->vett[i].quantita*num; //quantita totale per quell'ingrediente, in questo ordine 
        ingrediente_n=Ricetta->vett[i].ingrediente_n;
        if(Magazzino->vett[Ricetta->vett[i].hash]==NULL){ //non c'è alcuna quantita di quell' ingrediente
            
            if(flag){
                return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
            }
            Enqueue_OA(Lista_Ordini, r, num, Ricetta);
            printf("accettato\n");
            return 0;
        }
        else{
            punt=Magazzino->vett[Ricetta->vett[i].hash]; //vuol dire che almeno un nodo in quel bucket della tabella c'è
            while((punt!=NULL)&&(ingrediente_n!=punt->ingrediente_n)) //cerco il nodo corrispondente a quell'ingrediente
                punt=punt->next;
            if(punt==NULL){ //il nodo non c'è
                if(flag){
                    return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                }
                Enqueue_OA(Lista_Ordini, r, num, Ricetta);
                printf("accettato\n");
                return 0;
            }
            else{ //esiste il nodo per quell'ingrediente
                if(punt->minHeap_M->numero_elem>0){
                    Controlla_Scadenze(punt);
                    if(punt->qt_tot<n){ //vuol dire che quel minHeap non ha abbastanza quantita per soddisfare la ricetta
                        if(flag){
                            return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                        }
                        Enqueue_OA(Lista_Ordini, r, num, Ricetta);
                        printf("accettato\n");
                        return 0;
                        }  
                    else{
                        Check[i]=punt;
                    }
                }      
                else{
                    if(flag){
                        return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                    }
                    Enqueue_OA(Lista_Ordini, r, num, Ricetta);
                    printf("accettato\n");
                    return 0;                 
                }   
            }
        }
    } 
    for(i=0; i<Ricetta->n_ingredienti; i++){
        n=Ricetta->vett[i].quantita*num;
        while((Check[i]->minHeap_M->numero_elem>0)&&(n>=Check[i]->minHeap_M->vett[0].quantita)){ //devo inserire il controllo sul numero di elementi perche' può essere che ci siano esattamente le quantita giuste
            peso_tolto=Estrai_Min_M(Check[i]->minHeap_M);
            Check[i]->qt_tot-=peso_tolto;
            n-=peso_tolto; 
        }
        if(n>0){
            Check[i]->minHeap_M->vett[0].quantita-=n;
            Check[i]->qt_tot-=n;
        }
    }
    return Ricetta->peso*num;;
}

//aggiunge un ordine nella coda degli ordini in attesa
void Enqueue_OA(LO_Poynter Lista, char * r, int num, RICETTA_p Ricetta){
    Lista_Ordini_Attesa ptr=NULL;
    size_t lunghezza=strlen(r);
    ptr=(Lista_Ordini_Attesa)malloc(sizeof(Ordine_in_attesa)); //alloco e inizializzo nuovo nodo
    ptr->data=tempo;
    ptr->quantita=num;
    ptr->Ricetta=Ricetta;
    ptr->ordine=(char*)malloc(sizeof(char)*(lunghezza+1));
    strcpy(ptr->ordine, r);
    ptr->next=NULL;
    if(primo_OA){ //se è primo devo metterlo come primo nodo
        Lista->OA=ptr;
        Lista->OA_ult=ptr;
        primo_OA=0;
    }
    else {
        Lista->OA_ult->next=ptr;
        Lista->OA_ult=Lista->OA_ult->next;
    }
    return;
}

//aggiunge nel minheap un ordine pronto
void Enqueue_OP(minHeap_P minHeap, char *r, int num, int peso_ordine, int tempo){ //analogo della funzione Heap_Insert
    int i=0, k=0;
    Ordine_Stampabile tmp;  //posso poi dichiarare un puntatore a Ordine_Stampabile ptr=minHeap->V per fare piu veloce
    minHeap->numero_elem++; //incremento il numero di elementi
    minHeap->V[minHeap->numero_elem-1].data=tempo; //inserisco il nuovo Ordine pronto
    minHeap->V[minHeap->numero_elem-1].peso=peso_ordine;
    minHeap->V[minHeap->numero_elem-1].quantita=num;
    minHeap->V[minHeap->numero_elem-1].ordine=(char*)malloc(sizeof(char)*(strlen(r)+1));
    strcpy(minHeap->V[minHeap->numero_elem-1].ordine, r);
    i=minHeap->numero_elem-1;
    k=(i-1)/2; //indica l'indice del parent(i)

    while((i>0)&&(minHeap->V[k].data>minHeap->V[i].data)){
        //swap V[parent(i)], V[i]:
        tmp.data=minHeap->V[k].data;
        tmp.peso=minHeap->V[k].peso;
        tmp.quantita=minHeap->V[k].quantita;
        tmp.ordine=(char*)malloc(sizeof(char)*(strlen(minHeap->V[k].ordine)+1));
        strcpy(tmp.ordine, minHeap->V[k].ordine);
        //è uno swap, faccio tmp=V[K]
        minHeap->V[k].data=minHeap->V[i].data;
        minHeap->V[k].peso=minHeap->V[i].peso;
        minHeap->V[k].quantita=minHeap->V[i].quantita;
        free(minHeap->V[k].ordine);
        minHeap->V[k].ordine=(char*)malloc(sizeof(char)*(strlen(minHeap->V[i].ordine)+1));
        strcpy(minHeap->V[k].ordine, minHeap->V[i].ordine);
        //v[k]=v[i];
        minHeap->V[i].data=tmp.data;
        minHeap->V[i].peso=tmp.peso;
        minHeap->V[i].quantita=tmp.quantita;
        free(minHeap->V[i].ordine);
        minHeap->V[i].ordine=(char*)malloc(sizeof(char)*(strlen(tmp.ordine)+1));
        strcpy(minHeap->V[i].ordine, tmp.ordine);
        //v[i]=tmp
        free(tmp.ordine);
        i=(i-1)/2; //i=parent(i)
        k=(i-1)/2;  //k=parent(i)
    }
}

// estrae il minimo finché non arriva a capienza massima di peso
int Estrai_Min(minHeap_P minHeap){
    int peso_tot=0, n=0; //n conta quanti elementi estraggo
    if(minHeap->numero_elem==0){
        return 0; //se l'heap non ha elementi
    }
    while((minHeap->numero_elem-1>0)&&(peso_tot+minHeap->V[0].peso<=capienza)){ //finche ci sta nel camioncino estrai il minimo
        //metto in coda il massimo
        peso_tot+=minHeap->V[0].peso; //peso tot = peso tot + peso dell'ordine che ho caricato
        Coda_OS(minHeap->LS, minHeap->V[0].ordine, minHeap->V[0].quantita, minHeap->V[0].peso, minHeap->V[0].data);
        //copio i dati dell'ultimo nel primo
        minHeap->V[0].data=minHeap->V[minHeap->numero_elem-1].data;
        minHeap->V[0].peso=minHeap->V[minHeap->numero_elem-1].peso;
        minHeap->V[0].quantita=minHeap->V[minHeap->numero_elem-1].quantita;
        free(minHeap->V[0].ordine);
        minHeap->V[0].ordine=(char*)malloc(sizeof(char)*(strlen(minHeap->V[minHeap->numero_elem-1].ordine)+1));
        strcpy(minHeap->V[0].ordine, minHeap->V[minHeap->numero_elem-1].ordine);
        free(minHeap->V[minHeap->numero_elem-1].ordine); //la dealloco che cosi se devo reinserire non ho problemi
        minHeap->numero_elem--;
        n++;
        //ricostruisco il minheap con il nuovo elemento in cima
        MinHeapIfy(minHeap, 0);
    }
    if((minHeap->numero_elem>0)&&(peso_tot+minHeap->V[0].peso<=capienza)){ //può essere che sia rimasto un elemento nello heap
        Coda_OS(minHeap->LS, minHeap->V[0].ordine, minHeap->V[0].quantita, minHeap->V[0].peso, minHeap->V[0].data);
        free(minHeap->V[0].ordine);
        minHeap->V[0].ordine=NULL;
        minHeap->numero_elem--;
        n++;
    }
    return n;
}

//aggiunge in coda gli ordini che poi materializzerò nel vettore che poi mergesort ordinerà
void Coda_OS(Lista_Ordini_Stampabili * OS, char * r, int num, int peso_ordine, int data){ //potrei renderla davvero una coda per fare l'inserimento in O(1)
    Lista_OS ptr;
    ptr=(Lista_OS)malloc(sizeof(Nodo_Lista_OS));
    ptr->data=data;
    ptr->peso=peso_ordine;
    ptr->quantita=num;
    ptr->ordine=(char*)malloc(sizeof(char)*(strlen(r)+1));
    strcpy(ptr->ordine, r);
    ptr->next=NULL;
    if(primo_OS){ //se è primo devo metterlo come primo nodo
        OS->Primo_ordine=ptr;
        OS->Ultimo_ordine=ptr;
        primo_OS=0;
    }
    else {
        OS->Ultimo_ordine->next=ptr;
        OS->Ultimo_ordine=OS->Ultimo_ordine->next;
    }
    return;
}

//ricrea il minheap a seguito di un estrazione
void MinHeapIfy(minHeap_P minHeap, int n){
    int l=2*n+1, r=2*n+2, posmax;
    Ordine_Stampabile tmp;
    if((l<=minHeap->numero_elem)&&(minHeap->V[l].data<minHeap->V[n].data))
        posmax=l;
    else posmax=n;
    if((r<minHeap->numero_elem)&&(minHeap->V[r].data<minHeap->V[posmax].data))
        posmax=r;
    if(posmax!=n){
        tmp.data=minHeap->V[n].data;
        tmp.peso=minHeap->V[n].peso;
        tmp.quantita=minHeap->V[n].quantita;
        tmp.ordine=(char*)malloc(sizeof(char)*(strlen(minHeap->V[n].ordine)+1));
        strcpy(tmp.ordine, minHeap->V[n].ordine);
        //è uno swap, faccio tmp=V[n]
        minHeap->V[n].data=minHeap->V[posmax].data;
        minHeap->V[n].peso=minHeap->V[posmax].peso;
        minHeap->V[n].quantita=minHeap->V[posmax].quantita;
        free(minHeap->V[n].ordine);
        minHeap->V[n].ordine=(char*)malloc(sizeof(char)*(strlen(minHeap->V[posmax].ordine)+1));
        strcpy(minHeap->V[n].ordine, minHeap->V[posmax].ordine);
        //v[n]=v[posmax];
        minHeap->V[posmax].data=tmp.data;
        minHeap->V[posmax].peso=tmp.peso;
        minHeap->V[posmax].quantita=tmp.quantita;
        free(minHeap->V[posmax].ordine);
        minHeap->V[posmax].ordine=(char*)malloc(sizeof(char)*(strlen(tmp.ordine)+1));
        strcpy(minHeap->V[posmax].ordine, tmp.ordine);
        //v[posmax]=tmp
        free(tmp.ordine);
        MinHeapIfy(minHeap, posmax);
    }
    return;
}

//riceve in ingresso la lista e la materializza nel vettore, liberando al contempo la lista
void Riempi_vett(Ordine_Stampabile *v, minHeap_P minHeap){
    Lista_OS ptr=minHeap->LS->Primo_ordine, tmp;
    int i=0;
    while(ptr!=NULL){ //finche ptr è diverso da null devo copiare i suoi dati in v e poi farne la free
        v[i].data=ptr->data;
        v[i].peso=ptr->peso;
        v[i].quantita=ptr->quantita;
        v[i].ordine=(char*)malloc(sizeof(char)*(strlen(ptr->ordine)+1));
        strcpy(v[i].ordine, ptr->ordine);
        tmp=ptr->next;
        free(ptr->ordine);
        free(ptr);
        ptr=tmp;
        i++;
    }
    minHeap->LS->Primo_ordine=NULL;
    minHeap->LS->Ultimo_ordine=NULL;
    primo_OS=1;
    return;
}

//stampa gli ordini caricati nel furgoncino
void Corriere(LO_Poynter LO, minHeap_P OP){
    Ordine_Stampabile * v;
    int i, n=Estrai_Min(OP);
    if(n==0) {//se l'albero degli ordini pronti è vuoto
        printf("camioncino vuoto\n");
        return;
    }
    v=(Ordine_Stampabile *)malloc(sizeof(Ordine_Stampabile)*(n)); //ottengo un vettore dove linearizzare l'albero
    Riempi_vett(v, OP); //riempio il vettore con gli elementi da ordinare e stampare
    if(n>1) //lo chiamo solo se il vettore è più grande di 1
        MergeSort(v, 0, n-1); //ottengo il vettore ordinato in ordine crescente di peso
    for(i=n-1; i>=0; i--){
        printf("%d %s %d\n", v[i].data, v[i].ordine, v[i].quantita);
        free(v[i].ordine);
    }
    free(v);   
    return;
}

//ordina per peso il vettore di ordini pronti
void MergeSort(Ordine_Stampabile * v, int p, int r){ //lo ordino in ordine crescente e poi stampo al contrario
    int q;
    Ordine_Stampabile tmp;
    if(p<r){
        q=(p+r)/2;
        MergeSort(v, p, q);         
        MergeSort(v, q+1, r);       
        Merge(v, p, q, r);      
    }
    else{ //caso base della ricorsione, 2 elementi
        if(v[p].peso>v[r].peso){
            tmp.data=v[r].data;
            tmp.peso=v[r].peso;
            tmp.quantita=v[r].quantita;
            tmp.ordine=(char*)malloc(sizeof(char)*(strlen(v[r].ordine)+1));
            strcpy(tmp.ordine, v[r].ordine);
            //è uno swap, faccio tmp=v[r]
            v[r].data=v[p].data;
            v[r].peso=v[p].peso;
            v[r].quantita=v[p].quantita;
            free(v[r].ordine);
            v[r].ordine=(char*)malloc(sizeof(char)*(strlen(v[p].ordine)+1));
            strcpy(v[r].ordine, v[p].ordine);
            //v[r]=v[p];
            v[p].data=tmp.data;
            v[p].peso=tmp.peso;
            v[p].quantita=tmp.quantita;
            free(v[p].ordine);
            v[p].ordine=(char*)malloc(sizeof(char)*(strlen(tmp.ordine)+1));
            strcpy(v[p].ordine, tmp.ordine);
            //v[p]=tmp
            free(tmp.ordine);
        }
        else if(v[p].peso==v[r].peso){
            if(v[p].data>v[r].data){
                tmp.data=v[r].data;
                tmp.peso=v[r].peso;
                tmp.quantita=v[r].quantita;
                tmp.ordine=(char*)malloc(sizeof(char)*(strlen(v[r].ordine)+1));
                strcpy(tmp.ordine, v[r].ordine);
                //è uno swap, faccio tmp=v[r]
                v[r].data=v[p].data;
                v[r].peso=v[p].peso;
                v[r].quantita=v[p].quantita;
                free(v[r].ordine);
                v[r].ordine=(char*)malloc(sizeof(char)*(strlen(v[p].ordine)+1));
                strcpy(v[r].ordine, v[p].ordine);
                //v[r]=v[p];
                v[p].data=tmp.data;
                v[p].peso=tmp.peso;
                v[p].quantita=tmp.quantita;
                free(v[p].ordine);
                v[p].ordine=(char*)malloc(sizeof(char)*(strlen(tmp.ordine)+1));
                strcpy(v[p].ordine, tmp.ordine);
                //v[p]=tmp
                free(tmp.ordine);
            }
        }
    }
    return;
}

//chiude le chiamate ricorsive di mergesort
void Merge(Ordine_Stampabile * v, int p, int q, int r){ //è venuta più grande di quello che mi aspettassi
    int len1=q-p+1, len2=r-q, i, j, x, k;
    Ordine_Stampabile L[len1], R[len2];     //per risolvere qui prova ad allocarli con una malloc
    for(i=0; i<len1; i++){  //inizializza il vettore L
        x=p+i;    
        L[i].data=v[x].data;
        L[i].peso=v[x].peso;
        L[i].quantita=v[x].quantita;
        L[i].ordine=(char*)malloc(sizeof(char)*(strlen(v[x].ordine)+1));
        strcpy(L[i].ordine, v[x].ordine);
    }
    for(i=0; i<len2; i++){  //inizializza il vettore R
        x=q+i+1;
        R[i].data=v[x].data;
        R[i].peso=v[x].peso;
        R[i].quantita=v[x].quantita;
        R[i].ordine=(char*)malloc(sizeof(char)*(strlen(v[x].ordine)+1));
        strcpy(R[i].ordine, v[x].ordine);
    }
    i=0; 
    j=0;
    k=p;
    while((i<len1)&&(j<len2)){  //while grosso più grosso per inserire gli elementi in ordine di peso
        if(L[i].peso<R[j].peso){
            v[k].data=L[i].data;
            v[k].peso=L[i].peso;
            v[k].quantita=L[i].quantita;
            free(v[k].ordine);
            v[k].ordine=(char*)malloc(sizeof(char)*(strlen(L[i].ordine)+1));
            strcpy(v[k].ordine, L[i].ordine);
            free(L[i].ordine);
            i++;
        }
        else if(L[i].peso==R[j].peso){  //è il caso più rognoso perchè devo andare a guardare la data se hanno peso uguale
            if(L[i].data>R[j].data){
                v[k].data=L[i].data;
                v[k].peso=L[i].peso;
                v[k].quantita=L[i].quantita;
                free(v[k].ordine);
                v[k].ordine=(char*)malloc(sizeof(char)*(strlen(L[i].ordine)+1));
                strcpy(v[k].ordine, L[i].ordine);
                free(L[i].ordine);
                i++;
            }
            else{
                v[k].data=R[j].data;
                v[k].peso=R[j].peso;
                v[k].quantita=R[j].quantita;
                free(v[k].ordine);
                v[k].ordine=(char*)malloc(sizeof(char)*(strlen(R[j].ordine)+1));
                strcpy(v[k].ordine, R[j].ordine);
                free(R[j].ordine);
                j++;
            }
        }
        else{
            v[k].data=R[j].data;
            v[k].peso=R[j].peso;
            v[k].quantita=R[j].quantita;
            free(v[k].ordine);
            v[k].ordine=(char*)malloc(sizeof(char)*(strlen(R[j].ordine)+1));
            strcpy(v[k].ordine, R[j].ordine);
            free(R[j].ordine);
            j++;
        }
        k++;
    }
    while(i<len1){      //devo copiare i rimanenti elementi di uno dei due array
        v[k].data=L[i].data;
        v[k].peso=L[i].peso;
        v[k].quantita=L[i].quantita;
        free(v[k].ordine);
        v[k].ordine=(char*)malloc(sizeof(char)*(strlen(L[i].ordine)+1));
        strcpy(v[k].ordine, L[i].ordine);
        free(L[i].ordine);
        i++;
        k++;
    }   
    while(j<len2){  //può essere o R o L, faccio due while
        v[k].data=R[j].data;
        v[k].peso=R[j].peso;
        v[k].quantita=R[j].quantita;
        free(v[k].ordine);
        v[k].ordine=(char*)malloc(sizeof(char)*(strlen(R[j].ordine)+1));
        strcpy(v[k].ordine, R[j].ordine);
        free(R[j].ordine);
        j++;
        k++;
    }
    return;
}
