# `Yc::basic_intrusive_binary_tree<vptr>::erase` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::erase`

```C++
template<class Deleter>
static void erase(edge_proxy e, Deleter del)noexcept;
```

拆掉 `e` 所指代的 _树位置_ 上的整棵子树，并对每棵被摘下的 _二叉树_ 调用
`del(node_proxy)`。

- 调用 `del` 之前，该 _二叉树_ 已经与左右子 _二叉树_ 断开；
- 可向上导航版本还会先把该 _二叉树_ 与父 _二叉树_ 断开，因此回收器看到的是孤立的
  _二叉树_。

## 参数

||||
|-:|-|:-|
|`e`|-|要拆掉的子树所在的 _树位置_|
|`del`|-|回收器，接收该 _二叉树_ 的代理；必须 `noexcept`|

## 返回值

（无）

## 复杂度

时间与子树中 _二叉树_ 的数量成线性关系。不使用额外容器。

## 异常

（无）。`del` 必须不抛出异常，否则行为未定义（函数本身标记为 `noexcept`）。

## 示例

```C++
tree.erase(tree.root().get_right(), [](auto p)
{
    auto& n = static_cast<my_binary_tree&>(*p);
    destroy_binary_tree(n); // 销毁用户数据
});
```

## 备注

`erase` 只负责拆解树并调用 `del`；_二叉树_ 的销毁由调用方负责。
需要"只拆树、不销毁 _二叉树_"时使用 [`clear`](clear.md) 或传入空回收器。
