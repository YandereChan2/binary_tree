# `Yc::basic_intrusive_binary_tree<vptr>::erase` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::erase`

```C++
static void erase(edge_proxy e)noexcept; //(1)
```

```C++
template<class Deleter>
static void erase(edge_proxy e, Deleter del)noexcept; //(2)
```

- (1) 拆掉 `e` 所指代的 _树位置_ 上的整棵子树，并让其中的所有 _二叉树_ 脱管；
- (2) 同样拆掉子树并让所有 _二叉树_ 脱管，然后对每棵 _二叉树_ 调用
  `del(node_pointer)`。

对于 (2)：

- 调用 `del` 之前，该 _二叉树_ 已经**脱管**：所有链接字段都指向自身；
- 因此 `del` 可以安全地销毁它；用户代码不能在 `del` 内重新 `insert`、`cut`、`erase`
  或导航这棵 _二叉树_，也不能改变它的状态；
- `del` 只负责销毁或记录；树的销毁不由本函数完成；
- 子树中的所有 _二叉树_ 都会依次经历这个过程，回收器看到的始终是脱管的
  _二叉树_。

## 参数

||||
|-:|-|:-|
|`e`|-|要拆掉的子树所在的 _树位置_|
|`del`|-|回收器，接收指向该 _二叉树_ 的 `node_pointer`；必须 `noexcept`|

## 返回值

（无）

## 复杂度

时间与子树中 _二叉树_ 的数量成线性关系。不使用额外容器。

## 异常

（无）。`del` 必须不抛出异常，否则行为未定义（函数本身标记为 `noexcept`）。

## 示例

```C++
// 只脱管，不销毁
tree.erase(tree.root().get_right());

// 脱管后交给回收器销毁
tree.erase(tree.root().get_left(), [](auto p)
{
    assert(p->is_detached());
    auto& n = static_cast<my_binary_tree&>(*p);
    destroy_binary_tree(n); // 此时销毁是安全的
});
```

## 备注

`erase` 只负责拆解树、让子树脱管，并按需调用 `del`；_二叉树_ 的销毁由调用方负责。
[`clear`](clear.md) 等价于 `erase(root())`。
