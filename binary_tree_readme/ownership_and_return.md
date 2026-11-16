# 返回值与所有权

`Yc::binary_tree` 是**拥有节点**的容器：每个节点都由某棵 `binary_tree` 分配，
并在该树析构或 `erase` / `clear` 时销毁。几个修改器会把一棵子树从当前位置“换出”
并以返回值的形式交给调用方；**返回值拥有这棵子树**，因此不能随意丢弃。

## 总表

| 函数 | 返回值 | 丢弃返回值的后果 |
|:-|:-|:-|
| `insert(p, v)` | 拥有 `p` 处原子树的 `binary_tree` | 原子树被销毁，相关代理 / 引用失效 |
| `emplace(p, args...)` | 同上 | 同上 |
| `splice(to, from)` | 拥有 `to` 处原子树的 `binary_tree` | 原子树被销毁，相关代理 / 引用失效 |
| `cut(p)` | 拥有被切出子树的 `binary_tree` | 被切出的子树被销毁 |
| `recur_and_write(p, ...)` | 拥有 `p` 处原子树的 `binary_tree` | 原子树被销毁 |
| `erase(p)` / `clear()` | 无 | 节点按语义销毁 |
| `swap` / `swap_sub_tree` / `swap_node` / 旋转 | 无 | 不销毁节点 |

## 典型用法

```C++
// 正确：把被换出的原子树接住
auto old = t.insert(t.root(), 42);
// ... 可以继续使用 old，或让它自然析构 ...

// 错误：返回值被丢弃，old_subtree 立即销毁
t.insert(t.root(), 42); // 若 root 处原有子树，它在这里被销毁
```

## 与侵入式树的区别

`Yc::basic_intrusive_binary_tree` 不拥有节点，它的 `insert` / `cut` / `splice`
返回的树只是**子树的句柄**。丢弃这个句柄不会销毁节点，而是让子树失去唯一的树
对象入口，节点仍处于纳管状态但无法再通过树接口访问，通常等同于泄漏。详见
[侵入式树的返回值与所有权](../intrusive_binary_tree_readme/ownership_and_return.md)。

## 异常安全

`insert` / `emplace` 在构造元素抛出时保持原树不变（强异常保证）。`splice` /
`cut` / `erase` / `swap` 等为 `noexcept`。`recur_and_write` 在构造抛出时会把
`p` 处的原子树放回；构造函数 (5) 在构造抛出时会清理已建立的节点，但构造本身
不会成功。
