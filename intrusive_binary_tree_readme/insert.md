# `Yc::basic_intrusive_binary_tree<vptr>::insert` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::insert`

```C++
// basic_intrusive_binary_tree<vptr>
static basic_intrusive_binary_tree insert(
    edge_proxy pos, node_base& n)noexcept;

// basic_intrusive_parent_aware_binary_tree<vptr>
static basic_intrusive_parent_aware_binary_tree insert(
    edge_proxy pos, node_base& n)noexcept;
```

把 `n` 插到 `pos` 所指代的 _树位置_，并返回被换出的原子树。

- `n` 必须处于**脱管**状态；如果它已经纳管，行为未定义。
- `pos` 必须是本树的合法 _树位置_（通常来自 `root()` 或 `get_left()` / `get_right()`）；
  传入默认构造的代理或来自另一棵树的代理，行为未定义。
- `pos` 上原有的子树会被移动到返回的树中，而不是被销毁。
- 插入后 `n` 被纳管；可向上导航版本会把 `n` 的父设为 `pos` 所在的父 _二叉树_。
- **必须保留返回值**。返回的树是原子树的唯一句柄；如果丢弃返回值，临时树析构时会
  `clear()`，原子树的节点被脱管，但不会销毁。除非你手里另有节点指针 / 代理，否则
  通常无法再访问它们。

## 参数

|||
|-:|:-|
|`pos`|要插入到的 _树位置_；必须属于本树|
|`n`|要插入的 _二叉树_；必须脱管|

## 返回值

被换出的原子树。`pos` 原本为空时返回空树。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
my_binary_tree a(1), b(2), c(3);
Yc::basic_intrusive_binary_tree<> tree;

auto old = tree.insert(tree.root(), a); // old 为空树
assert(old.empty());

tree.insert(tree.root().get_left(), b);
auto old_right = tree.insert(tree.root().get_right(), c);
assert(old_right.empty());

auto sub = Yc::basic_intrusive_binary_tree<>::cut(tree.root().get_right());
assert(sub.nroot()); // c
sub.clear();
tree.clear();
```

## 备注

插入根和插入子位置都使用同一个接口：

- 插到根：`insert(tree.root(), n)`；
- 插到子位置：`insert(parent_edge, n)`；
- 保留原位置子树：使用返回值，不要先手动 [`cut`](cut.md)。

把同一棵 _二叉树_ 同时插到两个位置会破坏树形；这是未定义行为。
