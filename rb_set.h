#pragma once
#include "cookie_allocator.h"
#include "parent_aware_binary_tree.h"
#include "set_common.h"
#include <memory>
#include <algorithm>
#include <utility>
namespace Yc
{
    template<class T, class Compare = std::less<T>, class Alloc = std::allocator<T>>
    class rb_set
    {
        using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
            allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
        tree_type tree;
        [[no_unique_address]] Compare comp;
        size_t sz{};
        using edge_const_proxy = tree_type::edge_const_proxy;
        using node_const_proxy = tree_type::node_const_proxy;
        static constexpr inline size_t black = 0;
        static constexpr inline size_t red = 1;
    public:
        using key_compare = Compare;
        using value_compare = Compare;
        using allocator_type = Alloc;

        rb_set() = default;
        rb_set(const Alloc& a)noexcept :tree{ allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t > {a} }
        {}
        rb_set(const rb_set& other) :sz{ other.sz }, tree{
            [](edge_const_proxy p) -> std::pair<cookie_wrapper<size_t>, const T&> { return {cookie_wrapper<size_t>{p->cookie()}, p->value()}; },
            parent_aware_binary_tree_functional::get_children,
            other.tree.root(),
            other.tree.get_allocator()
        }
        {}
        void swap(rb_set& other)noexcept
        {
            tree.swap(other.tree);
            using std::swap;
            swap(comp, other.comp);
            std::swap(sz, other.sz);
        }

        rb_set& operator=(const rb_set& other)
        {
            rb_set tmp{ other };
            swap(tmp);
        }

        rb_set(rb_set&& other) :sz{ other.sz }, tree{
            std::move(other.tree) }, comp{ std::move(other.comp) }
        {
            other.clear();
        }

        rb_set& operator=(rb_set&& other)
        {
            rb_set tmp{ std::move(other) };
            swap(tmp);
        }

    private:
        template<class U>
        edge_const_proxy find_impl(U&& u)const noexcept
        {
            edge_const_proxy n{}; // 候选的节点
            edge_const_proxy r = tree.root();
            while (!r.null())
            {
                if (comp(r->value(), u))
                {
                    // r比查找值小，去右子树
                    r.go_right();
                }
                else
                {
                    // r >= 查找值，记录候选
                    n = r;
                    r.go_left();
                }
            }
            // 显然n >= 查找值，如果同时查找值 >= n，那么按照相等处理
            if (n.valid() && !comp(u, n->value()))
            {
                return n;
            }
            return r;
        }

        template<class U>
        node_const_proxy find_node_impl(U&& u)const noexcept
        {
            node_const_proxy n{}; // 候选的节点
            node_const_proxy r = tree.cnroot();
            while (!r.null())
            {
                if (comp(r->value(), u))
                {
                    // r比查找值小，去右子树
                    r.go_right();
                }
                else
                {
                    // r >= 查找值，记录候选
                    n = r;
                    r.go_left();
                }
            }
            // 显然n >= 查找值，如果同时查找值 >= n，那么按照相等处理
            if (!n.null() && !comp(u, n->value()))
            {
                return n;
            }
            return r;
        }

        size_t color(edge_const_proxy p)const noexcept
        {
            if (p.valid() && !p.null()) [[likely]]
            {
                return p->cookie();
            }
            return black;
        }

        void insert_post(edge_const_proxy n)noexcept
        {
            n->cookie() = red;
            while (n != tree.root())
            {
                edge_const_proxy p = n.get_parent();
                if (p->cookie() == black)
                    return;

                edge_const_proxy g = p.get_parent();
                auto [p1, p2] = g.get_children();
                bool p_l = p == p1;
                edge_const_proxy u = p_l ? p2 : p1;

                if (color(u) == red)
                {
                    g->cookie() = red;
                    p->cookie() = black;
                    u->cookie() = black;
                    n = g;
                    continue;
                }

                bool n_l = n == p.get_left();
                if (n_l != p_l)
                {
                    if (n_l)
                        tree.right_rotate(p);
                    else
                        tree.left_rotate(p);
                    n.go_up();
                }
                g->cookie() = red;
                if (p_l)
                    tree.right_rotate(g);
                else
                    tree.left_rotate(g);
                g->cookie() = black;
                return;
            }
            n->cookie() = black;
        }

