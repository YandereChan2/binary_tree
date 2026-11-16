# `Yc::basic_intrusive_binary_tree<vptr>::basic_intrusive_binary_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::basic_intrusive_parent_aware_binary_tree`

```C++
// basic_intrusive_binary_tree<vptr>
basic_intrusive_binary_tree()noexcept; //(1)
basic_intrusive_binary_tree(basic_intrusive_binary_tree&& other)noexcept; //(2)
basic_intrusive_binary_tree(const basic_intrusive_binary_tree&) = delete; //(3)
```

```C++
// basic_intrusive_parent_aware_binary_tree<vptr>
basic_intrusive_parent_aware_binary_tree()noexcept; //(1)
basic_intrusive_parent_aware_binary_tree(
    basic_intrusive_parent_aware_binary_tree&& other)noexcept; //(2)
basic_intrusive_parent_aware_binary_tree(
    const basic_intrusive_parent_aware_binary_tree&) = delete; //(3)
```

- (1) 构造空树。默认构造只建立空的根槽位 / 哨兵，不分配任何节点。
- (2) 移动构造：接管 `other` 的树，`other` 变为空树。各 _二叉树_ 的位置和父子关系
  不变；可向上导航版本会把根 _二叉树_ 的父修正为新树的 _虚拟二叉树_。
- (3) 不可复制。复制会让两个树对象指代同一组 _二叉树_。

## 参数

|||
|-:|:-|
|`other`|被移动的侵入式树|

## 复杂度

常数。

## 异常

（无）。两个移动构造都标记为 `noexcept`。

## 示例

```C++
Yc::basic_intrusive_binary_tree<> a;
Yc::basic_intrusive_binary_tree<> b{ std::move(a) };
assert(a.empty());
```

## 备注

侵入式树没有分配器参数；_二叉树_ 的创建与销毁由调用方负责。移动构造不会清理目标
（目标本来就是新对象），这与移动赋值不同：移动赋值会先 `clear()` 目标树。
