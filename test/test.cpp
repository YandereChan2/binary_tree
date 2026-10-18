#include "gtest/gtest.h"

#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "binary_tree.h"
#include "parent_aware_binary_tree.h"

namespace
{
    using BT = Yc::binary_tree<int>;
    using PA = Yc::parent_aware_binary_tree<int>;

    //            1
    //          /   \
    //         2     3
    //        / \     \
    //       4   5     6
    template<class Tree>
    void build_sample(Tree& t)
    {
        auto r = t.root();
        t.emplace(r, 1);
        t.emplace(r.get_left(), 2);
        t.emplace(r.get_right(), 3);
        t.emplace(r.get_left().get_left(), 4);
        t.emplace(r.get_left().get_right(), 5);
        t.emplace(r.get_right().get_right(), 6);
    }

    template<class Edge>
    std::string shape(Edge e)
    {
        if (!(bool)e)
        {
            return ".";
        }
        std::string s = "(" + std::to_string(*e) + " ";
        s += shape(e.get_left());
        s += " ";
        s += shape(e.get_right());
        s += ")";
        return s;
    }

    template<class Tree>
    std::string in_order(Tree& t)
    {
        std::string s;
        t.for_each([&](int& v) { s += std::to_string(v) + ","; });
        return s;
    }

    // 左单链：0 是最深（最左）的节点
    template<class Tree>
    void build_left_chain(Tree& t, int n)
    {
        auto e = t.root();
        for (int i = 0; i < n; ++i)
        {
            t.emplace(e, i);
            e = e.get_left();
        }
    }
}

// ============================ binary_tree ============================

TEST(binary_tree, default_construct)
{
    Yc::binary_tree<int> tree;
    EXPECT_TRUE(tree.empty());

    EXPECT_FALSE((bool)tree.root());
    EXPECT_TRUE(tree.root().null());
    EXPECT_TRUE(tree.root().valid());

    EXPECT_FALSE((bool)tree.croot());
    EXPECT_TRUE(tree.croot().null());
    EXPECT_TRUE(tree.croot().valid());

    EXPECT_FALSE((bool)tree.nroot());
    EXPECT_TRUE(tree.nroot().null());

    EXPECT_FALSE((bool)tree.cnroot());
    EXPECT_TRUE(tree.cnroot().null());
}

// 回归：const 版本的 root()/nroot()/cnroot() 曾经编不过
TEST(binary_tree, const_navigation)
{
    BT t;
    build_sample(t);
    const BT& ct = t;

    EXPECT_TRUE(ct.root().valid());
    EXPECT_EQ(*ct.root(), 1);
    EXPECT_EQ(*ct.croot(), 1);
    EXPECT_EQ(*ct.root().get_left(), 2);
    EXPECT_EQ(*ct.croot().get_left(), 2);
    EXPECT_EQ(*ct.nroot(), 1);
    EXPECT_EQ(*ct.cnroot(), 1);
    EXPECT_EQ(*ct.cnroot().get_right(), 3);

    // const 与 non-const 取到的必须是同一个槽位
    EXPECT_TRUE((BT::edge_const_proxy)ct.root() == (BT::edge_const_proxy)t.root());
}

TEST(binary_tree, copy_assignment)
{
    BT b;
    build_sample(b);
    const std::string want = shape(b.root());

    BT a;
    a.emplace(a.root(), 99);
    a = b;
    EXPECT_EQ(shape(a.root()), want);
    EXPECT_EQ(shape(b.root()), want);           // 源不受影响

    const BT& cb = b;
    a = cb;                                     // const 源
    EXPECT_EQ(shape(a.root()), want);

    a = a;                                      // 自赋值不能清空
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(shape(a.root()), want);

    a.emplace(a.root(), 100);                   // 深拷贝：改 a 不影响 b
    EXPECT_NE(shape(a.root()), shape(b.root()));
}

TEST(binary_tree, copy_constructor)
{
    BT b;
    build_sample(b);
    BT c{ b };
    EXPECT_EQ(shape(c.root()), shape(b.root()));
}

TEST(binary_tree, move_operations)
{
    BT b;
    build_sample(b);
    const std::string want = shape(b.root());

    BT m{ std::move(b) };
    EXPECT_TRUE(b.empty());
    EXPECT_EQ(shape(m.root()), want);

    BT n;
    n = std::move(m);
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(shape(n.root()), want);

    BT other;
    other.emplace(other.root(), 7);
    n.swap(other);
    EXPECT_EQ(*n.root(), 7);
    EXPECT_EQ(shape(other.root()), want);
}

