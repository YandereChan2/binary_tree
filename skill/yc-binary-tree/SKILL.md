---
name: yc-binary-tree
description: 设计与使用 Yc 库的两个二叉树头文件——binary_tree.h（无父指针）与 parent_aware_binary_tree.h（带父指针）。当需要遍历或改写树、使用 edge_proxy/node_proxy、做 cut/splice/emplace/旋转、用 recur_and_write 从外部结构建树、或需要分配器感知与 std::hash 支持时使用。
---

# Yc 二叉树

`C:\code\Yc` 下两个自包含头文件，同一套"边代理"抽象的两个实现。

## 一句话区分

| | `Yc::binary_tree<T, Alloc>` | `Yc::parent_aware_binary_tree<T, Alloc>` |
|---|---|---|
| 节点内容 | `left`、`right` | `left`、`right`、**`parent`** |
| 树的根 | 自己持有裸 `root_ptr` | **哨兵 `root_node`**，其 `left` 指向真实树根 |
| 能向上走 | ✗ | ✓ `go_up()` / `get_parent()` |
| 适用 | 只需向下遍历、在意每节点少一个指针 | 需要向上、实现平衡树、需要兄弟上下文 |

## 三条核心设计思想

### 1. 一切皆"边"

树不是"节点的集合"，而是**指针槽位的图**。`edge_proxy` 封装的不是节点，而是父节点里那个 `left`/`right` **槽位**。因此树根同样拥有一条边：

- `binary_tree`：`root()` 返回指向**树自身 `root_ptr` 成员**的边（无哨兵，但槽位语义仍然统一）。
- `parent_aware_binary_tree`：`root()` 返回**哨兵 `root_node` → 真实树根**的边。

好处：插入/删除/旋转/拼接全部只在"边"这一层实现，无需为根节点写特例分支。

两种边的内部表示并不相同：

| | `binary_tree` | `parent_aware_binary_tree` |
|---|---|---|
| 边持有什么 | `ptr`——指向槽位的指针（pointer-to-pointer） | `parent` + `ch`——父节点指针 + **成员指针**（`&base1::left` / `&node_base::right`） |
| 取槽位 | `*ptr` | `static_cast<base1&>(*parent).*ch`（私有 `child()`） |
| 向上 | 不支持 | `go_up()` / `get_parent()` |

`parent_aware` 的节点刻意拆成三层基类（`empty_placeholder` → `node_base1{left}` → `node_base{right,parent}` → `node`），正是为了让"指向 `left`/`right` 的成员指针"能安全地做偏移运算，同时靠空基类优化不增加开销。

### 2. 换出来的东西会还给你

所有"占位"操作都把**被顶掉的旧子树**按值返回：

```cpp
auto displaced = t.emplace(edge, 42);   // edge 处原有的子树被切下来，作为新树返回
```

所以 `emplace` 不是"插入并返回迭代器"，而是**替换并返回旧内容**——丢弃返回值就等于销毁旧子树，接住它就能复用。`cut` / `splice` / `insert` 同理。这是本库最反直觉也最关键的一点。

### 3. go_* 原地改，get_* 返回新的

文件头注释里定死的命名约定：

- `go_left()` / `go_right()` / `go_up()` —— 改**自身**，返回 `*this`，可链式调用
- `get_left()` / `get_right()` / `get_children()` / `get_parent()` —— 返回**新代理**，自身不动

`get_children()` 返回 `std::pair<edge_proxy, edge_proxy>`。

## 代理四件套

每个头文件各提供 4 个代理类型（下文 `X` 代表对应的树类）：

- `X::edge_proxy` —— 可写边
- `X::edge_const_proxy` —— 只读边，**多数成员函数收的就是它**；`edge_proxy` 可隐式转换过来，且转换后**共享同一槽位**
- `X::node_proxy` —— 直接代表节点
- `X::node_const_proxy` —— 只读节点代理

边的公共接口：`valid()`（边自身有效）／`null()`（边所指的子节点为空）／`explicit operator bool`（两者皆满足）／`operator*`、`operator->` 取值／`get_children()`／`operator==`（**比较槽位身份，不比较值**）。

节点代理：`null()`、`explicit operator bool`、`go_left()`、`go_right()`、`get_children()`；`parent_aware` 的另有 `go_up()`、`get_parent()`。

`std::hash` 对全部 8 个代理类型都有特化（按地址哈希），可直接用作 `unordered_set`/`unordered_map` 键。

## 常用模式

### 按边导航建 BST

```cpp
template <class Tree>
bool bst_insert(Tree& t, int v)
{
    auto e = t.root();
    while (true)
    {
        if (e.null()) { t.emplace(e, v); return true; }
        if (v < *e)      e.go_left();
        else if (v > *e) e.go_right();
        else             return false;      // 重复值
    }
}
```

### 从外部树形结构建树：recur_and_write

三个访问器 + 一个句柄，句柄必须可 `bool` 转换（`false` 表示空）：

