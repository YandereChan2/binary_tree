# `Yc::basic_intrusive_binary_tree<vptr>::node` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::node`

```C++
node_proxy node(node_base& n)const noexcept; // 不可向上导航版本
```

```C++
node_proxy node(
    intrusive_parent_aware_binary_tree_virtual_node_base<vptr>& n)const noexcept; // 可向上导航版本
```

由侵入式类型引用构造 _二叉树_ 的代理。该函数不要求这棵 _二叉树_ 当前属于本树，
也不改变树形；它只是得到对应的代理。

可向上导航版本可以接受真实 _二叉树_，也可以接受 [`parent_of_nroot()`](parent_of_nroot.md)
返回的 _虚拟二叉树_。

## 参数

||||
|-:|-|:-|
|`n`|-|_二叉树_ 的侵入式类型引用（可向上导航版本也可是 _虚拟二叉树_）|

## 返回值

指代 `n` 的 `node_proxy`。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
my_binary_tree n;
auto p = tree.node(n);
assert(p == tree.node(n));
```

## 备注

拥有型容器在创建 _二叉树_ 后，用本函数得到代理，再交给 [`link`](link.md) /
[`set_root`](set_root.md)。
