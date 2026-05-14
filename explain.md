## 整体结构

代码包含以下主要部分：

- **0. 跨平台高精度计时器与辅助功能**  
- **1. 二叉搜索树 (BST)**  
- **2. AVL 树**  
- **3. 红黑树**  
- **4. 5阶 B 树**  
- **5. 性能测试 (随机大数据比较时间)**  
- **6. 交互式树结构输出 (输入数组打印节点-父节点关系)**  
- **主函数 (模式选择)**

所有动态内存分配都使用 `safe_malloc` 统一处理，出错时立即退出程序。代码兼容 C 和 C++ 编译器（显式转换 `void*`）。

---

## 0. 跨平台高精度计时器与辅助功能

### 计时器 `get_time_in_seconds()`
- Windows 下使用 `QueryPerformanceFrequency` 和 `QueryPerformanceCounter` 获取高精度计时，返回秒数（`double`）。
- Linux/macOS 等使用 `clock_gettime(CLOCK_MONOTONIC)`，保证单调递增，不受系统时间修改影响。
- 精度达纳秒级，乘以 `1000.0` 后得到毫秒数。

### `safe_malloc(size_t size)`
- 调用 `malloc`，检查失败则打印错误并 `exit(EXIT_FAILURE)`。

### `swap_double(double* a, double* b)`
- 交换两个 `double` 变量的值，用于后续洗牌算法。

### `shuffle_array(double* arr, int n)`
- Fisher‑Yates 洗牌算法，将数组随机打乱。
- 用于生成随机搜索/删除的键序列。

---

## 1. 二叉搜索树 (BST)

### 节点结构 `BSTNode`
```c
typedef struct BSTNode {
    double key;
    struct BSTNode *left, *right;
} BSTNode;
```
- 每个节点包含一个键值和左右子指针。没有父指针。

### 创建节点 `create_bst_node(double key)`
- 分配内存，初始化键值，左右子置 `NULL`。

### 插入 `bst_insert(BSTNode* root, double key)`
- 递归实现：
  - 若当前根为 `NULL`，创建新节点返回。
  - 若 `key < root->key`，递归插入左子树。
  - 若 `key > root->key`，递归插入右子树。
  - 相等时忽略（不允许重复）。
- 返回新的子树根节点（`root` 不变，但可能重新链接）。

### 搜索 `bst_search(BSTNode* root, double key)`
- 递归：
  - 根为 `NULL` 或键相等时返回当前节点。
  - 否则根据大小递归左或右子树。

### 查找最小值 `bst_find_min(BSTNode* root)`
- 沿左子树一直走到底，返回最左节点。

### 删除 `bst_delete(BSTNode* root, double key)`
1. 递归查找目标节点：
   - 若 `key < root->key`，递归左删。
   - 若 `key > root->key`，递归右删。
2. 找到节点后分三种情况：
   - **无左子**：用右子替代，释放当前节点。
   - **无右子**：用左子替代，释放当前节点。
   - **有两个子节点**：找到右子树的最小节点（后继），用后继的键值覆盖当前节点，然后递归删除那个后继节点。
3. 返回新的子树根（可能被替换）。

### 释放整棵树 `bst_free`
- 后序遍历释放左右子树后释放根。

### 结构打印 `print_bst_structure`
- 使用递归辅助函数 `print_bst_subtree`，传入父节点键值和一个标志 `is_root`。
- 对于每个非空节点，打印 `Node: key, Parent: parent_key`（根节点显示 `NULL (root)`）。
- 遍历顺序为根→左→右，保证输出有序但不需要按层。用于直观展示父子关系。

---

## 2. AVL 树

### 节点结构 `AVLNode`
```c
typedef struct AVLNode {
    double key;
    int height;
    struct AVLNode *left, *right;
} AVLNode;
```
- 比 BST 多一个 `height` 字段，记录以该节点为根的子树高度。

### 辅助函数
- `avl_height(AVLNode* node)`：返回节点高度，`NULL` 视为 0。
- `avl_balance(AVLNode* node)`：左子树高度减右子树高度，得到平衡因子。
- `max_int(int a, int b)`：返回较大值。

### 旋转操作
- **右旋 `avl_right_rotate(AVLNode* y)`**  
  以 `y` 为轴，将其左子 `x` 提升为新的根。`x` 的原右子树 `T2` 挂到 `y` 的左子。更新 `y` 和 `x` 的高度。
