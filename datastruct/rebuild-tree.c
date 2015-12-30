/*************************************************************************
    > File Name: rebuild-tree.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: 2015-12-30 11:45:50
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

typedef struct bnode {
    int value;
    struct bnode * left;
    struct bnode * right;
} node;

node * rebuild(int * pbegin, int * pend, int * mbegin, int * mend)
{
    node * root = (node *)malloc(sizeof(node));
    root->value = *pbegin;
    root->left  = root->right = NULL;

    if (pbegin == pend && mbegin == mend) {
        return root;
    }

    int *root_index = mbegin;
    while (*root_index != root->value && root_index <= mend)
        root_index ++;
    
    int left_size = root_index - mbegin;
    int right_size = mend - root_index;

    if (left_size > 0) {
        root->left = rebuild(pbegin + 1, pbegin + left_size, mbegin, root_index - 1);
    }
    if (right_size > 0) {
        root->right = rebuild(pbegin + left_size + 1, pend, root_index + 1, mend);
    }
    return root;
}

void travel(node * root, int order, int deep)
{
    if (order == 0) {
        if (root != NULL) {
            printf("%d ", root->value);
            travel(root->left, order, deep + 1);
            travel(root->right, order, deep + 1);
        }
    } else if (order == 1) {
        if (root != NULL) {
            travel(root->left, order, deep + 1);
            printf("%d ", root->value);
            travel(root->right, order, deep + 1);
        }
    } else if (order == 2) {
        if (root != NULL) {
            travel(root->left, order, deep + 1);
            travel(root->right, order, deep + 1);
            printf("%d ", root->value);
        }
    }
}

int main()
{
    int pre[8] = {1,2,4,7,3,5,6,8};
    int in[8]  = {4,7,2,1,5,3,8,6};
    int len = sizeof(pre) / sizeof(pre[0]);
    
    node * root = rebuild(pre, pre + len - 1, in, in + len - 1);

    printf("travel, preoder:");
    travel(root, 0, 1);
    printf("\n");

    printf("travel, mid:");
    travel(root, 1, 1);
    printf("\n");

    printf("travel, suff:");
    travel(root, 2, 1);
    printf("\n");
    return 0;
}
