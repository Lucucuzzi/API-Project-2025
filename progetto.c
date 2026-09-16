#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Aereo per la lista degli aerei che ha ogni esagono
typedef struct ElementoLista{
    int x_dest;
    int y_dest;
    // elimino costo aereo perchè tanto è sempre costo dell'esagono
    struct ElementoLista *next;
} Aereo;

//esagono 
typedef struct{
    int key;
    int costo;
    Aereo *testa; //testa della lista degli aerei 
    int colore; //0->White, 1->Grey, 2->Black
    int dist;
} Ehx;

//Change cost
typedef struct El_Coda{
    Ehx *esagono;              
    struct El_Coda *next;
} NodoCoda;

typedef struct {
    NodoCoda *testa;
    NodoCoda *coda;
} Coda;

//Travel cost
typedef struct{
    Ehx **esagoni; //array di esagoni, doppio puntatore perchè nelle funzioni non devo passare una copia ma proprio quell'elemento
    int size; // quanti degli elementi dentro sono realmente un heap
    int length; //lunghezza totale dell'array dell'heap
} MinHeap;

typedef struct{
    int kpart;
    int kdest;
    int distanza;
    int valido; //0 non valido 
} Percorsi;
//Tutto globale per non perdere informazioni
Ehx *Mappa=NULL; 
Percorsi *Cache=NULL;
int max_colonne=0; //max x
int max_righe=0; //max y

MinHeap heap={NULL, 0, 0}; //Globale per evitare di crearlo a ogni travel cost

//Coda presa da https://www.docenti.unina.it/webdocenti-be/allegati/materiale-didattico/35004699
int CodaVuota(Coda *Q) {
    return (Q->testa == NULL);
}

void enqueue(Coda *Q, Ehx *e){
    //creo variabile di appoggio
    NodoCoda *nodo=malloc(sizeof(NodoCoda));
    nodo->esagono=e;  
    nodo->next=NULL;
    //se coda non esiste
    if(Q==NULL){
        return;
    }
    //se coda vuota inserisco in testa
    if(CodaVuota(Q)){
        Q->testa=nodo;
        Q->coda=Q->testa;
    }else {
        Q->coda->next=nodo;
        Q->coda=nodo;        
    }
}

Ehx* dequeue(Coda *Q){
    NodoCoda *tmp;
    Ehx *ritorno; //per returnare l'esagono che libero
    //se coda vuota
    if(CodaVuota(Q)) return NULL;
    
    tmp=Q->testa;
    ritorno=tmp->esagono;
    //se ha un solo elemento
    if (Q->testa==Q->coda){
        Q->testa=NULL;
        Q->coda=NULL;
    }
    else{ //elimino il primo
        Q->testa=Q->testa->next;
    }
    free(tmp); //elimino variabile temporanea che non mi serve 
    return ritorno; 
}

void Minheapify(int n){
    int l=2*n; //left
    int r=2*n+1; //right
    int posmin;
    // se sono dentro all'heap e la distanza di quello a sinistra è minore di quella passata allora la posizione dell'elemento minimo
    if(l<=heap.size){
        if(heap.esagoni[l]->dist<heap.esagoni[n]->dist)
            posmin=l;
        else 
            posmin=n;
    }
    else posmin=n;
    if (r<=heap.size)
        if(heap.esagoni[r]->dist<heap.esagoni[posmin]->dist) 
            posmin=r;
    if (posmin!=n){
        //swap a[i], a[min]
        Ehx *tmp;
        tmp=heap.esagoni[n];
        heap.esagoni[n]=heap.esagoni[posmin];
        heap.esagoni[posmin]=tmp;
        Minheapify(posmin);
    }  
}


void insertheap(Ehx *e){
    heap.size+=1;
    heap.esagoni[heap.size]=e;
    int i=heap.size;
    while (i>1 && heap.esagoni[(int)i/2]->dist>heap.esagoni[i]->dist){ //A[parent(i)]<A[i]
        //swap A[parent(i)], A[i]
        Ehx *tmp;
        tmp=heap.esagoni[(int)i/2];
        heap.esagoni[(int)i/2]=heap.esagoni[i];
        heap.esagoni[i]=tmp;
        i=(int)i/2; //i<-parent(i)
    }
}

