# `Yc::binary_tree<T,Allocator>::~binary_tree`

```C++
~binary_tree()
```

[析构函数](https://zh.cppreference.com/w/cpp/language/destructor)。 销毁保有的 _二叉树_ 。调用各元素的析构函数，并解分配所用的存储。注意，若元素是指针，则不销毁所指向的对象。

相当于调用了 [`erase(root())`](erase.md) 。

## 复杂度

与销毁的 _二叉树_ 数量呈线性。
