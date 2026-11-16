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
- (3,4) 尝试旋转。若 `e` 是合法 _树位置_，但旋转所需的子树不存在，返回 `false`；
  否则旋转并返回 `true`。

```plain text
     |                         |
    <A>                       <C>
   /   \                      / \
  <B>  <C>     =====>       <A> <E>
       / \                  / \
     <D> <E>              <B> <D>
```

图中展示左旋。左旋要求 A 所在 _树位置_ 有 _二叉树_，且 A 的右子 _二叉树_ C 存在；
右旋是镜像操作。可向上导航版本在旋转后同步维护父子关系。

如果 `e` 不是合法 _树位置_（例如默认构造的代理），四个重载都会解引用空槽位，行为
未定义；`checked_*` 只把“合法位置但为空 / 缺子树”变成 `false`，**不会**检查代理
本身是否合法。

在不能旋转的合法位置上调用无条件版本会导致未定义行为。

## 参数

|||
|-:|:-|
|`e`|想要进行旋转的 _树位置_；必须是合法位置|

## 返回值

- (1,2) （无）
- (3,4) 旋转能否进行。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
using tree = Yc::basic_intrusive_binary_tree<>;

struct node : Yc::intrusive_binary_tree_node_base<>
{
    int value;
    explicit node(int v) : value(v) {}
};

node a(1), b(2), c(3);
tree t;
tree::insert(t.root(), a);
tree::insert(t.root().get_left(), b);
tree::insert(t.root().get_right(), c);

t.left_rotate(t.root()); // 根和右孩子都存在
assert(static_cast<node&>(*t.nroot()).value == 3);

if (!t.checked_right_rotate(t.root().get_right()))
{
    // 该位置为空（合法树位置，但没有二叉树），返回 false
}
```
