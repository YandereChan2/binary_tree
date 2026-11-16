# Yc::binary_tree

在文件 `"binary_tree.h"` 定义。该头文件依赖 `"intrusive_binary_tree.h"` 和
`"union_data.h"`，使用时应把这三个头文件一起放进包含目录。

```C++
template<
    class T,
    class Alloc = std::allocator<T>
>class binary_tree;
```

`Yc::binary_tree` 是**拥有节点**的二叉树容器：它是对侵入式二叉树
`Yc::basic_intrusive_binary_tree` 的 RAII 适配器，负责节点内存的分配、
`T` 的构造/析构以及分配器传播。它不维护父节点指针，因此不能向上导航。

此处 _二叉树_ 的定义如下：
每一个 _二叉树_ 占据一个 _树位置_ ，一个 _树位置_ 或空闲，或存在一个 _二叉树_ 。一个 _二叉树_ 保有左右两个 _树位置_ 以及一个 `T` 类型的元素，两个 _树位置_ 上可以有也可以没有其他 _二叉树_ 。每个 `Yc::binary_tree` 对象内部保有一个 _树位置_ 。

当一个 _树位置_ 生存期结束时，其上的 _二叉树_ 要么提前转移至其他 _树位置_ ，要么提前结束。换言之， _树位置_ 只能在空闲时结束其生存期，否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

在本库中，使用 [`edge_proxy`](edge_proxy/edge_proxy.md) 指代一个 _树位置_ ，使用 [`node_proxy`](node_proxy/node_proxy.md) 指代一个 _二叉树_ 。

如果上面的定义让人难以理解，可以先查看[该文档](easy_for_understand.md)。

## 术语

| 术语 | 含义 |
|:-|:-|
| _树位置_ / 边 | 保存“指向某棵 _二叉树_ 的指针”的那个槽位；根槽位、父节点的 `left`/`right` 都是 _树位置_ 。 |
| _二叉树_ / 节点 | 一个元素加上它的左右两个 _树位置_ ；在本库中由一次 `emplace`/`insert` 建立。 |
| 代理 | 不拥有元素的轻量句柄；`edge_proxy` 指代 _树位置_ ，`node_proxy` 指代 _二叉树_ 。 |
| 失效 | 代理所绑定的槽位或节点不再存在；此后使用该代理是未定义行为。 |

“行为未定义”均指 C++ 的未定义行为；本库不会为违反前置条件的调用做运行时检查。
代理的失效规则见[代理失效规则](proxy_lifetime.md)，返回值与所有权的规则见
[返回值与所有权](ownership_and_return.md)。

## 模板参数

| | |
|-:|:-|
|`T`|元素的类型。 至少应当满足[_可擦除 (Erasable)_](https://zh.cppreference.com/w/cpp/named_req/Erasable)，但是一些成员函数提出了更高的要求。|
|`Alloc`|用于获取/释放内存及构造/析构内存中元素的分配器。类型必须满足[_分配器 (Allocator)_](https://zh.cppreference.com/w/cpp/named_req/Allocator)。|

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T`|
|`allocator_type`|`Alloc`|
|`reference`|`T&`|
|`pointer`|[`std::allocator_traits<Alloc>::pointer`](https://zh.cppreference.com/w/cpp/memory/allocator_traits)|
|[`edge_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象。|
|[`edge_const_proxy`](edge_proxy/edge_proxy.md)|表示 _树位置_ 的轻量级代理对象，对这个位置的树的元素提供只读访问。|
|[`node_proxy`](node_proxy/node_proxy.md)|表示 _二叉树_ 的轻量级代理对象。|
|[`node_const_proxy`](node_proxy/node_proxy.md)|表示 _二叉树_ 的轻量级代理对象，对这个树的元素提供只读访问。|

## 成员函数

|||
|:-|:-|
|[（构造函数）](constructor.md)|构造 `binary_tree` <br>（公开成员函数）|
|[（析构函数）](destructor.md)|析构 `binary_tree` <br>（公开成员函数）|
|[`operator=`](operator_assign.md)|对内容赋值<br>（公开成员函数）|
|[`get_allocator`](get_allocator.md)|返回关联的分配器<br>（公开成员函数）|

### 代理

|||
|:-|:-|
|[`root`<br>`croot`](root.md)|返回 `binary_tree` 对象内部的 _树位置_ 的代理。（公开成员函数）|
|[`nroot`<br>`cnroot`](nroot.md)|返回处于 `binary_tree` 对象内部的 _树位置_ 上的 _二叉树_ 的代理。（公开成员函数）|

### 修改器

|||
|:-|:-|
|[`clear`](clear.md)|清除内容<br>（公开成员函数）|
|[`insert`](insert.md)|在某 _树位置_ 插入元素，产生新的 _二叉树_ <br>（公开成员函数）|
|[`emplace`](emplace.md)|在某 _树位置_ 原位构造元素，产生新的 _二叉树_ <br>（公开成员函数）|
|[`erase`](erase.md)|擦除 _二叉树_ <br>（公开成员函数）|
|[`swap`](swap.md)|交换内容<br>（公开成员函数）|
|[`cut`](cut.md)|取出某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|[`swap_sub_tree`](swap_sub_tree.md)（静态）|交换两个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|[`swap_node`](swap_node.md)（静态）|交换两个节点上的值，子树留在原位，不需要邻接关系检查<br>（公开成员函数）|
|[`splice`](splice.md)|从其他 _树位置_ 转移 _二叉树_ <br>（公开成员函数）|
|[`recur_and_write`](recur_and_write.md)|消耗一个输入以在某个 _树位置_ 构造 _二叉树_ ，将递归的在其保有的 _树位置_ 继续构造 _二叉树_ <br>（公开成员函数）|

### 操作

|||
|:-|:-|
|[`left_rotate`<br>`right_rotate`](rotate.md)|无条件旋转某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|[`checked_left_rotate`<br>`checked_right_rotate`](rotate.md)|尝试旋转某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|

### 遍历

|||
|:-|:-|
|[`for_each`](for_each.md)|以未指定的顺序遍历并对每个元素调用所提供的函数<br>（公开成员函数）|

### 容量

|||
|:-|:-|
|[`empty`](empty.md)|返回 `binary_tree` 内部的 _树位置_ 上是否没有 _二叉树_ <br>（公开成员函数）|

### 通用工具

|||
|:-|:-|
|[`binary_tree_functional::get_children`](functional_get_children.md)|定制点对象，可统一对四种代理类型调用。|

### 非成员函数

|||
|:-|:-|
|[`Yc::swap`](adl_swap.md)|交换内容<br>（函数模板）|

## 另见

- [代理失效规则](proxy_lifetime.md)
- [返回值与所有权](ownership_and_return.md)
- [二叉树相关概念的阐述](easy_for_understand.md)
