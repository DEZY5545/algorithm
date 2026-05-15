#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    node->left = node->right = NULL;    //根节点初始化
    return node;
}

BSTNode* bst_insert(BSTNode* root, double key) {
    if (root == NULL) return create_bst_node(key);    //空树则建立根节点
    if (key < root->key)
        root->left = bst_insert(root->left, key);
    else if (key > root->key)
        root->right = bst_insert(root->right, key);    //左右子树嵌套插入
    return root;
}

BSTNode* bst_search(BSTNode* root, double key) {
    if (root == NULL || root->key == key) return root;      //空树或找到则返回root
    if (key < root->key) return bst_search(root->left, key);
    return bst_search(root->right, key);        //左右子树嵌套查找
}

BSTNode* bst_find_min(BSTNode* root) {
    while (root && root->left) root = root->left;       //循环寻找左子树
    return root;
}

BSTNode* bst_delete(BSTNode* root, double key) {
    if (root == NULL) return NULL;      //空树返回NULL
    if (key < root->key) {
        root->left = bst_delete(root->left, key);
    } else if (key > root->key) {
        root->right = bst_delete(root->right, key);     //左右子树嵌套删除
    } else {        //需要删除root
        if (root->left == NULL) {       //左子树空则上移右子树，free（root）
            BSTNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {       //右子树空则上移左子树，free（root）
            BSTNode* temp = root->left;
            free(root);
            return temp;
        }
        BSTNode* temp = bst_find_min(root->right);      //都有则找右子树的min，更新为root并删除min
        root->key = temp->key;
        root->right = bst_delete(root->right, temp->key);
    }
    return root;
}

void bst_free(BSTNode* root) {
    if (root) {
        bst_free(root->left);
        bst_free(root->right);      //嵌套释放左右子树
        free(root);     //释放根节点
    }
}

static void print_bst_subtree(BSTNode* node, double parent_key, int is_root) {
    if (!node) return;
    if (is_root)
        printf("  [BST] Node: %.2f, Parent: NULL (root)\n", node->key);
    else
        printf("  [BST] Node: %.2f, Parent: %.2f\n", node->key, parent_key);
    print_bst_subtree(node->left,  node->key, 0);
    print_bst_subtree(node->right, node->key, 0);       //嵌套打印左右子树
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
int avl_balance(AVLNode* n) { return n ? avl_height(n->left) - avl_height(n->right) : 0; }      //左height-右height
int max_int(int a, int b) { return a > b ? a : b; }

AVLNode* create_avl_node(double key) {
    AVLNode* node = (AVLNode*)safe_malloc(sizeof(AVLNode));
    node->key = key;
    node->height = 1;
    node->left = node->right = NULL;        //初始化根节点
    return node;
}

AVLNode* avl_right_rotate(AVLNode* y) {     //找y的左子树和左子树的右，更新参数
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

AVLNode* avl_left_rotate(AVLNode* x) {      //找x的右子树和右子树的左，更新参数
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

AVLNode* avl_insert(AVLNode* root, double key) {
    if (!root) return create_avl_node(key);     //空树则插入
    if (key < root->key)
        root->left = avl_insert(root->left, key);
    else if (key > root->key)
        root->right = avl_insert(root->right, key);     //左右子树嵌套插入
    else return root;

    root->height = 1 + max_int(avl_height(root->left), avl_height(root->right));
    int bal = avl_balance(root);        //更新height后算平衡因子

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
#define M 5
#define MAX_KEYS (M - 1)          //4
#define MIN_KEYS ((M + 1) / 2 - 1) //2

typedef struct BTreeNode {
    int num_keys;
    double keys[M];               //extra slot for temporary overflow during split
    struct BTreeNode* children[M + 1];
    bool is_leaf;
} BTreeNode;

BTreeNode* create_btree_node(bool is_leaf) {
    BTreeNode* node = (BTreeNode*)safe_malloc(sizeof(BTreeNode));
    node->num_keys = 0;
    node->is_leaf = is_leaf;
    for (int i = 0; i <= M; i++) node->children[i] = NULL;      //初始化子节点
    return node;
}

bool btree_search(BTreeNode* root, double key) {
    if (!root) return false;        //空树返回false
    int i = 0;
    while (i < root->num_keys && key > root->keys[i]) i++;
    if (i < root->num_keys && key == root->keys[i]) return true;        //若没遍历完就找到值就返回true
    if (root->is_leaf) return false;        //遍历到叶子了还没找到返回false
    return btree_search(root->children[i], key);        //子节点嵌套查找
}

/* ---------- Insertion---------- */
static bool btree_insert_into_leaf(BTreeNode* leaf, double key) {       //插入叶子节点
    int i = leaf->num_keys - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i + 1] = leaf->keys[i];
        i--;
    }
    if (i >= 0 && leaf->keys[i] == key) return false;   //发现相同值，重复插入，return false
    leaf->keys[i + 1] = key;
    leaf->num_keys++;       //插入并更新参数
    return leaf->num_keys > MAX_KEYS;       //检查是否合法
}

static BTreeNode* btree_split_node(BTreeNode* node, double* promoted_key) {
    int mid = (node->num_keys - 1) / 2;     //中位数
    *promoted_key = node->keys[mid];

    BTreeNode* right = create_btree_node(node->is_leaf);
    right->num_keys = node->num_keys - mid - 1;
    for (int i = 0; i < right->num_keys; i++)       //分裂参数
        right->keys[i] = node->keys[mid + 1 + i];
    if (!node->is_leaf) {       //若node不是叶子，则还要分裂子节点
        for (int i = 0; i <= right->num_keys; i++)
            right->children[i] = node->children[mid + 1 + i];
    }
    node->num_keys = mid;       //更新node参数
    return right;       //返回建好的节点
}

static BTreeNode* btree_insert_rec(BTreeNode* node, double key,
                                   bool* split, double* promoted_key, BTreeNode** promoted_child) {
    if (node->is_leaf) {
        bool overflow = btree_insert_into_leaf(node, key);
        if (!overflow) {        //合法返回即可
            *split = false;
            return node;
        }
        *split = true;      //不合法确定要分裂
        *promoted_child = btree_split_node(node, promoted_key);
        return node;
    } else {
        int i = 0;
        while (i < node->num_keys && key > node->keys[i]) i++;
        if (i < node->num_keys && key == node->keys[i]) {
            *split = false;
            return node;   //发现重复，忽视即可
        }
        BTreeNode* child = node->children[i];
        bool child_split;
        double child_promoted_key;
        BTreeNode* child_promoted_child;
        btree_insert_rec(child, key, &child_split, &child_promoted_key, &child_promoted_child);
        //嵌套子节点插入
        if (!child_split) {     //合法返回即可
            *split = false;
            return node;
        }
        for (int j = node->num_keys; j > i; j--)
            node->keys[j] = node->keys[j - 1];
        node->keys[i] = child_promoted_key;
        node->num_keys++;
        for (int j = node->num_keys; j > i + 1; j--)
            node->children[j] = node->children[j - 1];
        node->children[i + 1] = child_promoted_child;
        //更新node的键和子节点
        if (node->num_keys <= MAX_KEYS) {       //合法返回node即可
            *split = false;
            return node;
        }
        *split = true;
        *promoted_child = btree_split_node(node, promoted_key);
        return node;        //不合法就拿到分裂的字节点传给上一层嵌套
    }
}

BTreeNode* btree_insert(BTreeNode* root, double key) {
    if (!root) {        //空树初始化根节点
        root = create_btree_node(true);
        root->keys[0] = key;
        root->num_keys = 1;
        return root;
    }

    bool split;
    double promoted_key;
    BTreeNode* promoted_child;
    root = btree_insert_rec(root, key, &split, &promoted_key, &promoted_child);

    if (split) {        //如果需要分裂
        BTreeNode* new_root = create_btree_node(false);
        new_root->keys[0] = promoted_key;
        new_root->num_keys = 1;
        new_root->children[0] = root;
        new_root->children[1] = promoted_child;
        return new_root;
    }
    return root;
}

/* ---------- Deletion helpers ---------- */
static int btree_find_key(BTreeNode* node, double key) {
    int idx = 0;
    while (idx < node->num_keys && node->keys[idx] < key) idx++;
    return idx;
}       //找到并返回键

static void btree_remove_from_leaf(BTreeNode* node, int idx) {
    for (int i = idx + 1; i < node->num_keys; i++)
        node->keys[i - 1] = node->keys[i];
    node->num_keys--;
}       //删除叶子上的对应值

static double btree_get_predecessor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx];
    while (!cur->is_leaf) cur = cur->children[cur->num_keys];
    return cur->keys[cur->num_keys - 1];
}

static double btree_get_successor(BTreeNode* node, int idx) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->is_leaf) cur = cur->children[0];
    return cur->keys[0];
}