Ehx* CancellaMin(){
    //se non esiste heap
    if (heap.size<1) return NULL;
    Ehx *min=heap.esagoni[1];
    heap.esagoni[1]=heap.esagoni[heap.size];
    heap.size-=1;
    Minheapify(1);
    return min;
}

int init(int x, int y){
    //init
    //METODO 2 PROVA A USARE UNA MARTRICE 2D (DOPPIO PUNTATORE DOPPIA ROTTURA)
    //rinizializzo la Cache
    for(int i=0; i<8192;i++){
        Cache[i].valido=0;
    }

    if(Mappa!=NULL) {
        //elimino tutti gli aerei
        for(int i=0; i<max_colonne*max_righe; i++){
            if(Mappa[i].testa!=NULL){
                Aereo *a=Mappa[i].testa;
                while(a!=NULL){
                    Aereo *tmp=a;
                    a=a->next;
                    free(tmp);
                }
            }
        }
        free(Mappa); //se occupa tanto tempo prova con reallo
    }
    //alloco tutto subito così da non perdere tempo dopo. per cancellare tutto quello che c'è dentro fare heap.size=0
    if (heap.esagoni!=NULL) 
        free(heap.esagoni);
    heap.esagoni=malloc((x*y+1)*sizeof(Ehx)); //più 1 perchè min heap parte da 1
    heap.size=0;
    heap.length=x*y;
        
    //alloca lo spazio e inizializza tutti a 0
    Mappa=malloc(x*y*sizeof(Ehx)); 
    for (int i=0; i<x*y; i++){
        Mappa[i].costo=1;
        Mappa[i].testa=NULL;
        Mappa[i].colore=0;
        Mappa[i].dist=2000000000; //infinito
        Mappa[i].key=i;
    }
    max_colonne=x;
    max_righe=y;
    
    return 1;
}

//dovrebbe funzionare
int toggle_air_route(int x1, int y1, int x2, int  y2){
    //controlla se x1 è < 0 o x1 > x mappa uguale per il resto
    if(x1<0 || x1>=max_colonne || y1<0 || y1>=max_righe || x2<0 || x2>=max_colonne || y2<0 || y2>=max_righe)
        return -1;
    //rinizializzo la Cache
    for(int i=0; i<8192;i++){
        Cache[i].valido=0;
    }
    //se aereo già presente elimina
    //scorro lista, se trovo pop, altrimenti vado ad aggiungerlo 
    //https://www.reddit.com/r/csharp/comments/1hmqvvg/1d_vs_2d_array_performance/?tl=it accesso a vettore
    int i=0; //i=num_aerei
    Aereo *a;
    //se non c'è nessun aereo lo metto
    if (Mappa[y1*max_colonne+x1].testa==NULL){
        //crea aereo
        a=malloc(sizeof(Aereo));
        a->x_dest=x2;
        a->y_dest=y2;
        a->next=NULL;
        //potresti eliminare costoA tanto è sempre quello dell'esagono
        Mappa[y1*max_colonne+x1].testa=a;
        return 1;
    }
    else{
        a=Mappa[y1*max_colonne+x1].testa;
        i=1;
        //SE DEVI ELIMINARE LA TESTA 
        if (x2==a->x_dest && y2==a->y_dest){
            Mappa[y1*max_colonne+x1].testa=a->next;
            free(a);
            return 1;
        }

        while (a->next!=NULL) {
            if(a->next->x_dest==x2 && a->next->y_dest==y2){
                //elimina aereo
                Aereo *temp=a->next;
                a->next=temp->next;  
                free(temp);          
                return 1;
            }
            a=a->next; //scorro la lista
            i+=1;
        } //se non l'ho trovato e ci sono  meno di 5 aerei lo aggiungo
        if (i<5){
            Aereo *new;
            new=malloc(sizeof(Aereo));
            new->x_dest=x2;
            new->y_dest=y2;
            new->next=NULL;
            a->next=new;
            return 1;
        }
        else return -1;
    }

}

