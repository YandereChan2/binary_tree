# `Yc::binary_tree<T,Allocator>::emplace`

```C++
template<class... Args>
binary_tree emplace(edge_const_proxy p, Args&&... args);
```

在 `p` 所指代的 _树位置_ 建立一个 _二叉树_ ，并用 [`std::allocator_traits::construct`](https://zh.cppreference.com/w/cpp/memory/allocator_traits/construct) 构造元素，常用布置 `new` 在相应的位置原位构造元素。将实参 `args...` 作为 [`std::forward<Args>(args)...`](http://zh.cppreference.com/w/cpp/utility/forward) 转发给构造函数。 `args...` 可以直接或间接地指代 `*this` 中的值。不会在新 _二叉树_ 保有的 _树位置_ 上继续建立 _二叉树_ 。

如果 `p` 所指代的 _树位置_ 原先就有一个 _二叉树_ 那么将其通过返回值转移至 `*this` 外部。

如果 `p` 不从 `root()` 可达，那么行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

任何既有代理对象都不会失效。

## 参数

||||
|-:|-|:-|
|`p`|-|想要建立新 _二叉树_ 的 _树位置_ |
|`args`|-|转发给元素构造函数的实参。|

### 类型要求

- `T`（元素类型） 必须满足[_可就位构造 (EmplaceConstructible)_](https://zh.cppreference.com/w/cpp/named_req/EmplaceConstructible) 。

## 返回值

原先该 _树位置_ 上的 _二叉树_ 。如果原先该 _树位置_ 无 _二叉树_ ，那么返回一个空的 `binary_tree` 对象。

## 复杂度

常数。

## 异常

若抛出了异常（例如由构造函数），则保留 `binary_tree` 不修改，如同未曾调用过此函数（强异常保证）。

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。
