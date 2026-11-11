# Yc::intrusive_binary_tree

在文件 `"intrusive_binary_tree.h"` 定义。该头文件提供两套侵入式二叉树设施：

```C++
template<class vptr = void*>
class basic_intrusive_binary_tree;

template<class vptr = void*>
class basic_intrusive_parent_aware_binary_tree;
```

`Yc::basic_intrusive_binary_tree` 是不反向关联父 _二叉树_ 的版本；
`Yc::basic_intrusive_parent_aware_binary_tree` 是带父 _二叉树_、可在常数时间内
`go_up` 的版本。

此处 _二叉树_ 的定义与非侵入式容器相同：每棵 _二叉树_ 占据一个 _树位置_，保有左右两个
_树位置_ 以及（可选的）用户数据。区别在于：**每棵 _二叉树_ 由用户拥有，树对象不拥有
任何 _二叉树_**。树对象只指代根位置；可向上导航版本还指代一棵 _虚拟二叉树_。

如果对 proxy 的建模来源、侵入式链表和 `container_of` 的背景不熟悉，可以先阅读
[概念说明](easy_for_understand.md)。

## 用 proxy 建模

所有导航和修改都通过两种轻量代理完成：

- [`edge_proxy`](edge_proxy/edge_proxy.md)：指代一个 _树位置_；
- [`node_proxy`](node_proxy/node_proxy.md)：指代一棵 _二叉树_。

它们都可以默认构造，按 `bool` 判断是否有效；`operator*` 返回 _二叉树_ 的侵入式类型引用，
用户再 `static_cast` 到自己的类型。侵入式层不知道用户数据 `T`。

## 成员类型

|成员类型|定义|
|-:|:-|
|[`node_base`](node_base.md)|侵入式二叉树的类型；用户类型继承它|
|[`node_proxy`](node_proxy/node_proxy.md)|_二叉树_ 的代理对象|
|[`edge_proxy`](edge_proxy/edge_proxy.md)|_树位置_ 的代理对象|
|[`intrusive_binary_tree_node` / `intrusive_parent_aware_binary_tree_node`](data_node.md)|同时存放 `T` 与侵入式链接的类型|

## 成员函数

| | |
|:-|:-|
|[（构造函数）](constructor.md)|构造侵入式树（默认 / 移动；不可复制）|
|[（析构函数）](destructor.md)|析构树对象，不销毁 _二叉树_|
|[`operator=`](operator_assign.md)|移动赋值；不可复制赋值|
|[`swap`](swap.md)|交换两棵树|

### 代理与导航

| | |
|:-|:-|
|[`root`](root.md)|返回根 _树位置_ 的代理|
|[`nroot`](nroot.md)|返回根 _二叉树_ 的代理|
|[`parent_of_nroot`](parent_of_nroot.md)|可向上导航版本：返回 _虚拟二叉树_ 的代理|
|[`node`](node.md)|由侵入式类型引用构造 _二叉树_ 的代理|

### 修改器

| | |
|:-|:-|
|[`set_root` / `release_root`](set_root.md)|接管 / 摘下整棵树|
|[`link`](link.md)|把子树挂到 _树位置_ 上|
|[`unlink`](unlink.md)|可向上导航版本：摘下子树并断开父 _二叉树_|
|[`cut`](cut.md)|不可向上导航版本：摘下 _树位置_ 上的子树|
|[`splice`](splice.md)|搬移子树|
|[`swap_sub_tree`](swap_sub_tree.md)|交换两个 _树位置_ 上的子树|
|[`swap_node`](swap_node.md)|交换两棵 _二叉树_（值互换、形状不变）|
|[`erase`](erase.md)|拆子树，并对每棵 _二叉树_ 调用回收器|
|[`clear`](clear.md)|只拆树，不销毁 _二叉树_|

### 操作

| | |
|:-|:-|
|[`left_rotate` / `right_rotate`](rotate.md)|无条件旋转|
|[`checked_left_rotate` / `checked_right_rotate`](rotate.md)|尝试旋转|

### 遍历与空判断

| | |
|:-|:-|
|[`for_each`](for_each.md)|中序遍历，对每棵 _二叉树_ 调用函数对象|
|[`empty`](empty.md)|是否为空|

### 带 `T` 的类型与辅助类

| | |
|:-|:-|
|[带 `T` 的类型](data_node.md)|`intrusive_binary_tree_node` / `intrusive_parent_aware_binary_tree_node`|
|`std::hash<node_proxy>` / `std::hash<edge_proxy>`|见 [node_proxy](node_proxy/node_proxy.md)、[edge_proxy](edge_proxy/edge_proxy.md)|

## 模板参数

| | |
|-:|:-|
|`vptr`|指定侵入式链接使用的表示。默认 `void*`；通常保持默认即可。|
|`T`|仅用于带数据的类型 `intrusive_*_node<vptr, T>`。|

## 生命周期

- 树对象不拥有 _二叉树_：析构树对象不会销毁任何 _二叉树_。
- 销毁 _二叉树_ 前必须先把它们从树上摘下：`cut` / `unlink` / `erase` / `clear`。
- `erase` 对每棵 _二叉树_ 调用回收器；`clear` 只拆树，不销毁 _二叉树_。
- 可向上导航版本由树操作自动维护父子关系；`set_root` 会把根 _二叉树_ 的父设为新树的
  _虚拟二叉树_。

## 复杂度总览

- `link` / `unlink` / `cut` / `splice` / `swap_sub_tree` / `swap_node` / 旋转：常数。
- `erase` / `for_each` / `clear`：与涉及的 _二叉树_ 数量成线性关系。

## 示例

```C++
#include "intrusive_binary_tree.h"
#include <cassert>

struct my_binary_tree : Yc::intrusive_binary_tree_node_base<>
{
    int value;
};

int main()
{
    using tree = Yc::basic_intrusive_binary_tree<>;
    my_binary_tree a{1}, b{2}, c{3};

    tree t;
    t.set_root(t.node(a));
    tree::link(t.root().get_left(), t.node(b));
    tree::link(t.root().get_right(), t.node(c));

    assert(static_cast<my_binary_tree&>(*t.nroot().get_left()).value == 2);

    auto sub = tree::cut(t.root().get_right());
    assert(static_cast<my_binary_tree&>(*sub).value == 3);
    tree::splice(t.root().get_right(), t.root().get_left());

    t.clear(); // 只拆树，各 _二叉树_ 仍然存活
}
```

可向上导航版本多一个 `go_up`：

```C++
using ptree = Yc::basic_intrusive_parent_aware_binary_tree<>;
ptree t;
// ... 建树 ...
auto n = t.nroot().get_left();
n.go_up();
assert(n == t.nroot());
assert(t.nroot().get_parent() == t.parent_of_nroot());
```

## 与非侵入式容器的关系

`Yc::binary_tree` / `Yc::parent_aware_binary_tree` 是拥有型适配器：它们各自持有一棵
侵入式树，并负责其 _二叉树_ 的生命周期。它们的公开接口、分配器语义和可观察行为保持不变；
原有 [`binary_tree_readme`](../binary_tree_readme/binary_tree.md) /
[`parent_aware_binary_tree_readme`](../parent_aware_binary_tree_readme/parent_aware_binary_tree.md)
仍然适用。

## 自检

[`test_intrusive_binary_tree.cpp`](../test_intrusive_binary_tree.cpp) 覆盖两套设施的
proxy 导航、link/cut/splice、旋转、交换、erase/clear 的生命周期语义、遍历、移动和
父子关系。
