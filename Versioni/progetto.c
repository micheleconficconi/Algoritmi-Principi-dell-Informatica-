#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define DIM_RIC_BASE 4096
#define HASH_CONSTANT_R ((double) (sqrt(5)-1)/2)
#define HASH_CONSTANT_M ((double) (sqrt(5)-1)/2)
//opzioni di compilazione: gcc -Wall -Werror -std=gnu11 -O2 -lm primastesura.c -o primastesura
//posso aggiungere -g3 
//metti a posto la funzione corriere, cerca di debuggare, ci sono varie segmentation fault ingiro. problema con la gestione degli alberi RBTordini
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

typedef Lotto * RBTmagazzino; //questi puntatori, cioè le teste degli alberi, sono gli elementi del magazzino
//posso o preallocare un array da 256 oppure usare una malloc per allocare la dimensione esatta e risparmiare spazio, ma non tempo

//lista CHECK per tenere i riferimenti agli ingredienti
typedef struct CK{
    Lotto * ingrediente;
    struct CK * next;
}   Nodo;

typedef Nodo * CHECK;

typedef struct { //hash table magazzino
    int numero_ingredienti;
    int dimensione;
    RBTmagazzino * vett; //vettore contenete le testate degli alberi dei lotti;
} MAGAZZINO;
typedef MAGAZZINO* MAGAZZINO_P;

//lista Ordini in Attesa (semplice coda FIFO)

typedef struct OA {
    char * ordine;
    int quantita;
    int data;
    struct OA * next;
} Ordine_in_attesa;

typedef Ordine_in_attesa * Lista_Ordini_Attesa;

//albero lista Ordini Pronti, elementi delgi alberi RBTpeso e RBTdata della lista

typedef struct OP {
    char * ordine;
    int quantita;
    int data;
    int peso;
    char color; //r=rosso, n=nero
    struct OP * left;
    struct OP * right;
    struct OP * parent;
} Ordine_Pronto;

typedef Ordine_Pronto * RBTordini;


//lista Ordini Pronti: è una lista con due alberi, uno che ordina per peso e uno che ordina per data

typedef struct LOP {
    RBTordini RBTdata;
    struct LOP * next;
    int peso_tot;
    int prima_data;
    int ultima_data;
    int numero_elem;
} Elem_Lista_OP;

typedef Elem_Lista_OP * Lista_Ordini_pronti; //testa della lista sarà questo

typedef struct LO{ //struct che punta alle due liste per fare in modo che anche le altre funzioni possano modificarle
    Lista_Ordini_Attesa  OA;
    Lista_Ordini_pronti  OP;
} Lista_Ordini;

typedef Lista_Ordini * LO_Poynter; //devo allocare una struct dinamica che così passo solo il puntatore e le funzioni possono modificare
//le due liste senza restituire i puntatori

//lista per rappresentare le ricette nel ricettario: 
//ingrediente:
typedef struct IN {
    char * nome;
    int quantita;
    struct IN * next;
} Ingrediente;
//elemento della tabella hash RICETTARIO:
typedef struct {
    char * nome;
    Ingrediente * next;
} Ricetta;

//tabella hash per il ricettario:
typedef struct {
    int numero_ricette;
    int dimensione;
    Ricetta * vett; //vettore di ricette
} RICETTARIO;
typedef RICETTARIO * RICETTARIO_P;

//vettore per stampare gli ordini:
typedef struct {
    char * ordine;
    int data;
    int quantita;
    int peso;
} Ordine_Stampabile;

//testate delle funzioni:

Lista_Ordini_pronti AggiungiNodoLOP(Lista_Ordini_pronti);

void Aggiungi_Ricetta(RICETTARIO_P);
int Funzione_hash_R(RICETTARIO_P, int, char *, size_t, int);
int Funzione_hash_R_new(RICETTARIO_P, int, char *, size_t, int);

void Rimuovi_Ricetta(RICETTARIO_P, LO_Poynter);
int Check_Attesa(Lista_Ordini_Attesa, char *); //controlla se c'è un ordine in attesa per quella ricetta
void Check_OP(RBTordini, char *);
int Check_Ordini_Pronti(Lista_Ordini_pronti, char *);

void Rifornimento (MAGAZZINO_P, RICETTARIO_P, LO_Poynter);
int Funzione_hash_M(MAGAZZINO_P, int, char*, size_t);
int Funzione_hash_M_new(MAGAZZINO_P, int, char*, size_t);
void RB_Insert_M(MAGAZZINO_P, int, char *, int, int);
void RB_Insert_Fixup_M(MAGAZZINO_P, int, Lotto *);
void LeftRotate(MAGAZZINO_P, int, Lotto *);
void RightRotate(MAGAZZINO_P, int, Lotto *);
void Controlla_Scadenze(MAGAZZINO_P, int, Lotto*);
void RB_Delete_M(MAGAZZINO_P, int, Lotto *);
void RB_Delete_Fixup_M(MAGAZZINO_P, int, Lotto *);
Lotto * RB_Successor(Lotto *);
Lotto * RB_Min(Lotto *);
void Controlla_Ordini_Attesa(LO_Poynter, MAGAZZINO_P, RICETTARIO_P); //controlla per le ricette in attesa se ora ci sono gli ingredienti 