int change_cost(int x1, int y1, int v, int raggio){
    //controlliamo esistenza di x1, y1 in mappa e che raggio sia diverso da 0
    if(x1<0 || x1>=max_colonne || y1<0 || y1>=max_righe || raggio==0 || v<-10 || v>10)
        return -1;

    //rinizializzo la Cache
    for(int i=0; i<8192;i++){
        Cache[i].valido=0;
    }

    //DA CAMBIARE ASSOLUTAMENTEEEEEEEEEEEE rinizializzo tutti i nodi
    for(int i=0; i<max_colonne*max_righe; i++){
        Mappa[i].colore=0;
        Mappa[i].dist=2000000000;
    }

    //BFS
    Ehx *e;
    //Aereo *a; mi dice che non viene usata
    int x, y;
    Mappa[y1*max_colonne+x1].colore=1;
    Mappa[y1*max_colonne+x1].dist=0;
    Coda Q;
    Q.testa=NULL;
    Q.coda=NULL;
    enqueue(&Q, &Mappa[y1*max_colonne+x1]); //incodo testa
    while(!CodaVuota(&Q)){
        e=dequeue(&Q);
        if(e->dist<raggio){
            y=(int)(e->key/max_colonne);
            x=e->key-y*max_colonne;

            //cambia costo di E e delle rotte aeree
            Mappa[y*max_colonne+x].costo=e->costo+(int)floor(v*((float)(raggio-e->dist)/raggio));
            if(Mappa[y*max_colonne+x].costo>100) 
                Mappa[y*max_colonne+x].costo=100;
            if(Mappa[y*max_colonne+x].costo<0) 
                Mappa[y*max_colonne+x].costo=0;
            // non serve cambiare il costo delle rotte aeree visto che è sempre uguale al costo dell'esagono
 
        // fai gli adiacenti non in un ciclo tanto sono sempre sei 
        
            if(x-1>=0){
                if(Mappa[y*max_colonne+(x-1)].colore==0){
                    Mappa[y*max_colonne+(x-1)].colore=1;
                    Mappa[y*max_colonne+(x-1)].dist=e->dist+1;
                    enqueue(&Q, &Mappa[y*max_colonne+(x-1)]);
                }
            }
            if(x+1<max_colonne){
                if(Mappa[y*max_colonne+(x+1)].colore==0){
                    Mappa[y*max_colonne+(x+1)].colore=1;
                    Mappa[y*max_colonne+(x+1)].dist=e->dist+1;
                    enqueue(&Q, &Mappa[y*max_colonne+(x+1)]);
                }
            }
                
            if(y-1>=0){
                if(Mappa[(y-1)*max_colonne+x].colore==0){
                    Mappa[(y-1)*max_colonne+x].colore=1;
                    Mappa[(y-1)*max_colonne+x].dist=e->dist+1;
                    enqueue(&Q, &Mappa[(y-1)*max_colonne+x]);
                }
            }
                
            if(y+1<max_righe){
                if(Mappa[(y+1)*max_colonne+x].colore==0){
                    Mappa[(y+1)*max_colonne+x].colore=1;
                    Mappa[(y+1)*max_colonne+x].dist=e->dist+1;
                    enqueue(&Q, &Mappa[(y+1)*max_colonne+x]);
                }
            }
            //se riga è dispari
            if(y%2==1){
                if(y+1<max_righe && x+1<max_colonne){
                    if(Mappa[(y+1)*max_colonne+(x+1)].colore==0){
                        Mappa[(y+1)*max_colonne+(x+1)].colore=1;
                        Mappa[(y+1)*max_colonne+(x+1)].dist=e->dist+1;
                        enqueue(&Q, &Mappa[(y+1)*max_colonne+(x+1)]);
                    }
                }
                if(y-1>=0 && x+1<max_colonne){
                    if(Mappa[(y-1)*max_colonne+(x+1)].colore==0){
                        Mappa[(y-1)*max_colonne+(x+1)].colore=1;
                        Mappa[(y-1)*max_colonne+(x+1)].dist=e->dist+1;
                        enqueue(&Q, &Mappa[(y-1)*max_colonne+(x+1)]);
                    }
                }
            }
            //se y è dispari
            else{
                if(y+1<max_righe && x-1>=0){
                    if(Mappa[(y+1)*max_colonne+(x-1)].colore==0){
                        Mappa[(y+1)*max_colonne+(x-1)].colore=1;
                        Mappa[(y+1)*max_colonne+(x-1)].dist=e->dist+1;
                        enqueue(&Q, &Mappa[(y+1)*max_colonne+(x-1)]);
                    }
                }
                if(y-1>=0 && x-1>=0){
                    if(Mappa[(y-1)*max_colonne+(x-1)].colore==0){
                        Mappa[(y-1)*max_colonne+(x-1)].colore=1;
                        Mappa[(y-1)*max_colonne+(x-1)].dist=e->dist+1;
                        enqueue(&Q, &Mappa[(y-1)*max_colonne+(x-1)]);
                    }
                }
            }
                          
        }
        e->colore=2;        
    }
    return 1;
}

