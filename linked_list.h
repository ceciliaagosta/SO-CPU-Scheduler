#pragma once

typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

void List_init(ListHead* head);     //Inizializza la lista (tutti i campi a 0)

ListItem* List_find(ListHead* head, ListItem* item);        //Cerca item nella lista puntata da head con scansione lineare. Ritorna item se lo trova, 0 altrimenti

ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);      //Inserisce item dopo prev nella lista puntata da head, se item non è già presente e se                                                                               prev è presente

ListItem* List_detach(ListHead* head, ListItem* item);      //Rimuove item dalla lista, se presente, e lo restituisce

ListItem* List_pushBack(ListHead* head, ListItem* item);    //Inserisce item in fondo alla lista puntata da head

ListItem* List_pushFront(ListHead* head, ListItem* item);   //Inserisce item in testa alla lista puntata da head

ListItem* List_popFront(ListHead* head);        //Rimuove il primo elemento della lista puntata da head