```cpp
struct Src { int v; const Src* l; const Src* r; };

const Src n20{20, nullptr, nullptr};
const Src n30{30, nullptr, nullptr};
const Src n10{10, &n20, &n30};

auto vg = [](const Src* h) { return h->v; };
auto cg = [](const Src* h) { return std::pair<const Src*, const Src*>{h->l, h->r}; };

Yc::binary_tree<int> built{vg, cg, &n10};   // 三参构造
```

`InitializeHandle` 由 `h` 推导，而实现中会把 `cg` 返回的子句柄赋回 `h`，因此**三者必须使用同一个句柄类型**——统一用 `const Src*`；若 `h` 传 `Src*` 而 `cg` 返回 `const Src*`，二者类型不匹配。

拷贝构造与拷贝赋值就是用这条路径实现的；`Yc::binary_tree_functional::get_children`（以及 `parent_aware_binary_tree_functional::get_children`）是现成的 ChildrenGetter，对 4 种代理都可用。

### 旋转

```cpp
t.left_rotate(t.root());                       // 右子提升到 p 的位置
bool ok = t.checked_right_rotate(t.root());    // 左子不存在时返回 false 且不动
```

### 交换节点

`parent_aware` 的 `swap_node` 带 `child_check` 掩码（默认 `check_all`），用于识别父**子相邻**的四种形态并做专门重连——两个被交换的节点互为父子时，必须重连父子链而不是直接换指针：

```cpp
using PA = Yc::parent_aware_binary_tree<int>;
PA::swap_node(a, a.get_left());       // 默认 check_all，正确识别相邻情形
```

`binary_tree::swap_node` 没有这层检查，它假定两个节点不相邻。

## 两实现差异速查

| 维度 | `binary_tree` | `parent_aware_binary_tree` |
|---|---|---|
| `go_up` / `get_parent` | 无 | 有 |
| `node_proxy` → `edge_proxy` | **无此转换**（只能 `get_children()`） | 有 `operator edge_proxy()` |
| 额外 API | — | `parent_of_nroot()`、`parent_of_cnroot()`、`swap_node(..., child_check)` |
| 边哈希实现 | `std::hash<decltype(para.ptr)>{}(para.ptr)` | `std::addressof(para.child())`，无效边返回 0 |
| 移动构造 | `std::exchange(root_ptr, {})` + 移动分配器 | `alloc{other.alloc}` + `swap_sub_tree` |

## 迭代而非递归

`erase()` 用 `while` 循环，`recur_and_write_impl()` 用 `goto` 手工实现尾递归——都是为了让深树不爆栈。扩展这两个文件时请保持这一风格。

## 异常安全

- `emplace`：`T` 非 nothrow 构造时用局部 RAII guard 释放已分配节点。
- `recur_and_write` / 拷贝赋值：局部 `_guard` 在异常时把已构造部分 `splice` 回原树并恢复分配器。
- 移动构造/赋值：POCMA 感知；分配器不相等时退化为逐个移动。

## 使用契约

以下是正确使用的前提条件，写代码时需要遵守：

1. **`recur_and_write` 的目标边必须为空。** 它的语义是"往空槽位里写一棵树"。要覆盖已有子树，先 `cut` 或 `erase` 把旧内容取走，再写入。
2. **先确认 `valid()` 再访问。** 默认构造的边是合法状态，可以比较、可以哈希；但它的 `valid()` 为 `false`，此时 `null()`、`operator*`、`operator->`、`go_*`、`get_*`、`get_children()` 都要经由空指针取槽位，一律是未定义行为。只有 `(bool)e == true` 的边才能访问元素或继续导航。
3. **区分 `go_*` 与 `get_*`。** `go_left()` 会改掉代理自身，`get_left()` 不会。想保留原位置就用 `get_*`。
4. **务必处理返回值。** `emplace`/`insert`/`cut`/`splice` 返回的是被替换下来的旧子树；不接住它就等于当场销毁。要实现"插入后校验、失败则回滚"，就把返回值存起来再 `splice` 回去。
5. **`parent_aware` 的 `swap_node` 保持默认 `check_all`。** 只有确认两个节点不相邻时才传 `no_check`。
6. **`edge_const_proxy` 与 `edge_proxy` 共享槽位。** 从 `edge_proxy` 转换出来的 const 代理仍指向同一槽位，通过它的导航结果与原代理一致。
7. **边代理会随结构变动失效。** 边代理指代的是一个"树位置"；旋转、`swap`、`splice` 等改变结构的操作会让节点换位置，长期持有的旧边代理可能已经指到别处。需要跨操作稳定地指向某个节点时，改用 `node_proxy` / `node_const_proxy`。

## 扩展这两个文件时

- 保持 `go_*` 改自身、`get_*` 返回新对象的命名约定。
- 保持迭代 / 尾递归风格，不要引入随树深增长的递归。
- 新代理类型或新代理操作上线时，同步补上 `operator==` 与 `std::hash` 特化，并确保二者一致（相等者必同哈希）。

完整的成员与签名清单见 `references/api.md`。
