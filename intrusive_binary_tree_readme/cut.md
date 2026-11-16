# `Yc::basic_intrusive_binary_tree<vptr>::cut` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::cut`

```C++
// basic_intrusive_binary_tree<vptr>
static basic_intrusive_binary_tree cut(edge_proxy e)noexcept;

// basic_intrusive_parent_aware_binary_tree<vptr>
static basic_intrusive_parent_aware_binary_tree cut(edge_proxy e)noexcept;
```

摘下 `e` 所指代的 _树位置_ 上的整棵子树，并把它作为一棵独立的树返回；
原 _树位置_ 变空。

- `e` 必须是本树的合法 _树位置_；默认代理或来自另一棵树的代理会导致未定义行为。
- 返回的树继续管理这棵子树；子树原有的父子关系保持不变；
- 可向上导航版本会把子树根的父设为返回树的 _虚拟二叉树_；
- 如果 `e` 原本为空，返回空树；
- 本函数不销毁、不脱管任何 _二叉树_。

**必须保留返回值**：它是被摘下子树的唯一树句柄。丢弃返回值不会销毁节点，但临时树
析构时会 `clear()`，把这棵子树的所有节点脱管；如果手里没有节点指针 / 代理，之后就
无法再访问它们。

## 参数

|||
|-:|:-|
|`e`|要摘下的子树所在的 _树位置_；必须属于本树|

## 返回值

包含被摘下子树的独立树；`e` 为空时返回空树。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
// 不可向上导航版本；前提：tree 非空
auto sub = tree.cut(tree.root().get_right());
assert(sub.nroot());
sub.clear();

// 可向上导航版本
Yc::basic_intrusive_parent_aware_binary_tree<> ptree;
// ... 建树 ...
auto psub = ptree.cut(ptree.root().get_right());
assert(psub.nroot().get_parent() == psub.parent_of_nroot());
psub.clear();
```

## 备注

返回的树是一个真正的树对象，可以直接继续使用。
要销毁被摘下的**节点**，应对返回的树使用 [`erase`](erase.md) / [`clear`](clear.md)；
仅销毁返回的树对象只会脱管节点，不会销毁它们。
