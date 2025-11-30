//versione con controlla e prepara senza check e heap al posto della lista OP
//versione con qt tot negli rbt magazzino

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//8192, 16384, 32768, 65536
#define DIM_RIC_BASE 32768
#define DIM_MAG_BASE 32768
#define HASH_CONSTANT_R ((double) (sqrt(5)-1)/2)
#define HASH_CONSTANT_M ((double) (sqrt(5)-1)/2)
#define HEAP_SIZE 32768
//opzioni di compilazione: gcc -Wall -Werror -std=gnu11 -g3 -O2 -lm primastesura.c -o primastesura

//albero nel magazzino (Elemento Magazzino):
typedef struct EM {
    char *ingrediente; //metto come ingrediente 'N' per rappresentare il riferimento a T.null, ma facilmente identificabile
    int quantita;
    int scadenza;
    char color; //r=rosso, n=nero
    struct EM * left;
    struct EM * right;
    struct EM * parent;
} Lotto;

//RBT nel magazzino, in una cella, con un puntatore a next per gestire le collisioni
typedef struct RBM{
    char * ingrediente;
    int qt_tot;
    Lotto * RBT;
    struct RBM * next;
} RBTm;
//puntatori che escono fuori dal magazzino->vett[i]
typedef RBTm * RBTmagazzino;

//hash table magazzino
typedef struct {
    int numero_ingredienti;
    int dimensione;
    RBTmagazzino * vett; //vettore contenete le testate degli alberi dei lotti;
} MAGAZZINO;
typedef MAGAZZINO* MAGAZZINO_P;

//ingrediente in una ricetta:
typedef struct IN {
    char * nome;
    int quantita;
    int hash;
    struct IN * next;
} Ingrediente;
//elemento della tabella hash RICETTARIO, con prox per gestire le collisioni e next che punta all'ingrediente
typedef struct RIC {
    char * nome;
    int n_ingredienti;
    int peso;
    Ingrediente * next;
    struct RIC *prox;
} Ricetta;
//puntatore che esce fuori da Ricettario->vett[i]
typedef Ricetta * RICETTA_p;

//tabella hash per il ricettario:
typedef struct {
    int numero_ricette;
    int dimensione;
    RICETTA_p * vett; //vettore di ricette
} RICETTARIO;
typedef RICETTARIO * RICETTARIO_P;

