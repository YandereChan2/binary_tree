# `Yc::basic_intrusive_binary_tree<vptr>::insert` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::insert`

```C++
static basic_intrusive_binary_tree insert(
    edge_proxy pos, node_base& n)noexcept;
```

```C++
static basic_intrusive_parent_aware_binary_tree insert(
    edge_proxy pos, node_base& n)noexcept;
```

把 `n` 插到 `pos` 所指代的 _树位置_，并返回被换出的原子树。

- `n` 必须处于**脱管**状态；如果它已经纳管，行为未定义。
- `pos` 上原有的子树会被移动到返回的树中，而不是被销毁。
- 插入后 `n` 被纳管；可向上导航版本会把 `n` 的父设为 `pos` 所在的父 _二叉树_。
- 返回的树是独立的；它的根仍由它自己管理。若返回的树非空，销毁前应使用
  [`erase`](erase.md) / [`clear`](clear.md)，或把它继续作为树使用。

## 参数

||||
|-:|-|:-|
|`pos`|-|要插入到的 _树位置_|
|`n`|-|要插入的 _二叉树_；必须脱管|

## 返回值

被换出的原子树。`pos` 原本为空时返回空树。

## 复杂度

常数。

## 异常

（无）

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
