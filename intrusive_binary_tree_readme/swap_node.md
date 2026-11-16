# `Yc::basic_intrusive_binary_tree<vptr>::swap_node` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap_node`

```C++
// basic_intrusive_binary_tree<vptr>
static void swap_node(edge_proxy l, edge_proxy r)noexcept;
```

```C++
// basic_intrusive_parent_aware_binary_tree<vptr>
static void swap_node(
    edge_proxy l, edge_proxy r, child_check chk = check_all)noexcept;
```

交换两棵 _二叉树_。按**值标记**来看，净效果是两个 _树位置_ 上的值互换、子树留在
原位、树的形状不变；但相邻父子交换会改变节点身份，详见“代理与形状”。

- 不可向上导航版本：交换两个 _树位置_ 上的 _二叉树_ ，并交换它们的左右子树；
- 可向上导航版本：同上，并在每一步维护父子关系。`chk` 是相邻形态检查掩码，默认
  `check_all`。

## `child_check`

| 标志 | 探测的相邻形态 |
|:-|:-|
| `no_check` | 不检查，始终走通用路径 |
| `check_left` | `r` 是 `l` 的左孩子 |
| `check_right` | `r` 是 `l` 的右孩子 |
| `check_left_rev` | `l` 是 `r` 的左孩子 |
| `check_right_rev` | `l` 是 `r` 的右孩子 |
| `check_all` | 以上四种全部检查（默认） |

被选中的相邻形态直接做指针手术，其余情况走“先换槽位、再成对换孩子槽位”的通用
路径。`no_check` 适合调用方**确定两个位置不可能相邻**的场景，最典型的是跨树操作：

```C++
using pa_tree = Yc::basic_intrusive_parent_aware_binary_tree<>;
pa_tree ta, tb;
// ... 分别建树 ...
// 两棵树的不同节点不可能互为父子，直接走通用路径
pa_tree::swap_node(ta.root(), tb.root(), Yc::no_check);
```

## 代理与形状

对于直接父子交换，父节点会落到子节点原来的位置，子节点升到父位置。因此：

- 调用前取得的 `edge_proxy` 可能从此指向另一个节点；需要继续操作时请重新取得代理；
- 对同一对已经交换过的**槽位**重复调用，不保证恢复原状；要撤销交换，应重新取得
  当前节点的代理后再交换回去；
- 按“值标记”的树形确实不变，但按节点身份的拓扑会改变。

## 前置条件

- 两个 _树位置_ 上都必须有 _二叉树_ ；否则行为未定义（父感知版本的 `check_*`
  分支会在检查时解引用空节点）。
- `l` / `r` 必须指代合法的 _树位置_；默认代理是未定义行为。

## 参数

|||
|-:|:-|
|`l` , `r`|指代两棵存在的 _二叉树_ 的 _树位置_|
|`chk`|相邻形态检查掩码，默认 `check_all`（仅父感知版本）|

## 返回值

（无）。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
// 前提：tree 非空，且根有左孩子
tree.swap_node(tree.root(), tree.root().get_left());
```

## 备注

- 父子相邻的情形也可以直接交换。
- 本函数不复制、不销毁 _二叉树_。