//lista Ordini in Attesa (semplice coda FIFO)
typedef struct OA {
    char * ordine;
    int quantita;
    int data;
    int hash;
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

//typedef per lo heap
typedef struct {
    int dimensione;
    int numero_elem;
    Ordine_Stampabile * V;
    Lista_OS Ordini_da_Stampare;
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
int Funzione_hash_R_new(RICETTARIO_P, int, char *, size_t, int);
void Rimuovi_Ricetta(RICETTARIO_P, LO_Poynter);
int Check_Attesa(Lista_Ordini_Attesa, char *); //controlla se c'è un ordine in attesa per quella ricetta
int Check_Ordini_Pronti(minHeap_P, char *); //controlla se c'è un ordine pronto per quella ricetta

//gestione del magazzino:
void Rifornimento (MAGAZZINO_P, RICETTARIO_P, LO_Poynter);
int Funzione_hash_M(MAGAZZINO_P, int, char*, size_t);
void RB_Insert_M(RBTmagazzino, char *, int, int);
void RB_Insert_Fixup_M(RBTmagazzino, Lotto *);
void LeftRotate(RBTmagazzino, Lotto *);
void RightRotate(RBTmagazzino, Lotto *);
void Controlla_Scadenze(RBTmagazzino, Lotto*);
void RB_Delete_M(RBTmagazzino, Lotto *);
void RB_Delete_Fixup_M(RBTmagazzino, Lotto *);
Lotto * RB_Successor(Lotto *);
Lotto * RB_Min(Lotto *);
void Controlla_Ordini_Attesa(LO_Poynter, MAGAZZINO_P, RICETTARIO_P); //controlla per le ricette in attesa se ora ci sono gli ingredienti 

void Ordine(RICETTARIO_P, MAGAZZINO_P, LO_Poynter);
int ControllaEPrepara(RICETTA_p, MAGAZZINO_P, Ingrediente *, LO_Poynter, int, char *, int, int);
void Enqueue_OA(LO_Poynter, char *, int, int, RICETTA_p);
void Enqueue_OP(minHeap_P ,char *, int, int, int);
int Estrai_Min(minHeap_P);
Lista_OS Coda_OS(Lista_OS OS, char *, int, int, int);
void MinHeapIfy(minHeap_P, int);
void Riempi_vett(Ordine_Stampabile *, minHeap_P);

void Corriere(LO_Poynter, minHeap_P);
void MergeSort(Ordine_Stampabile *, int, int);
void Merge(Ordine_Stampabile *, int, int, int);


int capienza=0, periodicita=0, tempo=0; //3 variabili globali per comodità
Lotto * T_null=NULL; //nodo T_null per gli rbt del magazzino
int primo=1;
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
    Ricettario->numero_ricette=0;
    Ricettario->dimensione=DIM_RIC_BASE;

    //alloco l'unico nodo T.null per gli rbtmagazzino
    T_null=(Lotto*)malloc(sizeof(Lotto));
    T_null->ingrediente=(char*)malloc(sizeof(char));
    T_null->ingrediente[0]='N';
    T_null->color='b';
    T_null->quantita=0;
    T_null->scadenza=0;
    T_null->left=NULL;
    T_null->right=NULL;
    T_null->parent=NULL;

    //alloco e inizializzo Magazzino:
    MAGAZZINO_P Magazzino;
    Magazzino=(MAGAZZINO_P)malloc(sizeof(MAGAZZINO));
    Magazzino->vett=(RBTmagazzino *)malloc(sizeof(RBTmagazzino)*DIM_MAG_BASE); //vettore di puntatori a testate di alberi
    for(i=0; i<DIM_MAG_BASE; i++){
        Magazzino->vett[i]=NULL;
    }
    Magazzino->dimensione=DIM_MAG_BASE;  

    //alloco e inizializzo minHeap:
    minHeap_P minHeap;
    minHeap=(minHeap_P)malloc(sizeof(Heap));
    minHeap->V=(Ordine_Stampabile *)calloc(HEAP_SIZE, sizeof(Ordine_Stampabile));
    minHeap->dimensione=HEAP_SIZE;
    minHeap->numero_elem=0;
    minHeap->Ordini_da_Stampare=NULL;

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
    int k=0, Hk=0, n_ingredienti=0, peso=0, prof=1;
    size_t lunghezza; //conterra il risultato della strlen
    Ingrediente * ptr=NULL; //serve per allocare gli ingredienti della ricetta
    Ricetta * punt=NULL, *prev=NULL; //scorrono la lista nel bucket, tra le varie ricette (che collidono)
    if(scanf("%s", r)>0){ //copio il nome della ricetta
        lunghezza=strlen(r);
        Hk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 0);
        if(Ricettario->vett[Hk]==NULL){ //inserimento su cella vuota 
            Ricettario->vett[Hk]=(RICETTA_p)malloc(sizeof(Ricetta));
            Ricettario->vett[Hk]->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
            strcpy(Ricettario->vett[Hk]->nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
            Ricettario->vett[Hk]->next=(Ingrediente*)malloc(sizeof(Ingrediente)); //alloca un nodo per il primo ingrediente
            ptr=Ricettario->vett[Hk]->next; //ptr scorre gli ingredienti
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita>
                    n_ingredienti++;
                    peso+=k;
                    lunghezza=strlen(r); //lunghezza nome ingrediente
                    ptr->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //alloca una stringa con la dimensione giusta per contenere il nome dell'ingrediente
                    strcpy(ptr->nome, r);  //salva il nome dell'ingrediente
                    ptr->quantita=k;        //salva la quantita
                    ptr->hash=Funzione_hash_M(Magazzino, Magazzino->dimensione, r, strlen(r));
                    if(c!='\n'){ //alloca un altro nodo, perche' c'e' uno spazio e non va a capo=>il comando non e' finito
                        ptr->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                        ptr=ptr->next;
                    }
                    else ptr->next=NULL;
                }
            }   
            Ricettario->vett[Hk]->n_ingredienti=n_ingredienti;
            Ricettario->vett[Hk]->peso=peso;
            Ricettario->vett[Hk]->prox=NULL; //prox nel bucket
            printf("aggiunta\n");
            return;
        } 
        else if(strcmp(Ricettario->vett[Hk]->nome, r)!=0){ //caso in cui è già presente una ricetta ma con nome diverso =>cerco da Ricettario->vett[Hk]->prox
            if(Ricettario->vett[Hk]->prox==NULL){ //inserisco direttamente su quella dopo, nel bucket c'e' solo una ricetta
                Ricettario->vett[Hk]->prox=(Ricetta*)malloc(sizeof(Ricetta));
                Ricettario->vett[Hk]->prox->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
                strcpy(Ricettario->vett[Hk]->prox->nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
                Ricettario->vett[Hk]->prox->next=(Ingrediente*)malloc(sizeof(Ingrediente)); //alloca un nodo per il primo ingrediente
                ptr=Ricettario->vett[Hk]->prox->next;
                while(c!='\n'){
                    if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita> ...
                        n_ingredienti++;
                        peso+=k;
                        lunghezza=strlen(r); //lunghezza nome ingrediente
                        ptr->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //alloca una stringa con la dimensione giusta per contenere il nome dell'ingrediente
                        strcpy(ptr->nome, r);  //salva il nome dell'ingrediente
                        ptr->quantita=k;        //salva la quantita
                        ptr->hash=Funzione_hash_M(Magazzino, Magazzino->dimensione, r, strlen(r));
                        if(c!='\n'){ //alloca un altro nodo
                            ptr->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                            ptr=ptr->next;
                        }
                        else ptr->next=NULL;
                    }
                }   
                Ricettario->vett[Hk]->prox->n_ingredienti=n_ingredienti;
                Ricettario->vett[Hk]->prox->peso=peso;
                Ricettario->vett[Hk]->prox->prox=NULL;
                printf("aggiunta\n");
                return;
            }
            else{ //caso in cui la seconda e' !=NULL, scorro finche' non arrivo alla fine o trovo che c'e' gia' una ricetta con quel nome
                punt=Ricettario->vett[Hk]->prox;
                while((punt!=NULL)&&(strcmp(punt->nome, r)!=0)){
                    prev=punt; //tengo un riferimento a prev per inserire il nuovo nodo piu' facilmente
                    punt=punt->prox;
                    prof++;
                }
                if(punt==NULL){ //se c'è spazio vuoto aggiungo
                    prev->prox=(Ricetta *)malloc(sizeof(Ricetta));
                    prev->prox->nome=(char*)malloc(sizeof(char)*(strlen(r)+1));
                    strcpy(prev->prox->nome, r);
                    prev->prox->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                    ptr=prev->prox->next;
                    while(c!='\n'){
                        if(scanf("%s %d%c", r, &k, &c)>0){ //<ingrediente> <quantita> ...
                            n_ingredienti++;
                            peso+=k;
                            lunghezza=strlen(r); //lunghezza nome ingrediente
                            ptr->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //alloca una stringa con la dimensione giusta per contenere il nome dell'ingrediente
                            strcpy(ptr->nome, r);  //salva il nome dell'ingrediente
                            ptr->quantita=k;        //salva la quantita
                            ptr->hash=Funzione_hash_M(Magazzino, Magazzino->dimensione, r, strlen(r));
                            if(c!='\n'){ //alloca un altro nodo
                                ptr->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                                ptr=ptr->next;
                            }
                            else ptr->next=NULL;
                        }
                    }
                    prev->prox->n_ingredienti=n_ingredienti;
                    prev->prox->peso=peso;
                    prev->prox->prox=NULL;
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
        else if(strcmp(Ricettario->vett[Hk]->nome, r)==0){  //se la ricetta è già presente nel primo nodo
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0); //consumo tutta la riga
            }
            printf("ignorato\n");
            return;
        }
    }
}

