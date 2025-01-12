# `Yc::binary_tree<T,Allocator>::recur_and_write` , `Yc::binary_tree<T,Allocator>::recur_and_overwrite`

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

```C++
template<
	class ValueGetter,
	class ChildrenGetter,
	class InitializeHandle
>
void recur_and_overwrite(
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
		) //(2)
```

尝试在 `p` 所指代的 _树位置_ 建立 _二叉树_ ，消耗资源句柄 `h` 进行递归式构建，使用 `vg` 获取句柄所指向的值，使用 `cg` 获取和 `h` 关联的两个子资源的句柄。

- (1) 会将此 _树位置_ 原有的 _二叉树_ 通过返回值转移至外部。
- (2) 会在不转移 _二叉树_ 的情况下尽量进行原地修改，但是仍然会有销毁或者创建的动作。

设仅用于阐释的内部函数 _`recur_and_write_impl(edge_const_proxy p, ValueGetter& vg, ChildrenGetter& cg, InitializeHandle& h)`_，函数 (1) 会在进行裁剪动作后把参数转给该函数。

_`recur_and_write_impl`_ 会首先把传入的句柄 `h` [按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，如果为 `false` 则立即返回，否则首先使用 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 转发给构造函数在 `p` 位置创建新的 _二叉树_ ，然后使用[结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) [`auto [lh,rh] = std::invoke(cg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 来获取和 `h` 相关联的子资源，并递归地使用 _`recur_and_write_impl`_ 在该 _二叉树_ 内部的左右两个树位置消耗 `lh` 和 `rh` 创建 _二叉树_ 。

对于函数 (2) 的操作类似，只是不会转移 _二叉树_ ，而是在`h` 按语境转换成 `bool` 得到 `true` 时按照实际情况进行赋值或者原地析构再构造，在 `false` 时使用 [`erase(p)`](erase.md) 销毁该 _树位置_ 的 _二叉树_ 。

函数 (2) 在遇到 _树位置_ 存在 _二叉树_ 时会首先尝试使用赋值运算符修改该位置的元素，如果该调用非良构或者潜在抛出，那么会尝试构造临时对象并调用赋值运算符，如果该过程非良构或者潜在抛出，那么会尝试析构并原位构造元素，如果该过程非良构或者潜在抛出那么程序非良构。

## 参数

||||
|-:|-|:-|
|`p`|-|想要建立新 _二叉树_ 的 _树位置_ |
|`vg`|-|获取资源值的函数对象。|
|`cg`|-|获取关联子资源的函数对象|
|`h`|-|起始资源句柄|

### 类型要求

- 要使用函数 (1) ，`InitializeHandle` 必须可以[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，并且使用转换为 `false` 代表资源无效。并且上文提到的各个函数调用必须良构，并且 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 可以用于构造 `T` 类型的对象。

- 要使用函数 (2) ，`InitializeHandle` 必须可以[按语境转换](https://zh.cppreference.com/w/cpp/language/implicit_conversion)成 `bool` ，并且使用转换为 `false` 代表资源无效。并且上文提到的各个函数调用必须良构。并且 [`std::invoke(vg, h)`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) 可以用于构造 `T` 类型的对象，且该构造必须不抛出。

需要注意的是，目前的技术无法为[结构化绑定](https://zh.cppreference.com/w/cpp/language/structured_binding) 失败提供友好的报错。

## 返回值

(1) 返回转移出的 _二叉树_ 。

(2) （无）

## 异常

若抛出了异常（例如由构造函数），则构造操作会停止。保证 `binary_tree` 处于一个合法但未指定的状态，并且不会随意隐瞒或改动抛出的异常（[基本异常保证且异常中立](https://zh.cppreference.com/w/cpp/language/exceptions)）。并且保证所有原先不从 `p` 可达的 _二叉树_ 不被修改。

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。
