/*
 * Please refer to README
 * This is the coding question from Pulse Secure:
 * - Sort a given singly-linked list
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct mylist {
	int data;
	struct mylist *next;
} mylist;

void insert(mylist **head, int val) {
	/* This function inserts data val at the end of the list */
	mylist *current, *temp;

	current = *head;
	temp = malloc(sizeof(mylist));
	temp->data = val;
	temp->next = NULL;

	if (*head == NULL) {
		*head = temp;
		return;
	}

	/* Traverse the list and add temp at the end of the list */
	while(current->next != NULL) {
		current = current->next;
	}
	current->next = temp;
}

void print_list(mylist *head) {
	while(head != NULL) {
		printf("%d->", head->data);
		head = head->next;
	}
	printf("NULL\n");
}

void swap(int *first, int *second) {
	int temp;
	temp = *first;
	*first = *second;
	*second = temp;
}

/*
 * This is like bubble sort where the maximum number
 * moves to the last of the list and is then pointed 
 * by rptr.
 * Subsequently next max number is parked at last but
 * one position and then rptr is updated to point to
 * last but one...and so on
 */
void sort(mylist **head) {
	int swapped = 0;
	mylist *rptr = NULL;
	mylist *current;
	
	do {
		swapped = 0;
		current = *head;
		while(current->next != rptr) {
			if (current->data > current->next->data) {
				swap(&(current->data), &(current->next->data));
				swapped = 1;
			}
			current = current->next;
		}
		rptr = current;
	} while (swapped);
}

int main() {
	mylist *head = NULL;
	insert(&head, 10);
	insert(&head, 40);
	insert(&head, 4);
	insert(&head, 4);
	insert(&head, 90);
	insert(&head, 30);
	insert(&head, 20);
	print_list(head);
	sort(&head);
	print_list(head);
	return 0;
}