static void btree_borrow_from_left(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* left_sibling = node->children[idx - 1];

    for (int i = child->num_keys; i > 0; i--)
        child->keys[i] = child->keys[i - 1];
    child->num_keys++;
    child->keys[0] = node->keys[idx - 1];
    node->keys[idx - 1] = left_sibling->keys[left_sibling->num_keys - 1];

    if (!child->is_leaf) {
        for (int i = child->num_keys; i > 0; i--)
            child->children[i] = child->children[i - 1];
        child->children[0] = left_sibling->children[left_sibling->num_keys];
    }
    left_sibling->num_keys--;
}

static void btree_borrow_from_right(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* right_sibling = node->children[idx + 1];

    child->keys[child->num_keys] = node->keys[idx];
    child->num_keys++;
    node->keys[idx] = right_sibling->keys[0];

    if (!child->is_leaf) {
        child->children[child->num_keys] = right_sibling->children[0];
    }

    for (int i = 1; i < right_sibling->num_keys; i++)
        right_sibling->keys[i - 1] = right_sibling->keys[i];
    if (!right_sibling->is_leaf) {
        for (int i = 1; i <= right_sibling->num_keys; i++)
            right_sibling->children[i - 1] = right_sibling->children[i];
    }
    right_sibling->num_keys--;
}

