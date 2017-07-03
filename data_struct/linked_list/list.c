#include "mylist.h"

int main() {
  mylist *head = NULL;
  int count, num;

  printf("Enter the number of nodes: ");
  scanf("%d", &count);

  while (count != 0) {
    scanf("%d", &num);
    insert(&head, num);
    count--;
  }
  printlist(head);
  printf("\n");
  printf("Which node to delete: ");
  scanf("%d", &num);
  deletenode(&head, num);
  printlist(head);
  printf("\n");
}
