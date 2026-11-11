# `Yc::basic_intrusive_binary_tree<vptr>::swap_node` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap_node`

```C++
static void swap_node(edge_proxy l, edge_proxy r)noexcept;
```

交换两棵 _二叉树_。净效果是**两个 _树位置_ 上的值互换、子树留在原位、树的形状不变**。

- 不可向上导航版本：交换两个 _树位置_ 上的 _二叉树_，并交换它们的左右子树；
- 可向上导航版本：同上，并在每一步维护父子关系。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代两棵存在的 _二叉树_ 的 _树位置_|

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
tree.swap_node(tree.root(), tree.root().get_left());
```

## 备注

- 两个 _树位置_ 上都必须有 _二叉树_；否则行为未定义。
- 父子相邻的情形也可以直接交换。需要可逆操作时，请重新取得代理后再调用；
  对同一对已经交换过的位置重复调用不保证恢复原状。
- 本函数不复制、不销毁 _二叉树_。