- **左旋 `avl_left_rotate(AVLNode* x)`**  
  对称操作。

### 插入 `avl_insert(AVLNode* root, double key)`
1. 标准 BST 递归插入。
2. 从插入点向上回溯，更新每个节点的高度。
3. 计算平衡因子 `balance`。
4. 四种失衡情况及旋转调整：
   - **左左 (LL)**：`balance > 1 && key < root->left->key` → 右旋。
   - **右右 (RR)**：`balance < -1 && key > root->right->key` → 左旋。
   - **左右 (LR)**：`balance > 1 && key > root->left->key` → 先左旋左子树，再右旋。
   - **右左 (RL)**：`balance < -1 && key < root->right->key` → 先右旋右子树，再左旋。
5. 返回旋转后的新根。

### 搜索 `avl_search`
- 同 BST 递归搜索。

### 删除 `avl_delete(AVLNode* root, double key)`
1. 递归查找节点，若找到，分三种情况处理：
   - 若左或右子树为空：用非空子树替换（或直接删除叶子），`temp = root->left ? root->left : root->right`；若 `temp` 为空则直接删，否则用 `temp` 的内容复制到 `root`（`*root = *temp`），然后释放 `temp`。
   - 若有两个子节点：取右子树最小值，替换键值，递归删除该后继节点。
2. 若删除后根为 `NULL`，直接返回。
3. 更新高度，计算平衡因子。
4. 根据平衡因子和子树的平衡因子进行旋转，与插入时的平衡调整类似，但使用子树的平衡因子判断：
   - 左左/左斜：`balance > 1 && avl_balance(root->left) >= 0` → 右旋。
   - 左右：`balance > 1 && avl_balance(root->left) < 0` → 先左旋左子树，再右旋。
   - 右右/右斜：`balance < -1 && avl_balance(root->right) <= 0` → 左旋。
   - 右左：`balance < -1 && avl_balance(root->right) > 0` → 先右旋右子树，再左旋。
5. 返回调整后的根。

### 释放与结构打印
- `avl_free` 后序释放。
- `print_avl_structure` 类似于 BST，递归输出节点及父节点键值。

---

## 3. 红黑树 (Red‑Black Tree)

### 颜色枚举与节点结构
```c
typedef enum { RED, BLACK } NodeColor;
typedef struct RBNode {
    double key;
    NodeColor color;
    struct RBNode *left, *right, *parent;
} RBNode;
```
- 节点包含父指针，便于实现旋转和修复。
- 全局哨兵节点 `NIL_NODE` 表示叶子（黑色），所有空指针都指向 `NIL`。

### 初始化
- `NIL` 是预定义的黑色节点，其左右父均为 `NULL`（但实际使用中不关心），用作共享的叶子哨兵。

### 旋转
- `rb_left_rotate(RBNode** root, RBNode* x)`
  1. 记录 `y = x->right`。
  2. 将 `y` 的左子移到 `x` 的右子。
  3. 若 `y->left != NIL`，更新该子的父指针。
  4. 将 `y` 的父指针设为 `x` 的父指针；若 `x` 是根，则更新 `*root` 为 `y`；否则根据 `x` 是其父的左或右子，将相应指针指向 `y`。
  5. 将 `x` 挂到 `y` 的左子，更新 `x` 的父指针。

- `rb_right_rotate` 对称。

### 插入 `rb_insert(RBNode** root, double key)`
1. 创建红色新节点 `z`。
2. 像 BST 一样找到插入位置（`y` 为父节点），若键已存在则直接释放并返回。
3. 将 `z` 的父亲设为 `y`；若 `y == NIL`，则 `z` 为根，直接染黑返回。
4. 若 `z` 的父亲是黑色或父亲是根，直接返回（无需调整）。
5. 否则调用 `rb_insert_fixup` 修复红黑性质。

### 插入修复 `rb_insert_fixup(RBNode** root, RBNode* z)`
- 循环条件：`z->parent->color == RED`（违反了不能连续两个红色的性质）。
- 分两种情况：父节点是祖父的左子或右子（对称），核心处理方式相同：
  - 先看叔叔 `y` 的颜色。
  - **叔叔为红**：父亲和叔叔染黑，祖父染红，将 `z` 上移至祖父继续循环。
  - **叔叔为黑**：
    - 若 `z` 是其父的右子（左斜情况），则以父亲为轴左旋（或右旋），使 `z` 与父亲换位。
    - 然后父亲染黑，祖父染红，并绕祖父进行右旋（或左旋）。
