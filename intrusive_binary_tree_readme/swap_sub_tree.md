# `Yc::basic_intrusive_binary_tree<vptr>::swap_sub_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap_sub_tree`

```C++
static void swap_sub_tree(edge_proxy l, edge_proxy r)noexcept;
```

交换 `l` 与 `r` 两个 _树位置_ 上的子树。

- 不可向上导航版本：交换两棵子树；
- 可向上导航版本：交换后，把两个子树根的父分别设为对方 _树位置_ 所在的父 _二叉树_。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|要交换的两个 _树位置_|

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
tree.swap_sub_tree(tree.root().get_left(), tree.root().get_right());
```

## 备注

- 两个 _树位置_ 可以是空的；空位也随子树一起交换。
- `l == r` 时是无操作。
- 本函数不复制、不销毁 _二叉树_。
