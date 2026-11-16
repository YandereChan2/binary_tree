# Yc::intrusive_binary_tree

在文件 `"intrusive_binary_tree.h"` 定义。该头文件提供两套侵入式二叉树设施：

```C++
template<class vptr = void*>
class basic_intrusive_binary_tree;

template<class vptr = void*>
class basic_intrusive_parent_aware_binary_tree;
```

- [`basic_intrusive_binary_tree`](basic_intrusive_binary_tree.md)：不反向关联父
  _二叉树_ 的版本，节点只有左右两个钩子，最轻量；
- [`basic_intrusive_parent_aware_binary_tree`](basic_intrusive_parent_aware_binary_tree.md)：
  带父 _二叉树_ 、可在常数时间内 `go_up` 的版本。

两套树共享同一套节点与代理模型，本文介绍这些公共部分；每个类的完整 API 见各自的
索引页。具体操作页（如 `insert.md`、`swap_node.md`）在页面内按类分节列出签名和
差异，不再把两套树的接口混成一段。

## 公共模型

此处 _二叉树_ 的定义与非侵入式容器相同：每棵 _二叉树_ 占据一个 _树位置_ ，保有左右
两个 _树位置_ 以及（可选的）用户数据。区别在于：**每棵 _二叉树_ 由用户拥有，树对象
不拥有任何 _二叉树_**。树对象只指代根位置；可向上导航版本还持有一棵 _虚拟二叉树_
作为哨兵。

所有导航和修改都通过两种轻量代理完成：

- [`edge_proxy`](edge_proxy/edge_proxy.md)：指代一个 _树位置_ ；
- [`node_proxy`](node_proxy/node_proxy.md)：指代一棵 _二叉树_ 。

它们都可以默认构造，按 `bool` 判断是否非空；`operator*` 返回 _二叉树_ 的侵入式类型
引用，用户再 `static_cast` 到自己的类型。侵入式层不知道用户数据 `T`。

节点钩子类型见 [`node_base`](node_base.md)；同时存放 `T` 与链接的类型见
[`data_node`](data_node.md)。

## 术语

| 术语 | 含义 |
|:-|:-|
| _树位置_ / 边 | 保存“指向某棵 _二叉树_ 的指针”的槽位；根槽位、节点的 `left` / `right` 都是 _树位置_ 。 |
| _二叉树_ / 节点 | 一个用户对象加上它的左右链接（父感知版本还有 `parent`）。 |
| 代理 | `edge_proxy` / `node_proxy` 及其 const 版本；不拥有节点。 |
| 脱管 | 节点不属于任何树；所有链接指向自身。只有脱管节点可以析构。 |
| 纳管 | 节点已经被 `insert` 接入树；链接由树操作维护。 |
| 虚拟二叉树 | 父感知版本内部只含 `left` 的哨兵，根 _二叉树_ 的父指向它。 |
| proxy | 代码标识符中的 `proxy` 与正文中的“代理”同义。 |

“行为未定义”均指 C++ 的未定义行为；本库不会为违反前置条件的调用做运行时检查。
代理失效与返回值所有权分别见[代理失效规则](proxy_lifetime.md) 和
[返回值与所有权](ownership_and_return.md)。

## 缀饰指针与 `vptr`

`vptr` 不是“节点指针类型本身”，而是节点指针的**种子类型**。库内部会做：

```C++
using node_pointer =
    typename std::pointer_traits<vptr>::template rebind<node_base>;
```

也就是说，所有 `left` / `right` / `parent` 链接都使用
`pointer_traits<vptr>::rebind<节点基类>` 得到的指针类型。默认 `vptr = void*` 时，
链接就是普通指针；如果分配器使用缀饰指针（fancy pointer，例如
`boost::interprocess::offset_ptr` 或自定义的检查指针），就应该把分配器的
`pointer` 类型作为 `vptr` 传入，让库 rebind 出指向节点基类的同族指针。

一个缀饰指针至少要满足：

