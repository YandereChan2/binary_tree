# `Yc::basic_intrusive_binary_tree<vptr>::for_each` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::for_each`

```C++
template<class F>
F for_each(F f)noexcept(/* 见下文 */);
```

中序遍历整棵树，对每棵 _二叉树_ 的侵入式类型引用调用 `f`。

侵入式层不知道 `T`，因此 `f` 接收的是 _二叉树_ 的侵入式类型引用：

```C++
tree.for_each([](auto& n)
{
    auto& user = static_cast<my_binary_tree&>(n);
    // ...
});
```

## 参数

||||
|-:|-|:-|
|`f`|-|遍历函数对象，接收 _二叉树_ 的侵入式类型引用|

## 返回值

按值返回 `f`。与 `std::for_each` 一致，有状态函数对象可以把状态带出来：

```C++
auto acc = tree.for_each(accumulator{});
```

## 复杂度

与 _二叉树_ 的数量成线性关系。

## 异常

`noexcept` 规格等价于：

```C++
noexcept(std::is_nothrow_invocable_v<F&, node_base&> &&
         std::is_nothrow_move_constructible_v<F>)
```

`f` 抛出时异常正常向外传播，树的形状保持不变。`f` 不应改变树形。

## 备注

遍历顺序为中序。默认构造的函数对象在空树上原样返回。