void Ordine(RICETTARIO_P, MAGAZZINO_P, LO_Poynter);
int ControllaEPrepara(MAGAZZINO_P, Ingrediente *, LO_Poynter, int, char *, int);
Lista_Ordini_Attesa Enqueue_OA(Lista_Ordini_Attesa, char *, int);
CHECK Enqueue_CK(CHECK, Lotto *);
void Enqueue_OP(Lista_Ordini_pronti, char *, int, int, int);
void RB_Insert_OP(Lista_Ordini_pronti, char *, int, int, int);
void RB_Insert_Fixup_LOP(Lista_Ordini_pronti, Ordine_Pronto *);
void LeftRotate_LOP(Lista_Ordini_pronti, Ordine_Pronto *);
void RightRotate_LOP(Lista_Ordini_pronti, Ordine_Pronto *);
Ordine_Pronto * RB_Max(Ordine_Pronto *);
void RB_Delete_LOP(Lista_Ordini_pronti, Ordine_Pronto *);
void RB_Delete_Fixup_LOP(Lista_Ordini_pronti, Ordine_Pronto *);
Ordine_Pronto * RB_Successor_LOP(Ordine_Pronto *);
Ordine_Pronto * RB_Min_LOP(Ordine_Pronto *);

void Corriere(LO_Poynter, Lista_Ordini_pronti);
void Riempi_vett(Ordine_Stampabile *, RBTordini);
void MergeSort(Ordine_Stampabile *, int, int);
void Merge(Ordine_Stampabile *, int, int, int);


int capienza=0, periodicita=0, tempo=0; //3 variabili globali per comodità
int indice=0; //variabile globale comoda per riempire il vettore dell'albero linearizzato delgi ordini
int res=0; //variabile globale per scorrere l'albero degli RBTdata per vedere se c'è un ordine in sospeso o no
Lotto * T_null=NULL;
Ordine_Pronto * TOP_null=NULL;  //2 nodi T.null per gli alberi, del magazzino e deli Ordini Pronti
int main( ){
    //per prima cosa devo inizializzare le strutture dati: 
    int i=0;
    RICETTARIO_P Ricettario;
    Ricettario=(RICETTARIO*)malloc(sizeof(RICETTARIO));
    Ricettario->vett=(Ricetta*)malloc(sizeof(Ricetta)*DIM_RIC_BASE); //vettore di ricette
    for(i=0; i<4096; i++){
        Ricettario->vett[i].nome=NULL;
        Ricettario->vett[i].next=NULL;
    }
    Ricettario->numero_ricette=0;
    Ricettario->dimensione=4096;

    //alloco l'unico nodo T.null
    T_null=(Lotto*)malloc(sizeof(Lotto));
    T_null->ingrediente=(char*)malloc(sizeof(char));
    T_null->ingrediente[0]='N';
    T_null->color='b';
    T_null->quantita=0;
    T_null->scadenza=0;
    T_null->left=NULL;
    T_null->right=NULL;
    T_null->parent=NULL;

    MAGAZZINO_P Magazzino;
    Magazzino=(MAGAZZINO_P)malloc(sizeof(MAGAZZINO));
    Magazzino->vett=(RBTmagazzino *)malloc(sizeof(RBTmagazzino)*4096); //vettore di puntatori a testate di alberi
    for(i=0; i<4096; i++){
        Magazzino->vett[i]=T_null;
    }
    Magazzino->numero_ingredienti=0;   
    Magazzino->dimensione=4096;  

    //inizializzo il t.null per gli rbt degli ordini
    TOP_null=(Ordine_Pronto *)malloc(sizeof(Ordine_Pronto));
    TOP_null->ordine=(char*)malloc(sizeof(char));
    TOP_null->ordine[0]='N';
    TOP_null->color='b';
    TOP_null->data=0;
    TOP_null->peso=0;
    TOP_null->quantita=0;
    TOP_null->left=NULL;
    TOP_null->right=NULL;
    TOP_null->parent=NULL;

    LO_Poynter Lista_Or;
    Lista_Or=(LO_Poynter)malloc(sizeof(Lista_Ordini));    
    Lista_Or->OP=NULL;
    Lista_Or->OP=AggiungiNodoLOP(Lista_Or->OP);
    Lista_Or->OA=NULL;
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
                //chiama funzione aggiungi ricetta
                Aggiungi_Ricetta(Ricettario);
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


Lista_Ordini_pronti AggiungiNodoLOP(Lista_Ordini_pronti lista){ //verrà eventualmente chiamata sul .next di un elemento della lista ordini pronti
    Lista_Ordini_pronti ptr, cur=lista;
    ptr=(Lista_Ordini_pronti)malloc(sizeof(Elem_Lista_OP));
    ptr->RBTdata=TOP_null; //inizializzo RBT peso e data contenuti nel nodo della lista, ma vuoti
    ptr->peso_tot=0;
    ptr->prima_data=0;
    ptr->ultima_data=0;
    ptr->numero_elem=0;
    ptr->next=NULL;
    if(lista==NULL)
        return ptr;
    else{
        while(cur->next!=NULL)
            cur=cur->next;
        cur->next=ptr;
    }
    return lista;
}


void Aggiungi_Ricetta(RICETTARIO_P Ricettario){
    char r[256], c='a';
    int k=0, Hk=0;
    size_t lunghezza;
    Ingrediente * ptr=NULL;
    if(scanf("%s", r)>0){ //copio il nome della ricetta
        lunghezza=strlen(r);
        Hk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 0);
        if((Ricettario->vett[Hk].nome==NULL)&&(Ricettario->vett[Hk].next==NULL)){ //inserimento su cella vuota 
            Ricettario->vett[Hk].nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
            strcpy(Ricettario->vett[Hk].nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
            Ricettario->vett[Hk].next=(Ingrediente*)malloc(sizeof(Ingrediente)); //alloca un nodo per il primo ingrediente
            ptr=Ricettario->vett[Hk].next;
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0){
                    lunghezza=strlen(r); //lunghezza nome ingrediente
                    ptr->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //alloca una stringa con la dimensione giusta per contenere il nome dell'ingrediente
                    strcpy(ptr->nome, r);  //salva il nome dell'ingrediente
                    ptr->quantita=k;        //salva la quantita
                    if(c!='\n'){ //alloca un altro nodo
                        ptr->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                        ptr=ptr->next;
                    }
                    else ptr->next=NULL;
                }
            }   
            //Ricettario->numero_ricette++; //aggiorna il numero di ricette
            printf("aggiunta\n");
            return;
        }
        else if(strcmp(Ricettario->vett[Hk].nome, "tomb")==0){ //inserisco su tomb
            free(Ricettario->vett[Hk].nome);
            Ricettario->vett[Hk].nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //+1 per il terminatore
            strcpy(Ricettario->vett[Hk].nome, r); //aggiunto il nome della ricetta devo aggiungere gli ingredienti
            Ricettario->vett[Hk].next=(Ingrediente*)malloc(sizeof(Ingrediente)); //alloca un nodo per il primo ingrediente
            ptr=Ricettario->vett[Hk].next;
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0){
                    lunghezza=strlen(r); //lunghezza nome ingrediente
                    ptr->nome=(char*)malloc(sizeof(char)*(lunghezza+1)); //alloca una stringa con la dimensione giusta per contenere il nome dell'ingrediente
                    strcpy(ptr->nome, r);  //salva il nome dell'ingrediente
                    ptr->quantita=k;        //salva la quantita
                    if(c!='\n'){ //alloca un altro nodo
                        ptr->next=(Ingrediente*)malloc(sizeof(Ingrediente));
                        ptr=ptr->next;
                    }
                    else ptr->next=NULL;
                }
            }   
            //Ricettario->numero_ricette++; //aggiorna il numero di ricette
            printf("aggiunta\n");
            return;
        }
        else if(strcmp(Ricettario->vett[Hk].nome, r)==0){  //se la ricetta è già presente
            while(c!='\n'){
                if(scanf("%s %d%c", r, &k, &c)>0); //consumo tutta la riga
            }
            printf("ignorato\n");
            return;
        }
    }
}

