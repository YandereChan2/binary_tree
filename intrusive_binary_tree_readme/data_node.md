# `Yc::intrusive_binary_tree_node<vptr, T>` , `Yc::intrusive_parent_aware_binary_tree_node<vptr, T>`

同时存放一个 `T` 与侵入式链接的类型。创建、销毁和生命周期管理由调用方负责。

```C++
template<class vptr, class T>
class intrusive_binary_tree_node
    : public intrusive_binary_tree_node_base<vptr>;

template<class vptr, class T>
class intrusive_parent_aware_binary_tree_node
    : public intrusive_parent_aware_binary_tree_node_base<vptr>;
```

## 成员函数

### 构造函数

```C++
intrusive_binary_tree_node() = default;
```

默认构造只建立一棵**脱管**的 _二叉树_；`T` 尚未构造。

### construct_value

```C++
template<class Alloc, class... Args>
void construct_value(Alloc& alloc, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>);
```

在该对象内构造 `T`。`alloc` 用于 `T` 的构造。

### destroy_value

```C++
template<class Alloc>
void destroy_value(Alloc& alloc)noexcept;
```

析构该对象内的 `T`。调用前必须已经 `construct_value` 过。

### value

```C++
T& value()noexcept;
const T& value()const noexcept;
```

访问该对象内的 `T`。`T` 未构造时行为未定义。

## 生命周期

`T` 的构造 / 析构与把该 _二叉树_ 挂到树上 / 从树上摘下是分开的：

1. 创建该类型的对象（此时 _二叉树_ 脱管）；
2. 用 `construct_value` 构造 `T`；
3. 把该 _二叉树_ 挂到树上（纳管）；
4. 用 [`erase`](erase.md) / [`clear`](clear.md) 从树上摘下，使其重新脱管；
5. 用 `destroy_value` 析构 `T`；
6. 销毁该对象。

若第 6 步之前 _二叉树_ 仍处于纳管状态且还有孩子，析构会导致未定义行为。
纳管但没有孩子的 _二叉树_ 可以析构。

## 复杂度

`construct_value` / `destroy_value` 的复杂度取决于 `T` 的构造函数 / 析构函数。

## 异常

`construct_value` 可能抛出 `T` 构造函数产生的异常；此时 `T` 未构造，该对象仍可正常销毁。
`destroy_value` 为 `noexcept`。
