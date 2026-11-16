# `Yc::binary_tree<T,Allocator>::swap`

```C++
void swap(binary_tree& other)noexcept
```

交换保有的 _二叉树_ 。语义上相当于
[`swap_sub_tree(root(), other.root())`](swap_sub_tree.md)，但两者有一个关键区别：

- `swap` 在 `std::allocator_traits<Alloc>::propagate_on_container_swap::value` 为
  `true` 时**同时交换分配器**；因此即使两个分配器不相等，只要允许传播，交换后每棵树
  仍然用自己持有的分配器释放节点。
- `swap_sub_tree` 只交换槽位，不交换分配器；两个分配器不相等且不传播时，交换会让
  节点最终由错误的分配器释放，因此是未定义行为。

如果 `get_allocator() != other.get_allocator()`，并且
[`std::allocator_traits<Alloc>::propagate_on_container_swap::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits)
为 `false` ，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

|||
|-:|:-|
|`other`|想要与之交换的 `binary_tree` 对象。|

## 返回值

（无）。

## 复杂度

常数。

## 异常

（无）。
