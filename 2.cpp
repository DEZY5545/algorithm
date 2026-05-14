/**
 * balanced_trees_complete.c
 *
 * Balanced Trees Performance Comparison + Structure Visualization
 * Supports: BST, AVL, Red-Black, Order-5 B-Tree
 *
 * Features:
 *   1. Performance test with random large data.
 *   2. Tree structure output: enter an array, print node-parent relationships.
 *
 * Note: All allocations use explicit casts to satisfy C++ compilers.
 */

#include <stdio.h>
#include <stdlib.h>


/* ============================================================================
 * 0. Cross-platform high-precision timer & utilities
 * ============================================================================ */


#include <windows.h>
double get_time_in_seconds() {
    LARGE_INTEGER freq, time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (double)time.QuadPart / freq.QuadPart;
}
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void swap_double(double* a, double* b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

void shuffle_array(double* arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap_double(&arr[i], &arr[j]);
    }
}


/* ============================================================================
 * 1. Binary Search Tree (BST)
 * ============================================================================ */
typedef struct BSTNode {
    double key;
    struct BSTNode *left, *right;
} BSTNode;

BSTNode* create_bst_node(double key) {
    BSTNode* node = (BSTNode*)safe_malloc(sizeof(BSTNode));
    node->key = key;
    node->left = node->right = NULL;
    return node;
}

BSTNode* bst_insert(BSTNode* root, double key) {
    if (root == NULL) return create_bst_node(key);
    if (key < root->key)
        root->left = bst_insert(root->left, key);
    else if (key > root->key)
        root->right = bst_insert(root->right, key);
    return root;
}

BSTNode* bst_search(BSTNode* root, double key) {
    if (root == NULL || root->key == key) return root;
    if (key < root->key) return bst_search(root->left, key);
    return bst_search(root->right, key);
}

BSTNode* bst_find_min(BSTNode* root) {
    while (root && root->left) root = root->left;
    return root;
}

BSTNode* bst_delete(BSTNode* root, double key) {
    if (root == NULL) return NULL;
    if (key < root->key) {
        root->left = bst_delete(root->left, key);
    } else if (key > root->key) {
        root->right = bst_delete(root->right, key);
    } else {
        if (root->left == NULL) {
            BSTNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            BSTNode* temp = root->left;
            free(root);
            return temp;
        }
        BSTNode* temp = bst_find_min(root->right);
        root->key = temp->key;
        root->right = bst_delete(root->right, temp->key);
    }
    return root;
}

void bst_free(BSTNode* root) {
    if (root) {
        bst_free(root->left);
        bst_free(root->right);
        free(root);
    }
}

/* --- BST structure printer --- */
static void print_bst_subtree(BSTNode* node, double parent_key, int is_root) {
    if (!node) return;
    if (is_root)
        printf("  [BST] Node: %.2f, Parent: NULL (root)\n", node->key);
    else
        printf("  [BST] Node: %.2f, Parent: %.2f\n", node->key, parent_key);
    print_bst_subtree(node->left,  node->key, 0);
    print_bst_subtree(node->right, node->key, 0);
}

void print_bst_structure(BSTNode* root) {
    printf("--- BST structure (Node -> Parent) ---\n");
    if (!root) printf("  (empty tree)\n");
    else print_bst_subtree(root, 0.0, 1);
    printf("---------------------------------------\n");
}


/* ============================================================================
 * 2. AVL Tree
 * ============================================================================ */
typedef struct AVLNode {
    double key;
    int height;
    struct AVLNode *left, *right;
} AVLNode;

int avl_height(AVLNode* n) { return n ? n->height : 0; }
int avl_balance(AVLNode* n) { return n ? avl_height(n->left) - avl_height(n->right) : 0; }
int max_int(int a, int b) { return a > b ? a : b; }