- 循环结束后，将根染黑。

### 搜索 `rb_search`
- 迭代实现，沿着二叉树查找，直到找到键或遇到 `NIL`。

### 删除 `rb_delete(RBNode** root, double key)`
1. 找到待删节点 `z`（若不存在直接返回）。
2. 记录实际被删除或被移动的节点 `y` 及其原始颜色 `y_original_color`。
3. 根据 `z` 的子节点情况：
   - 若 `z` 无左子：用右子 `x` 替换 `z`。
   - 若 `z` 无右子：用左子 `x` 替换 `z`。
   - 若 `z` 有两个子节点：找到后继 `y`（右子树最小值），记录其原始颜色；将 `y` 的右子 `x` 提升到 `y` 的位置（用移植操作）；然后将 `y` 放到 `z` 的位置，继承 `z` 的颜色，`y` 的左子指向 `z` 的左子。
4. 释放 `z`。
5. 若被移除的原始颜色是 `BLACK`，则可能违反黑高性质，调用 `rb_delete_fixup(root, x)` 修复。

### 移植操作 `rb_transplant`
- 用子树 `v` 替换以 `u` 为根的子树，更新父指针和父节点的子指针。

### 删除修复 `rb_delete_fixup(RBNode** root, RBNode* x)`
- `x` 是替代被删除节点的节点（可能为双重黑色）。
- 循环直到 `x` 成为根或 `x` 为红色（此时直接染黑即可）。
- 若 `x` 是其父的左子：
  - 取兄弟 `w`。
  - **情况1**：`w` 红色 → 将 `w` 染黑，父染红，左旋父节点，更新 `w` 为新的兄弟。
  - **情况2**：`w` 两子皆黑 → 将 `w` 染红，`x` 上移到父节点继续。
  - **情况3**：`w` 右子黑 → 将 `w` 左子染黑，`w` 染红，右旋 `w`，更新 `w` 为 `x` 的兄弟。
  - **情况4**：`w` 右子红 → 将 `w` 颜色设为父颜色，父染黑，`w` 右子染黑，左旋父节点，然后 `x = *root` 结束循环。
- 对称处理 `x` 为右子的情况。
- 循环结束后将 `x` 染黑。

### 释放
- `rb_free` 递归遍历，但跳过 `NIL`，只释放实际分配的节点。

### 结构打印
- 利用父指针直接输出每个节点的键值、颜色和父节点信息。

---

## 4. 5阶 B 树 (Order‑5 B‑Tree)

### 参数定义
- `M_ORDER = 5`，每个节点最多 `MAX_KEYS = 4` 个键，最少 `MIN_KEYS = 2`（分裂时用到）。实际代码中分裂逻辑保证最少键数。

### 节点结构 `BTreeNode`
```c
typedef struct BTreeNode {
    int num_keys;
    double keys[MAX_KEYS + 1];         // 多一个位置用于分裂时的暂存
    struct BTreeNode* children[M_ORDER + 1];
    bool is_leaf;
} BTreeNode;
```
- `num_keys`：当前节点包含的键数目。
- `keys`：数组长度比最大键数多 1，以便在分裂前暂时容纳溢出的键。
- `children`：指针数组，长度为 `M_ORDER + 1`，非叶节点的孩子数比键数多 1。

### 创建节点 `create_btree_node(bool is_leaf)`
- 分配内存，初始化键数为 0，子指针全为 `NULL`，设置 `is_leaf`。

### 搜索 `btree_search(BTreeNode* root, double key)`
- 在节点内顺序查找（线性扫描），找到第一个 `keys[i] >= key` 的位置。
- 若匹配直接返回 `true`；若是叶子则返回 `false`；否则递归到对应的子节点。

### 分裂子节点 `btree_split_child(BTreeNode* parent, int i, BTreeNode* full_child)`
- 用于分裂一个已满的子节点（键数 = `MAX_KEYS`）。
- 分裂点 `mid = MAX_KEYS / 2 = 2`（索引 2），左半部分保留在 `full_child` 中（键数变为 `mid`）。
- 新建一个节点 `new_node` 存放右半部分的键（`MAX_KEYS - mid - 1 = 1` 个键）。
- 若 `full_child` 不是叶节点，还需将对应的孩子指针复制到 `new_node`。
- 在父节点 `parent` 中，将 `new_node` 插入到 `children[i+1]`，将中间的键 `full_child->keys[mid]` 提升到父节点的 `keys[i]` 处，父节点的键数加 1。

