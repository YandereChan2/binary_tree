# `Yc::basic_intrusive_binary_tree<vptr>::splice` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::splice`

```C++
static basic_intrusive_binary_tree splice(
    edge_proxy to, edge_proxy from)noexcept;
```

```C++
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy to, edge_proxy from)noexcept;
```

```C++
static basic_intrusive_binary_tree splice(
    edge_proxy p, basic_intrusive_binary_tree& tree)noexcept;
static basic_intrusive_binary_tree splice(
    edge_proxy p, basic_intrusive_binary_tree&& tree)noexcept;
```

```C++
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy p, basic_intrusive_parent_aware_binary_tree& tree)noexcept;
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy p, basic_intrusive_parent_aware_binary_tree&& tree)noexcept;
```

把 `from` 上的子树搬到 `to`，`from` 变为空，并返回被换出的 `to` 原子树。
接收整棵树的版本等价于把该树的根搬到 `p`。

- 不可向上导航版本：只搬移子树；
- 可向上导航版本：搬完后，子树根的父 _二叉树_ 被设为 `to` 所在位置的父 _二叉树_。

## 参数

||||
|-:|-|:-|
|`to`|-|目标 _树位置_|
|`from`|-|来源 _树位置_；搬移后变空|

## 返回值

被换出的 `to` 原子树；`to` 原本为空时返回空树。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
auto old = tree.splice(tree.root().get_right(), tree.root().get_left());
old.clear();
```

## 备注

- `to` 上原有的子树由返回值接管，不会被丢弃；
- `to == from` 时会把该位置的子树换出，原位置变空；
- `to` 不能在 `from` 的子树中，否则树会损坏；
- 本函数不销毁 _二叉树_。
