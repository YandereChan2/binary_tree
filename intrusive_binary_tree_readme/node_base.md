# `Yc::intrusive_binary_tree_node_base<vptr>` , `Yc::intrusive_parent_aware_binary_tree_virtual_node_base<vptr>` , `Yc::intrusive_parent_aware_binary_tree_node_base<vptr>`

侵入式二叉树的类型。用户类型通过继承这些类型，使自己可以作为 _二叉树_ 参与树：

```C++
struct my_binary_tree : Yc::intrusive_binary_tree_node_base<>
{
    int value;
};

struct my_parent_aware_binary_tree
    : Yc::intrusive_parent_aware_binary_tree_node_base<>
{
    int value;
};
```

## 三个类型

- `intrusive_binary_tree_node_base<vptr>`：用于不可向上导航版本的真实 _二叉树_；
- `intrusive_parent_aware_binary_tree_virtual_node_base<vptr>`：可向上导航版本中
  _虚拟二叉树_ 使用的类型，也是真实 _二叉树_ 的公共部分；
- `intrusive_parent_aware_binary_tree_node_base<vptr>`：继承前者，用于可向上导航版本的
  真实 _二叉树_。

用户创建可向上导航版本的真实 _二叉树_ 时继承 `..._node_base`；通常不需要直接继承
`..._virtual_node_base`。不需要直接操作这些类型的任何成员。
导航和访问请使用 [`node_proxy`](node_proxy/node_proxy.md) /
[`edge_proxy`](edge_proxy/edge_proxy.md)。

## 构造与复制

```C++
type() = default;
type(const type&) = delete;
type& operator=(const type&) = delete;
```

- 默认构造得到一棵不处于任何树中的 _二叉树_；
- 不可复制。每棵 _二叉树_ 在任一时刻只应属于一棵树。

## 生命周期

这些类型本身不销毁用户数据。每棵 _二叉树_ 在从树上摘下之前不能被销毁；
可向上导航版本的父子关系由树操作自动维护。

## 复杂度

常数。

## 异常

（无）
