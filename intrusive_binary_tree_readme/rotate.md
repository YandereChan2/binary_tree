# `Yc::basic_intrusive_binary_tree<vptr>::left_rotate` , `...::right_rotate` , `...::checked_left_rotate` , `...::checked_right_rotate` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::...`

```C++
static void left_rotate(edge_proxy e)noexcept; //(1)
```

```C++
static void right_rotate(edge_proxy e)noexcept; //(2)
```

```C++
static bool checked_left_rotate(edge_proxy e)noexcept; //(3)
```

```C++
static bool checked_right_rotate(edge_proxy e)noexcept; //(4)
```

- (1,2) 无条件在 `e` 所指代的 _树位置_ 上旋转；
- (3,4) 检查先决条件：不满足返回 `false`，满足则旋转并返回 `true`。

```plain text
     |                         |
    <A>                       <C>
   /   \                      / \
  <B>  <C>     =====>       <A> <E>
       / \                  / \
     <D> <E>              <B> <D>
```

图中展示左旋。左旋要求 A 所在 _树位置_ 与 A 的右子 _二叉树_ C 所在的 _树位置_
都有 _二叉树_；右旋是镜像操作。可向上导航版本在旋转后同步维护父子关系。

在不能旋转的位置调用无条件版本会导致未定义行为。

## 参数

||||
|-:|-|:-|
|`e`|-|想要进行旋转的 _树位置_|

## 返回值

- (1,2) （无）
- (3,4) 旋转能否进行。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
tree.left_rotate(tree.root());

if (!tree.checked_right_rotate(tree.root().get_left()))
{
    // 左子 _二叉树_ 没有左子树，未旋转
}
```
