# `Yc::binary_tree<T,Allocator>::operator=`

```C++
binary_tree& operator=(const binary_tree& b); //(1)
```

```C++
binary_tree& operator=(binary_tree&& b)noexcept(/* 见下文 */); //(2)
```

以 `b` 的内容替换 `*this` 的内容。

- (1) 拷贝赋值运算符。递归复制 `b` 保有的所有 _二叉树_ 到 `*this` 。提供强异常安全保障：若复制过程中抛出异常，`*this` 会回退到赋值前的状态。若 `*this` 与 `b` 是同一个对象，则不做任何操作，`*this` 保持不变（自赋值是安全的，不会清空树）。
- (2) 移动赋值运算符。若分配器可以传播（POCMA 为 `true`）或两个分配器比较相等，则直接交换两棵树的根和分配器，复杂度常数；否则通过递归移动 `b` 中的元素构造一棵新树，再与本树交换。

无论走哪条分支，操作后 `b` 都处于**有效但未指定**的状态：它可能变为空树，也可能仍
持有自己的旧树（元素已被移动），或在交换分支中持有 `*this` 原有的树。不要假设 `b`
为空；若需要确定状态，请在移动后重新赋值或 `clear()`。

## 分配器感知

若 [`std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits) 为 `true` ，则 (1) 会将源分配器复制到 `*this` 。否则不替换分配器。

若 [`std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits) 为 `true` ，则 (2) 会交换分配器。若为 `false` 且分配器不相等，则通过递归移动构造完成赋值。

## 参数

|||
|-:|:-|
|`b`|要赋值其所含 _二叉树_ 的 `binary_tree` 对象|

## 返回值

`*this` 。

## 复杂度

- (1) 与创建的 _二叉树_ 数量成正比（即 O(n)，n 为 `b` 中的节点数）。
- (2) 常数（分配器传播或相等时）或与 `b` 中的节点数成正比（分配器不传播且不相等时）。

## 异常

- (1) 如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。元素复制也可能产生异常。提供强异常安全保障。
- (2) 在 [`std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits) `||` [`std::allocator_traits<Alloc>::is_always_equal::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits) 为 `true` 时 `noexcept` 。
