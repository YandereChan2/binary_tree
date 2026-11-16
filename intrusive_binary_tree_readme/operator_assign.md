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

- (1) 移动赋值：先调用 `clear()` 拆掉本树原有的树（让其中所有 _二叉树_ 脱管，但
  不销毁它们），再接管 `other` 的根；`other` 变为空树。
- (2) 不可复制赋值。

两个版本的步骤相同，区别只在可向上导航版本会额外把新根 _二叉树_ 的父指回本树的
_虚拟二叉树_。

## 参数

|||
|-:|:-|
|`other`|被移动的侵入式树|

## 返回值

`*this` 。

## 复杂度

与**本树原有** _二叉树_ 的数量成线性关系（`clear()` 的代价）；接管 `other` 本身是
常数时间。因此总复杂度不是常数。

## 异常

（无）。两个移动赋值都标记为 `noexcept`；`clear()` 只改链接，不销毁节点。

## 备注

拥有型容器不会依赖本移动赋值来转移所有权；它们使用 [`swap`](swap.md) 或自己的
复制 / 移动逻辑来正确管理 _二叉树_ 的生命周期。移动赋值会先拆掉目标树，这一点与
移动构造不同：移动构造只接管源树，不清理目标（目标本来就是空树）。
