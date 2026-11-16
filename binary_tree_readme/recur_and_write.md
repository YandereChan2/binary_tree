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

若句柄 `h` [按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` 为 `false` ，则不构造任何新元素；但函数仍会先把 `p` 处原有的 _二叉树_ 切出，并由返回值带回，因此 `p` 会变为空 _树位置_ 。否则在 `p` 位置用 `vg(h)` 得到值构造元素，再用 `cg(h)` 取得与 `h` 关联的两个子资源句柄，并对它们各自递归地做同样的事。

两个子资源对应的 _树位置_ 之间的处理顺序**未指定**，调用方不应依赖它。

## 前置条件

- `p` 必须是**合法**的 _树位置_ ，并且从 `*this` 的 [`root()`](root.md) 可达；
  否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

|||
|-:|:-|
|`p`|想要建立新 _二叉树_ 的 _树位置_|
|`vg`|获取资源值的函数对象。|
|`cg`|获取关联子资源的函数对象|
|`h`|起始资源句柄|

### 类型要求

- 要使用函数 (1) ，`InitializeHandle` 必须可以[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，并且使用转换为 `false` 代表资源无效。并且上文提到的各个函数调用必须良构，并且 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 可以用于构造 `T` 类型的对象。

若 `cg` 的返回类型不满足上述要求，编译器给出的报错可能不够友好。

## 返回值

(1) 返回被切出的原子树，**返回值拥有它**。如果丢弃返回值，返回的临时
`binary_tree` 会销毁这棵原子树；只有在保留返回值时，原子树中的代理和引用才继续有效。

## 异常

若抛出了异常（例如由构造函数），则构造操作会停止，保证 `binary_tree` 回滚到之前的状态。

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。
