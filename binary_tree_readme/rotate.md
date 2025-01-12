# `Yc::binary_tree<T,Allocator>::left_rotate`,`Yc::binary_tree<T,Allocator>::right_rotate`,`Yc::binary_tree<T,Allocator>::checked_left_rotate`,`Yc::binary_tree<T,Allocator>::checked_right_rotate`

```C++
void left_rotate(edge_const_proxy p)noexcept //(1)
```

```C++
void right_rotate(edge_const_proxy p)noexcept //(2)
```

```C++
bool checked_left_rotate(edge_const_proxy p)noexcept //(3)
```

```C++
bool checked_right_rotate(edge_const_proxy p)noexcept //(4)
```

- (1,2) 无条件在 `p` 所指代的 _树位置_ 执行旋转操作。
- (3,4) 尝试进行旋转，如果旋转的先决条件不满足则返回 `false` ，否则返回 `true`。

```
     |                         |
    <A>                       <C>
   /   \                      / \
  <B>  <C>     =====>       <A> <E>
       / \                  / \
     <D> <E>              <B> <D>
```

旋转操作的语义如图所示（图中展示的是左旋）。旋转的先决条件指的是旋转前， A 所在的 _树位置_ 和 C 所在的 _树位置_ 必须有 _二叉树_ 。

在不能旋转的地方进行旋转会导致[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`p`|-|想要进行旋转的 _树位置_ 。|

## 返回值

- (1,2) （无）
- (3,4) 旋转能否进行。

## 复杂度

常数

## 异常

（无）