AVLNode* create_avl_node(double key) {
    AVLNode* node = (AVLNode*)safe_malloc(sizeof(AVLNode));
    node->key = key;
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

AVLNode* avl_right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

AVLNode* avl_left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

AVLNode* avl_insert(AVLNode* root, double key) {
    if (!root) return create_avl_node(key);
    if (key < root->key)
        root->left = avl_insert(root->left, key);
    else if (key > root->key)
        root->right = avl_insert(root->right, key);
    else return root;

    root->height = 1 + max_int(avl_height(root->left), avl_height(root->right));
    int bal = avl_balance(root);

    if (bal > 1 && key < root->left->key) return avl_right_rotate(root);
    if (bal < -1 && key > root->right->key) return avl_left_rotate(root);
    if (bal > 1 && key > root->left->key) {
        root->left = avl_left_rotate(root->left);
        return avl_right_rotate(root);
    }
    if (bal < -1 && key < root->right->key) {
        root->right = avl_right_rotate(root->right);
        return avl_left_rotate(root);
    }
    return root;
}

AVLNode* avl_search(AVLNode* root, double key) {
    if (!root || root->key == key) return root;
    if (key < root->key) return avl_search(root->left, key);
    return avl_search(root->right, key);
}

AVLNode* avl_find_min(AVLNode* root) {
    while (root && root->left) root = root->left;
    return root;
}

AVLNode* avl_delete(AVLNode* root, double key) {
    if (!root) return NULL;
    if (key < root->key) {
        root->left = avl_delete(root->left, key);
    } else if (key > root->key) {
        root->right = avl_delete(root->right, key);
    } else {
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            AVLNode* temp = avl_find_min(root->right);
            root->key = temp->key;
            root->right = avl_delete(root->right, temp->key);
        }
    }
    if (!root) return NULL;
    root->height = 1 + max_int(avl_height(root->left), avl_height(root->right));
    int bal = avl_balance(root);

    if (bal > 1 && avl_balance(root->left) >= 0) return avl_right_rotate(root);
    if (bal > 1 && avl_balance(root->left) < 0) {
        root->left = avl_left_rotate(root->left);
        return avl_right_rotate(root);
    }
    if (bal < -1 && avl_balance(root->right) <= 0) return avl_left_rotate(root);
    if (bal < -1 && avl_balance(root->right) > 0) {
        root->right = avl_right_rotate(root->right);
        return avl_left_rotate(root);
    }
    return root;
}

void avl_free(AVLNode* root) {
    if (root) {
        avl_free(root->left);
        avl_free(root->right);
        free(root);
    }
}

/* --- AVL structure printer --- */
static void print_avl_subtree(AVLNode* node, double parent_key, int is_root) {
    if (!node) return;
    if (is_root)
        printf("  [AVL] Node: %.2f, Parent: NULL (root)\n", node->key);
    else
        printf("  [AVL] Node: %.2f, Parent: %.2f\n", node->key, parent_key);
    print_avl_subtree(node->left,  node->key, 0);
    print_avl_subtree(node->right, node->key, 0);
}

void print_avl_structure(AVLNode* root) {
    printf("--- AVL structure (Node -> Parent) ---\n");
    if (!root) printf("  (empty tree)\n");
    else print_avl_subtree(root, 0.0, 1);
    printf("---------------------------------------\n");
}


/* ============================================================================
 * 3. Red-Black Tree
 * ============================================================================ */
typedef enum { RED, BLACK } NodeColor;
typedef struct RBNode {
    double key;
    NodeColor color;
    struct RBNode *left, *right, *parent;
} RBNode;

RBNode NIL_NODE = {0, BLACK, NULL, NULL, NULL};
#define NIL (&NIL_NODE)

RBNode* create_rb_node(double key) {
    RBNode* node = (RBNode*)safe_malloc(sizeof(RBNode));
    node->key = key;
    node->color = RED;
    node->left = node->right = node->parent = NIL;
    return node;
}

void rb_left_rotate(RBNode** root, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left != NIL) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == NIL) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void rb_right_rotate(RBNode** root, RBNode* y) {
    RBNode* x = y->left;
    y->left = x->right;
    if (x->right != NIL) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == NIL) *root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
}

void rb_insert_fixup(RBNode** root, RBNode* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_left_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_right_rotate(root, z->parent->parent);
            }
        } else {
            RBNode* y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_right_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rb_left_rotate(root, z->parent->parent);
            }
        }
    }
    (*root)->color = BLACK;
}

void rb_insert(RBNode** root, double key) {
    RBNode* z = create_rb_node(key);
    RBNode* y = NIL;
    RBNode* x = *root;
    while (x != NIL) {
        y = x;
        if (z->key < x->key) x = x->left;
        else if (z->key > x->key) x = x->right;
        else { free(z); return; }
    }
    z->parent = y;
    if (y == NIL) *root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    if (z->parent == NIL) { z->color = BLACK; return; }
    if (z->parent->parent == NIL) return;
    rb_insert_fixup(root, z);
}

