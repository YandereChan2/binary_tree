# `Yc::binary_tree<T, Allocator>::for_each`

```C++
template<class F>
F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&& std::is_nothrow_move_constructible_v<F>); //(1)
```

```C++
template<class F>
F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&& std::is_nothrow_move_constructible_v<F>); //(2)
```

中序遍历 `binary_tree` 对象内部的 _树位置_ 上的 _二叉树_ ，对每个 _二叉树_ 保有的元素调用 `f` ，调用顺序为左 _树位置_ 上的 _二叉树_ 、本 _二叉树_ 、右 _树位置_ 上的 _二叉树_ 。

- (1) 非 const 版本，`f` 收到 `T&` 。
- (2) const 版本，`f` 收到 `const T&` 。

`f` 不应改变树的结构（插入、擦除、旋转等），否则遍历位置会失效，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

本函数是条件性 `noexcept` 的：仅当 `f` 可以无异常地接受对应类型的左值、且 `f` 可以无异常地移动构造时 `noexcept` 为 `true` ，否则为 `false` 。

该函数主要用于容器在销毁节点之前做收尾工作：例如哈希表需要先遍历所有桶，把其中的元素从桶的链表上摘下来，再调用 [`clear`](clear.md) 销毁桶节点。

## 参数

||||
|-:|-|:-|
|`f`|-|要对每个元素调用的函数对象。按值传入（与 `std::for_each` 一致），因此有状态函数对象会在副本上累积状态。|

## 返回值

遍历结束后 `f` 的副本（按值返回，与 `std::for_each` 一致）。因此有状态函数对象可以带着累积结果出来：

```C++
struct accumulator
{
    int sum = 0;
    void operator()(int& v) noexcept { sum += v; }
};

auto acc = t.for_each(accumulator{});
// acc.sum 就是整棵树的和
```

传入的那个函数对象本身不会被修改（函数内部操作的是它的副本）。

## 复杂度

与遍历的 _二叉树_ 的数量成正比。

## 异常

本函数是条件性 `noexcept` 的。`f` 抛出异常时，异常向调用方传播（树保持完整，因为遍历不改结构；`f` 的副本随栈展开销毁）；此时本函数的 `noexcept` 为 `false` ，因为 `f` 不能无异常地接受对应类型的左值。若 `f` 的移动构造可能抛出，本函数的 `noexcept` 同样为 `false`。
