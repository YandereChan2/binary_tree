# 侵入式二叉树相关概念的阐述

本文解释三件事：

- 为什么二叉树接口会从"一级指针"演化到"二级指针"，以及这如何引出
  [`edge_proxy`](edge_proxy/edge_proxy.md) / [`node_proxy`](node_proxy/node_proxy.md)
  的建模；
- 侵入式链表的背景：为什么可以把链接放进用户自己的对象里；
- `container_of` 一类的做法，以及 C++ 中更安全的替代方式。

## 从普通二叉树说起

一般而言，程序中定义二叉树时会写出这样的类型：

```C++
struct tree_element
{
    tree_element* left;
    tree_element* right;
    T value;
};
```

并且用 `tree_element*` 管理根或者某棵子树。

如果"在某个位置插入子树"也用 `tree_element*` 作为参数，就必须区分"插到左边"和
"插到右边"，根位置还要特殊处理。删除更麻烦：你可以 `delete t->left`，但不能直接
`delete t`，因为 `t` 可能是某个父对象的子 _二叉树_，直接销毁它无法让父对象里指向它的
位置同步变空。

## 一级指针与二级指针

为了解决上面的问题，可以把参数改成 `tree_element**`（或 `tree_element*&`）：

```C++
void erase(tree_element** place)
{
    delete *place;
    *place = nullptr;
}
```

这里的 `place` 不是指向某棵 _二叉树_，而是指向"保存这棵 _二叉树_ 的那个位置"。
无论是根位置，还是某个父对象里的 `left` / `right`，都可以用同一种方式处理：

```C++
erase(&root);
erase(&parent->left);
erase(&parent->right);
```

于是插入、删除、旋转、切割、拼接都可以在"位置"这一层统一描述。

本库把这种位置称为 _树位置_，并使用 [`edge_proxy`](edge_proxy/edge_proxy.md) 建模；
把一级指针所指的那棵 _二叉树_ 用 [`node_proxy`](node_proxy/node_proxy.md) 建模。

二叉树的递归定义是：

> 一棵二叉树或为空，或保有一个元素，以及左右两棵二叉树（称为二叉子树）。

因此：

- 一级指针指代一棵 _二叉树_；
- 二级指针指代一个 _树位置_；
- 树的根也是一棵 _二叉树_，根所在的位置也是一个 _树位置_。

## 侵入式链表的背景

普通链表通常由容器分配链表元素：

```C++
struct list_node
{
    list_node* prev;
    list_node* next;
    T value;
};
```

元素的值和链接在同一个由链表分配的对象里。

侵入式链表反过来：**链接放在用户自己的对象里**，链表不分配、不拥有对象。

```C++
struct task
{
    task* prev;
    task* next;
    int id;
};
```

链表只需要保存头部位置（或者一个占位元素）。插入和删除只改 `prev` / `next`，
不需要分配，也不需要为了删除而先查找：

```C++
task a{...}, b{...};
// 把 b 插到 a 后面、摘掉 b，都只改几个链接
```

这样做的好处是：

- 元素地址稳定，移动链接不会搬移元素；
- 容器不拥有元素，元素的创建和销毁由用户决定；
- 同一个元素可以被多个容器/算法以不同的链接方式使用（每个链接成员对应一种用途）；
- 删除、拼接、转移通常只需要常数时间。

限制也很明显：

- 一个链接成员在同一时刻只能属于一条链/一棵树；
- 如果同一对象要同时出现在多个容器里，需要提供多个链接成员，或者继承多个不同的
  侵入式类型；
- 对象被销毁前必须先把它从所有链/树中摘下。

侵入式二叉树采用同样的思路：用户对象自己携带左右（以及可选的父）链接，
`basic_intrusive_binary_tree` 只保存根 _树位置_，不负责任何 _二叉树_ 的创建或销毁。

## container_of 一类的背景