// 回归：for_each 曾经漏掉"只有左孩子"的节点
TEST(binary_tree, for_each_in_order)
{
    BT t;
    build_sample(t);
    EXPECT_EQ(in_order(t), "4,2,5,1,3,6,");

    const BT& ct = t;
    std::string s;
    ct.for_each([&](const int& v) { s += std::to_string(v) + ","; });
    EXPECT_EQ(s, "4,2,5,1,3,6,");
}

TEST(binary_tree, for_each_left_chain_visits_every_node)
{
    BT t;
    build_left_chain(t, 6);                     // 5 是最深节点
    EXPECT_EQ(in_order(t), "5,4,3,2,1,0,");
}

// for_each 与 std::for_each 一样按值返回函数对象：有状态函数对象能带出累积结果
namespace
{
    struct summing_functor
    {
        int sum = 0;
        std::size_t count = 0;
        void operator()(const int& v) noexcept
        {
            sum += v;
            ++count;
        }
    };
}

TEST(binary_tree, for_each_returns_stateful_functor)
{
    BT t;
    build_sample(t);

    auto acc = t.for_each(summing_functor{});
    EXPECT_EQ(acc.sum, 21);                     // 1+2+3+4+5+6
    EXPECT_EQ(acc.count, std::size_t{ 6 });

    const BT& ct = t;
    auto cacc = ct.for_each(summing_functor{});
    EXPECT_EQ(cacc.sum, 21);

    // 按值传入：调用方自己的对象不受影响
    summing_functor own;
    auto acc2 = t.for_each(own);
    EXPECT_EQ(own.sum, 0);
    EXPECT_EQ(acc2.sum, 21);

    // 空树：函数对象原样返回
    BT empty;
    auto nothing = empty.for_each(summing_functor{});
    EXPECT_EQ(nothing.count, std::size_t{ 0 });
}

TEST(parent_aware_binary_tree, for_each_returns_stateful_functor)
{
    PA t;
    build_sample(t);

    auto acc = t.for_each(summing_functor{});
    EXPECT_EQ(acc.sum, 21);
    EXPECT_EQ(acc.count, std::size_t{ 6 });

    const PA& ct = t;
    auto cacc = ct.for_each(summing_functor{});
    EXPECT_EQ(cacc.sum, 21);
}

TEST(binary_tree, for_each_right_chain_visits_every_node)
{
    BT t;
    auto e = t.root();
    for (int i = 0; i < 6; ++i)
    {
        t.emplace(e, i);
        e = e.get_right();
    }
    EXPECT_EQ(in_order(t), "0,1,2,3,4,5,");
}

// 回归：swap_node 交换相邻（父子）节点曾经会破坏结构
TEST(binary_tree, swap_node)
{
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e, e.get_left());
        EXPECT_EQ(shape(t.root()), "(2 (1 (4 . .) (5 . .)) (3 . (6 . .)))");
    }
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e, e.get_right());
        EXPECT_EQ(shape(t.root()), "(3 (2 (4 . .) (5 . .)) (1 . (6 . .)))");
    }
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e.get_left(), e);
        EXPECT_EQ(shape(t.root()), "(2 (1 (4 . .) (5 . .)) (3 . (6 . .)))");
    }
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e.get_right(), e);
        EXPECT_EQ(shape(t.root()), "(3 (2 (4 . .) (5 . .)) (1 . (6 . .)))");
    }
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e.get_left(), e.get_right());     // 不相邻
        EXPECT_EQ(shape(t.root()), "(1 (3 (4 . .) (5 . .)) (2 . (6 . .)))");
    }
    {
        BT t;
        build_sample(t);
        auto e = t.root();
        BT::swap_node(e.get_left(), e.get_right(), Yc::no_check);
        EXPECT_EQ(shape(t.root()), "(1 (3 (4 . .) (5 . .)) (2 . (6 . .)))");
    }
}