int Funzione_hash_R(RICETTARIO_P Ricettario, int m, char * r, size_t lunghezza, int p){  
    int i, z;
    // double A=(sqrt(5)-1)/2;
    double k=0, n;
    n=(double)m;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        k=k*3+r[i];
        //k=k+r[i];
    }
    z=(int)floor(n*(k*HASH_CONSTANT_R-floor(k*HASH_CONSTANT_R))); //corrisponde al calcolo di H1, su H(k,i)=(H1(k)+H2(k)*i)mod m
    i=0;
    //ricerca (z=1): se trovi tomb devi andare avanti (chiamato da rimuovi_ricetta)
    if(p){
        while((Ricettario->vett[z].nome!=NULL)&&(strcmp(Ricettario->vett[z].nome, r)!=0)){
            z=(z+(2*i+1))%(m);
            i++;
        }
    }
    //inserimento: se trovi tomb inserisci (chiamato da aggiungi_ricetta)
    else {
        while((Ricettario->vett[z].nome!=NULL)&&(strcmp(Ricettario->vett[z].nome, r)!=0)&&(strcmp(Ricettario->vett[z].nome, "tomb")!=0)){
            z=(z+(2*i+1))%(m);
            i++;
        }
    }
    return z;
}

int Funzione_hash_R_new(RICETTARIO_P Ricettario, int m, char * r, size_t lunghezza, int p){  
    int i, z=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        z^=r[i];
    }
    i=0;
    if(p){
        while((Ricettario->vett[z].nome!=NULL)&&(strcmp(Ricettario->vett[z].nome, r)!=0)){
            z=(z+(2*i+1))%(m);
            i++;
        }
    }
    //inserimento: se trovi tomb inserisci (chiamato da aggiungi_ricetta)
    else {
        while((Ricettario->vett[z].nome!=NULL)&&(strcmp(Ricettario->vett[z].nome, r)!=0)&&(strcmp(Ricettario->vett[z].nome, "tomb")!=0)){
            z=(z+(2*i+1))%(m);
            i++;
        }
    }
    return z;
}


void Rimuovi_Ricetta(RICETTARIO_P Ricettario, LO_Poynter Lista_Ordini){
    char r[256];
    int Hk, compare;             //potrei dichiarare lunghezza come int e castare il risultato di strlen a int
    size_t lunghezza;
    Ingrediente * ptr=NULL, * tmp=NULL;
    if(scanf("%s", r)>0){
        lunghezza=strlen(r);
        Hk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 1);
        if(Ricettario->vett[Hk].nome!=NULL){
            compare=strcmp(Ricettario->vett[Hk].nome, r); // Ricettario->vett[Hk].nome può essere null
            if((compare==0)&&((Check_Attesa(Lista_Ordini->OA, r))+(Check_Ordini_Pronti(Lista_Ordini->OP, r)))){
                printf("ordini in sospeso\n");
                res=0;
                return;
            }
            else if(compare==0){ //la ricetta va rimossa
                //scorro tutta la lista degli ingredienti deallocandoli
                ptr=Ricettario->vett[Hk].next;
                while(ptr!=NULL){
                    tmp=ptr->next;
                    free(ptr->nome);
                    free(ptr);
                    ptr=tmp;
                }
                free(Ricettario->vett[Hk].nome);
                Ricettario->vett[Hk].nome=(char*)malloc(sizeof(char)*5);
                strcpy(Ricettario->vett[Hk].nome, "tomb");
                Ricettario->vett[Hk].next=NULL;
                Ricettario->numero_ricette--;
                printf("rimossa\n");
                return;
            }
            else{ //caso in cui c'è tomb
                printf("non presente\n");
                return;
            }

        }
        else {
            printf("non presente\n");
            return;
        }
    }
}

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

