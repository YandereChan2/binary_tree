# `Yc::basic_intrusive_binary_tree<vptr>::swap_sub_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap_sub_tree`

```C++
// basic_intrusive_binary_tree<vptr>
static void swap_sub_tree(edge_proxy l, edge_proxy r)noexcept;

// basic_intrusive_parent_aware_binary_tree<vptr>
static void swap_sub_tree(edge_proxy l, edge_proxy r)noexcept;
```

交换 `l` 与 `r` 两个 _树位置_ 上的子树。

- 不可向上导航版本：交换两棵子树；
- 可向上导航版本：交换后，把两个子树根的父分别设为对方 _树位置_ 所在的父 _二叉树_。

## 前置条件

- `l` 和 `r` 必须指代**合法**的 _树位置_。
- `l` 和 `r` **不能互为祖先 / 后代**；直接交换槽位不支持把祖先子树移动到自己
  的内部，违反时会产生自环和悬垂，行为未定义。
- `l == r` 是无操作；其中一个为空 _树位置_ 也是允许的。

## 参数

|||
|-:|:-|
|`l` , `r`|要交换的两个 _树位置_|

## 返回值

（无）。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
// 前提：两个位置都有效，且不互为祖先 / 后代
tree.swap_sub_tree(tree.root().get_left(), tree.root().get_right());
```

## 备注

- 两个 _树位置_ 可以是空的；空位也随子树一起交换。
- 本函数不复制、不销毁 _二叉树_。
