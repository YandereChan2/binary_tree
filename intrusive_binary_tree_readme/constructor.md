# `Yc::basic_intrusive_binary_tree<vptr>::basic_intrusive_binary_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::basic_intrusive_parent_aware_binary_tree`

```C++
basic_intrusive_binary_tree()noexcept; //(1)
```

```C++
basic_intrusive_binary_tree(basic_intrusive_binary_tree&& other)noexcept; //(2)
```

```C++
basic_intrusive_binary_tree(const basic_intrusive_binary_tree&) = delete; //(3)
```

可向上导航版本对应为：

```C++
basic_intrusive_parent_aware_binary_tree()noexcept;
basic_intrusive_parent_aware_binary_tree(
    basic_intrusive_parent_aware_binary_tree&& other)noexcept;
basic_intrusive_parent_aware_binary_tree(
    const basic_intrusive_parent_aware_binary_tree&) = delete;
```

- (1) 构造空树。
- (2) 移动构造：接管 `other` 的树，`other` 变为空树。各 _二叉树_ 的位置和父子关系不变；
  可向上导航版本会把根 _二叉树_ 的父修正为新树的 _虚拟二叉树_。
- (3) 不可复制。复制会让两个树对象指代同一组 _二叉树_。

## 参数

||||
|-:|-|:-|
|`other`|-|被移动的侵入式树|

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
Yc::basic_intrusive_binary_tree<> a;
Yc::basic_intrusive_binary_tree<> b{ std::move(a) };
assert(a.empty());
```

## 备注

侵入式树没有分配器参数；_二叉树_ 的创建与销毁由调用方负责。
