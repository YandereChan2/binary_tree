// intrusive_binary_tree.h 的最小自检 / 用法示例。
//
// Build with MSVC:
//   cl /nologo /std:c++latest /EHsc /utf-8 /I . test_intrusive_binary_tree.cpp
//
// 覆盖：无父指针与带父指针两套侵入式树、proxy 建模、link/cut/splice、
//      旋转、swap_sub_tree/swap_node、erase/clear 的生命周期语义、
//       遍历、移动，以及带 T 的侵入式节点。
// 同时静态检查：侵入式公开类型不提供 pointer / node_pointer 这类指针别名。
#include "intrusive_binary_tree.h"

#include <cassert>
#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
    template<class T, class = void>
    struct has_pointer_alias : std::false_type {};
    template<class T>
    struct has_pointer_alias<T, std::void_t<typename T::pointer>> : std::true_type {};

    template<class T, class = void>
    struct has_node_pointer_alias : std::false_type {};
    template<class T>
    struct has_node_pointer_alias<T, std::void_t<typename T::node_pointer>> : std::true_type {};

    static_assert(!has_pointer_alias<Yc::intrusive_binary_tree_node_base<>>::value);
    static_assert(!has_node_pointer_alias<Yc::intrusive_binary_tree_node_base<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_binary_tree_node_proxy<>>::value);
    static_assert(!has_node_pointer_alias<Yc::intrusive_binary_tree_node_proxy<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_binary_tree_edge_proxy<>>::value);
    static_assert(!has_node_pointer_alias<Yc::intrusive_binary_tree_edge_proxy<>>::value);
    static_assert(!has_pointer_alias<Yc::basic_intrusive_binary_tree<>>::value);
    static_assert(!has_node_pointer_alias<Yc::basic_intrusive_binary_tree<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_parent_aware_binary_tree_node_base<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_parent_aware_binary_tree_virtual_node_base<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_parent_aware_binary_tree_node_proxy<>>::value);
    static_assert(!has_pointer_alias<Yc::intrusive_parent_aware_binary_tree_edge_proxy<>>::value);
    static_assert(!has_pointer_alias<Yc::basic_intrusive_parent_aware_binary_tree<>>::value);

    using plain_base = Yc::intrusive_binary_tree_node_base<>;
    using plain_tree = Yc::basic_intrusive_binary_tree<>;

    struct plain_node : plain_base
    {
        int v = 0;
        explicit plain_node(int x) : v(x) {}
    };

    plain_node& as_plain(plain_base& p) noexcept
    {
        return static_cast<plain_node&>(p);
    }

    // 中序收集值
    template<class Tree>
    std::vector<int> in_order(Tree& t)
    {
        std::vector<int> out;
        t.for_each([&](plain_base& n) { out.push_back(as_plain(n).v); });
        return out;
    }

    // 结构形状：只通过 proxy 导航
    std::string shape(plain_tree::node_proxy p)
    {
        if (!p)
        {
            return ".";
        }
        auto& n = as_plain(*p);
        return "(" + std::to_string(n.v) + " " + shape(p.get_left()) + " "
            + shape(p.get_right()) + ")";
    }

    using pa_base = Yc::intrusive_parent_aware_binary_tree_node_base<>;
    using pa_tree = Yc::basic_intrusive_parent_aware_binary_tree<>;

    struct pa_node : pa_base
    {
        int v = 0;
        explicit pa_node(int x) : v(x) {}
    };

    std::string pa_shape(pa_tree::node_proxy p)
    {
        if (!p)
        {
            return ".";
        }
        auto& n = static_cast<pa_node&>(*p);
        return "(" + std::to_string(n.v) + " " + pa_shape(p.get_left()) + " "
            + pa_shape(p.get_right()) + ")";
    }

    // 递归验证 parent 链：每个节点的 get_parent() 都指向实际父节点
    void check_parents(pa_tree::node_proxy p, pa_tree::node_proxy expected)
    {
        if (!p)
        {
            return;
        }
        assert(p.get_parent() == expected);
        check_parents(p.get_left(), p);
        check_parents(p.get_right(), p);
    }
}

