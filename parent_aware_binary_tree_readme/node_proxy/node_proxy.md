# `Yc::parent_aware_binary_tree<T, Allocator>::node_proxy` , `Yc::parent_aware_binary_tree<T, Allocator>::node_const_proxy`

指代一个 _二叉树_ 或者 _虚拟二叉树_ ，并提供对该 _二叉树_ 保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。

`node_proxy` 可以[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)到 `node_const_proxy` ,反过来不行。

和 `Yc::binary_tree` 不同，`edge_proxy` 和 `edge_const_proxy` 与 `node_proxy` 和 `node_const_proxy` 可以互相[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)。

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T` 或 `const T`|

## 成员函数

下文中用 `proxy` 表示具体是哪个类型。

### null

```C++
bool null()const noexcept;
```

该类型是否不指代 _二叉树_ 或 _虚拟二叉树_ 。默认构造的，或者尝试从无 _二叉树_ 的 _树位置_ 获取的，都会导致这一点。注意， `node_proxy` 没有是否合法之说。

这种代理对象的失效规则更加简单，非常适合在需要元素引用以及元素关联关系时使用它。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)。

#### 可能的实现

```C++
explicit operator bool()const noexcept
{
    return !null();
}
```

### go_left, go_right

```C++
proxy& go_left()noexcept;
```

```C++
proxy& go_right()noexcept;
```

将代理对象转移至所指代的 _二叉树_ 所保有的左右 _树位置_ 上的 _二叉树_ 。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。如果对应的 _树位置_ 上没有 _二叉树_ ，那么会使其 `null() == true` 。其返回 `*this` 以支持连续调用。

如果代理对象指代 _虚拟二叉树_ ，那么调用 `go_right` 行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### go_up

```C++
proxy& go_up()noexcept;
```

将代理对象转移至提供其所指代的 _二叉树_ 所占据的 _树位置_ 的 _二叉树_ 或者 _虚拟二叉树_ 上。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` 或者指代 _虚拟二叉树_ 那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。其返回 `*this` 以支持连续调用。

### operator*, operator->

```C++
T& operator*()const noexcept;
```

```C++
T* operator->()const noexcept;
```

运算符重载，以提供访问其指代的 _二叉树_ 所保有的元素的能力。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` 或者指代 _虚拟二叉树_ ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### get_left, get_right

```C++
proxy get_left()const noexcept;
```

```C++
proxy get_right()const noexcept;
```

返回指代其所指代的 _二叉树_ 所保有的左右 _树位置_ 上的 _二叉树_ 的代理对象。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

如果代理对象指代 _虚拟二叉树_ ，那么调用 `get_right` 行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### get_parent

```C++
proxy get_parent()const noexcept;
```

返回指代提供其所指代的 _二叉树_ 所占据的 _树位置_ 的 _二叉树_ 或者 _虚拟二叉树_ 的代理对象。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true`或者指代 _虚拟二叉树_ 那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

### get_children

```C++
std::pair<edge(_const)_proxy, edge(_const)_proxy> get_children()const noexcept;
```

获取指代其指代的 _二叉树_ 所保有的左右 _树位置_ 的两个 [_`edge`_](../edge_proxy/edge_proxy.md) 代理对象。如果其不[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `true` 或者指代虚拟二叉树 ，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当其都不指代 _二叉树_ ，或指代同一个 _二叉树_ 时返回 `true` 。