有了侵入式链接之后，链表/树算法手里通常只有一个指向"链接"的指针。例如：

```C++
struct task
{
    task* prev;
    task* next;
    int id;
};

task* t = ...;
task* next = t->next; // 这里已经拿到 task*
```

但在更一般的情况里，算法可能只拿到 `&t->next` 这个"链接的地址"，而不是 `t` 本身。
要取回包含这个链接的 `task` 对象，就需要从"成员地址"反推"宿主对象地址"。

Linux 内核里的 `container_of` 就是这个思路：

```C
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))
```

`ptr` 是指向 `member` 的指针；减去 `member` 在 `type` 中的偏移，就得到宿主对象的地址。
链表、哈希桶、红黑树等侵入式容器经常用这种方式从链接回到用户数据。

在 C++ 中，本库不使用手写的 `offsetof` 宏，而是利用"继承 + `static_cast`"：
用户类型继承侵入式类型，代理返回侵入式类型的引用，用户再把它转回自己的类型：

```C++
struct task : Yc::intrusive_binary_tree_node_base<>
{
    int id;
    explicit task(int i) : id(i) {}
};

task t{...};
auto p = tree.node(t);          // 指代这棵 _二叉树_
auto& same = static_cast<task&>(*p);
```

这和 `container_of` 的目的相同：从"链接/基类子对象"回到"完整用户对象"。
区别是：

- 不需要手写偏移；
- 不需要 `reinterpret_cast`；
- 由 C++ 的基类到派生类转换规则保证正确性（基类必须是明确且非虚的）。

## 带父 _二叉树_ 的情况

不反向关联父 _二叉树_ 的版本可以用二级指针（_树位置_）统一表示位置。
可向上导航的版本还需要支持 `go_up`，而二级指针本身不携带"父对象是谁"的信息。

一种自然的表示是：

```C++
struct parent_aware_position
{
    parent_aware_element* parent;
    bool is_left;
};
```

也就是用"父对象 + 左/右方向"表示一个 _树位置_。可向上导航版本的
[`edge_proxy`](edge_proxy/edge_proxy.md) 就采用这种建模。

根 _二叉树_ 没有真正的父对象。为了让根也有统一的位置表示，本库使用一个
_虚拟二叉树_：它只有左 _树位置_，其左 _树位置_ 上是根 _二叉树_。这样：

- 根 _二叉树_ 的父是 _虚拟二叉树_；
- 根 _树位置_ 可以和其他 _树位置_ 使用同一套操作；
- 用户可以通过 [`parent_of_nroot()`](parent_of_nroot.md) 取得这棵 _虚拟二叉树_。

## 纳管与脱管

侵入式二叉树还给每棵 _二叉树_ 增加了状态：

- **脱管**：默认状态。`left` / `right`（以及可向上导航版本的 `parent`）都指向自身，
  表示它不属于任何树，也没有孩子；
- **纳管**：已经被 [`insert`](insert.md) 接入树，链接字段由树操作维护。

脱管状态的“指向自身”有两个作用：

- 一眼就能看出它还没有被任何树接管；
- 销毁一个脱管的 _二叉树_ 不会影响其他 _二叉树_，因此是安全的。

纳管的 _二叉树_ 如果有孩子，直接析构会留下指向它的父 _树位置_，所以是未定义行为。
要销毁一棵子树，应使用树的删除接口：

- [`erase(pos)`](erase.md)：拆掉子树，让所有 _二叉树_ 脱管；
- [`erase(pos, deleter)`](erase.md)：脱管后再对每棵 _二叉树_ 调用回收器；
- [`clear`](clear.md)：拆掉整棵树，让所有 _二叉树_ 脱管。

_二叉树_ 不能自己脱管；脱管只能通过 [`erase`](erase.md) / [`clear`](clear.md)。
树的 [`cut`](cut.md) 则把子树交给返回的独立树继续管理，不会直接让它脱管。

## proxy 建模的设计来源