int main()
{
    // ---- 无父指针：insert / 遍历 / 形状 ----
    {
        plain_node a(1), b(2), c(3), d(4), e(5);
        plain_tree t;
        assert((*t.node(a)).is_detached());
        plain_tree::insert(t.root(), a);
        assert((*t.node(a)).is_managed());
        auto r = t.root();
        plain_tree::insert(r.get_left(), b);
        plain_tree::insert(r.get_right(), c);
        plain_tree::insert(r.get_left().get_left(), d);
        plain_tree::insert(r.get_left().get_right(), e);

        assert((in_order(t) == std::vector<int>{4, 2, 5, 1, 3}));
        assert(shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");

        // 二叉树 / 边 proxy 的导航
        auto root_node = t.nroot();
        assert(as_plain(*root_node).v == 1);
        assert(as_plain(*root_node.get_left()).v == 2);
        assert(as_plain(*root_node.get_right()).v == 3);
        assert(!(*root_node.get_left().get_left()).has_children());
        t.clear();
        assert((*t.node(a)).is_detached());
        assert((*t.node(b)).is_detached());
    }

    // ---- 无父指针：insert 换出原子树 ----
    {
        plain_node a(1), b(2), c(3);
        plain_tree t;
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);
        plain_tree old = plain_tree::insert(t.root().get_left(), c);
        assert(shape(old.nroot()) == "(2 . .)");
        assert(shape(t.nroot()) == "(1 (3 . .) .)");
        old.clear();
        t.clear();
    }

    // ---- 无父指针：旋转 ----
    {
        plain_node a(1), b(2), c(3), d(4), e(5);
        plain_tree t;
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);
        plain_tree::insert(t.root().get_right(), c);
        plain_tree::insert(t.root().get_right().get_left(), d);
        plain_tree::insert(t.root().get_right().get_right(), e);

        t.left_rotate(t.root());
        assert(as_plain(*t.nroot()).v == 3);
        assert(shape(t.nroot()) == "(3 (1 (2 . .) (4 . .)) (5 . .))");
        t.right_rotate(t.root());
        assert(shape(t.nroot()) == "(1 (2 . .) (3 (4 . .) (5 . .)))");

        assert(t.checked_left_rotate(t.root()));
        assert(!t.checked_right_rotate(t.root().get_left().get_left()));
        t.right_rotate(t.root());
        assert(shape(t.nroot()) == "(1 (2 . .) (3 (4 . .) (5 . .)))");
        t.clear();
    }

    // ---- 无父指针：cut / splice / swap_sub_tree / swap_node ----
    {
        plain_node a(1), b(2), c(3), d(4), e(5);
        plain_tree t;
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);
        plain_tree::insert(t.root().get_right(), c);
        plain_tree::insert(t.root().get_left().get_left(), d);
        plain_tree::insert(t.root().get_left().get_right(), e);

        // 切下右边子树，再拼回去
        plain_tree sub = plain_tree::cut(t.root().get_right());
        assert(as_plain(*sub.nroot()).v == 3);
        assert((*sub.nroot()).is_managed());
        assert(shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) .)");
        plain_tree::splice(t.root().get_right(), sub.root());
        assert(shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");

        // 交换两个兄弟子树
        plain_tree::swap_sub_tree(t.root().get_left(), t.root().get_right());
        assert(shape(t.nroot()) == "(1 (3 . .) (2 (4 . .) (5 . .)))");
        plain_tree::swap_sub_tree(t.root().get_left(), t.root().get_right());
        assert(shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");

        // 交换两个兄弟：值互换、各自的孩子留在原位
        plain_tree::swap_node(t.root().get_left(), t.root().get_right());
        assert(shape(t.nroot()) == "(1 (3 (4 . .) (5 . .)) (2 . .))");
        plain_tree::swap_node(t.root().get_left(), t.root().get_right());
        assert(shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");

        // 交换父子（单次调用；相邻情形依赖槽位自指在同一次调用内被解开）
        plain_tree::swap_node(t.root(), t.root().get_left());
        assert(shape(t.nroot()) == "(2 (1 (4 . .) (5 . .)) (3 . .))");

        // 移动：根槽位 O(1) 转移（相邻交换后根是 b）
        plain_tree moved{ std::move(t) };
        assert(t.empty());
        assert(shape(moved.nroot()) == "(2 (1 (4 . .) (5 . .)) (3 . .))");
        moved.clear();
    }

    // ---- 无父指针：erase / clear 不销毁节点 ----
    {
        int deleted = 0;
        plain_node a(1), b(2), c(3);
        plain_tree t;
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);
        plain_tree::insert(t.root().get_right(), c);

        plain_tree::erase(t.root(), [&](auto p)
        {
            ++deleted;
            assert(p);
            assert(p->is_detached());
        });
        assert(deleted == 3);
        assert(t.empty());
        assert((*t.node(a)).is_detached());
        assert((*t.node(b)).is_detached());

        // clear 只拆结构
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);
        t.clear();
        assert(t.empty());
        assert(!t.nroot());
        assert((*t.node(a)).is_detached());

        // 无 deleter 的 erase 只让子树脱管
        plain_tree t2;
        plain_tree::insert(t2.root(), a);
        plain_tree::insert(t2.root().get_left(), b);
        plain_tree::erase(t2.root());
        assert(t2.empty());
        assert((*t2.node(a)).is_detached());
        assert((*t2.node(b)).is_detached());
    }

    // ---- 带父指针：insert 换出原子树 ----
    {
        pa_node a(1), b(2), c(3);
        pa_tree t;
        pa_tree::insert(t.root(), a);
        pa_tree::insert(t.root().get_left(), b);
        pa_tree old = pa_tree::insert(t.root().get_left(), c);
        assert(pa_shape(old.nroot()) == "(2 . .)");
        assert(old.nroot().get_parent() == old.parent_of_nroot());
        assert(pa_shape(t.nroot()) == "(1 (3 . .) .)");
        old.clear();
        t.clear();
    }

    // ---- 带父指针：parent 链 ----
    {
        pa_node a(1), b(2), c(3), d(4), e(5);
        pa_tree t;
        assert((*t.node(a)).is_detached());
        pa_tree::insert(t.root(), a);
        assert((*t.node(a)).is_managed());
        pa_tree::insert(t.root().get_left(), b);
        pa_tree::insert(t.root().get_right(), c);
        pa_tree::insert(t.root().get_left().get_left(), d);
        pa_tree::insert(t.root().get_left().get_right(), e);

        assert(t.nroot().get_parent() == t.parent_of_nroot());
        assert(t.nroot().get_left().get_parent() == t.nroot());
        assert(t.nroot().get_left().get_left().get_parent() == t.nroot().get_left());
        check_parents(t.nroot(), t.parent_of_nroot());

        // 旋转后 parent 仍然正确
        t.left_rotate(t.root());
        assert(pa_shape(t.nroot()) == "(3 (1 (2 (4 . .) (5 . .)) .) .)");
        check_parents(t.nroot(), t.parent_of_nroot());
        t.right_rotate(t.root());
        assert(pa_shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");
        check_parents(t.nroot(), t.parent_of_nroot());

        // cut 后子树根在新树里指向新哨兵；splice 回来后再指向原哨兵
        {
            pa_tree sub = pa_tree::cut(t.root().get_right());
            assert((*sub.nroot()).is_managed());
            assert(sub.nroot().get_parent() == sub.parent_of_nroot());
            assert(pa_shape(sub.nroot()) == "(3 . .)");
            pa_tree::splice(t.root().get_right(), sub.root());
            assert(t.nroot().get_right().get_parent() == t.nroot());
            assert(pa_shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");
        }

        // 交换两个兄弟（可逆）
        pa_tree::swap_node(t.root().get_left(), t.root().get_right());
        assert(pa_shape(t.nroot()) == "(1 (3 (4 . .) (5 . .)) (2 . .))");
        check_parents(t.nroot(), t.parent_of_nroot());
        pa_tree::swap_node(t.root().get_left(), t.root().get_right());
        assert(pa_shape(t.nroot()) == "(1 (2 (4 . .) (5 . .)) (3 . .))");
        check_parents(t.nroot(), t.parent_of_nroot());

        // 交换父子（单次）
        pa_tree::swap_node(t.root(), t.root().get_left());
        assert(pa_shape(t.nroot()) == "(2 (1 (4 . .) (5 . .)) (3 . .))");
        check_parents(t.nroot(), t.parent_of_nroot());

        // 移动：根 parent 指向新树的哨兵
        pa_tree moved{ std::move(t) };
        assert(t.empty());
        assert(static_cast<pa_node&>(*moved.nroot()).v == 2);
        check_parents(moved.nroot(), moved.parent_of_nroot());
        moved.clear();
        assert((*moved.node(a)).is_detached());
    }

    // ---- 带父指针：erase / clear ----
    {
        int deleted = 0;
        pa_node a(1), b(2), c(3), d(4);
        pa_tree t;
        pa_tree::insert(t.root(), a);
        pa_tree::insert(t.root().get_left(), b);
        pa_tree::insert(t.root().get_right(), c);
        pa_tree::insert(t.root().get_right().get_left(), d);

        pa_tree::erase(t.root(), [&](auto p)
        {
            ++deleted;
            assert(p->is_detached());   // 交给 deleter 前已经脱管
        });
        assert(deleted == 4);
        assert(t.empty());
        assert((*t.node(a)).is_detached());
        assert((*t.node(b)).is_detached());

        // clear 只拆结构
        pa_node f(6), g(7);
        pa_tree::insert(t.root(), f);
        pa_tree::insert(t.root().get_left(), g);
        t.clear();
        assert(t.empty());
        assert(!t.nroot());
        assert((*t.node(f)).is_detached());

        // 无 deleter 的 erase 只让子树脱管
        pa_tree t2;
        pa_tree::insert(t2.root(), f);
        pa_tree::insert(t2.root().get_left(), g);
        pa_tree::erase(t2.root());
        assert(t2.empty());
        assert((*t2.node(f)).is_detached());
        assert((*t2.node(g)).is_detached());
    }

    // ---- const 代理 / 自由 swap / splice 返回旧树 ----
    {
        plain_node a(1), b(2), c(3);
        plain_tree t;
        plain_tree::insert(t.root(), a);
        plain_tree::insert(t.root().get_left(), b);

        const plain_tree& ct = t;
        auto ce = ct.root();
        auto ce2 = ct.croot();
        auto cn = ct.nroot();
        auto cn2 = ct.cnroot();
        assert(ce);
        assert(ce2);
        assert(cn);
        assert(cn2);
        assert(static_cast<const plain_node&>(*cn).v == 1);
        assert(ce.get_children().first);
        assert(std::hash<plain_tree::node_const_proxy>{}(cn) != 0);
        assert(std::hash<plain_tree::edge_const_proxy>{}(ce) != 0);
        ct.for_each([](const plain_tree::node_base&) {});

        plain_tree t2;
        plain_tree::insert(t2.root(), c);
        auto old = plain_tree::splice(t.root().get_left(), t2.root());
        assert(static_cast<plain_node&>(*old.nroot()).v == 2);
        assert(static_cast<plain_node&>(*t.nroot().get_left()).v == 3);

        plain_tree x, y;
        Yc::swap(x, y);
        old.clear();
        t.clear();
        t2.clear();
    }

    {
        pa_node a(1), b(2);
        pa_tree t;
        pa_tree::insert(t.root(), a);
        pa_tree::insert(t.root().get_left(), b);

        const pa_tree& ct = t;
        auto ce = ct.croot();
        auto cn = ct.cnroot();
        auto cp = ct.parent_of_cnroot();
        assert(ce);
        assert(cn);
        assert(cp);
        assert(ct.parent_of_nroot() == cp);
        auto e = static_cast<pa_tree::edge_proxy>(t.nroot());
        assert(e == t.root());
        auto ec = static_cast<pa_tree::edge_const_proxy>(ct.nroot());
        assert(ec.is_left());
        assert(std::hash<pa_tree::node_const_proxy>{}(cn) != 0);
        assert(std::hash<pa_tree::edge_const_proxy>{}(ec) != 0);
        ct.for_each([](const pa_tree::node_base&) {});

        t.clear();
    }

    // ---- 带 T 的侵入式节点：延迟构造 / 析构 ----
    {
        using data_node = Yc::intrusive_binary_tree_node<void*, std::string>;
        std::allocator<data_node> a;
        data_node n;
        n.construct_value(a, "plain");
        assert(n.value() == "plain");
        n.destroy_value(a);

        using pa_data_node =
            Yc::intrusive_parent_aware_binary_tree_node<void*, std::string>;
        std::allocator<pa_data_node> pa;
        pa_data_node pn;
        pn.construct_value(pa, "parent-aware");
        assert(pn.value() == "parent-aware");
        pn.destroy_value(pa);
    }

    std::puts("test_intrusive_binary_tree: ok");
    return 0;
}
