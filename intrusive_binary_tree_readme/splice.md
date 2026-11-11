# `Yc::basic_intrusive_binary_tree<vptr>::splice` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::splice`

```C++
static void splice(edge_proxy to, edge_proxy from)noexcept;
```

把 `from` 上的子树搬到 `to`，`from` 变为空。

- 不可向上导航版本：只搬移子树；
- 可向上导航版本：搬完后，子树根的父 _二叉树_ 被设为 `to` 所在位置的父 _二叉树_。

## 参数

||||
|-:|-|:-|
|`to`|-|目标 _树位置_|
|`from`|-|来源 _树位置_；搬移后变空|

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
tree.splice(tree.root().get_right(), tree.root().get_left());
```

## 备注

- `to` 上原有的子树会被覆盖且不会返回；需要保留时先 [`cut`](cut.md)。
- `to == from` 时是无操作。
- `to` 不能在 `from` 的子树中，否则树会损坏。
- 本函数不销毁 _二叉树_。