        template<class... Args>
        void emplace_impl(edge_const_proxy p, Args&&... args)
        {
            auto tmp = tree.emplace(p, std::forward<Args>(args)...);
            ++sz;
            insert_post(p);
            if (!tmp.empty())
            {
                std::unreachable();
            }
        }

        void insert_node_impl(edge_const_proxy p, tree_type& node)noexcept
        {
            auto tmp = tree.splice(p, node);
            ++sz;
            insert_post(p);
            if (!tmp.empty())
            {
                std::unreachable();
            }
        }

        void erase_impl(edge_const_proxy n)noexcept
        {
            auto [l, r] = n.get_children();
            bool lf = !l.null(), rf = !r.null();

            if (lf && rf)
            {
                details::set_iterator<T, Alloc> tmp{ (node_const_proxy)n };
                ++tmp;
                edge_const_proxy q{ tmp.p };
                size_t old_cookie_n = n->cookie();
                size_t old_cookie_q = q->cookie();
                node_const_proxy saved_n = (node_const_proxy)n;
                tree.swap_node(n, q, Yc::check_right);
                n->cookie() = old_cookie_n;
                saved_n->cookie() = old_cookie_q;
                n = (edge_const_proxy)saved_n;
                auto [l1, r1] = n.get_children();
                lf = !l1.null();
                rf = !r1.null();
            }

            if (!lf && !rf)
            {
                size_t color_n = n->cookie();
                tree.erase(n);
                --sz;
                if (color_n == red || tree.empty())
                {
                    return;
                }
                while (true)
                {
                    if (n == tree.root())
                        return;
                    edge_const_proxy p = n.get_parent();
                    auto [n1, n2] = p.get_children();
                    bool n_l = n == n1;
                    edge_const_proxy s = n_l ? n2 : n1;
                    if (color(s) == red)
                    {
                        p->cookie() = red;
                        s->cookie() = black;
                        if (n_l)
                        {
                            tree.left_rotate(p);
                            auto y = p.get_left();
                            n = y.get_left();
                            s = y.get_right();
                        }
                        else
                        {
                            tree.right_rotate(p);
                            auto y = p.get_right();
                            n = y.get_right();
                            s = y.get_left();
                        }
                    }
                    auto [c, d] = s.get_children();
                    if (!s.null() && color(c) == black && color(d) == black)
                    {
                        s->cookie() = red;
                        if (n.get_parent()->cookie() == red || n.get_parent() == tree.root())
                        {
                            n.get_parent()->cookie() = black;
                            return;
                        }
                        else
                        {
                            n = n.get_parent();
                            continue;
                        }
                    }

                    if (!s.null() && ((n_l && color(d) == black) || (!n_l && color(c) == black)))
                    {
                        s->cookie() = black;
                        if (n_l)
                        {
                            tree.right_rotate(s);
                        }
                        else
                        {
                            tree.left_rotate(s);
                        }
                        auto [new_n1, new_n2] = n.get_parent().get_children();
                        s = (n == new_n1) ? new_n2 : new_n1;
                        s->cookie() = red;
                    }

                    if (s.null())
                    {
                        n = n.get_parent();
                        continue;
                    }

                    auto y = n.get_parent();
                    size_t old_color = y->cookie();
                    if (n_l)
                    {
                        tree.left_rotate(y);
                    }
                    else
                    {
                        tree.right_rotate(y);
                    }
                    auto [c1, d1] = y.get_children();
                    if (c1) c1->cookie() = black;
                    if (d1) d1->cookie() = black;
                    y->cookie() = old_color;
                    return;
                }
            }

            edge_const_proxy child = lf ? n.get_left() : n.get_right();
            auto tmp = tree.cut(n);
            auto q = tmp.root();
            if (lf)
                q.go_left();
            else
                q.go_right();
            tree.splice(n, q);
            --sz;
            n->cookie() = black;
        }

