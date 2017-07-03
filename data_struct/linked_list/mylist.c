#include "mylist.h"

void insert(mylist **head, int data) {
	mylist *temp, *temp1, *prev;

	temp = malloc(sizeof(mylist));
	temp->data = data;
	temp->next = NULL;
	temp1 = *head;

	if (*head == NULL) {
		*head = temp;
		return;
	}
	while (temp1->next != NULL) {
		temp1 = temp1->next;
	}
	temp1->next = temp;
}

void printlist(mylist *head) {
	while (head != NULL) {
		printf("%d->", head->data);
		head = head->next;
	}
	printf("NULL");
}

void deletenode(mylist **head_ref, int key) {
	// Store head node
	mylist* temp = *head_ref, *prev;

	// If head node itself holds the key or multiple occurrences of key
	while (temp != NULL && temp->data == key)
	{
		*head_ref = temp->next;   // Changed head
		free(temp);               // free old head
		temp = *head_ref;         // Change Temp
	}

	// Delete occurrences other than head
	while (temp != NULL)
	{
		// Search for the key to be deleted, keep track of the
		// previous node as we need to change 'prev->next'
		while (temp != NULL && temp->data != key)
		{
			prev = temp;
			temp = temp->next;
		}

		// If key was not present in linked list
		if (temp == NULL) return;

		// Unlink the node from linked list
		prev->next = temp->next;

		free(temp);  // Free memory

		//Update Temp for next iteration of outer loop
		temp = prev->next;
	}
}
