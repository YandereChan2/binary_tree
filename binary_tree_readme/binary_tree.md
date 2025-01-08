# Yc::binary_tree

在文件 `"binary_tree"` 定义

```C++
template<
    class T,
    class Alloc = std::allocator<T>
>class binary_tree;
```

`Yc::binary_tree` 是对不反向关联父节点的二叉树数据结构的封装。

此处 _二叉树_ 的定义定义如下：
每一个 _二叉树_ 占据一个 _树位置_ ，一个 _树位置_ 或空闲，或存在一个 _二叉树_ 。一个 _二叉树_ 保有左右两个 _树位置_ 以及一个 `T` 类型的元素，两个 _树位置_ 上可以有也可以没有其他 _二叉树_ 。每个 `binary_tree` 对象内部保有一个 _树位置_ 。

当一个 _树位置_ 生存期结束时，其上的 _二叉树_ 要么提前转移至其他 _树位置_ ，要么提前结束。换言之，

在本库中，使用 `edge_proxy` 指代一个 _树位置_ ，使用 `node_proxy` 指代一个 _二叉树_ 。 _树位置_ 只能在空闲时结束其生存期，否则行为未定义。

## 模板参数

| | |
|-:|:-|
|`T`|元素的类型。 至少应当满足[_可擦除 (Erasable)_](https://zh.cppreference.com/w/cpp/named_req/Erasable)，但是一些成员函数提出了更高的要求。|
|`Alloc`|用于获取/释放内存及构造/析构内存中元素的分配器。类型必须满足[_分配器 (Allocator)_](https://zh.cppreference.com/w/cpp/named_req/Allocator)。|

## 成员类型

|成员类型|定义|
|:-|:-|
|`value_type`|`T`|
|`allocator_type`|`Alloc`|
|`edge_proxy`|表示 _树位置_ 的轻量级代理对象。|
|`edge_const_proxy`|表示 _树位置_ 的轻量级代理对象。对这个位置的树的元素提供只读访问|
|`node_proxy`|表示 _二叉树_ 的轻量级代理对象。|
|`node_const_proxy`|表示 _二叉树_ 的轻量级代理对象。对这个树的元素提供只读访问|

## 成员函数

|||
|:-|:-|
|（构造函数）|构造 `xor_list` <br>（公开成员函数）|
|（析构函数）|析构 `xor_list` <br>（公开成员函数）|
|`get_allocator`|返回关联的分配器<br>（公开成员函数）|

### 代理

|||
|:-|:-|
|`root`<br>`croot`|返回 `binary_tree` 对象内部的 _树位置_ 的代理。（公开成员函数）|
|`nroot`<br>`cnroot`|返回处于 `binary_tree` 对象内部的 _树位置_ 上的 _二叉树_ 的代理。（公开成员函数）|

### 修改器

|||
|:-|:-|
|`clear`|清除内容<br>（公开成员函数）|
|`insert`|在某 _树位置_ 插入元素，产生新的 _二叉树_ <br>（公开成员函数）|
|`emplace`|在某 _树位置_ 原位构造元素，产生新的 _二叉树_ <br>（公开成员函数）|
|`erase`|擦除 _二叉树_ <br>（公开成员函数）|
|`swap`|交换内容<br>（公开成员函数）|
|`cut`|取出某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|`swap_sub_tree`（静态）|交换两个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|`splice`|从其他 _树位置_ 转移 _二叉树_ <br>（公开成员函数）|
|`recur_and_write`<br>`recur_and_overwrite`|消耗一个输入以在某个 _树位置_ 构造 _二叉树_ ，将递归的在其保有的树位置继续构造 _二叉树_ <br>（公开成员函数）|

### 操作

|||
|:-|:-|
|`left_rotate`<br>`right_rotate`|无条件旋转某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|
|`checked_left_rotate`<br>`checked_right_rotate`|尝试旋转某个 _树位置_ 上的 _二叉树_ <br>（公开成员函数）|

### 非成员函数

|||
|:-|:-|
|`Yc::swap`|交换内容<br>（函数模板）|