    public:
        using iterator = details::set_iterator<T, Alloc>;
        using const_iterator = iterator;
        using value_type = T;

        allocator_type get_allocator()const noexcept
        {
            return tree.get_allocator().get_allocator();
        }

        const_iterator end()const noexcept
        {
            return { tree.parent_of_cnroot() };
        }

        const_iterator begin() const noexcept
        {
            if (tree.empty())
            {
                return end();
            }
            auto node = tree.cnroot();
            while (true)
            {
                auto left = node.get_left();
                if (!left)
                    break;
                node = left;
            }
            return node;
        }

        const_iterator cend()const noexcept
        {
            return { tree.parent_of_cnroot() };
        }

        const_iterator cbegin()const noexcept
        {
            if (tree.empty())
            {
                return end();
            }
            auto node = tree.cnroot();
            while (true)
            {
                auto left = node.get_left();
                if (!left)
                    break;
                node = left;
            }
            return node;
        }

        size_t size()const noexcept
        {
            return sz;
        }

        void clear()noexcept
        {
            tree.clear();
            sz = 0;
        }

        bool empty()const noexcept
        {
            return sz == 0;
        }

        std::pair<iterator, bool> insert(const value_type& value)
        {
            edge_const_proxy p = find_impl(value);
            if (p)
            {
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
            }
            emplace_impl(p, value);
            return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, true };
        }

        std::pair<iterator, bool> insert(value_type&& value)
        {
            edge_const_proxy p = find_impl(value);
            if (p)
            {
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
            }
            emplace_impl(p, std::move(value));
            return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, true };
        }

        template< class... Args >
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            tree_type tmp{ tree.get_allocator() };
            tmp.emplace(tmp.root(), std::forward<Args>(args)...);
            edge_const_proxy p = find_impl(tmp.root()->value());
            if (p)
            {
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
            }
            else
            {
                insert_node_impl(p, tmp);
                if (!tmp.empty())
                {
                    std::unreachable();
                }
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, true };
            }
        }

        iterator erase(const_iterator pos)noexcept
        {
            edge_const_proxy p = (edge_const_proxy)pos.p;
            ++pos;
            erase_impl(p);
            return pos;
        }

        size_t erase(const value_type& k)noexcept
        {
            edge_const_proxy p = find_impl(k);
            if (!p.null())
            {
                erase_impl(p);
                return 1;
            }
            return 0;
        }

        const_iterator find(const value_type& key)const
        {
            node_const_proxy p = find_node_impl(key);
            if (!p.null())
            {
                return { node_const_proxy{p} };
            }
            return end();
        }

        const_iterator lower_bound(const value_type& key) const
        {
            node_const_proxy ret = tree.parent_of_cnroot();
            node_const_proxy tmp = tree.cnroot();
            while (tmp)
            {
                if (comp(tmp->value(), key))
                {
                    tmp.go_right();
                }
                else
                {
                    ret = tmp;
                    tmp.go_left();
                }
            }
            return { ret };
        }

        const_iterator upper_bound(const value_type& key) const
        {
            node_const_proxy ret = tree.parent_of_cnroot();
            node_const_proxy tmp = tree.cnroot();
            while (tmp)
            {
                if (comp(key, tmp->value()))
                {
                    ret = tmp;
                    tmp.go_left();
                }
                else
                {
                    tmp.go_right();
                }
            }
            return { ret };
        }

        key_compare key_comp()const
        {
            return comp;
        }

        value_compare value_comp()const
        {
            return comp;
        }
    };
}