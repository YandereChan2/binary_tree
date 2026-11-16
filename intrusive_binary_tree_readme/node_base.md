# `Yc::intrusive_binary_tree_node_base<vptr>` , `Yc::intrusive_parent_aware_binary_tree_virtual_node_base<vptr>` , `Yc::intrusive_parent_aware_binary_tree_node_base<vptr>`

侵入式二叉树的节点钩子类型。用户类型通过继承这些类型，使自己可以作为 _二叉树_ 参与树：

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

- `intrusive_binary_tree_node_base<vptr>`：用于不可向上导航版本的真实 _二叉树_ ；
- `intrusive_parent_aware_binary_tree_virtual_node_base<vptr>`：可向上导航版本中
  _虚拟二叉树_ 使用的类型，也是真实 _二叉树_ 的公共部分（只含 `left`）；
- `intrusive_parent_aware_binary_tree_node_base<vptr>`：继承前者，增加 `right` 和
  `parent`，用于可向上导航版本的真实 _二叉树_ 。

用户创建可向上导航版本的真实 _二叉树_ 时继承 `..._node_base`；通常不需要直接继承
`..._virtual_node_base`。不需要直接操作这些类型的成员。导航和访问请使用
[`node_proxy`](node_proxy/node_proxy.md) / [`edge_proxy`](edge_proxy/edge_proxy.md)。

## 状态查询

三个类型都提供以下公开函数（父感知版本的实现见
[`intrusive_binary_tree.h:878`](../../intrusive_binary_tree.h#L878-L892)）：

```C++
bool is_detached()const noexcept;
bool is_managed()const noexcept;
bool has_children()const noexcept;
```

- `is_detached()`：所有链接字段都指向自身，表示节点处于**脱管**状态；
- `is_managed()`：`!is_detached()`；
- `has_children()`：节点已纳管，且至少一个孩子槽位非空。

注意：`has_children() == false` **不表示**节点没有被树引用。一个纳管叶子仍然被父
节点的 `left` / `right` 槽位（或树根）指向；这三个函数只检查节点自己的链接，不检查
外部引用。

## 纳管与脱管

每棵 _二叉树_ 有两种状态：

- **脱管**：默认状态。所有链接字段都指向自身，表示它不属于任何树，也没有孩子；
- **纳管**：已经被 [`insert`](insert.md) 接入树。

链接字段始终由树操作维护，使用者不能直接读写。脱管的 _二叉树_ 可以通过
[`insert`](insert.md) 重新纳管；纳管时，它的孩子字段会从“指向自身”变为空。

脱管节点上的 `get_left()` / `go_left()` 等导航会得到**节点自身**，而不是空代理。
**不要导航脱管节点**；导航只对已纳管节点有意义。

## 构造与复制

```C++
type() = default;
type(const type&) = delete;
type& operator=(const type&) = delete;
```

- 默认构造得到一棵**脱管**的 _二叉树_ ；
- 不可复制。每棵 _二叉树_ 在任一时刻只应属于一棵树。

## 析构

- **只有脱管节点可以安全析构**；
- 纳管节点（无论有没有孩子）在析构前必须先通过树的删除接口脱管，否则树中指向它的
  槽位会悬垂，后续遍历、`erase` 或 `clear` 都是未定义行为。

销毁整棵子树前应先调用树的删除接口（[`erase`](erase.md) / [`clear`](clear.md)），
它们会让子树中的所有 _二叉树_ 脱管。树对象自身的析构函数也会调用 `clear()`，因此
销毁树对象时，其中的节点会被自动脱管（但不会被销毁）。

## 主动脱管

_二叉树_ 自身没有脱管操作。脱管只能通过树的删除接口
[`erase`](erase.md) / [`clear`](clear.md)：它们让整棵子树的所有 _二叉树_ 全部脱管。

树的 [`cut`](cut.md) 不会让子树根脱管，而是把它交给返回的独立树继续管理。
要销毁被摘下的子树，必须对返回的树使用 `erase` / `clear`。

## 生命周期

这些类型本身不销毁用户数据。脱管节点可以销毁；纳管节点（无论是否有孩子）不能直接
销毁，需先用 [`erase`](erase.md) / [`clear`](clear.md) 让整棵子树脱管，或销毁其所属
的树对象。可向上导航版本的父子关系由树操作自动维护。

## 复杂度

这些状态查询都是常数时间。

## 异常

（无）。
