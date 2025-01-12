# Yc::binary_tree<T, Allocator>::swap

```C++
void swap(binary_tree& other)noexcept
```

交换保有的 _二叉树_ ，语义上相当于 [`swap_sub_tree(root(), other.root())`](swap_sub_tree.md) ，但是略有区别。

如果 `get_allocator() != other.get_allocator()` ，并且[`std::allocator_traits<Alloc>::propagate_on_container_swap::value`](https://zh.cppreference.com/w/cpp/memory/allocator_traits) 为 `false` ，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

想要与之交换的 `binary_tree` 对象。

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）
