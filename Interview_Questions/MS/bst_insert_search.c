#include <stdio.h>
#include <stdlib.h>

typedef struct bst_node_t {
	int data;
	struct bst_node_t *left;
	struct bst_node_t*right;
} bst_node;

bst_node *getNewNode()
{
	bst_node *root = malloc(sizeof(bst_node));
	return root;
}

bst_node *Insert(bst_node *root, int data)
{
	if (root == NULL) {
		root = getNewNode();
		root->data = data;
		root->left = root->right = NULL;
	} else if (data <= root->data) {
		root->left = Insert(root->left, data);
	} else if (data > root->data) {
		root->right = Insert(root->right, data);
	}
	return root;
}

bst_node *Q[100] = {0};
int end = 0;
int start = 0;

void enqueue(bst_node *data)
{
	if (end < 100)
		Q[end++] = data;
}

bst_node *dequeue()
{
	if (start >= end)
		return NULL;
	return Q[start++];
}

void level_order(bst_node *root)
{
	if (root == NULL)
		return;
	printf("%d ", root->data);
	if (root->left)
		enqueue(root->left);
	if (root->right)
		enqueue(root->right);
	if ((root = dequeue()) != NULL) {
//		printf("\n====\n%d\n=====\n", root->data);
		level_order(root);
	}
}

void preorder(bst_node *root)
{
	if (root == NULL)
		return;
	if (root->left)
		printf("%d ", root->left->data);
}

int main()
{
	bst_node *root = NULL;
	root = Insert(root, 8);
	root = Insert(root, 10);
	root = Insert(root, 4);
	root = Insert(root, 9);
	root = Insert(root, 25);
	level_order(root);
	preorder(root);
}
