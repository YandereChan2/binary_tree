# `Yc::basic_intrusive_binary_tree<vptr>::operator=` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::operator=`

```C++
basic_intrusive_binary_tree& operator=(
    basic_intrusive_binary_tree&& other)noexcept; //(1)
```

```C++
basic_intrusive_binary_tree& operator=(
    const basic_intrusive_binary_tree&) = delete; //(2)
```

可向上导航版本对应为：

```C++
basic_intrusive_parent_aware_binary_tree& operator=(
    basic_intrusive_parent_aware_binary_tree&& other)noexcept;
basic_intrusive_parent_aware_binary_tree& operator=(
    const basic_intrusive_parent_aware_binary_tree&) = delete;
```

- (1) 移动赋值：接管 `other` 的树，`other` 变为空树。
  - 不可向上导航版本：本树原来的 _二叉树_ 不再由本树指代；它们之间可能仍然保持父子关系，
    因此调用前应确保本树为空，或先调用 [`clear`](clear.md)。
  - 可向上导航版本：先拆掉本树原来的树，再接管 `other` 的根；被拆下的 _二叉树_ 不会销毁。
- (2) 不可复制赋值。

## 参数

||||
|-:|-|:-|
|`other`|-|被移动的侵入式树|

## 返回值

`*this`

## 复杂度

常数。

## 异常

（无）

## 备注

拥有型容器不会依赖本移动赋值来转移所有权；它们使用 [`swap`](swap.md) 或自己的
复制 / 移动逻辑来正确管理 _二叉树_ 的生命周期。