把上面的概念对应到本库：

| 概念 | 代理 |
|:-|:-|
| 一棵 _二叉树_ | [`node_proxy`](node_proxy/node_proxy.md) |
| 一个 _树位置_ | [`edge_proxy`](edge_proxy/edge_proxy.md) |
| 根 _二叉树_ | `nroot()` 返回的 `node_proxy` |
| 根 _树位置_ | `root()` 返回的 `edge_proxy` |
| 根 _二叉树_ 的父（可向上导航版本） | `parent_of_nroot()` 返回的 `node_proxy`，指代 _虚拟二叉树_ |

代理的价值在于：

- **类型上区分"一棵 _二叉树_"和"一个 _树位置_**，不会把两者混用；
- 用 `go_left` / `go_right` / `get_left` / `get_right` 等命名表达导航，而不是让使用者
  直接算术或解引用；
- 操作以代理为参数，插入、删除、旋转、交换在接口层统一；
- 侵入式层不需要向使用者暴露链接的表示方式；使用者只和"一棵 _二叉树_ / 一个 _树位置_"
  打交道；
- 代理不参与所有权：树对象不拥有 _二叉树_，使用者仍然负责 _二叉树_ 的生命周期。

## 在代码中的对应关系

- 用户类型继承 [`node_base`](node_base.md) 系列类型，获得侵入式链接能力；
- [`node_proxy`](node_proxy/node_proxy.md) 用来导航和访问一棵 _二叉树_；
- [`edge_proxy`](edge_proxy/edge_proxy.md) 用来表示和操作一个 _树位置_；
- `basic_intrusive_binary_tree` / `basic_intrusive_parent_aware_binary_tree` 是树对象，
  只指代根 _树位置_；
- [`data_node`](data_node.md) 提供"把 `T` 和侵入式链接放在一起"的便利类型。

一个完整的例子：

```C++
struct task : Yc::intrusive_parent_aware_binary_tree_node_base<>
{
    int id;
    explicit task(int i) : id(i) {}
};

Yc::basic_intrusive_parent_aware_binary_tree<> tree;

task root{1};
task child{2};

Yc::basic_intrusive_parent_aware_binary_tree<>::insert(tree.root(), root);
Yc::basic_intrusive_parent_aware_binary_tree<>::insert(
    tree.root().get_left(), child);

auto p = tree.nroot().get_left();      // 指代 child 这棵 _二叉树_
auto& c = static_cast<task&>(*p);      // 回到完整的 task 对象
assert(c.id == 2);
assert(p.get_parent() == tree.nroot());

tree.clear(); // root / child 都脱管，可以安全析构
```

## 与非侵入式容器的关系

非侵入式容器（`Yc::binary_tree` / `Yc::parent_aware_binary_tree`）在概念上使用同样的
_二叉树_ / _树位置_ / 代理模型，但它们拥有 _二叉树_，并负责其创建和销毁。
侵入式二叉树把这层"所有权"拿掉，只保留树形操作，因此适合作为更底层的构建块：

- 想要"容器拥有元素"时，使用非侵入式容器；
- 想要"元素自己携带链接、容器只负责组织"时，使用侵入式二叉树；
- 非侵入式容器本身也持有一棵侵入式树。

## 小结

- 一级指针指代一棵 _二叉树_，二级指针指代一个 _树位置_；这两个概念分别由
  [`node_proxy`](node_proxy/node_proxy.md) 和 [`edge_proxy`](edge_proxy/edge_proxy.md)
  建模。
- 侵入式链表把链接放进用户对象，容器不拥有元素；侵入式二叉树同理。
- `container_of` 从"成员地址"反推"宿主对象"；本库用"继承 + `static_cast`"完成
  同样的事情。
- 带父 _二叉树_ 的版本用"父 _二叉树_ + 方向"表示 _树位置_，并用 _虚拟二叉树_
  统一根的情况。