static BTreeNode* btree_merge(BTreeNode* node, int idx) {
    BTreeNode* left_child = node->children[idx];
    BTreeNode* right_child = node->children[idx + 1];

    left_child->keys[left_child->num_keys] = node->keys[idx];
    left_child->num_keys++;

    for (int i = 0; i < right_child->num_keys; i++)
        left_child->keys[left_child->num_keys + i] = right_child->keys[i];
    if (!left_child->is_leaf) {
        for (int i = 0; i <= right_child->num_keys; i++)
            left_child->children[left_child->num_keys + i] = right_child->children[i];
    }
    left_child->num_keys += right_child->num_keys;


    for (int i = idx; i < node->num_keys - 1; i++)
        node->keys[i] = node->keys[i + 1];
    for (int i = idx + 1; i < node->num_keys; i++)
        node->children[i] = node->children[i + 1];
    node->num_keys--;

    free(right_child);

    if (left_child->num_keys > MAX_KEYS) {
        double promoted_key;
        BTreeNode* new_right = btree_split_node(left_child, &promoted_key);
        for (int i = node->num_keys; i > idx; i--)
            node->keys[i] = node->keys[i - 1];
        node->keys[idx] = promoted_key;
        node->num_keys++;
        for (int i = node->num_keys; i > idx + 1; i--)
            node->children[i] = node->children[i - 1];
        node->children[idx + 1] = new_right;
    }
    return left_child;
}

