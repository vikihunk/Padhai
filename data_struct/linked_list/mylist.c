#include "mylist.h"

void insert(mylist **head, int data) {
  mylist *temp, *temp1;

  temp = malloc(sizeof(mylist));
  temp->data = data;
  temp->next = NULL;

  if (*head == NULL) {
    *head = temp;
    return;
  }
  temp1 = *head;
  while (temp1 != NULL) {
    temp1 = temp1->next;
  }
  temp1 = temp;
}
