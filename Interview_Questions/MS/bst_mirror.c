#include <stdio.h>
#include <stdlib.h>

#define MAX_Q_LEN	10

typedef struct node {
	struct node *left;
	struct node *right;
	int data;
} node_t;

int front;
int rear;

node_t *Q[MAX_Q_LEN];

void enqueue(node_t *root)
{
	Q[rear] = root;
	rear++;
}

node_t * dequeue()
{
	front++;
	return Q[front-1];
}

node_t * newNode(int data)
{
	node_t *temp = malloc(sizeof(node_t));
	temp->data = data;
	temp->left = temp->right = NULL;
	return temp;
}

void levelOrderTraversal(node_t *root)
{
	if (root == NULL) return;
	node_t *temp = root;
	while (temp) {
		printf("%d ", temp->data);
		if (temp->left != NULL)
			enqueue(temp->left);
		if (temp->right != NULL)
			enqueue(temp->right);
		temp = dequeue();
	}
	printf("\n");
}

void mirror(node_t *root)
{
	node_t *temp;

	if (root == NULL)
		return;
	mirror(root->left);
	mirror(root->right);
	temp = root->left;
	root->left = root->right;
	root->right = temp;	
}

int main()
{
	int i = 0;
	node_t *root = newNode(10);
	root->left = newNode(5);
	root->right = newNode(12);
	root->left->left = newNode(3);
	root->left->right = newNode(7);
	levelOrderTraversal(root);
	mirror(root);
	front = rear = 0;
	for (i = 0; i<MAX_Q_LEN; i++)
		Q[i] = 0;
	levelOrderTraversal(root);
}