int Check_Ordini_Pronti(Lista_Ordini_pronti OP, char * r){
    if(OP->RBTdata!=TOP_null){
        Check_OP(OP->RBTdata, r);
        if(res==1)
            return res;
        }
    if((OP->next!=NULL)&&(OP->RBTdata!=TOP_null))
        Check_Ordini_Pronti(OP->next, r);
    return res;
}

void Check_OP(RBTordini T, char * r){     //deve cercare in RBTdata se ci sono ordini con lo stesso nome
    if(strcmp(T->ordine, r)==0){
        res=1;
        return;
    }
    if((res==0)&&(T->left!=TOP_null))
        Check_OP(T->left, r);
    if((res==0)&&(T->right!=TOP_null))
        Check_OP(T->right, r);
    return;
}


void Rifornimento(MAGAZZINO_P Magazzino, RICETTARIO_P Ricettario, LO_Poynter Lista_Ordini){
    char r[256], c='a';
    int quantita, scadenza, Hk;
    size_t lunghezza;
    while(c!='\n'){
        if(scanf("%s %d %d%c", r, &quantita, &scadenza, &c)>0){ //<ingrediente> <qt> <scadenza> ...
            if(scadenza>tempo){
                lunghezza=strlen(r);
                Hk=Funzione_hash_M_new(Magazzino, Magazzino->dimensione, r, lunghezza);
                RB_Insert_M(Magazzino, Hk, r, quantita, scadenza);
                Controlla_Scadenze(Magazzino, Hk, Magazzino->vett[Hk]);
            }
            //Magazzino->numero_ingredienti++;
        }
    }
    Controlla_Ordini_Attesa(Lista_Ordini, Magazzino, Ricettario); //voglio che controlli per ogni ricetta in lista d'attesa di avere gli ingredienti e se si la prepari
    printf("rifornito\n");
    return;
}

int Funzione_hash_M(MAGAZZINO_P Magazzino, int m, char* r, size_t lunghezza){
    int i, z;
    // double A=(sqrt(5)-1)/2, k=0, n;
    double k=0, n;
    n=(double)m;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        k=k*3+r[i];
    }
    z=(int)floor(n*(k*HASH_CONSTANT_M-floor(k*HASH_CONSTANT_M)));
    i=0;
    if(T_null->ingrediente==NULL)
        printf("suca\n");
    while((Magazzino->vett[z]!=T_null)&&(strcmp(Magazzino->vett[z]->ingrediente, r)!=0)){
        z=(z+(2*i+1))%(m);
        i++;
    }
    return z;
}

int Funzione_hash_M_new(MAGAZZINO_P Magazzino, int m, char* r, size_t lunghezza){
    int i, z=0;
    for(i=0; i<lunghezza; i++){ //calcolo della chiave, unica anche se le lettere sono le stesse ma in ordine diverso
        z^=r[i];
    }
    i=0;

    // while((Magazzino->vett[z]!=T_null)&&(strcmp(Magazzino->vett[z]->ingrediente, r)!=0)){
    //     z=(z+(2*i+1))%(m);
    //     i++;
    // }
    return z;
}


void RB_Insert_M(MAGAZZINO_P Magazzino, int Hk, char * ingrediente, int qt, int scadenza){
    Lotto * pre=T_null, *cur=Magazzino->vett[Hk], *new=NULL;
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
        if(new==NULL)
            printf("suca\n");
        Magazzino->vett[Hk]=new;
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
    RB_Insert_Fixup_M(Magazzino, Hk, new);
    return;
}

void RB_Insert_Fixup_M(MAGAZZINO_P Magazzino, int Hk, Lotto * new){
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
                    LeftRotate(Magazzino, Hk, new);
                }
                new->parent->color='b';
                new->parent->parent->color='r';
                RightRotate(Magazzino, Hk, new->parent->parent);
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
                    RightRotate(Magazzino, Hk, new);
                }   
                new->parent->color='b';
                new->parent->parent->color='r';
                LeftRotate(Magazzino, Hk, new->parent->parent);
            }
        }
    }
    Magazzino->vett[Hk]->color='b';
    return;
}

