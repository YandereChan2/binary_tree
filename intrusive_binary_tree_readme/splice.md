# `Yc::basic_intrusive_binary_tree<vptr>::splice` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::splice`

```C++
// basic_intrusive_binary_tree<vptr>
static basic_intrusive_binary_tree splice(
    edge_proxy to, edge_proxy from)noexcept;
static basic_intrusive_binary_tree splice(
    edge_proxy p, basic_intrusive_binary_tree& tree)noexcept;
static basic_intrusive_binary_tree splice(
    edge_proxy p, basic_intrusive_binary_tree&& tree)noexcept;
```

```C++
// 父感知版本
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy to, edge_proxy from)noexcept;
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy p, basic_intrusive_parent_aware_binary_tree& tree)noexcept;
static basic_intrusive_parent_aware_binary_tree splice(
    edge_proxy p, basic_intrusive_parent_aware_binary_tree&& tree)noexcept;
```

把 `from` 上的子树搬到 `to`，`from` 变为空，并返回被换出的 `to` 原子树。
接收整棵树的版本等价于把该树的根搬到 `p`；传入的树变为空树。

## 前置条件

- `to` 和 `from` 必须指代**合法**的 _树位置_；默认代理是未定义行为。
- `to` 不能在 `from` 的子树中，否则会形成自环，行为未定义。
- 如果使用的是接收整棵树的版本，来源树必须仍然存活。

## 参数

|||
|-:|:-|
|`to`|目标 _树位置_|
|`from`|来源 _树位置_ 或树；搬移后变空|

## 返回值

被换出的 `to` 原子树；`to` 原本为空时返回空树。**必须保留返回值**：它是原子树的
唯一树句柄。丢弃时临时树析构会 `clear()`，节点被脱管但不会销毁；除非另有节点代理，
否则无法再访问。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
// 前提：tree 非空，to 不在 from 的子树中
auto old = tree.splice(tree.root().get_right(), tree.root().get_left());
old.clear();
```

## 备注

- `to` 上原有的子树由返回值接管，不会被丢弃；
- `to == from` 时会把该位置的子树换出，原位置变空；
- `to` 不能在 `from` 的子树中，否则树会损坏；
- 本函数不销毁 _二叉树_。