TEST(binary_tree, hash_of_edges)
{
    EXPECT_EQ(std::hash<BT::edge_const_proxy>{}(BT::edge_const_proxy{}), 0u);
    EXPECT_EQ(std::hash<BT::edge_proxy>{}(BT::edge_proxy{}), 0u);

    BT t;
    build_sample(t);
    auto l = t.root().get_left();
    EXPECT_EQ(std::hash<BT::edge_const_proxy>{}(l), std::hash<BT::edge_const_proxy>{}(l));

    std::unordered_set<BT::edge_const_proxy> s;
    s.insert(l);
    s.insert(l);
    s.insert(t.root().get_right());
    EXPECT_EQ(s.size(), 2u);
}

// ====================== parent_aware_binary_tree ======================

TEST(parent_aware_binary_tree, default_construct)
{
    PA tree;
    EXPECT_TRUE(tree.empty());

    EXPECT_FALSE((bool)tree.root());
    EXPECT_TRUE(tree.root().null());
    EXPECT_TRUE(tree.root().valid());

    EXPECT_FALSE((bool)tree.croot());
    EXPECT_TRUE(tree.croot().null());
    EXPECT_TRUE(tree.croot().valid());

    EXPECT_FALSE((bool)tree.nroot());
    EXPECT_TRUE(tree.nroot().null());

    EXPECT_FALSE((bool)tree.cnroot());
    EXPECT_TRUE(tree.cnroot().null());

    EXPECT_TRUE((bool)tree.parent_of_nroot());
    EXPECT_TRUE((bool)tree.parent_of_cnroot());
}

TEST(parent_aware_binary_tree, const_navigation)
{
    PA t;
    build_sample(t);
    const PA& ct = t;

    EXPECT_TRUE(ct.root().valid());
    EXPECT_EQ(*ct.root(), 1);
    EXPECT_EQ(*ct.croot(), 1);
    EXPECT_EQ(*ct.nroot(), 1);
    EXPECT_EQ(*ct.cnroot(), 1);
    EXPECT_TRUE(ct.nroot().get_parent() == ct.parent_of_cnroot());
}

TEST(parent_aware_binary_tree, copy_assignment)
{
    PA b;
    build_sample(b);
    const std::string want = shape(b.root());

    PA a;
    a.emplace(a.root(), 99);
    a = b;
    EXPECT_EQ(shape(a.root()), want);
    EXPECT_EQ(shape(b.root()), want);

    a = a;                                      // 自赋值不能清空
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(shape(a.root()), want);

    // 拷贝后 parent 链必须正确
    auto nl = a.nroot().get_left();
    auto nr = a.nroot().get_right();
    EXPECT_TRUE(nl.get_parent() == a.nroot());
    EXPECT_TRUE(nr.get_parent() == a.nroot());
    EXPECT_TRUE(nl.get_left().get_parent() == nl);
    EXPECT_EQ(*a.nroot().get_parent().get_left(), 1);   // 哨兵的 left 是真根
}

TEST(parent_aware_binary_tree, move_operations)
{
    PA b;
    build_sample(b);
    const std::string want = shape(b.root());

    PA m{ std::move(b) };
    EXPECT_TRUE(b.empty());
    EXPECT_EQ(shape(m.root()), want);
    EXPECT_TRUE(m.nroot().get_left().get_parent() == m.nroot());

    PA n;
    n = std::move(m);
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(shape(n.root()), want);
}

TEST(parent_aware_binary_tree, for_each_in_order)
{
    PA t;
    build_sample(t);
    EXPECT_EQ(in_order(t), "4,2,5,1,3,6,");

    const PA& ct = t;
    std::string s;
    ct.for_each([&](const int& v) { s += std::to_string(v) + ","; });
    EXPECT_EQ(s, "4,2,5,1,3,6,");
}

TEST(parent_aware_binary_tree, for_each_left_chain_visits_every_node)
{
    PA t;
    build_left_chain(t, 6);
    EXPECT_EQ(in_order(t), "5,4,3,2,1,0,");
}