void rb_transplant(RBNode** root, RBNode* u, RBNode* v) {
    if (u->parent == NIL) *root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

RBNode* rb_tree_minimum(RBNode* node) {
    while (node->left != NIL) node = node->left;
    return node;
}

void rb_delete_fixup(RBNode** root, RBNode* x) {
    while (x != *root && x->color == BLACK) {
        if (x == x->parent->left) {
            RBNode* w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_left_rotate(root, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rb_right_rotate(root, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rb_left_rotate(root, x->parent);
                x = *root;
            }
        } else {
            RBNode* w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_right_rotate(root, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rb_left_rotate(root, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rb_right_rotate(root, x->parent);
                x = *root;
            }
        }
    }
    x->color = BLACK;
}

void rb_delete(RBNode** root, double key) {
    RBNode* z = *root;
    while (z != NIL) {
        if (z->key == key) break;
        if (key < z->key) z = z->left;
        else z = z->right;
    }
    if (z == NIL) return;

    RBNode* y = z, *x;
    NodeColor y_original_color = y->color;
    if (z->left == NIL) {
        x = z->right;
        rb_transplant(root, z, z->right);
    } else if (z->right == NIL) {
        x = z->left;
        rb_transplant(root, z, z->left);
    } else {
        y = rb_tree_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rb_transplant(root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    free(z);
    if (y_original_color == BLACK) rb_delete_fixup(root, x);
}

RBNode* rb_search(RBNode* root, double key) {
    while (root != NIL && root->key != key) {
        if (key < root->key) root = root->left;
        else root = root->right;
    }
    return (root == NIL) ? NULL : root;
}

void rb_free(RBNode* root) {
    if (root != NIL) {
        rb_free(root->left);
        rb_free(root->right);
        free(root);
    }
}

/* --- Red-Black structure printer (uses parent pointers) --- */
static void print_rb_subtree(RBNode* node) {
    if (node == NIL) return;
    if (node->parent == NIL)
        printf("  [RBT] Node: %.2f (%s), Parent: NULL (root)\n",
               node->key, node->color == RED ? "R" : "B");
    else
        printf("  [RBT] Node: %.2f (%s), Parent: %.2f (%s)\n",
               node->key, node->color == RED ? "R" : "B",
               node->parent->key, node->parent->color == RED ? "R" : "B");
    print_rb_subtree(node->left);
    print_rb_subtree(node->right);
}

void print_rb_structure(RBNode* root) {
    printf("--- Red-Black Tree structure (Node -> Parent) ---\n");
    if (root == NIL) printf("  (empty tree)\n");
    else print_rb_subtree(root);
    printf("--------------------------------------------------\n");
}


/* ============================================================================
 * 4. Order-5 B-Tree
 * ============================================================================ */
#define M_ORDER 5
#define MAX_KEYS (M_ORDER - 1)        // 4
#define MIN_KEYS ((M_ORDER + 1)/2 - 1) // 2

typedef struct BTreeNode {
    int num_keys;
    double keys[MAX_KEYS + 1];
    struct BTreeNode* children[M_ORDER + 1];
    bool is_leaf;
} BTreeNode;

BTreeNode* create_btree_node(bool is_leaf) {
    BTreeNode* node = (BTreeNode*)safe_malloc(sizeof(BTreeNode));
    node->num_keys = 0;
    node->is_leaf = is_leaf;
    for (int i = 0; i <= M_ORDER; i++) node->children[i] = NULL;
    return node;
}

bool btree_search(BTreeNode* root, double key) {
    if (!root) return false;
    int i = 0;
    while (i < root->num_keys && key > root->keys[i]) i++;
    if (i < root->num_keys && key == root->keys[i]) return true;
    if (root->is_leaf) return false;
    return btree_search(root->children[i], key);
}

void btree_split_child(BTreeNode* parent, int i, BTreeNode* full_child) {
    BTreeNode* new_node = create_btree_node(full_child->is_leaf);
    int mid = MAX_KEYS / 2;   // 2
    new_node->num_keys = MAX_KEYS - mid - 1; // 1
    for (int j = 0; j < new_node->num_keys; j++)
        new_node->keys[j] = full_child->keys[j + mid + 1];
    if (!full_child->is_leaf) {
        for (int j = 0; j <= new_node->num_keys; j++)
            new_node->children[j] = full_child->children[j + mid + 1];
    }
    full_child->num_keys = mid;
    for (int j = parent->num_keys; j >= i + 1; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = new_node;
    for (int j = parent->num_keys - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = full_child->keys[mid];
    parent->num_keys++;
}

void btree_insert_non_full(BTreeNode* node, double key) {
    int i = node->num_keys - 1;
    if (node->is_leaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        while (i >= 0 && node->keys[i] > key) i--;
        i++;
        if (node->children[i]->num_keys == MAX_KEYS) {
            btree_split_child(node, i, node->children[i]);
            if (key > node->keys[i]) i++;
        }
        btree_insert_non_full(node->children[i], key);
    }
}

BTreeNode* btree_insert(BTreeNode* root, double key) {
    if (!root) {
        root = create_btree_node(true);
        root->keys[0] = key;
        root->num_keys = 1;
        return root;
    }
    if (root->num_keys == MAX_KEYS) {
        BTreeNode* new_root = create_btree_node(false);
        new_root->children[0] = root;
        btree_split_child(new_root, 0, root);
        btree_insert_non_full(new_root, key);
        return new_root;
    } else {
        btree_insert_non_full(root, key);
        return root;
    }
}

int btree_find_key(BTreeNode* node, double key) {
    int idx = 0;
    while (idx < node->num_keys && node->keys[idx] < key) idx++;
    return idx;
}

BTreeNode* btree_delete(BTreeNode* root, double key) {
    if (!root) return NULL;
    int idx = btree_find_key(root, key);
    if (idx < root->num_keys && root->keys[idx] == key) {
        if (root->is_leaf) {
            for (int i = idx + 1; i < root->num_keys; i++)
                root->keys[i - 1] = root->keys[i];
            root->num_keys--;
        } else {
            BTreeNode* cur = root->children[idx + 1];
            while (!cur->is_leaf) cur = cur->children[0];
            double succ = cur->keys[0];
            root->keys[idx] = succ;
            root->children[idx + 1] = btree_delete(root->children[idx + 1], succ);
        }
    } else {
        if (!root->is_leaf)
            root->children[idx] = btree_delete(root->children[idx], key);
    }
    return root;
}

void btree_free(BTreeNode* root) {
    if (root) {
        if (!root->is_leaf)
            for (int i = 0; i <= root->num_keys; i++)
                btree_free(root->children[i]);
        free(root);
    }
}

/* --- B-Tree structure printer --- */
static void print_btree_subtree(BTreeNode* node, BTreeNode* parent) {
    if (!node) return;
    printf("  [B-Tree] Node keys: [");
    for (int i = 0; i < node->num_keys; i++) {
        printf("%.2f", node->keys[i]);
        if (i < node->num_keys - 1) printf(", ");
    }
    printf("]");
    if (!parent) {
        printf(", Parent: NULL (root)\n");
    } else {
        printf(", Parent keys: [");
        for (int i = 0; i < parent->num_keys; i++) {
            printf("%.2f", parent->keys[i]);
            if (i < parent->num_keys - 1) printf(", ");
        }
        printf("]\n");
    }
    if (!node->is_leaf)
        for (int i = 0; i <= node->num_keys; i++)
            print_btree_subtree(node->children[i], node);
}

void print_btree_structure(BTreeNode* root) {
    printf("--- B-Tree structure (Node block -> Parent block) ---\n");
    if (!root) printf("  (empty tree)\n");
    else print_btree_subtree(root, NULL);
    printf("------------------------------------------------------\n");
}


/* ============================================================================
 * 5. Performance test (original, with explicit casts for C++ compatibility)
 * ============================================================================ */
void run_performance_test() {
    int N = 100000, M = 50000;
    printf("=================================================================\n");
    printf("   Balanced Trees Performance Comparison (N=%d, M=%d)\n", N, M);
    printf("   Unit: Milliseconds (ms)\n");
    printf("=================================================================\n");
    printf("%-10s | %-15s | %-15s | %-15s\n", "Tree Type", "Insert(ms)", "Search(ms)", "Delete(ms)");
    printf("-----------------------------------------------------------------\n");

    srand(12345);
    double* insert_data = (double*)safe_malloc(N * sizeof(double));
    for (int i = 0; i < N; i++)
        insert_data[i] = ((double)rand() / RAND_MAX) * 1000000.0;

    double* search_data = (double*)safe_malloc(M * sizeof(double));
    for (int i = 0; i < M; i++) search_data[i] = insert_data[i];
    shuffle_array(search_data, M);

    double start, end, time_ins, time_search, time_del;

    /* BST */
    BSTNode* bst_root = NULL;
    start = get_time_in_seconds();
    for (int i = 0; i < N; i++) bst_root = bst_insert(bst_root, insert_data[i]);
    end = get_time_in_seconds();
    time_ins = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) bst_search(bst_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) bst_root = bst_delete(bst_root, search_data[i]);
    end = get_time_in_seconds();
    time_del = (end - start) * 1000.0;
    printf("%-10s | %-15.6f | %-15.6f | %-15.6f\n", "BST", time_ins, time_search, time_del);
    bst_free(bst_root);

    /* AVL */
    AVLNode* avl_root = NULL;
    start = get_time_in_seconds();
    for (int i = 0; i < N; i++) avl_root = avl_insert(avl_root, insert_data[i]);
    end = get_time_in_seconds();
    time_ins = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) avl_search(avl_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) avl_root = avl_delete(avl_root, search_data[i]);
    end = get_time_in_seconds();
    time_del = (end - start) * 1000.0;
    printf("%-10s | %-15.6f | %-15.6f | %-15.6f\n", "AVL", time_ins, time_search, time_del);
    avl_free(avl_root);

    /* Red-Black */
    RBNode* rb_root = NIL;
    start = get_time_in_seconds();
    for (int i = 0; i < N; i++) rb_insert(&rb_root, insert_data[i]);
    end = get_time_in_seconds();
    time_ins = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) rb_search(rb_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) rb_delete(&rb_root, search_data[i]);
    end = get_time_in_seconds();
    time_del = (end - start) * 1000.0;
    printf("%-10s | %-15.6f | %-15.6f | %-15.6f\n", "RB-Tree", time_ins, time_search, time_del);
    rb_free(rb_root);

    /* B-Tree */
    BTreeNode* btree_root = NULL;
    start = get_time_in_seconds();
    for (int i = 0; i < N; i++) btree_root = btree_insert(btree_root, insert_data[i]);
    end = get_time_in_seconds();
    time_ins = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) btree_search(btree_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < M; i++) btree_root = btree_delete(btree_root, search_data[i]);
    end = get_time_in_seconds();
    time_del = (end - start) * 1000.0;
    printf("%-10s | %-15.6f | %-15.6f | %-15.6f\n", "5-B-Tree", time_ins, time_search, time_del);
    btree_free(btree_root);

    printf("=================================================================\n");
    free(insert_data);
    free(search_data);
}


/* ============================================================================
 * 6. Interactive tree display (with robust input & explicit casts)
 * ============================================================================ */
void run_tree_display() {
    int n;
    printf("Enter number of elements: ");
    scanf("%d", &n);
    if (n <= 0) {
        printf("Number must be positive.\n");
        return;
    }

    double* arr = (double*)safe_malloc(n * sizeof(double));   // Fixed: added (double*) cast

    // Clear the newline left by scanf
    while (getchar() != '\n');

    printf("Enter %d real numbers (you may use spaces, commas, or newlines):\n", n);
    char line[4096];
    if (!fgets(line, sizeof(line), stdin)) {
        printf("Input error.\n");
        free(arr);
        return;
    }

    // Replace commas with spaces to help sscanf / strtod
    for (char* p = line; *p; p++)
        if (*p == ',') *p = ' ';

    // Parse the numbers manually
    int count = 0;
    char* ptr = line;
    while (count < n) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;
        char* endptr;
        double val = strtod(ptr, &endptr);
        if (endptr == ptr) break; // no conversion
        arr[count++] = val;
        ptr = endptr;
    }

    if (count < n) {
        printf("Warning: only %d numbers read, filling remaining with 0.0.\n", count);
        for (int i = count; i < n; i++) arr[i] = 0.0;
    }

    printf("\n========== Building trees and printing structures ==========\n");

    /* BST */
    BSTNode* bst_root = NULL;
    for (int i = 0; i < n; i++) bst_root = bst_insert(bst_root, arr[i]);
    print_bst_structure(bst_root);
    bst_free(bst_root);

    /* AVL */
    AVLNode* avl_root = NULL;
    for (int i = 0; i < n; i++) avl_root = avl_insert(avl_root, arr[i]);
    print_avl_structure(avl_root);
    avl_free(avl_root);

    /* Red-Black */
    RBNode* rb_root = NIL;
    for (int i = 0; i < n; i++) rb_insert(&rb_root, arr[i]);
    print_rb_structure(rb_root);
    rb_free(rb_root);

    /* B-Tree */
    BTreeNode* btree_root = NULL;
    for (int i = 0; i < n; i++) btree_root = btree_insert(btree_root, arr[i]);
    print_btree_structure(btree_root);
    btree_free(btree_root);

    free(arr);
    printf("========================================================\n");
}


/* ============================================================================
 * Main: mode selection
 * ============================================================================ */
int main() {
    int mode;
    printf("====================================================\n");
    printf("  Balanced Tree Experiment\n");
    printf("  1 - Performance test (random large data)\n");
    printf("  2 - Tree structure output (enter an array)\n");
    printf("====================================================\n");
    printf("Choose mode (1 or 2): ");
    scanf("%d", &mode);

    if (mode == 1) {
        run_performance_test();
    } else if (mode == 2) {
        run_tree_display();
    } else {
        printf("Invalid choice. Exiting.\n");
    }
    return 0;
}