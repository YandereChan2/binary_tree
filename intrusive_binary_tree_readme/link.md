# `Yc::basic_intrusive_binary_tree<vptr>::link` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::link`

```C++
static void link(edge_proxy to, node_proxy child)noexcept;
```

把 `child` 这棵子树挂到 `to` 所指代的 _树位置_ 上。

- 不可向上导航版本：只建立该 _树位置_ 与子树的关联；
- 可向上导航版本：同时把子树的根 _二叉树_ 的父设为 `to` 所在位置的父 _二叉树_
  （根 _树位置_ 则是 _虚拟二叉树_）。

## 参数

||||
|-:|-|:-|
|`to`|-|目标 _树位置_；必须是有效代理|
|`child`|-|要挂上去的子树根；可以是空代理|

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
my_binary_tree a, b;
Yc::basic_intrusive_binary_tree<> tree;
tree.set_root(tree.node(a));
Yc::basic_intrusive_binary_tree<>::link(
    tree.root().get_left(), tree.node(b));
```

## 备注

`link` 会覆盖 `to` 上原有的子树，且不会返回它。需要保留原内容时，先用
[`cut`](cut.md) 摘下，或用 [`splice`](splice.md) 完成"取出旧的、放入新的"。
