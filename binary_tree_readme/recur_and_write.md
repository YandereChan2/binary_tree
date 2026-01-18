# `Yc::binary_tree<T,Allocator>::recur_and_write`

```C++
template<
    class ValueGetter,
    class ChildrenGetter,
    class InitializeHandle
>
binary_tree recur_and_write(
    edge_const_proxy p,
    ValueGetter vg,
    ChildrenGetter cg,
    InitializeHandle h
) //(1)
```

尝试在 `p` 所指代的 _树位置_ 建立 _二叉树_ ，消耗资源句柄 `h` 进行递归式构建，使用 `vg` 获取句柄所指向的值，使用 `cg` 获取和 `h` 关联的两个子资源的句柄。

- (1) 会将此 _树位置_ 原有的 _二叉树_ 通过返回值转移至外部。

设仅用于阐释的内部函数 _`recur_and_write_impl(edge_const_proxy p, ValueGetter& vg, ChildrenGetter& cg, InitializeHandle& h)`_，函数 (1) 会在进行裁剪动作后把参数转给该函数。

_`recur_and_write_impl`_ 会首先把传入的句柄 `h` [按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，如果为 `false` 则立即返回，否则首先使用 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 转发给构造函数在 `p` 位置创建新的 _二叉树_ ，然后使用[结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) [`auto [lh,rh] = std::invoke(cg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 来获取和 `h` 相关联的子资源，并递归地使用 _`recur_and_write_impl`_ 在该 _二叉树_ 内部的左右两个树位置消耗 `lh` 和 `rh` 创建 _二叉树_ 。

## 参数

||||
|-:|-|:-|
|`p`|-|想要建立新 _二叉树_ 的 _树位置_ |
|`vg`|-|获取资源值的函数对象。|
|`cg`|-|获取关联子资源的函数对象|
|`h`|-|起始资源句柄|

### 类型要求

- 要使用函数 (1) ，`InitializeHandle` 必须可以[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，并且使用转换为 `false` 代表资源无效。并且上文提到的各个函数调用必须良构，并且 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 可以用于构造 `T` 类型的对象。

需要注意的是，目前的技术无法为[结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) 失败提供友好的报错。

## 返回值

(1) 返回转移出的 _二叉树_ 。

## 异常

若抛出了异常（例如由构造函数），则构造操作会停止，保证 `binary_tree` 回滚到之前的状态。

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。

