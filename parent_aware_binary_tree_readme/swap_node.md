# `Yc::parent_aware_binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r,
    child_check chk = check_all)noexcept
```

交换两个 _二叉树_ 节点。净效果是**两个 _树位置_ 上的值互换、子树留在原位**（树的形状不变）。

`l` 和 `r` 都必须是指代**存在的节点**的 _树位置_。传入空 _树位置_（`(bool)l == false`，例如对没有左孩子的节点取 `get_left()`）会解引用空节点，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

两个节点是否互为父子**不影响调用方式**，调用方不需要提供任何提示。`chk` 是相邻形态检查掩码：只有掩码中指定的方向会被探测，未指定的方向直接走通用交换路径。

| 标志 | 探测的相邻形态 |
|-:|:-|
| `no_check` | 不检查，始终走通用路径 |
| `check_left` | `r` 是 `l` 的左孩子 |
| `check_right` | `r` 是 `l` 的右孩子 |
| `check_left_rev` | `l` 是 `r` 的左孩子 |
| `check_right_rev` | `l` 是 `r` 的右孩子 |
| `check_all` | 以上四种全部检查（默认） |

被选中的相邻形态直接做指针手术，其余情况走“先换槽位、再成对换孩子槽位”的通用路径；两种路径的净结果一致。交换后向上导航（`go_up` 一类）依然正确。

如果两个 _树位置_ 的来源 `parent_aware_binary_tree` 的分配器不比较相等，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代要进行交换的两个 _二叉树_ 对应的 _树位置_ |
|`chk`|-|相邻形态检查掩码，默认 `check_all` |

## 返回值

（无）

## 复杂度

常数

## 异常

（无）