1. 可默认构造，默认值表示空指针；可按语境转换为 `bool`；
2. 支持 `==` / `!=`；
3. 支持 `std::pointer_traits` 的 `pointer_to`（从对象左值得到指针）和 `rebind`；
4. `rebind<U>` 得到的类型满足 `pointer_traits<...>::element_type` 为 `U`；
5. 能解引用（`*p`）并取成员（`p->member`）。

库对节点基类的 `rebind` 结果有编译期检查：

```C++
static_assert(
    std::is_same_v<
        typename std::pointer_traits<node_pointer>::element_type,
        intrusive_binary_tree_node_base>,
    "pointer_traits<vptr>::rebind must yield a pointer to the node base");
```

因此：

```C++
using fancy = my_fancy_pointer<T>;             // 分配器的 pointer 类型
using tree = Yc::basic_intrusive_binary_tree<fancy>;
```

是正确用法；库会 rebind 出 `my_fancy_pointer<node_base>`。

**不要传 `fancy<void>`。** 某些缀饰指针的 `pointer_to` 需要 `void&`，无法从
`node_base&` 形成，会在编译期失败。传 `fancy<T>`（或任何能 rebind 出
`fancy<node_base>` 的类型）即可；`vptr` 本身的 `element_type` 不会成为节点类型。

不要把 `void*` 节点和 `fancy<node_base>` 节点混在同一棵树里：同一次实例化的所有
链接都使用同一个 `node_pointer` 类型。

## 生命周期

- 每棵 _二叉树_ 默认处于**脱管**状态，所有链接字段指向自身。
- `insert` 把脱管的 _二叉树_ 纳管；**只有脱管节点可以析构**。纳管节点（无论有没有
  孩子）直接析构都会让树中指向它的槽位悬垂，是未定义行为。
- 树对象不拥有 _二叉树_：析构树对象会调用 `clear()`，让所有节点脱管（父感知版本还会
  清除 `parent`），但**不会销毁节点**。
- 销毁整棵子树前，使用 `erase` / `clear` 让其中的所有 _二叉树_ 脱管。
  `cut` 会把子树作为独立树返回，返回的树继续管理这棵子树。
- `erase(pos)` 让子树中的所有 _二叉树_ 脱管；`erase(pos, deleter)` 还会对每棵脱管后的
  _二叉树_ 调用回收器。回收器内不得修改树结构（不能重新接入、导航或改变链接），
  但**可以销毁**该节点；回收器必须 `noexcept`，抛出会调用 `std::terminate`。
- `clear` 只拆树并让所有 _二叉树_ 脱管，不销毁 _二叉树_。
- 可向上导航版本由树操作自动维护父子关系；`insert` 会把新根的父设为 `pos` 所在的
  父 _二叉树_。

## 复杂度总览

- `insert` / `cut` / `splice` / `swap_sub_tree` / `swap_node` / 旋转 / `swap` /
  移动构造：常数；
- `erase` / `for_each` / `clear` / 析构 / 移动赋值：与涉及的 _二叉树_ 数量成线性
  关系。

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

父感知版本多出的 `go_up` / `parent_of_nroot` / `child_check` 见
[父感知版本索引](basic_intrusive_parent_aware_binary_tree.md)。

## 与非侵入式容器的关系

`Yc::binary_tree` / `Yc::parent_aware_binary_tree` 是拥有型适配器：它们各自持有一棵
侵入式树，并负责其 _二叉树_ 的生命周期。它们的公开接口、分配器语义和可观察行为
保持不变；原有 [`binary_tree_readme`](../binary_tree_readme/binary_tree.md) /
[`parent_aware_binary_tree_readme`](../parent_aware_binary_tree_readme/parent_aware_binary_tree.md)
仍然适用。

## 自检

[`test_intrusive_binary_tree.cpp`](../../test_intrusive_binary_tree.cpp) 覆盖两套设施的
proxy 导航、insert/cut/splice、旋转、交换、erase/clear 的生命周期语义、遍历、移动和
父子关系。