TEST(parent_aware_binary_tree, swap_node)
{
    {
        PA t;
        build_sample(t);
        auto e = t.root();
        PA::swap_node(e, e.get_left());
        EXPECT_EQ(shape(t.root()), "(2 (1 (4 . .) (5 . .)) (3 . (6 . .)))");
        EXPECT_TRUE(t.nroot().get_left().get_parent() == t.nroot());
        EXPECT_TRUE(t.nroot().get_left().get_left().get_parent() == t.nroot().get_left());
    }
    {
        PA t;
        build_sample(t);
        auto e = t.root();
        PA::swap_node(e, e.get_right());
        EXPECT_EQ(shape(t.root()), "(3 (2 (4 . .) (5 . .)) (1 . (6 . .)))");
    }
    {
        PA t;
        build_sample(t);
        auto e = t.root();
        PA::swap_node(e.get_left(), e.get_right());     // 不相邻
        EXPECT_EQ(shape(t.root()), "(1 (3 (4 . .) (5 . .)) (2 . (6 . .)))");
    }
}

TEST(parent_aware_binary_tree, hash_of_edges)
{
    EXPECT_EQ(std::hash<PA::edge_const_proxy>{}(PA::edge_const_proxy{}), 0u);
    EXPECT_EQ(std::hash<PA::edge_proxy>{}(PA::edge_proxy{}), 0u);

    PA t;
    build_sample(t);
    auto l = t.root().get_left();
    std::unordered_set<PA::edge_const_proxy> s;
    s.insert(l);
    s.insert(l);
    s.insert(t.root().get_right());
    EXPECT_EQ(s.size(), 2u);
}

// ============================== 代理约定 ==============================

// 与 const 迭代器一致：value_type 不带 const，const 性由 reference 表达
TEST(proxy, value_type_follows_const_iterator_convention)
{
    static_assert(std::is_same_v<BT::edge_proxy::value_type, int>);
    static_assert(std::is_same_v<BT::edge_const_proxy::value_type, int>);
    static_assert(std::is_same_v<BT::node_proxy::value_type, int>);
    static_assert(std::is_same_v<BT::node_const_proxy::value_type, int>);
    static_assert(std::is_same_v<PA::edge_proxy::value_type, int>);
    static_assert(std::is_same_v<PA::edge_const_proxy::value_type, int>);
    static_assert(std::is_same_v<PA::node_proxy::value_type, int>);
    static_assert(std::is_same_v<PA::node_const_proxy::value_type, int>);

    static_assert(std::is_same_v<BT::edge_proxy::reference, int&>);
    static_assert(std::is_same_v<BT::edge_const_proxy::reference, const int&>);
    static_assert(std::is_same_v<BT::node_proxy::reference, int&>);
    static_assert(std::is_same_v<BT::node_const_proxy::reference, const int&>);
    static_assert(std::is_same_v<PA::edge_proxy::reference, int&>);
    static_assert(std::is_same_v<PA::edge_const_proxy::reference, const int&>);
    static_assert(std::is_same_v<PA::node_proxy::reference, int&>);
    static_assert(std::is_same_v<PA::node_const_proxy::reference, const int&>);

    static_assert(std::is_same_v<decltype(*std::declval<BT::edge_const_proxy>()), const int&>);
    static_assert(std::is_same_v<decltype(*std::declval<BT::node_proxy>()), int&>);
    SUCCEED();
}

TEST(proxy, get_children_is_noexcept_on_node_proxies)
{
    static_assert(noexcept(std::declval<BT::node_proxy&>().get_children()));
    static_assert(noexcept(std::declval<BT::node_const_proxy&>().get_children()));
    static_assert(noexcept(std::declval<PA::node_proxy&>().get_children()));
    static_assert(noexcept(std::declval<PA::node_const_proxy&>().get_children()));
    static_assert(noexcept(std::declval<BT::edge_proxy&>().get_children()));
    static_assert(noexcept(std::declval<PA::edge_const_proxy&>().get_children()));
    SUCCEED();
}

// 两个头文件共用同一份 child_check 定义，可同时包含
TEST(proxy, child_check_is_shared_between_headers)
{
    static_assert(Yc::no_check == 0);
    static_assert(Yc::check_left == 1);
    static_assert(Yc::check_right == 2);
    static_assert(Yc::check_left_rev == 4);
    static_assert(Yc::check_right_rev == 8);
    static_assert(Yc::check_all == 15);
    SUCCEED();
}

TEST(swap_node, is_noexcept_and_accepts_child_check)
{
    static_assert(noexcept(BT::swap_node(std::declval<BT::edge_const_proxy>(), std::declval<BT::edge_const_proxy>())));
    static_assert(noexcept(PA::swap_node(std::declval<PA::edge_const_proxy>(), std::declval<PA::edge_const_proxy>())));
    SUCCEED();
}
