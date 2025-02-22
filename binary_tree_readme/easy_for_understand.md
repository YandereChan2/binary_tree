# 二叉树相关概念的阐述

一般而言，在程序中定义二叉树时，往往会引入如下的结构体。

```C++
struct node_t
{
    node_t* left;
    node_t* right;
    union data_t
    {
        T element;
        //省略为通过编译提供的成员函数
    }data;
};
```

并且使用 `node_t*` 以管理二叉树的根节点或者其他节点。

但是，在尝试进行接口抽象时，“在一个位置插入子树”这个动作，如果使用 `node_t*` 作为参数，那么就势必要提供左子树版本和右子树版本，甚至还要对根节点的情形进行特殊处理。而这个问题在删除时会更严重。因为你可以 `delete t->left` 和 `delete t->right` 。但是你不能 `delete t` 。因为如果 `t` 是某一个节点的子节点，那么就无法向那个节点同步状态，进一步，会导致那个节点持有一个“野指针”。

为了解决这个问题，有人提出使用 `node_t**` （或者 `node_t*&` ）作为参数。在这种情况下，参数指向的是根节点的那个指针或者 `node_t` 对象内部的指针。这时候 `delete *p` 和 `*p = nullptr` 即可依次进行。不会导致 `node_t` 对象内部的指针成为一个“野指针”。并且在这种写法下，插入删除等操作都会有一个统一的，规范的接口。

本库中使用 _树位置_ 来描述一般情况下使用二级指针描述的概念。也就是说， `node_t` 里面的 `left` 和 `right` 所在的位置分别就是两个 _树位置_ 。另外，作为一个非侵入式的[RAII](https://zh.cppreference.com/w/cpp/language/raii)风格的二叉树管理器， `binary_tree` 对象内部也有一个指针，它的位置也是一个 _树位置_ 。

另外，二叉树的递归定义如下：**一个二叉树或为空，或保有一个元素，以及左右两颗二叉树，称为二叉子树。**

因此本库中把节点的一级指针，称为其指代 _二叉树_ 。

本库中使用 `edge_proxy` 和 `node_proxy` 实现二级指针和一级指针的封装和语义。