int travel_cost(int xp, int yp, int xd, int yd){
    //travel_cost
    if(xp<0 || xp>=max_colonne || yp<0 || yp>=max_righe || xd<0 || xd>=max_colonne || yd<0 || yd>=max_righe)
        return -1;
    if(xp==xd && yp==yd)
        return 0;

    //in caso sia già in cache
    int i=0;
    Percorsi p;
    while(Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192].valido!=0){
        if(Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192].kpart==yp*max_colonne+xp && Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192].kdest==yd*max_colonne+xd)
            return Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192].distanza;
        else i+=1;
    }

    //DA CAMBIAREEEEEE rinizializzo tutti i nodi
    for(int i=0; i<max_colonne*max_righe; i++){
        Mappa[i].colore=0;
        Mappa[i].dist=2000000000;
    }
    heap.size=0; //rinizializzo lo heap
    //DIJKSTRA come negli appunti del prof
    Mappa[yp*max_colonne+xp].dist=0;
    Aereo *a;
    int x,y;
    insertheap(&Mappa[yp*max_colonne+xp]);
    while (heap.size>0){
        Ehx *u;
        u=CancellaMin();
        
        if(u->key==yd*max_colonne+xd){
            //inserire u->dist nella cache
            //doppio hashing funzione h1 xpart+ypart, h2 2(xdest+ydest)+1
            i=0;
            while(Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192].valido!=0){
                i+=1;
                //controllo se loop infinito, in caso rinizializzo cache
                if(i==8192){
                    for(int i=0; i<8192;i++){
                        Cache[i].valido=0;
                    }
                }
            }
            p.distanza=u->dist;
            p.kpart=yp*max_colonne+xp;
            p.kdest=yd*max_colonne+xd;
            p.valido=1;
            Cache[((xp+yp)+i*(2*(xd+yd)+1))%8192]=p;

            return u->dist;
        }            

        y=(int)(u->key/max_colonne);
        x=u->key-y*max_colonne;
        int nuovocosoto=Mappa[y*max_colonne+x].dist+Mappa[y*max_colonne+x].costo;
        if(u->colore!=2){
            if (Mappa[y*max_colonne+x].costo!=0){ //se il costo del nodo visitato non è zero allora posso visitare i vicini
                a=u->testa;

                while(a!=NULL){
                    if(Mappa[a->y_dest*max_colonne+a->x_dest].dist>nuovocosoto){ //se il nuovo path è migliore per arrivare a qul nodo lo aggiorno
                        Mappa[a->y_dest*max_colonne+a->x_dest].dist=nuovocosoto;
                        insertheap(&Mappa[a->y_dest*max_colonne+a->x_dest]);                        
                    }  
                    a=a->next;
                }
                
                if(x-1>=0){
                    if(Mappa[y*max_colonne+x-1].dist>nuovocosoto){
                        Mappa[y*max_colonne+x-1].dist=nuovocosoto;
                        insertheap(&Mappa[y*max_colonne+x-1]);
                    }
                }
                if(x+1<max_colonne){
                    if(Mappa[y*max_colonne+x+1].dist>nuovocosoto){
                        Mappa[y*max_colonne+x+1].dist=nuovocosoto;
                        insertheap(&Mappa[y*max_colonne+x+1]);    
                    }
                }
                    
                if(y-1>=0){
                    if(Mappa[(y-1)*max_colonne+x].dist>nuovocosoto){
                        Mappa[(y-1)*max_colonne+x].dist=nuovocosoto;
                        insertheap(&Mappa[(y-1)*max_colonne+x]);
                    }
                }
                    
                if(y+1<max_righe){
                        if(Mappa[(y+1)*max_colonne+x].dist>nuovocosoto){
                        Mappa[(y+1)*max_colonne+x].dist=nuovocosoto;
                        insertheap(&Mappa[(y+1)*max_colonne+x]);
                    }
                }
                //se riga è dispari
                if(y%2==1){
                    if(y+1<max_righe && x+1<max_colonne){
                        if(Mappa[(y+1)*max_colonne+x+1].dist>nuovocosoto){
                            Mappa[(y+1)*max_colonne+x+1].dist=nuovocosoto;
                            insertheap(&Mappa[(y+1)*max_colonne+x+1]);
                        }
                    }
                    if(y-1>=0 && x+1<max_colonne){
                        if(Mappa[(y-1)*max_colonne+x+1].dist>nuovocosoto){
                            Mappa[(y-1)*max_colonne+x+1].dist=nuovocosoto;
                            insertheap(&Mappa[(y-1)*max_colonne+x+1]);
                        }
                    }
                }
                //se riga è pari
                else{
                    if(y+1<max_righe && x-1>=0){
                        if(Mappa[(y+1)*max_colonne+x-1].dist>nuovocosoto){
                            Mappa[(y+1)*max_colonne+x-1].dist=nuovocosoto;
                            insertheap(&Mappa[(y+1)*max_colonne+x-1]);
                        }
                    }
                    if(y-1>=0 && x-1>=0){
                        if(Mappa[(y-1)*max_colonne+x-1].dist>nuovocosoto){
                            Mappa[(y-1)*max_colonne+x-1].dist=nuovocosoto;
                            insertheap(&Mappa[(y-1)*max_colonne+x-1]);
                        }
                        
                    }
                }
                u->colore=2;

            }
        }
        
    } 
    return -1; 
}

