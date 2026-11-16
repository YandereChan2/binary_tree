# `Yc::basic_intrusive_parent_aware_binary_tree`

在文件 `"intrusive_binary_tree.h"` 定义。

```C++
template<class vptr = void*>
class basic_intrusive_parent_aware_binary_tree;
```

带父指针的侵入式二叉树。每个节点额外保存 `parent`，容器内部有一个只含 `left` 的
_虚拟二叉树_ 作为哨兵，因此可以在常数时间内 `go_up`，所有结构操作都会维护 `parent`
链。树对象不拥有节点，也不分配内存。

不维护父指针的轻量版本见
[`basic_intrusive_binary_tree`](basic_intrusive_binary_tree.md)。

## 模板参数

| | |
|-:|:-|
|`vptr`|节点指针的“种子”类型。默认 `void*`；节点链接、父链接和哨兵链接都由 [`std::pointer_traits<vptr>::rebind`](https://zh.cppreference.com/w/cpp/memory/pointer_traits) 得到。缀饰指针的详细要求见[主页面](intrusive_binary_tree.md#缀饰指针与-vptr)。|

## 成员类型

|成员类型|定义|
|-:|:-|
|`node_base`|[`intrusive_parent_aware_binary_tree_node_base<vptr>`](node_base.md)|
|`virtual_base`|[`intrusive_parent_aware_binary_tree_virtual_node_base<vptr>`](node_base.md)|
|`node_proxy`|[`intrusive_parent_aware_binary_tree_node_proxy<vptr>`](node_proxy/node_proxy.md)|
|`edge_proxy`|[`intrusive_parent_aware_binary_tree_edge_proxy<vptr>`](edge_proxy/edge_proxy.md)|
|`node_const_proxy`|[`intrusive_parent_aware_binary_tree_node_const_proxy<vptr>`](node_const_proxy.md)|
|`edge_const_proxy`|[`intrusive_parent_aware_binary_tree_edge_const_proxy<vptr>`](edge_const_proxy.md)|

## 成员函数

| | |
|:-|:-|
|[（构造函数）](constructor.md)|构造空树；可移动，不可复制|
|[（析构函数）](destructor.md)|脱管所有节点并清除父指针，但不销毁它们|
|[`operator=`](operator_assign.md)|移动赋值；不可复制赋值|
|[`swap`](swap.md)|交换两棵树的根，并修正根父指针|

### 代理与导航

| | |
|:-|:-|
|[`root` / `croot`](root.md)|返回根 _树位置_ 的代理|
|[`nroot` / `cnroot`](nroot.md)|返回根 _二叉树_ 的代理|
|[`parent_of_nroot` / `parent_of_cnroot`](parent_of_nroot.md)|返回 _虚拟二叉树_ 的代理|

### 修改器

| | |
|:-|:-|
|[`insert`](insert.md)|把脱管节点插到 _树位置_，返回被换出的原子树|
|[`cut`](cut.md)|摘下子树并作为独立树返回|
|[`splice`](splice.md)|搬移子树，返回被换出的旧树|
|[`swap_sub_tree`](swap_sub_tree.md)|交换两个 _树位置_ 上的子树|
|[`swap_node`](swap_node.md)|交换两棵 _二叉树_；可传 `child_check` 掩码|
|[`erase`](erase.md)|拆子树并让所有 _二叉树_ 脱管；可带回收器|
|[`clear`](clear.md)|只拆树并脱管，不销毁 _二叉树_|

### 操作

| | |
|:-|:-|
|[`left_rotate` / `right_rotate`](rotate.md)|无条件旋转，并维护父指针|
|[`checked_left_rotate` / `checked_right_rotate`](rotate.md)|尝试旋转|

### 遍历与容量

| | |
|:-|:-|
|[`for_each`](for_each.md)|遍历整棵树；顺序未指定|
|[`empty`](empty.md)|是否为空|

## 带 `T` 的辅助类型

[`intrusive_parent_aware_binary_tree_node<vptr, T>`](data_node.md) 把父感知节点钩子和
一个 `T` 放在同一块存储里；`vptr` 没有默认值，需要显式写出，例如
`Yc::intrusive_parent_aware_binary_tree_node<void*, int>`。

## 生命周期

- 每棵 _二叉树_ 默认处于**脱管**状态，所有链接字段（包括 `parent`）指向自身；
- `insert` 把脱管节点纳管，并把新根的父设为 `pos` 所在的父 _二叉树_；
- 只有**脱管节点可以析构**；纳管节点（无论有没有孩子）直接析构都会让父槽位悬垂，
  是未定义行为；
- 树对象析构时调用 `clear()`，让所有节点脱管并把 `parent` 清回自身，**避免**根节点
  留下指向已销毁哨兵的悬垂 `parent`；节点本身不被销毁；
- 可向上导航版本由树操作自动维护父子关系。

## 复杂度总览

与不维护父指针的版本相同：

- `insert` / `cut` / `splice` / `swap_sub_tree` / `swap_node` / 旋转 / `swap` /
  移动构造：常数；
- `erase` / `for_each` / `clear` / 析构 / 移动赋值：与涉及的节点数成线性关系。

## 示例

```C++
#include "intrusive_binary_tree.h"
#include <cassert>

struct task : Yc::intrusive_parent_aware_binary_tree_node_base<>
{
    int id;
    explicit task(int i) : id(i) {}
};

int main()
{
    using tree = Yc::basic_intrusive_parent_aware_binary_tree<>;
    task root{1}, child{2};

    tree t;
    tree::insert(t.root(), root);
    tree::insert(t.root().get_left(), child);

    auto p = t.nroot().get_left();
    assert(static_cast<task&>(*p).id == 2);
    assert(p.get_parent() == t.nroot());

    t.clear(); // root / child 都脱管，可以安全析构
}
```

## 另见

- [代理失效规则](proxy_lifetime.md)
- [返回值与所有权](ownership_and_return.md)
- [侵入式二叉树概念说明](easy_for_understand.md)
