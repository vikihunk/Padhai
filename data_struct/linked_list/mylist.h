#include <stdlib.h>
#include <stdio.h>

typedef struct mylist {
  int data;
  struct mylist *next;
} mylist;

void insert(mylist **head, int data);
void printlist(mylist *head);
void deletenode(mylist **head, int data);
