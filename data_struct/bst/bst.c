/*
 * Simple implementation of inorder, preorder, postorder traversal
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 7

typedef struct mytree {
	int data;
	struct mytree *left_tree;
	struct mytree *right_tree;
} mytree;

void levelorder(mytree *root) {
	int stack[MAX_LEN];
	int i = 0;

	if(root) {
		stack[i++] = root->data;
	} else if (root->left_tree) {
		levelorder(root->left_tree);
	} else if(root->right_tree) {
		levelorder(root->right_tree);
	} else {
		return;
	}
}

void inorder(mytree *root)  {
	if(root == NULL)
		return;

	inorder(root->left_tree);
	printf("%d ", root->data);
	inorder(root->right_tree);
}

void postorder(mytree *root) {
	if(root == NULL)
		return;

	postorder(root->left_tree);
	postorder(root->right_tree);
	printf("%d ", root->data);
}

void preorder(mytree *root) {
	if(root == NULL) {
		return;
	}
	printf("%d ", root->data);
	preorder(root->left_tree);
	preorder(root->right_tree);
}

void insert(mytree **root, int val) {
	mytree *newNode;

	newNode = malloc(sizeof(mytree));
	newNode->data = val;
	newNode->left_tree = NULL;
	newNode->right_tree = NULL;

	if(*root == NULL) {
		*root = newNode;
		return;
	}

	if((*root)->data < val) {
		insert(&((*root)->right_tree), val);
	} else if ((*root)->data > val) {
		insert(&((*root)->left_tree), val);
	} else {
		*root = newNode;
	}
	return;
}

int main() {
	mytree *root;

	root = malloc(sizeof(mytree));
	root->data = 15;
	insert(&root, 2);
	insert(&root, 10);
	insert(&root, 22);
	insert(&root, 32);
	insert(&root, 1);
	insert(&root, 17);
	printf("Preorder: ");
	preorder(root);
	printf("\n");
	printf("Postorder: ");
	postorder(root);
	printf("\n");
	printf("Inorder: ");
	inorder(root);
	printf("\n");
	printf("Levelorder: ");
	levelorder(root);
	printf("\n");
	return 0;
}