/*di prova da eliminare
void printatutto() {

    for (int riga = 0; riga < max_righe; riga++) {
        for (int colonna = 0; colonna < max_colonne; colonna++) {
            printf("%d ", Mappa[riga*max_colonne+colonna].costo);
        }
        printf("\n");
    }
    
    Aereo *a;
    Ehx *e;
    for(int i=0; i<max_righe*max_colonne; i++){
        e=&Mappa[i];
        a=e->testa;
        while(a!=NULL){
                a->costoA=e->costo;
                printf("Aereo parte da %d arriva a %d %d e costa %d\n", e->key, a->x_dest, a->y_dest, a->costoA);
                a=a->next;
        }
    }
    
}*/


int main(int argc, char *argv[]) {
    
    int a,b,c,d;
    char str[17]; //massimo peso di una stringa toggle_air_route

    Cache=malloc(8192*sizeof(Percorsi));
    for(int i=0; i<8192;i++){
        Cache[i].kpart=0;
        Cache[i].kdest=0;
        Cache[i].distanza=0;
        Cache[i].valido=0;
    }

    while(scanf("%s", str)!=-1){
        if(strcmp(str, "init")==0){
            if(scanf("%d %d", &a, &b))
                if(init(a, b)) printf("OK\n");
        }
        else if(strcmp(str, "travel_cost")==0){
                if(scanf("%d %d %d %d", &a, &b, &c, &d)){
                  printf("%d\n", travel_cost(a, b, c, d)); 
                }
                
                } 
        else if(strcmp(str, "change_cost")==0){ 
                if(scanf("%d %d %d %d", &a, &b, &c, &d)){
                    if(change_cost(a, b, c, d)==1) printf("OK\n");
                    else printf("KO\n");
                }
            } 
        else if(strcmp(str, "toggle_air_route")==0){
                if(scanf("%d %d %d %d", &a, &b, &c, &d)){
                    if(toggle_air_route(a, b, c, d)==1) printf("OK\n");
                    else printf("KO\n");
                    }
        }
        //else printatutto();

    }
    
    return 0;
}