### 插入非满节点 `btree_insert_non_full(BTreeNode* node, double key)`
- 若节点是叶子：找到插入位置，将键后移，插入并增加键数。
- 若节点是非叶节点：找到应插入的子节点索引 `i`；若该子节点已满，先分裂，然后再递归插入到合适的子节点（分裂后可能需要调整索引 `i`）。
- 最终调用自身递归。

### 插入主入口 `btree_insert(BTreeNode* root, double key)`
- 若根为空，创建新叶子节点插入。
- 若根已满，创建新根（非叶节点），将原根作为第一个孩子，并分裂原根；然后调用 `btree_insert_non_full` 插入键。
- 否则直接调用 `btree_insert_non_full`。

### 查找键在节点中的位置 `btree_find_key(BTreeNode* node, double key)`
- 返回第一个大于等于 `key` 的索引（若存在相等键，则返回其位置；否则返回可插入的子节点索引）。

### 删除 `btree_delete(BTreeNode* root, double key)` (简化实现)
- 此版本只实现了基本的删除逻辑，未处理节点下溢合并，仅作为性能测试使用（但提供的结构输出模式只使用插入，不影响）。
- 找到键的位置 `idx`：
  - 若在当前节点：若是叶子，直接删除（移动后序键）；否则用后继（右子树最左键）替换，然后递归删除后继。
  - 否则，若为非叶节点，递归到相应子节点删除。
- 返回根指针（可能不变）。

### 释放 `btree_free`
- 递归释放所有子节点后释放自身。

### 结构打印
- 每个节点打印其键列表，以及父节点的键列表（父为 NULL 时显示 root）。
- 通过递归遍历所有子节点。

---

## 5. 性能测试 `run_performance_test()`

- 设定 `N=100000`（插入数量）和 `M=50000`（搜索/删除数量）。
- 固定随机种子 `srand(12345)` 保证可重复性。
- 生成 `insert_data` 数组（0～100万的浮点数），并复制前 `M` 个到 `search_data`，然后打乱 `search_data` 用于搜索和删除。
- 分别对四种树进行相同的操作并计时：
  - 插入：循环插入 `N` 个键。
  - 搜索：循环搜索 `M` 个键。
  - 删除：循环删除 `M` 个键。
- 每次操作前后用 `get_time_in_seconds()` 计时，转换为毫秒并打印。
- 测试后释放所有树的节点及数据数组。

---

## 6. 交互式树结构输出 `run_tree_display()`

- 提示用户输入元素个数 `n`。
- 分配 `double` 数组 `arr`。
- 利用 `fgets` 读取整行输入，将逗号替换为空格，然后使用 `strtod` 逐个解析实数，直到读够 `n` 个数（不足则用 0 填充并警告）。
- 依次构建四种树（BST、AVL、红黑、B 树），分别调用相应的结构打印函数输出父子关系。
- 释放数组和树节点。

---

## 7. 主函数 `main()`

- 打印菜单，提示用户选择模式：
  - **1**：运行性能测试。
  - **2**：运行树结构输出。
- 读取选择并调用对应函数。若输入无效，提示退出。

---

## 关键逻辑总结

- **BST**：朴素二叉搜索树，操作时间复杂度取决于树高，最差 O(n)，但提供了基线。
- **AVL**：通过维护高度和旋转，保证严格平衡，高度约 1.44 log₂n，插入/删除均为 O(log n)。
- **红黑树**：使用颜色和旋转维护近似平衡，旋转次数较 AVL 少，但平衡性稍弱，高度 ≤ 2 log₂(n+1)，插入/删除也是 O(log n)。
- **B 树**：一个节点可容纳多个键，降低树高，特别适合磁盘等大块数据场景，5 阶 B 树每个节点最多 4 键，高扇出，搜索、插入、删除均为 O(log n)。

所有树的结构输出采用“节点→父节点”的格式，尤其 B 树打印了每个节点的键块和父节点键块，便于手工绘制树形图。性能测试使用高精度计时器，并以毫秒为单位输出结果，可直观对比不同平衡树的性能差异。