#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode {
	int val;
	struct ListNode *next;
} ListNode;

ListNode* l1 = NULL;
ListNode* l2 = NULL;

void addNode(ListNode **head, int val) {
	ListNode *temp;
	temp = (ListNode *)malloc(sizeof(ListNode));
	temp->val = val;
	temp->next = NULL;
	if (!*head) {
		*head = temp;
		return;
	}
	ListNode *temp_head = *head;
	while(temp_head->next) {
		temp_head = temp_head->next;	
	}
	temp_head->next = temp;
}

void printNode(ListNode *head)
{
	ListNode *temp = head;
	if (!temp) {
		printf("[]\n");
	}
	do {
		if (temp->next == NULL) {
			printf("[%d]\n", temp->val);
			return;
		}
		printf("[%d]-> ", temp->val);
		temp = temp->next;
	} while(temp);
}

ListNode* addTwoNumbers(ListNode* l1, struct ListNode*l2) {
	int sum = 0; int c = 0;
	ListNode* l3 = NULL;
	while (l1 || l2 ) {
		sum = 0;
		if (l1 != NULL) {
			sum += l1->val;
			l1 = l1->next;
		}
		if (l2 != NULL) {
			sum += l2->val;
			l2 = l2->next;
		}
		sum += c;
		c = sum/10;
		addNode(&l3, sum%10);
	}
	if (c != 0) {
		addNode(&l3, c);
	}
	return l3;
}

int main() {
	addNode(&l1, 9);
	addNode(&l1, 9);
	addNode(&l1, 9);
	addNode(&l2, 9);
	addNode(&l2, 9);
	addNode(&l2, 9);
	printNode(l1);
	printNode(l2);
	ListNode *l3 = addTwoNumbers(l1, l2);
	printNode(l3);
}
