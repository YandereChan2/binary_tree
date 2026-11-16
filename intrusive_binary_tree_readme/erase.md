# `Yc::basic_intrusive_binary_tree<vptr>::erase` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::erase`

```C++
// 两套树都提供
static void erase(edge_proxy e)noexcept; //(1)

template<class Deleter>
static void erase(edge_proxy e, Deleter del)noexcept; //(2)
```

- (1) 拆掉 `e` 所指代的 _树位置_ 上的整棵子树，并让其中的所有 _二叉树_ 脱管；
- (2) 同样拆掉子树并让所有 _二叉树_ 脱管，然后对每棵 _二叉树_ 调用
  `del(node_pointer)`。

## 前置条件

- `e` 必须是本树的合法 _树位置_；默认代理或来自另一棵树的代理是未定义行为。
- `del` 必须不抛出异常。

对于 (2)：

- 调用 `del` 之前，该 _二叉树_ 已经**脱管**：所有链接字段都指向自身；
- 因此 `del` 可以安全地销毁它；
- `del` 内**不得修改树结构**（不能 `insert` / `cut` / `erase` / 导航这棵树，也不能
  把这些节点重新接回树），但销毁节点本身是允许的——这正是 `del` 的用途；
- `del` 只负责销毁或记录节点；树对象本身不受影响；
- 子树中的所有 _二叉树_ 都会依次经历这个过程，回收器看到的始终是脱管的
  _二叉树_。

## 参数

|||
|-:|:-|
|`e`|要拆掉的子树所在的 _树位置_；必须属于本树|
|`del`|回收器，接收指向该 _二叉树_ 的 `node_pointer`；必须 `noexcept`|

## 返回值

（无）。

## 复杂度

时间与子树中 _二叉树_ 的数量成线性关系。不使用额外容器；不可向上导航版本在双孩子
节点上使用递归，额外栈空间与树高成正比。

## 异常

（无）。`del` 抛出时，异常会从标记为 `noexcept` 的 `erase` 中逃出，从而调用
`std::terminate`；这不是“未定义行为”，而是明确的终止。`del` 必须不抛出异常。

## 示例

```C++
// 前提：tree 非空
// 只脱管，不销毁
tree.erase(tree.root().get_right());

// 脱管后交给回收器销毁；lambda 必须 noexcept
tree.erase(tree.root().get_left(), [](auto p) noexcept
{
    assert(p->is_detached());
    auto& n = static_cast<my_binary_tree&>(*p);
    destroy_binary_tree(n); // 假设这是用户提供的销毁函数
});
```

## 备注

`erase` 只负责拆解树、让子树脱管，并按需调用 `del`；_二叉树_ 的销毁由调用方负责。
[`clear`](clear.md) 等价于 `erase(root())`。