static void btree_fill(BTreeNode* node, int idx) {
    if (idx > 0 && node->children[idx - 1]->num_keys >= MIN_KEYS + 1)
        btree_borrow_from_left(node, idx);
    else if (idx < node->num_keys && node->children[idx + 1]->num_keys >= MIN_KEYS + 1)
        btree_borrow_from_right(node, idx);
    else {
        if (idx > 0)
            btree_merge(node, idx - 1);
        else
            btree_merge(node, idx);
    }
}

/* ---------- Main deletion function (bottom‑up recursive) ---------- */
BTreeNode* btree_delete(BTreeNode* root, double key) {
    if (!root) return NULL;

    int idx = btree_find_key(root, key);

    if (idx < root->num_keys && root->keys[idx] == key) {
        if (root->is_leaf) {
            btree_remove_from_leaf(root, idx);
        } else {
            if (root->children[idx]->num_keys >= MIN_KEYS + 1) {
                double pred = btree_get_predecessor(root, idx);
                root->keys[idx] = pred;
                root->children[idx] = btree_delete(root->children[idx], pred);
            } else if (root->children[idx + 1]->num_keys >= MIN_KEYS + 1) {
                double succ = btree_get_successor(root, idx);
                root->keys[idx] = succ;
                root->children[idx + 1] = btree_delete(root->children[idx + 1], succ);
            } else {
                btree_merge(root, idx);
                root->children[idx] = btree_delete(root->children[idx], key);
            }
        }
    } else {
        if (root->is_leaf) return root;
        if (root->children[idx]->num_keys == MIN_KEYS) {
            btree_fill(root, idx);
        }
        if (root->num_keys == 0) {
            BTreeNode* new_root = root->children[0];
            free(root);
            return btree_delete(new_root, key);
        }

        idx = btree_find_key(root, key);
        root->children[idx] = btree_delete(root->children[idx], key);
    }

    if (root->num_keys == 0 && !root->is_leaf) {
        BTreeNode* new_root = root->children[0];
        free(root);
        return new_root;
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
 * 5. Performance test
 * ============================================================================ */
void run_performance_test() {
    int N = 4000, L = 2000;
    printf("=================================================================\n");
    printf("   Balanced Trees Performance Comparison (N=%d, L=%d)\n", N, L);
    printf("   Unit: Milliseconds (ms)\n");
    printf("=================================================================\n");
    printf("%-10s | %-15s | %-15s | %-15s\n", "Tree Type", "Insert(ms)", "Search(ms)", "Delete(ms)");
    printf("-----------------------------------------------------------------\n");

    srand(12345);
    double* insert_data = (double*)safe_malloc(N * sizeof(double));
    for (int i = 0; i < N; i++)
        insert_data[i] = ((double)rand() / RAND_MAX) * 1000000.0;

    double* search_data = (double*)safe_malloc(L * sizeof(double));
    for (int i = 0; i < L; i++) search_data[i] = insert_data[i];
    shuffle_array(search_data, L);

    double start, end, time_ins, time_search, time_del;

    /* BST */
    BSTNode* bst_root = NULL;
    start = get_time_in_seconds();
    for (int i = 0; i < N; i++) bst_root = bst_insert(bst_root, insert_data[i]);
    end = get_time_in_seconds();
    time_ins = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < L; i++) bst_search(bst_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < L; i++) bst_root = bst_delete(bst_root, search_data[i]);
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
    for (int i = 0; i < L; i++) avl_search(avl_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < L; i++) avl_root = avl_delete(avl_root, search_data[i]);
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
    for (int i = 0; i < L; i++) rb_search(rb_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < L; i++) rb_delete(&rb_root, search_data[i]);
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
    for (int i = 0; i < L; i++) btree_search(btree_root, search_data[i]);
    end = get_time_in_seconds();
    time_search = (end - start) * 1000.0;

    start = get_time_in_seconds();
    for (int i = 0; i < L; i++) btree_root = btree_delete(btree_root, search_data[i]);
    end = get_time_in_seconds();
    time_del = (end - start) * 1000.0;
    printf("%-10s | %-15.6f | %-15.6f | %-15.6f\n", "5-B-Tree", time_ins, time_search, time_del);
    btree_free(btree_root);

    printf("=================================================================\n");
    free(insert_data);
    free(search_data);
}

/* ============================================================================
 * 6. Interactive tree operations (mode 2)
 * ============================================================================ */
void run_tree_operations() {
    int n;
    printf("Enter number of initial elements: ");
    scanf("%d", &n);
    if (n <= 0) {
        printf("Number must be positive.\n");
        return;
    }

    double* arr = (double*)safe_malloc(n * sizeof(double));
    while (getchar() != '\n');

    printf("Enter %d real numbers:\n", n);
    char line[4096];
    if (!fgets(line, sizeof(line), stdin)) {
        printf("Input error.\n");
        free(arr);
        return;
    }

    for (char* p = line; *p; p++)
        if (*p == ',') *p = ' ';

    int count = 0;
    char* ptr = line;
    while (count < n) {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;
        char* endptr;
        double val = strtod(ptr, &endptr);
        if (endptr == ptr) break;
        arr[count++] = val;
        ptr = endptr;
    }
    if (count < n) {
        printf("Warning: only %d numbers read, filling rest with 0.0.\n", count);
        for (int i = count; i < n; i++) arr[i] = 0.0;
    }

    BSTNode* bst_root = NULL;
    AVLNode* avl_root = NULL;
    RBNode* rb_root = NIL;
    BTreeNode* btree_root = NULL;
    for (int i = 0; i < n; i++) {
        bst_root = bst_insert(bst_root, arr[i]);
        avl_root = avl_insert(avl_root, arr[i]);
        rb_insert(&rb_root, arr[i]);
        btree_root = btree_insert(btree_root, arr[i]);
    }
    free(arr);

    int choice;
    double key;
    do {
        printf("\n=== Interactive Tree Operations ===\n");
        printf("1. Insert a key into all trees\n");
        printf("2. Delete a key from all trees\n");
        printf("3. Print current structures of all trees\n");
        printf("4. Exit to main menu\n");
        printf("Choose: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("Enter key to insert: ");
            scanf("%lf", &key);
            bst_root = bst_insert(bst_root, key);
            avl_root = avl_insert(avl_root, key);
            rb_insert(&rb_root, key);
            btree_root = btree_insert(btree_root, key);
            printf("Key %.2f inserted.\n", key);
            break;
        case 2:
            printf("Enter key to delete: ");
            scanf("%lf", &key);
            bst_root = bst_delete(bst_root, key);
            avl_root = avl_delete(avl_root, key);
            rb_delete(&rb_root, key);
            btree_root = btree_delete(btree_root, key);
            printf("Key %.2f deleted (if existed).\n", key);
            break;
        case 3:
            print_bst_structure(bst_root);
            print_avl_structure(avl_root);
            print_rb_structure(rb_root);
            print_btree_structure(btree_root);
            break;
        case 4:
            printf("Returning to main menu.\n");
            break;
        default:
            printf("Invalid option.\n");
        }
    } while (choice != 4);

    bst_free(bst_root);
    avl_free(avl_root);
    rb_free(rb_root);
    btree_free(btree_root);
}

/* ============================================================================
 * Main
 * ============================================================================ */
int main() {
    int mode;
    printf("====================================================\n");
    printf("  Balanced Tree Experiment\n");
    printf("  1 - Performance test\n");
    printf("  2 - Interactive tree operations\n");
    printf("====================================================\n");
    printf("Choose mode (1 or 2): ");
    scanf("%d", &mode);

    if (mode == 1) {
        run_performance_test();
    } else if (mode == 2) {
        run_tree_operations();
    } else {
        printf("Invalid choice. Exiting.\n");
    }
    return 0;
}