//calcola l'indice della tabella hash_Ric partendo dal nome della ricetta
int Funzione_hash_R_new(RICETTARIO_P Ricettario, int m, char * r, size_t lunghezza, int p){  
    int i=0, z=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        z^=r[i];
    }
    z=(int)floor(((double) m)*(((double) z)*HASH_CONSTANT_M-floor(((double) z)*HASH_CONSTANT_M)));
    return z;
}

//rimuove la ricetta, se presente e se non è in attesa/pronto
void Rimuovi_Ricetta(RICETTARIO_P Ricettario, LO_Poynter Lista_Ordini){
    char r[256];
    int Hk;             
    size_t lunghezza;
    Ingrediente * ptr=NULL, * tmp=NULL;
    Ricetta * punt=NULL, * prev=NULL;
    if(scanf("%s", r)>0){
        lunghezza=strlen(r);
        Hk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 1);
        punt=Ricettario->vett[Hk];
        while((punt!=NULL)&&(strcmp(punt->nome, r)!=0)){ //scorro il bucket finche o trovo null o la ricetta
            prev=punt;
            punt=punt->prox;
        }
        if(punt!=NULL){ //in questo momento se entro nell'if, punt punta alla ricetta da rimuovere
            if((Check_Attesa(Lista_Ordini->OA, r))+(Check_Ordini_Pronti(Lista_Ordini->OP, r))){ //controllo che non ci siano ordini in attesa/pronto con quella ricetta
                printf("ordini in sospeso\n");
                return;
            }
            else{//in questo caso la ricetta va rimossa
                ptr=punt->next;
                while(ptr!=NULL){ //dealloco tutti gli ingredienti
                    tmp=ptr->next;
                    free(ptr->nome);
                    free(ptr);
                    ptr=tmp;
                }
                free(punt->nome);
                //orda distinguo 3 casi, a seconda che punt sia un nodo iniziale, centrale o finale della lista nel bucket
                if(punt==Ricettario->vett[Hk]){ //sono nel primo nodo del bucket
                    Ricettario->vett[Hk]=punt->prox;  //aggancio il vettore al secondo elemento e elimino punt (va bene sia che sia null che no)
                    free(punt);
                    printf("rimossa\n");
                    //ritorno all'inizio del bucket e riassegno le profondita'
                    punt=Ricettario->vett[Hk];
                    return;
                }
                else if(punt->prox==NULL){ //se è l'ultimo nodo della lista
                    prev->prox=NULL;
                    free(punt);
                    printf("rimossa\n");
                    //ritorno all'inizio del bucket e riassegno le profondita'
                    punt=Ricettario->vett[Hk];
                    return;
                }
                else{//è un nodo centrale
                    prev->prox=punt->prox;
                    free(punt);
                    printf("rimossa\n");
                    //ritorno all'inizio del bucket e riassegno le profondita'
                    punt=Ricettario->vett[Hk];
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
    int quantita, scadenza, Hk;
    size_t lunghezza;
    RBTmagazzino ptr=NULL;
    while(c!='\n'){
        if(scanf("%s %d %d%c", r, &quantita, &scadenza, &c)>0){ //<ingrediente> <qt> <scadenza> ...
            if(scadenza>tempo){
                lunghezza=strlen(r);
                Hk=Funzione_hash_M(Magazzino, Magazzino->dimensione, r, lunghezza); //uscito da qui ho 3 casi: 1)puntatore a null =>inserisco
                if(Magazzino->vett[Hk]==NULL){ //caso 1)
                    Magazzino->vett[Hk]=(RBTmagazzino)malloc(sizeof(RBTm)); //alloco e inizializzo
                    Magazzino->vett[Hk]->qt_tot=quantita;
                    Magazzino->vett[Hk]->next=NULL;
                    Magazzino->vett[Hk]->ingrediente=(char*)malloc(sizeof(char)*(strlen(r)+1));
                    strcpy(Magazzino->vett[Hk]->ingrediente, r);
                    Magazzino->vett[Hk]->RBT=T_null;
                    RB_Insert_M(Magazzino->vett[Hk], r, quantita, scadenza);
                }
                else if(strcmp(Magazzino->vett[Hk]->ingrediente, r)==0){ //caso 2) ho un nodo al primo posto già inizializzato per quell'ingrediente
                    Magazzino->vett[Hk]->qt_tot+=quantita;
                    RB_Insert_M(Magazzino->vett[Hk], r, quantita, scadenza);
                    Controlla_Scadenze(Magazzino->vett[Hk], Magazzino->vett[Hk]->RBT);
                }
                else{ //caso 3)collisione, due sottocasi a)/b)
                    ptr=Magazzino->vett[Hk];
                    while((ptr->next!=NULL)&&(strcmp(ptr->next->ingrediente, r)!=0))
                        ptr=ptr->next;
                    if(ptr->next==NULL){ //a)il nodo non c'è =>ne alloco uno nuovo
                        ptr->next=(RBTmagazzino)malloc(sizeof(RBTm));
                        ptr->next->qt_tot=quantita;
                        ptr->next->RBT=T_null;
                        ptr->next->next=NULL;
                        ptr->next->ingrediente=(char*)malloc(sizeof(char)*(strlen(r)+1));
                        strcpy(ptr->next->ingrediente, r);
                        RB_Insert_M(ptr->next, r, quantita, scadenza);
                    }
                    else{ //b) il nodo c'è già
                        ptr->next->qt_tot+=quantita;
                        RB_Insert_M(ptr->next, r, quantita, scadenza);
                        Controlla_Scadenze(ptr->next, ptr->next->RBT);
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
int Funzione_hash_M(MAGAZZINO_P Magazzino, int m, char* r, size_t lunghezza){
    int i, z, k=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        // k=k*3+r[i];
        k ^= r[i];
    }
    z=(int)floor(((double) m)*(((double) k)*HASH_CONSTANT_M-floor(((double) k)*HASH_CONSTANT_M)));
    return z;
}

//inserisce il lotto nel suo RBT
void RB_Insert_M(RBTmagazzino Root, char * ingrediente, int qt, int scadenza){
    Lotto * pre=T_null, *cur=Root->RBT, *new=NULL;
    size_t lunghezza;
    while((cur!=T_null)&&(cur->scadenza!=scadenza)){ //devo gestire anche il caso in cui siano uguali le scadenze, se sono uguali sommo le quantita e fine
        pre=cur;
        if(scadenza < cur->scadenza){
            cur=cur->left;
        }
        else 
            cur=cur->right;
    } //trovato il posto dove metterlo devo allocare il nodo e lo inizializzo a meno che non abbia la stessa scadenza
    if(cur->scadenza==scadenza){
        cur->quantita=cur->quantita+qt;
        return;
    } 
    new=(Lotto *)malloc(sizeof(Lotto));
    lunghezza=strlen(ingrediente);
    new->ingrediente=(char *)malloc(sizeof(char)*(lunghezza+1));
    strcpy(new->ingrediente, ingrediente);
    new->quantita=qt;
    new->scadenza=scadenza;

    new->parent=pre;
    if(pre==T_null){ //è il primo nodo, la radice
        Root->RBT=new;
        new->left=T_null;
        new->right=T_null;
        new->color='b';
        return;
    }
    else if(new->scadenza<pre->scadenza){
        pre->left=new;
    }
    else pre->right=new;
    new->left=T_null;
    new->right=T_null;
    new->color='r';
    RB_Insert_Fixup_M(Root, new);
    return;
}

//supporto a RB_insert per mantenere l'albero bilanciato
void RB_Insert_Fixup_M(RBTmagazzino Root, Lotto * new){
    Lotto * y=NULL;
    while(new->parent->color=='r'){
        if(new->parent==new->parent->parent->left){
            y=new->parent->parent->right;
            if(y->color=='r'){
                new->parent->color='b';
                y->color='b';
                new->parent->parent->color='r';
                new=new->parent->parent;
            }
            else {if(new==new->parent->right){
                    new=new->parent;
                    LeftRotate(Root, new);
                }
                new->parent->color='b';
                new->parent->parent->color='r';
                RightRotate(Root, new->parent->parent);
            }
        }
        else {
            y=new->parent->parent->left;
            if(y->color=='r'){
                new->parent->color='b';
                y->color='b';
                new->parent->parent->color='r';
                new=new->parent->parent;
            }
            else {if(new==new->parent->left){
                    new=new->parent;
                    RightRotate(Root, new);
                }   
                new->parent->color='b';
                new->parent->parent->color='r';
                LeftRotate(Root, new->parent->parent);
            }
        }
    }
    Root->RBT->color='b';
    return;
}

//rotazione a sinistra negli rbt
void LeftRotate(RBTmagazzino Root, Lotto * x){
    Lotto * y=NULL;
    y=x->right;
    x->right=y->left;
    if(y->left!=T_null){
        y->left->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==T_null){
        Root->RBT=y;
    }
    else if(x==x->parent->left){
        x->parent->left=y;
    }
    else {
        x->parent->right=y;
    }
    y->left=x;
    x->parent=y;
    return;
}

//rotazione a destra negli rbt
void RightRotate(RBTmagazzino Root, Lotto * x){
    Lotto * y=NULL;
    y=x->left;
    x->left=y->right;
    if(y->right!=T_null){
        y->right->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==T_null){
        Root->RBT=y;
    }
    else if(x==x->parent->right){
        x->parent->right=y;
    }
    else {
        x->parent->left=y;
    }
    y->right=x;
    x->parent=y;
}

//cancella lotti scaduti
void Controlla_Scadenze(RBTmagazzino Root, Lotto * ptr){ //ptr scorre i nodi dell'albero
    while((ptr->scadenza>tempo)&&(ptr!=T_null)){ 
        ptr=ptr->left;
    }  
    if(ptr!=T_null){
        Root->qt_tot-=ptr->quantita;
        RB_Delete_M(Root, ptr);
        Controlla_Scadenze(Root, Root->RBT);
        return;
    }
    return;
}

//elimina il nodo dall'albero
void RB_Delete_M(RBTmagazzino Root, Lotto * z){ 
    Lotto * x=NULL, *dacanc=NULL;         
    size_t lunghezza;

//quanti figli ha il nodo da cancellare?
    if((z->left==T_null)||(z->right==T_null)){
     //caso in cui il nodo da cancellare ha 0 o 1 figlo
        dacanc=z;
    } else {
    //caso in cui ha due figli
        dacanc=RB_Successor(z);
    }
    //x è nodo che sostituisce ciò che cancello
    if(dacanc->left!=T_null)
        x=dacanc->left;
    else 
        x=dacanc->right;
    x->parent=dacanc->parent;
    //qui so il nodo da cancellare
    
    if(dacanc->parent==T_null){
        Root->RBT=x;
    }
    else if(dacanc==dacanc->parent->left)
        dacanc->parent->left=x;
    else 
        dacanc->parent->right=x;
//se nodo che cancello differisce da quello che voglio allora
    if(dacanc!=z){
        z->scadenza=dacanc->scadenza;
        z->quantita=dacanc->quantita;
        lunghezza=strlen(dacanc->ingrediente);
        free(z->ingrediente);
        z->ingrediente=(char *)malloc(sizeof(char)*(lunghezza+1));
        strcpy(z->ingrediente, dacanc->ingrediente);
    }
    if(dacanc->color=='b')
        RB_Delete_Fixup_M(Root, x);
    free(dacanc->ingrediente);
    dacanc->ingrediente=NULL;
    free(dacanc);
    dacanc=NULL;
    return;
}

//restituisce il successore nell'RBT del nodo x
Lotto * RB_Successor(Lotto * x){
    Lotto * y;
    if(x->right!=T_null){
        return RB_Min(x->right);
    }
    y=x->parent;
    while((y!=T_null)&&(y->right==x)){
        x=y;
        y=y->parent;
    }
    return y;
}

//restituisce il minimo nel sottoalbero a partire da x
Lotto * RB_Min(Lotto * x){
    Lotto * cur=x;
    while(cur->left!=T_null)
        cur=cur->left;
    return cur;
}

//supporto alla RB_delete per mantenere l'albero bilanciato
void RB_Delete_Fixup_M(RBTmagazzino Root, Lotto * x){
    Lotto *w=NULL;
    while((x!=Root->RBT)&&(x->color=='b')){
        if(x==x->parent->left){
            w=x->parent->right;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                LeftRotate(Root, x->parent);
                w=x->parent->right;
            }
            if((w->left->color=='b')&&(w->right->color=='b')){
                w->color='r';
                x=x->parent;
            }
            else{
                if(w->right->color=='b'){
                    w->left->color='b';
                    w->color='r';
                    RightRotate(Root, w);
                    w=x->parent->right;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->right->color='b';
                LeftRotate(Root, x->parent);
                x=Root->RBT;
            }
        }
        else{
            w=x->parent->left;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                RightRotate(Root, x->parent);
                w=x->parent->left;
            }
            if((w->right->color=='b')&&(w->left->color=='b')){
                w->color='r';
                x=x->parent;
            }
            else{
                if(w->left->color=='b'){
                    w->right->color='b';
                    w->color='r';
                    LeftRotate(Root, w);
                    w=x->parent->left;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->left->color='b';
                RightRotate(Root, x->parent);
                x=Root->RBT;
            }
        }
    }
    x->color='b';
    return;
}

//scorre gli ordini in attesa e vede se sono preparabili
void Controlla_Ordini_Attesa(LO_Poynter Lista_Ordini, MAGAZZINO_P Magazzino, RICETTARIO_P Ricettario){ //deve scorrere gli ordini in attesa e vedere se sono preparabili o no 
    Lista_Ordini_Attesa ptr=Lista_Ordini->OA, prev=NULL;
    Ingrediente * pt=NULL;
    RICETTA_p punt;
    int peso_ordine;                                                                     //possibile raffinamento successivo: nella lista ordini attesa aggiungo un puntatore alla
    while(ptr!=NULL){                                                          //ricetta per non dover ricarlcolare la funzione di hash
        //non dovrei controllare se la ricetta c'è o no perché non può essere rimossa se è in attesa
        punt=ptr->Ricetta;
        pt=punt->next; //pt punta al primo ingrediente
        peso_ordine=ControllaEPrepara(ptr->Ricetta, Magazzino, pt, Lista_Ordini, ptr->quantita, ptr->ordine, 1, ptr->hash);
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
        primo=1;
    return;
}

//riceve l'ordine e verifica di avere la ricetta, e se e' preparabile lo aggiunge nello heap
void Ordine(RICETTARIO_P Ricettario, MAGAZZINO_P Magazzino, LO_Poynter Lista_Ordini){
    char r[256];
    int num, Rk, peso_ordine;
    size_t lunghezza;
    Ingrediente * ptr=NULL;
    RICETTA_p punt=NULL;
    if(scanf("%s %d", r, &num)>0){
        lunghezza=strlen(r);
        Rk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 1); //verifico di avere la ricetta
        punt=Ricettario->vett[Rk];
        while((punt!=NULL)&&(strcmp(punt->nome, r)!=0))
            punt=punt->prox;
        if((punt==NULL)){
            printf("rifiutato\n");
            return;
        }
        else{
            ptr=punt->next; //punta al primo ingrediente della ricetta. Per rendere più facile controlla_ordini_attesa voglio scrivere 
            //una funzione separata che, data una ricetta, controlli di avere gli ingredienti, li tiene con una lista e se li ho tutti prepara la ricetta
            peso_ordine=ControllaEPrepara(punt, Magazzino, ptr, Lista_Ordini, num, r, 0, Rk);
            if(peso_ordine){
                printf("accettato\n");
                Enqueue_OP(Lista_Ordini->OP, r, num, peso_ordine, tempo);
                }
        }
    }
    return;
}

//verifica di avere gli ingredienti e se li ha li rimuove preparando l'ordine
int ControllaEPrepara(RICETTA_p Ricetta, MAGAZZINO_P Magazzino, Ingrediente * primo_ingr_ricetta, LO_Poynter Lista_Ordini, int num, char * r, int flag, int Hk_R){
    int n, peso_tot=0; 
    Ingrediente * ptr=primo_ingr_ricetta;
    Lotto * pt =NULL, *temp=NULL;
    RBTmagazzino punt=NULL;
    while(ptr!=NULL){
        n=ptr->quantita*num;
        if(Magazzino->vett[ptr->hash]==NULL){ //non c'è alcuna quantita di quell' ingrediente
            if(flag){
                return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
            }
            Enqueue_OA(Lista_Ordini, r, num, Hk_R, Ricetta);
            printf("accettato\n");
            return 0;
        }
        else{
            punt=Magazzino->vett[ptr->hash]; //vuol dire che almeno un nodo in quel bucket della tabella c'è
            while((punt!=NULL)&&(strcmp(punt->ingrediente, ptr->nome)!=0)) //cerco il nodo corrispondente a quell'ingrediente
                punt=punt->next;
            if(punt==NULL){ //il nodo non c'è
                if(flag){
                    return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                }
                Enqueue_OA(Lista_Ordini, r, num, Hk_R, Ricetta);
                printf("accettato\n");
                return 0;
            }
            else{ //esiste il nodo per quell'ingrediente
                Controlla_Scadenze(punt, punt->RBT);
                if(punt->RBT!=T_null){ //se l'RBT non è stato totalmente eliminato da controlla_scadenze
                    if(punt->qt_tot>=n){ //vuol dire che quell'RBT ha abbastanza quantita per soddisfare la ricetta
                        ptr=ptr->next;//passo al prossimo ingrediente
                    }
                    else{
                        if(flag){
                            return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                        }
                        Enqueue_OA(Lista_Ordini, r, num, Hk_R, Ricetta);
                        printf("accettato\n");
                        return 0;
                    }
                }
                else{ //l'RBT è stato eliminato
                    if(flag){
                        return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
                    }
                    Enqueue_OA(Lista_Ordini, r, num, Hk_R, Ricetta);
                    printf("accettato\n");
                    return 0;
                }
            }
        }
    } 
    ptr=primo_ingr_ricetta;
    while(ptr!=NULL){
        n=ptr->quantita*num;
        punt=Magazzino->vett[ptr->hash]; //vuol dire che almeno un nodo in quel bucket della tabella c'è
        while((punt!=NULL)&&(strcmp(punt->ingrediente, ptr->nome)!=0)) //cerco il nodo corrispondente a quell'ingrediente
            punt=punt->next;
        pt=RB_Min(punt->RBT);
        while((pt!=T_null)&&(n>pt->quantita)){
            temp=pt;
            pt=RB_Successor(pt);
            punt->qt_tot-=temp->quantita;
            n-=temp->quantita;
            RB_Delete_M(punt, temp);
        }
        if(pt!=T_null){
            pt->quantita-=n;
            punt->qt_tot-=n;
        }
        ptr=ptr->next;
    }
    peso_tot=Ricetta->peso*num;
    return peso_tot;
}

//aggiunge un ordine nella coda degli ordini in attesa
void Enqueue_OA(LO_Poynter Lista, char * r, int num, int hash, RICETTA_p Ricetta){
    Lista_Ordini_Attesa ptr=NULL;
    size_t lunghezza=strlen(r);
    ptr=(Lista_Ordini_Attesa)malloc(sizeof(Ordine_in_attesa)); //alloco e inizializzo nuovo nodo
    ptr->data=tempo;
    ptr->quantita=num;
    ptr->hash=hash;
    ptr->Ricetta=Ricetta;
    ptr->ordine=(char*)malloc(sizeof(char)*(lunghezza+1));
    strcpy(ptr->ordine, r);
    ptr->next=NULL;
    if(primo){ //se è primo devo metterlo come primo nodo
        Lista->OA=ptr;
        Lista->OA_ult=ptr;
        primo=0;
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
        minHeap->Ordini_da_Stampare=Coda_OS(minHeap->Ordini_da_Stampare, minHeap->V[0].ordine, minHeap->V[0].quantita, minHeap->V[0].peso, minHeap->V[0].data);
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
        minHeap->Ordini_da_Stampare=Coda_OS(minHeap->Ordini_da_Stampare, minHeap->V[0].ordine, minHeap->V[0].quantita, minHeap->V[0].peso, minHeap->V[0].data);
        free(minHeap->V[0].ordine);
        minHeap->V[0].ordine=NULL;
        minHeap->numero_elem--;
        n++;
    }
    return n;
}

//aggiunge in coda gli ordini che poi materializzerò nel vettore che poi mergesort ordinerà
Lista_OS Coda_OS(Lista_OS OS, char * r, int num, int peso_ordine, int data){ //potrei renderla davvero una coda per fare l'inserimento in O(1)
    Lista_OS ptr, cur=OS;
    ptr=(Lista_OS)malloc(sizeof(Nodo_Lista_OS));
    ptr->data=data;
    ptr->peso=peso_ordine;
    ptr->quantita=num;
    ptr->ordine=(char*)malloc(sizeof(char)*(strlen(r)+1));
    strcpy(ptr->ordine, r);
    ptr->next=NULL;
    if(OS==NULL)
        return ptr;
    else{
        while(cur->next!=NULL)
            cur=cur->next;
        cur->next=ptr;
    }
    return OS;
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
    Lista_OS ptr=minHeap->Ordini_da_Stampare, tmp;
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
    minHeap->Ordini_da_Stampare=NULL;
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