void LeftRotate(MAGAZZINO_P Magazzino, int Hk, Lotto * x){
    Lotto * y=NULL;
    y=x->right;
    x->right=y->left;
    if(y->left!=T_null){
        y->left->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==T_null){
        Magazzino->vett[Hk]=y;
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

void RightRotate(MAGAZZINO_P Magazzino, int Hk, Lotto * x){
    Lotto * y=NULL;
    y=x->left;
    x->left=y->right;
    if(y->right!=T_null){
        y->right->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==T_null){
        Magazzino->vett[Hk]=y;
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

//algoritmo cancellazione lotti scaduti
void Controlla_Scadenze(MAGAZZINO_P Magazzino, int Hk, Lotto * ptr){
    while((ptr->scadenza>tempo)&&(ptr!=T_null)){ 
        ptr=ptr->left;
    }  
    if(ptr!=T_null){
        if(ptr->left!=T_null)
            Controlla_Scadenze(Magazzino, Hk, ptr->left);
        if(ptr->right!=T_null)
            Controlla_Scadenze(Magazzino, Hk, ptr->right);
        RB_Delete_M(Magazzino, Hk, ptr);
        return;
    }
    return;
}

//elimina il nodo dall'albero
void RB_Delete_M(MAGAZZINO_P Magazzino, int Hk, Lotto * z){ 
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
        Magazzino->vett[Hk]=x;
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
        RB_Delete_Fixup_M(Magazzino, Hk, x);
    free(dacanc->ingrediente);
    dacanc->ingrediente=NULL;
    free(dacanc);
    dacanc=NULL;
    return;
}

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

Lotto * RB_Min(Lotto * x){
    Lotto * cur=x;
    while(cur->left!=T_null)
        cur=cur->left;
    return cur;
}

void RB_Delete_Fixup_M(MAGAZZINO_P Magazzino, int Hk, Lotto * x){
    Lotto *w=NULL;
    while((x!=Magazzino->vett[Hk])&&(x->color=='b')){
        if(x==x->parent->left){
            w=x->parent->right;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                LeftRotate(Magazzino, Hk, x->parent);
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
                    RightRotate(Magazzino, Hk, w);
                    w=x->parent->right;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->right->color='b';
                LeftRotate(Magazzino, Hk, x->parent);
                x=Magazzino->vett[Hk];
            }
        }
        else{
            w=x->parent->left;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                RightRotate(Magazzino, Hk, x->parent);
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
                    LeftRotate(Magazzino, Hk, w);
                    w=x->parent->left;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->left->color='b';
                RightRotate(Magazzino, Hk, x->parent);
                x=Magazzino->vett[Hk];
            }
        }
    }
    x->color='b';
    return;
}

void Controlla_Ordini_Attesa(LO_Poynter Lista_Ordini, MAGAZZINO_P Magazzino, RICETTARIO_P Ricettario){ //deve scorrere gli ordini in attesa e vedere se sono preparabili o no 
    Lista_Ordini_Attesa ptr=Lista_Ordini->OA, prev=NULL;
    Ingrediente * pt=NULL;
    int Hk, peso_ordine;                                                                     //possibile raffinamento successivo: nella lista ordini attesa aggiungo un puntatore alla
    while(ptr!=NULL){                                                          //ricetta per non dover ricarlcolare la funzione di hash
        Hk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, ptr->ordine, strlen(ptr->ordine), 1); //non dovrei controllare se la ricetta c'è o no perché non può essere rimossa se è in attesa
        pt=Ricettario->vett[Hk].next; //pt punta al primo ingrediente
        peso_ordine=ControllaEPrepara(Magazzino, pt, Lista_Ordini, ptr->quantita, ptr->ordine, 1);
        if(peso_ordine){ //se è maggiore di 0 devo metterlo nella lista degli ordini pronti e eliminarlo da quella di attesa
            Enqueue_OP(Lista_Ordini->OP, ptr->ordine, ptr->quantita, peso_ordine, ptr->data);
            if(prev!=NULL){ //se è un nodo centrale nella lista o l'ultimo
                prev->next=ptr->next;
                free(ptr->ordine);
                free(ptr);
                ptr=prev;
            }
            else { //vuol dire che ptr è il primo nodo della lista
                Lista_Ordini->OA=ptr->next;
                free(ptr->ordine);
                free(ptr);
                ptr=Lista_Ordini->OA;
            }
        }
        else {
            prev=ptr;
            ptr=ptr->next;
        }
    }
    return;
}


void Ordine(RICETTARIO_P Ricettario, MAGAZZINO_P Magazzino, LO_Poynter Lista_Ordini){
    char r[256];
    int num, Rk, peso_ordine;
    size_t lunghezza;
    Ingrediente * ptr=NULL;
    if(scanf("%s %d", r, &num)>0){
        lunghezza=strlen(r);
        Rk=Funzione_hash_R_new(Ricettario, Ricettario->dimensione, r, lunghezza, 1); //verifico di avere la ricetta
        if((Ricettario->vett[Rk].nome==NULL)||(strcmp(Ricettario->vett[Rk].nome, "tomb")==0)){
            printf("rifiutato\n");
            return;
        }
        else{
            ptr=Ricettario->vett[Rk].next; //punta al primo ingrediente della ricetta. Per rendere più facile controlla_ordini_attesa voglio scrivere 
            //una funzione separata che, data una ricetta, controlli di avere gli ingredienti, li tiene con una lista e se li ho tutti prepara la ricetta
            peso_ordine=ControllaEPrepara(Magazzino, ptr, Lista_Ordini, num, r, 0);
            if(peso_ordine){
                printf("accettato\n");
                if(Lista_Ordini->OP!=NULL)
                    Enqueue_OP(Lista_Ordini->OP, r, num, peso_ordine, tempo);
                else{
                    Lista_Ordini->OP=AggiungiNodoLOP(Lista_Ordini->OP);
                    Enqueue_OP(Lista_Ordini->OP, r, num, peso_ordine, tempo);               
                }
            }
        }
    }
    return;
}

int ControllaEPrepara(MAGAZZINO_P Magazzino, Ingrediente * ricetta, LO_Poynter Lista_Ordini, int num, char * r, int flag){//restituisce peso tot se ha preparato ricetta, se no 0
    int Mk, n, residuo, peso_tot=0;                                         //<nome_ricetta> <numero_di_ordini> r, num
    CHECK Check=NULL, tmp=NULL;
    Ingrediente * ptr=ricetta; //punta al primo ingrediente della ricetta
    Lotto * pt=NULL; //ptr scorre la ricetta puntando all'inizio al primo ingrediente, pt scorre gli alberi
    while(ptr!=NULL){ //prima controllo gli ingredienti, se li ho preparo (=>rimuovo gli ingredienti e aggiungo a LO->OP)
        residuo=0;
        n=0;
        Mk=Funzione_hash_M_new(Magazzino, Magazzino->dimensione, ptr->nome, strlen(ptr->nome)); 
        if(Magazzino->vett[Mk]==T_null){ // se l'ingrediente non c'è l'ordine va in attesa
            if(flag){
                return 0; //se z=1 vuol dire che viene da controlla_ordini_attesa, quindi dalla coda di attesa, non deve accodarlo di nuovo;
            }
            Lista_Ordini->OA=Enqueue_OA(Lista_Ordini->OA, r, num);
            printf("accettato\n");
            return 0;
        }
        else {
            Controlla_Scadenze(Magazzino, Mk, Magazzino->vett[Mk]); //ripulisco l'albero da eventuali lotti scaduti
            if(Magazzino->vett[Mk]!=T_null){ //se l'albero non è stato toalmente eliminato da controlla_scadenze
                n=ptr->quantita*num; //qt totale necessaria per quell'ingrediente
                pt=RB_Min(Magazzino->vett[Mk]); //trovo l'ingrediente a scadenza più prossima. siccome ho controllato che sia diverso da T_null almeno un minimo ci deve essere
                Check=Enqueue_CK(Check, pt);    //aggiungo il nodo nella coda dei riferimenti
                residuo=n-pt->quantita;         //quantita che mi manca per quell'ingrediente, vado nel while se il lotto era troppo piccolo
                while((residuo>0)&&(pt!=T_null)){
                    pt=RB_Successor(pt);
                    if(pt!=T_null){
                        Check=Enqueue_CK(Check, pt);
                        residuo=residuo-pt->quantita;
                    }
                }
                if((residuo>0)){ //vuol dire che non ho abbastanza ingredienti e l'albero non ha successori
                    if(flag){ 
                        while(Check!=NULL){//devo deallocare Check 
                            tmp=Check->next;
                            free(Check);
                            Check=tmp;
                        }
                        return 0;
                    }
                    Lista_Ordini->OA=Enqueue_OA(Lista_Ordini->OA, r, num);
                    printf("accettato\n");
                    while(Check!=NULL){//devo deallocare Check 
                        tmp=Check->next;
                        free(Check);
                        Check=tmp;
                    }
                    return 0;
                }
            }
            else{
                if(flag){
                    while(Check!=NULL){//devo deallocare Check, può essere un ingrediente successivo per cui prima avevo messo qualcosa in coda a check
                        tmp=Check->next;
                        free(Check);
                        Check=tmp;
                    }
                    return 0;
                }
                Lista_Ordini->OA=Enqueue_OA(Lista_Ordini->OA, r, num);
                printf("accettato\n");
                while(Check!=NULL){//devo deallocare Check, può essere un ingrediente successivo per cui prima avevo messo qualcosa in coda a check
                    tmp=Check->next;
                    free(Check);
                    Check=tmp;
                }
                return 0;
            }
        }
        ptr=ptr->next;            
    } //se esco dal while vuol dire che ho abbastanza ingredienti, tutti in coda a check => devo rimuoverli
    ptr=ricetta; //ripunta al primo ingrediente
    while(ptr!=NULL){
        n=ptr->quantita*num;
        peso_tot=peso_tot+n; //tengo un contatore che pesi la ricetta in toto perché così lo ho già pronto per inserirlo nell'albero del peso
        Mk=Funzione_hash_M_new(Magazzino, Magazzino->dimensione, ptr->nome, strlen(ptr->nome));
        while((Check!=NULL)&&(Check->ingrediente->quantita<=n)){
            n=n-Check->ingrediente->quantita;
            tmp=Check->next;
            RB_Delete_M(Magazzino, Mk, Check->ingrediente);
            free(Check);
            Check=tmp;
        }
        if(Check!=NULL){ //può essere che con la RB_Delete_M io avessi esattamente i lotti che mi servivano, e ora check è null
            Check->ingrediente->quantita-=n;
            tmp=Check->next;
            free(Check);        
            Check=tmp;
        }
        ptr=ptr->next;
    }
    if(Check!=NULL) //può rimanere un nodo non completamente eliminato
        free(Check);
    return peso_tot;
}

Lista_Ordini_Attesa Enqueue_OA(Lista_Ordini_Attesa lista, char * r, int num){
    Lista_Ordini_Attesa ptr=NULL, cur=lista;
    size_t lunghezza=strlen(r);
    ptr=(Lista_Ordini_Attesa)malloc(sizeof(Ordine_in_attesa));
    ptr->data=tempo;
    ptr->quantita=num;
    ptr->ordine=(char*)malloc(sizeof(char)*(lunghezza+1));
    strcpy(ptr->ordine, r);
    ptr->next=NULL;
    if(lista==NULL)
        return ptr;
    else{
        while(cur->next!=NULL)
            cur=cur->next;
        cur->next=ptr;
    }
    return lista;
}

CHECK Enqueue_CK (CHECK lista, Lotto * ptr){
    CHECK punt, cur=lista;
    punt=(CHECK)malloc(sizeof(Nodo));
    punt->ingrediente=ptr;
    punt->next=NULL;
    if(lista==NULL)
        return punt;
    else{
        while(cur->next!=NULL)
            cur=cur->next;
        cur->next=punt;
    }
    return lista;
}

void Enqueue_OP(Lista_Ordini_pronti OP, char * r, int num, int peso_ordine, int data){
    Ordine_Pronto * ptr=NULL;
    if((OP->peso_tot+peso_ordine)<=capienza){ //caso in cui inserisco e basta in entrambi gli alberi
        if((OP->next!=NULL)&&(OP->next->prima_data<data)){ //se c'è un ordine che qui non ci stava ed è prima di questo devo metterlo dopo 
            Enqueue_OP(OP->next, r, num, peso_ordine, data);
            return;
        }
        RB_Insert_OP(OP, r, num, data, peso_ordine);
        OP->peso_tot=OP->peso_tot+peso_ordine;
        OP->numero_elem++;
        if(OP->prima_data==0)
            OP->prima_data=data;
        else if(OP->prima_data>data)
            OP->prima_data=data;
        if(OP->ultima_data<data)
            OP->ultima_data=data;
    }
    else if(((OP->peso_tot+peso_ordine)>capienza)&&(data<OP->ultima_data)){//caso in cui il peso eccede ma l'ordine va inserito al posto dell'ultimo
        while((OP->peso_tot+peso_ordine)>capienza){
            ptr=RB_Max(OP->RBTdata);
            if(ptr->data>data){
                if(OP->next!=NULL)
                    Enqueue_OP(OP->next, ptr->ordine, ptr->quantita, ptr->peso, ptr->data);
                else{
                    OP->next=AggiungiNodoLOP(OP->next);
                    Enqueue_OP(OP->next, ptr->ordine, ptr->quantita, ptr->peso, ptr->data);
                }
                OP->peso_tot-=ptr->peso;
                OP->numero_elem--;
                RB_Delete_LOP(OP, ptr);
                }
            else {
                Enqueue_OP(OP->next, r, num, peso_ordine, data); //lo aggiungo in quello dopo e aggiorno l'ultima data
                OP->ultima_data=ptr->data; //ptr è già il massimo corrente
                return;
            }
        }
        RB_Insert_OP(OP, r, num, data, peso_ordine);
        OP->peso_tot+=peso_ordine;
        OP->numero_elem++;
        ptr=RB_Max(OP->RBTdata); //aggiorno le date
        if(data>ptr->data)
            OP->ultima_data=data;
        else
           OP->ultima_data=ptr->data;
        if(data<OP->prima_data)
            OP->prima_data=data;
    }
    else{
        if(OP->next!=NULL){
            Enqueue_OP(OP->next, r, num, peso_ordine, data);
        }
        else {
            OP->next=AggiungiNodoLOP(OP->next);
            Enqueue_OP(OP->next, r, num, peso_ordine, data);
        }
    }
    return;
}

void RB_Insert_OP(Lista_Ordini_pronti LOP, char * nome, int quantita, int data, int peso){
    Ordine_Pronto * pre=TOP_null, *cur=LOP->RBTdata, *new=NULL;
    size_t lunghezza; 
    while(cur!=TOP_null){ //sono sicuro che la chiave sia unica!!
        pre=cur;
        if(data<cur->data)
            cur=cur->left;
        else
            cur=cur->right;
    }
    new=(Ordine_Pronto*)malloc(sizeof(Ordine_Pronto));
    lunghezza=strlen(nome);
    new->ordine=(char*)malloc(sizeof(char)*(lunghezza+1));
    strcpy(new->ordine, nome);
    new->data=data;
    new->quantita=quantita;
    new->peso=peso;
    
    new->parent=pre;
    if(pre==TOP_null){
        LOP->RBTdata=new;
        new->left=TOP_null;
        new->right=TOP_null;
        new->color='b';
        return;
    }
    else if(data<pre->data)
        pre->left=new;
    else    
        pre->right=new;
    new->left=TOP_null;
    new->right=TOP_null;
    new->color='r';
    RB_Insert_Fixup_LOP(LOP, new);
    return;
}

void RB_Insert_Fixup_LOP(Lista_Ordini_pronti LOP, Ordine_Pronto * new){
    Ordine_Pronto * y=NULL;
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
                    LeftRotate_LOP(LOP, new);
                }
                new->parent->color='b';
                new->parent->parent->color='r';
                RightRotate_LOP(LOP, new->parent->parent);
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
                    RightRotate_LOP(LOP, new);
                }   
                new->parent->color='b';
                new->parent->parent->color='r';
                LeftRotate_LOP(LOP, new->parent->parent);
            }
        }
    }
    LOP->RBTdata->color='b';
    return;
}

