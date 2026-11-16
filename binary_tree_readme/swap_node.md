# `Yc::binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)noexcept
```

交换两个 _二叉树_ 节点。按**值标记**来看，净效果是两个 _树位置_ 上的值互换、子树
留在原位（树的形状不变）；但节点身份和边代理的指向会变化，详见下文。

`l` 和 `r` 都必须是指代**存在的节点**的 _树位置_。本接口按此前提条件使用；虽然当前
非父感知实现碰巧允许其中一个为空（退化为 `swap_sub_tree`），但不要依赖这一点。
对空 _树位置_ 取 `get_left()` 之类的操作本身也可能是未定义行为。

两个节点是否互为父子**不影响调用方式**，调用方不需要提供任何提示。四种相邻形态
（左右各两种顺序）与非相邻形态产生相同的值布局。但是，相邻父子交换会改变节点身份：
父节点会落到子节点原来的位置，子节点升到父位置。因此：

- 调用前取得的 `edge_proxy` 在相邻交换后可能指向另一个节点；需要继续操作时请重新
  取得代理。
- 对同一对**已经交换过的槽位**重复调用，不保证恢复原状；要撤销交换，应重新取得
  当前节点的代理后再交换回去。

交换不复制、不销毁任何节点。净语义与 `Yc::parent_aware_binary_tree::swap_node`
一致。父感知版本额外接受 `child_check` 掩码，可选择性地探测相邻形态；本函数固定走
通用路径，调用方式仍然是两个参数。

如果两个 _树位置_ 的来源 `binary_tree` 的分配器不比较相等，则行为
[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

|||
|-:|:-|
|`l` , `r`|指代要进行交换的两个 _二叉树_ 对应的 _树位置_|

## 返回值

（无）。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
#include "binary_tree.h"
#include <iostream>
struct simple_binary_tree_node
{
    simple_binary_tree_node* left{};
    simple_binary_tree_node* right{};
    int val;
    std::pair<simple_binary_tree_node*, simple_binary_tree_node*> children()const noexcept
    {
        return { left,right };
    }
};
template<class H, class F>
void print_sub_tree(H root, F cg)
{
    if (root)
    {
        std::cout << '{' << *root;
        auto [l, r] = std::invoke(cg, root);
        print_sub_tree(l, cg);
        print_sub_tree(r, cg);
        std::cout << '}';
    }
    else
    {
        std::cout << ' ';
    }
}
int main()
{
    std::cout << "================================================================\n";
    auto cg = &Yc::binary_tree<int>::edge_proxy::get_children;
    {
        simple_binary_tree_node tree[]{
                                    {tree + 1,tree + 2,1},
            //                      |
            //              ---------------------
            //              |                    |
                            {{},{},2},           {{tree + 3},{tree + 4},3},
            //                                   |
            //                          -----------------
            //                          |                |
                                        { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{ &simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0] };
        print_sub_tree(b.root(), cg);
        auto [l, r] = b.root().get_children();
        std::cout << '\n';
        Yc::binary_tree<int>::swap_node(l, r);
        print_sub_tree(b.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2  }{3{4  }{5  }}}
{1{3  }{2{4  }{5  }}}
================================================================

```
