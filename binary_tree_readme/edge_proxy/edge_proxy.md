# `Yc::binary_tree<T, Allocator>::edge_proxy` , `Yc::binary_tree<T, Allocator>::edge_const_proxy`

指代一个 _树位置_ ，并提供对该 _树位置_ 上的 _二叉树_ 保有的元素的访问。

它是 [_可默认构造 (DefaultConstructible)_](https://zh.cppreference.com/w/cpp/named_req/DefaultConstructible) 且 [_可平凡复制 (TriviallyCopyable)_](https://zh.cppreference.com/w/cpp/named_req/TriviallyCopyable) 的轻量代理对象。

`edge_proxy` 可以[隐式转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)到 `edge_const_proxy` ,反过来不行。

这种代理对象会在相应的 _树位置_ 失效时失效，这种失效规则更加复杂。长期保有这种代理并且期间树的结构有变动时可能会导致令人困惑的结果。但是各种树操作依赖这种代理对象。如果希望得到更稳定的元素代理，请转换成 [`node_proxy`](../node_proxy/node_proxy.md) 或 [`node_const_proxy`](../node_proxy/node_proxy.md) 。

## 成员类型

|成员类型|定义|
|-:|:-|
|`value_type`|`T` 或 `const T`|

## 成员函数

下文中用 `proxy` 表示具体是哪个类型。

### valid

```C++
bool valid()const noexcept;
```

该类型是否合法指代一个 _树位置_ 。只有默认构造的这种对象返回 `false` 。**注意，这里的**_**合法**_**和上文的**_**失效**_**不是一回事。**

### null

```C++
bool null()const noexcept;
```

该类型指代的 _树位置_ 上面是否没有 _二叉树_ 。注意，在 `valid() == false` 时调用该函数会引发未定义行为。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的按语境转换，只有在既有效又在所指代的 _树位置_ 上有 _二叉树_ 的时候返回 `true` 。

#### 可能的实现

```C++
explicit operator bool()const noexcept
{
    return valid() && (!null());
}
```

### go_left, go_right

```C++
proxy& go_left()noexcept;
```

```C++
proxy& go_right()noexcept;
```

将代理对象转移至所指代的 _树位置_ 上的 _二叉树_ 所保有的左右 _树位置_ 上。如果其不按语境转换成 `true` ，那么行为未定义。其返回 `*this` 以支持连续调用。

### operator*, operator->

```C++
T& operator*()const noexcept;
```

```C++
T* operator->()const noexcept;
```

运算符重载，以提供访问其指代的 _树位置_ 上的 _二叉树_ 所保有的元素的能力。如果其不按语境转换成 `true` ，那么行为未定义。

### get_children

```C++
std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> get_children()const noexcept;
```

获取指代其指代的 _树位置_ 上的 _二叉树_ 所保有的左右 _树位置_ 的两个代理对象。如果其不按语境转换成 `true` ，那么行为未定义。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当其都无效，或指代同一个 _树位置_ 时返回 `true` 。

## 辅助类

### std::hash<edge_proxy>,std::hash<edge_const_proxy>

对于代理对象[启用](https://zh.cppreference.com/w/cpp/utility/hash)的 [`std::hash`](https://zh.cppreference.com/w/cpp/utility/hash) 特化。保证对相等的 `edge_proxy` 与 `edge_const_proxy` 返回相同值。