void LeftRotate_LOP(Lista_Ordini_pronti LOP, Ordine_Pronto * x){
    Ordine_Pronto * y=NULL;
    y=x->right;
    x->right=y->left;
    if(y->left!=TOP_null){
        y->left->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==TOP_null){
        LOP->RBTdata=y;
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

void RightRotate_LOP(Lista_Ordini_pronti LOP, Ordine_Pronto * x){
    Ordine_Pronto * y=NULL;
    y=x->left;
    x->left=y->right;
    if(y->right!=TOP_null){
        y->right->parent=x;
    }
    y->parent=x->parent;
    if(x->parent==TOP_null){
        LOP->RBTdata=y;
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

Ordine_Pronto * RB_Max(Ordine_Pronto * x){
    Ordine_Pronto * cur=x;
    while(cur->right!=TOP_null)
        cur=cur->right;
    return cur;
}

void RB_Delete_LOP(Lista_Ordini_pronti LOP, Ordine_Pronto * z){
    Ordine_Pronto * dacanc=NULL, * x=NULL; 
    size_t lunghezza;
    if((z->left==TOP_null)||(z->right==TOP_null))
        dacanc=z;
    else
        dacanc=RB_Successor_LOP(z);
    if(dacanc->left!=TOP_null)
        x=dacanc->left;
    else 
        x=dacanc->right;
    x->parent=dacanc->parent;
    if(dacanc->parent==TOP_null)
        LOP->RBTdata=x;
    else if(dacanc==dacanc->parent->left)
        dacanc->parent->left=x;
    else 
        dacanc->parent->right=x;
    if(dacanc!=z){
        z->peso=dacanc->peso;
        z->quantita=dacanc->quantita;
        z->data=dacanc->data;
        lunghezza=strlen(dacanc->ordine);
        free(z->ordine);
        z->ordine=(char *)malloc(sizeof(char)*(lunghezza+1));
        strcpy(z->ordine, dacanc->ordine);
    }
    if(dacanc->color=='b')
        RB_Delete_Fixup_LOP(LOP, x);
    free(dacanc->ordine);
    free(dacanc);
    return;
}

void RB_Delete_Fixup_LOP(Lista_Ordini_pronti LOP, Ordine_Pronto * x){
    Ordine_Pronto * w=NULL;
    while((x!=LOP->RBTdata)&&(x->color=='b')){
        if(x==x->parent->left){
            w=x->parent->right;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                LeftRotate_LOP(LOP, x->parent);
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
                    RightRotate_LOP(LOP, w);
                    w=x->parent->right;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->right->color='b';
                LeftRotate_LOP(LOP, x->parent);
                x=LOP->RBTdata;
            }
        }
        else{
            w=x->parent->left;
            if(w->color=='r'){
                w->color='b';
                x->parent->color='r';
                RightRotate_LOP(LOP, x->parent);
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
                    LeftRotate_LOP(LOP, w);
                    w=x->parent->left;
                }
                w->color=x->parent->color;
                x->parent->color='b';
                w->left->color='b';
                RightRotate_LOP(LOP, x->parent);
                x=LOP->RBTdata;
            }
        }
    }
    x->color='b';
    return;
}

Ordine_Pronto * RB_Successor_LOP(Ordine_Pronto * x){
    Ordine_Pronto * y;
    if(x->right!=TOP_null){
        return RB_Min_LOP(x->right);
    }
    y=x->parent;
    while((y!=TOP_null)&&(y->right==x)){
        x=y;
        y=y->parent;
    }
    return y;
}

Ordine_Pronto * RB_Min_LOP(Ordine_Pronto * x){
    Ordine_Pronto * cur=x;
    while(cur->left!=TOP_null)
        cur=cur->left;
    return cur;
}


void Corriere(LO_Poynter LO, Lista_Ordini_pronti OP){
    Ordine_Stampabile * v;
    int i;
    if(OP->RBTdata==TOP_null) {//se l'albero degli ordini pronti è vuoto
        printf("camioncino vuoto\n");
        return;
    }
    v=(Ordine_Stampabile *)malloc(sizeof(Ordine_Stampabile)*(OP->numero_elem)); //ottengo un vettore dove linearizzare l'albero
    Riempi_vett(v, OP->RBTdata); //riempio il vettore con gli elementi da ordinare e stampare
    indice=0; //rimetto indice=0 per la prossima chiamata
    if(OP->numero_elem>1) //lo chiamo solo se il vettore è più grande di 1
        MergeSort(v, 0, OP->numero_elem-1); //ottengo il vettore ordinato in ordine crescente di peso
    for(i=OP->numero_elem-1; i>=0; i--){
        printf("%d %s %d\n", v[i].data, v[i].ordine, v[i].quantita);
        free(v[i].ordine);
    }
    free(v);
    if(LO->OP->next==NULL){ //se non c'è nessun altro nodo lo preparo
        free(LO->OP);
        LO->OP=NULL;
        LO->OP=AggiungiNodoLOP(LO->OP);
    }
    else{//se c'è già un altro nodo aggancio e basta
        LO->OP=OP->next;
        free(OP);   
    }
    return;
}

void Riempi_vett(Ordine_Stampabile * v, RBTordini T){
    size_t lunghezza;
    if(T->left!=TOP_null)
        Riempi_vett(v, T->left);
    v[indice].data=T->data;
    v[indice].peso=T->peso;
    v[indice].quantita=T->quantita;
    lunghezza=strlen(T->ordine);
    v[indice].ordine=(char*)malloc(sizeof(char)*(lunghezza+1));
    strcpy(v[indice].ordine, T->ordine);
    indice++;
    if(T->right!=TOP_null)
        Riempi_vett(v, T->right);
    free(T->ordine);
    free(T);
    return;
}

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