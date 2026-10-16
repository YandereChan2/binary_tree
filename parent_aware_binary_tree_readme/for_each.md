# `Yc::parent_aware_binary_tree<T, Allocator>::for_each`

```C++
template<class F>
void for_each(F&& f)noexcept(std::is_nothrow_invocable_v<F&, T&>); //(1)
```

```C++
template<class F>
void for_each(F&& f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>); //(2)
```

中序遍历 `parent_aware_binary_tree` 对象内部的 _虚拟二叉树_ 的左 _树位置_ 上的 _二叉树_ ，对每个 _二叉树_ 保有的元素调用 `f` ，调用顺序为左 _树位置_ 上的 _二叉树_ 、本 _二叉树_ 、右 _树位置_ 上的 _二叉树_ 。

- (1) 非 const 版本，`f` 收到 `T&` 。
- (2) const 版本，`f` 收到 `const T&` 。

`f` 不应改变树的结构（插入、擦除、旋转等），否则遍历位置会失效，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

本函数是条件性 `noexcept` 的：仅当 `f` 可以无异常地接受对应类型的左值时 `noexcept` 为 `true` ，否则为 `false` 。

该函数主要用于容器在销毁节点之前做收尾工作：例如哈希表需要先遍历所有桶，把其中的元素从桶的链表上摘下来，再调用 [`clear`](clear.md) 销毁桶节点。

## 参数

||||
|-:|-|:-|
|`f`|-|要对每个元素调用的函数对象。|

## 返回值

（无）

## 复杂度

与遍历的 _二叉树_ 的数量成正比。

## 异常

本函数是条件性 `noexcept` 的。`f` 抛出异常时，异常向调用方传播；此时本函数的 `noexcept` 为 `false` ，因为 `f` 不能无异常地接受对应类型的左值。
