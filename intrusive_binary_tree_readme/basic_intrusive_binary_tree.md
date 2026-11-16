# `Yc::basic_intrusive_binary_tree`

在文件 `"intrusive_binary_tree.h"` 定义。

```C++
template<class vptr = void*>
class basic_intrusive_binary_tree;
```

不维护父指针的侵入式二叉树。节点只有 `left`、`right` 两个链接，是两套设施中最轻量的
一套；它不知道用户数据 `T`，也不分配、不拥有任何节点。导航和修改都只通过
[`edge_proxy`](edge_proxy/edge_proxy.md) / [`node_proxy`](node_proxy/node_proxy.md)
进行。

父感知版本见
[`basic_intrusive_parent_aware_binary_tree`](basic_intrusive_parent_aware_binary_tree.md)。

## 模板参数

| | |
|-:|:-|
|`vptr`|节点指针的“种子”类型。默认 `void*`；所有链接由 [`std::pointer_traits<vptr>::rebind`](https://zh.cppreference.com/w/cpp/memory/pointer_traits) 得到。缀饰指针的详细要求见[主页面](intrusive_binary_tree.md#缀饰指针与-vptr)。|

## 成员类型

|成员类型|定义|
|-:|:-|
|`node_base`|[`intrusive_binary_tree_node_base<vptr>`](node_base.md)|
|`node_proxy`|[`intrusive_binary_tree_node_proxy<vptr>`](node_proxy/node_proxy.md)|
|`edge_proxy`|[`intrusive_binary_tree_edge_proxy<vptr>`](edge_proxy/edge_proxy.md)|
|`node_const_proxy`|[`intrusive_binary_tree_node_const_proxy<vptr>`](node_const_proxy.md)|
|`edge_const_proxy`|[`intrusive_binary_tree_edge_const_proxy<vptr>`](edge_const_proxy.md)|

## 成员函数

| | |
|:-|:-|
|[（构造函数）](constructor.md)|构造空树；可移动，不可复制|
|[（析构函数）](destructor.md)|脱管所有节点，但不销毁它们|
|[`operator=`](operator_assign.md)|移动赋值；不可复制赋值|
|[`swap`](swap.md)|交换两棵树的根|

### 代理

| | |
|:-|:-|
|[`root` / `croot`](root.md)|返回根 _树位置_ 的代理|
|[`nroot` / `cnroot`](nroot.md)|返回根 _二叉树_ 的代理|

### 修改器

| | |
|:-|:-|
|[`insert`](insert.md)|把脱管节点插到 _树位置_，返回被换出的原子树|
|[`cut`](cut.md)|摘下子树并作为独立树返回|
|[`splice`](splice.md)|搬移子树，返回被换出的旧树|
|[`swap_sub_tree`](swap_sub_tree.md)|交换两个 _树位置_ 上的子树|
|[`swap_node`](swap_node.md)|交换两棵 _二叉树_（值互换、形状按值标记不变）|
|[`erase`](erase.md)|拆子树并让所有 _二叉树_ 脱管；可带回收器|
|[`clear`](clear.md)|只拆树并脱管，不销毁 _二叉树_|

### 操作

| | |
|:-|:-|
|[`left_rotate` / `right_rotate`](rotate.md)|无条件旋转|
|[`checked_left_rotate` / `checked_right_rotate`](rotate.md)|尝试旋转|

### 遍历与容量

| | |
|:-|:-|
|[`for_each`](for_each.md)|遍历整棵树；顺序未指定|
|[`empty`](empty.md)|是否为空|

## 带 `T` 的辅助类型

[`intrusive_binary_tree_node<vptr, T>`](data_node.md) 把节点钩子和一个 `T` 放在同一块
存储里；`vptr` 没有默认值，需要显式写出，例如
`Yc::intrusive_binary_tree_node<void*, int>`。生命周期由调用方用
`construct_value` / `destroy_value` 管理。

## 生命周期

- 每棵 _二叉树_ 默认处于**脱管**状态，所有链接字段指向自身；
- `insert` 把脱管节点纳管；
- 只有**脱管节点可以析构**；纳管节点（无论有没有孩子）直接析构都会让树中指向它的槽位
  悬垂，是未定义行为；
- 树对象不拥有节点：析构树对象会调用 `clear()`，让所有节点脱管，但**不销毁**它们；
- 销毁整棵子树前，使用 `erase` / `clear` 让其中的节点脱管，再销毁节点；
  `cut` 会把子树作为独立树返回，返回的树继续管理这棵子树。

## 复杂度总览

- `insert` / `cut` / `splice` / `swap_sub_tree` / `swap_node` / 旋转 / `swap` /
  移动构造：常数；
- `erase` / `for_each` / `clear` / 析构 / 移动赋值：与涉及的节点数成线性关系。

## 示例

```C++
#include "intrusive_binary_tree.h"
#include <cassert>

struct my_binary_tree : Yc::intrusive_binary_tree_node_base<>
{
    int value;
    explicit my_binary_tree(int v) : value(v) {}
};

int main()
{
    using tree = Yc::basic_intrusive_binary_tree<>;
    my_binary_tree a{1}, b{2}, c{3};

    tree t;
    tree::insert(t.root(), a);
    tree::insert(t.root().get_left(), b);
    tree::insert(t.root().get_right(), c);

    assert(static_cast<my_binary_tree&>(*t.nroot().get_left()).value == 2);

    auto sub = tree::cut(t.root().get_right());
    assert(static_cast<my_binary_tree&>(*sub.nroot()).value == 3);
    sub.clear();

    t.clear(); // 只拆树，各 _二叉树_ 仍然存活
}
```

## 另见

- [代理失效规则](proxy_lifetime.md)
- [返回值与所有权](ownership_and_return.md)
- [侵入式二叉树概念说明](easy_for_understand.md)
