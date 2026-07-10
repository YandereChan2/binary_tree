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
    class llrb_set
    {
        using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
            allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
        tree_type tree;
        [[no_unique_address]] Compare comp;
        size_t sz{};
        using edge_const_proxy = tree_type::edge_const_proxy;
        using node_const_proxy = tree_type::node_const_proxy;
        static constexpr inline size_t black = 0;
        static constexpr inline size_t red   = 1;
    public:
        using key_compare = Compare;
        using value_compare = Compare;
        using allocator_type = Alloc;
        llrb_set() = default;
        llrb_set(const Alloc& a)noexcept :tree{ allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t > {a} }
        {}
        llrb_set(const llrb_set& other) :sz{ other.sz }, tree{
            [](edge_const_proxy p) -> std::pair<cookie_wrapper<size_t>, const T&> { return {cookie_wrapper<size_t>{p->cookie()}, p->value()}; },
            parent_aware_binary_tree_functional::get_children,
            other.tree.root(),
            other.tree.get_allocator()
        }
        {}

        void swap(llrb_set& other)noexcept
        {
            tree.swap(other.tree);
            using std::swap;
            swap(comp, other.comp);
            std::swap(sz, other.sz);
        }

        llrb_set& operator=(const llrb_set& other)
        {
            llrb_set tmp{ other };
            swap(tmp);
            return *this;
        }

        llrb_set(llrb_set&& other) :sz{ other.sz }, tree{
            std::move(other.tree) }, comp{ std::move(other.comp) }
        {
            other.clear();
        }

        llrb_set& operator=(llrb_set&& other)
        {
            llrb_set tmp{ std::move(other) };
            swap(tmp);
            return *this;
        }

    private:
        template<class U>
        edge_const_proxy find_impl(U&& u)const noexcept
        {
            edge_const_proxy n{};
            edge_const_proxy r = tree.root();
            while (!r.null())
            {
                if (comp(r->value(), u))
                    r.go_right();
                else
                {
                    n = r;
                    r.go_left();
                }
            }
            if (n.valid() && !comp(u, n->value()))
                return n;
            return r;
        }

        size_t color(edge_const_proxy p)const noexcept
        {
            if (p.valid() && !p.null()) [[likely]]
                return p->cookie();
            return black;
        }

        bool is_red(edge_const_proxy p)const noexcept
        {
            return color(p) == red;
        }

        void flip_colors(edge_const_proxy p)noexcept
        {
            p->cookie() = red;
            p.get_left()->cookie() = black;
            p.get_right()->cookie() = black;
        }

        void insert_post(edge_const_proxy n)noexcept
        {
            n->cookie() = red;

            while (n != tree.root())
            {
                edge_const_proxy p = n.get_parent();

                if (is_red(p.get_right()) && !is_red(p.get_left()))
                {
                    tree.left_rotate(p);
                    n = p;
                    n.go_left();
                    p = n.get_parent();
                }

                {
                    auto pl = p.get_left();
                    if (is_red(pl) && is_red(pl.get_left()))
                    {
                        tree.right_rotate(p);
                        n = p;
                        n.go_right();
                        p = n.get_parent();
                    }
                }

                if (is_red(p.get_left()) && is_red(p.get_right()))
                {
                    flip_colors(p);
                    n = p;
                    continue;
                }

                break;
            }
            tree.root()->cookie() = black;
        }

        template<class... Args>
        void emplace_impl(edge_const_proxy p, Args&&... args)
        {
            auto tmp = tree.emplace(p, std::forward<Args>(args)...);
            ++sz;
            insert_post(p);
            if (!tmp.empty())
                std::unreachable();
        }

        void insert_node_impl(edge_const_proxy p, tree_type& node)noexcept
        {
            auto tmp = tree.splice(p, node);
            ++sz;
            insert_post(p);
            if (!tmp.empty())
                std::unreachable();
        }

        void erase_fixup(edge_const_proxy n)noexcept
        {
            while (n != tree.root())
            {
                edge_const_proxy p = n.get_parent();

                if (is_red(p.get_right()) && !is_red(p.get_left()))
                {
                    tree.left_rotate(p);
                    n = p;
                    n.go_left();
                    p = n.get_parent();
                }

                {
                    auto pl = p.get_left();
                    if (is_red(pl) && is_red(pl.get_left()))
                    {
                        tree.right_rotate(p);
                        n = p;
                        n.go_right();
                        p = n.get_parent();
                    }
                }

                if (is_red(p.get_left()) && is_red(p.get_right()))
                {
                    flip_colors(p);
                    n = p;
                    continue;
                }

                break;
            }
            if (!tree.empty())
                tree.root()->cookie() = black;
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
                    return;
                erase_fixup(n);
                return;
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
                return end();
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
                return end();
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
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
            emplace_impl(p, value);
            return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, true };
        }

        std::pair<iterator, bool> insert(value_type&& value)
        {
            edge_const_proxy p = find_impl(value);
            if (p)
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
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
                return std::pair<iterator, bool>{ iterator{ node_const_proxy{ p } }, false };
            else
            {
                insert_node_impl(p, tmp);
                if (!tmp.empty())
                    std::unreachable();
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
            edge_const_proxy p = find_impl(key);
            if (p)
                return { node_const_proxy{p} };
            return end();
        }

        const_iterator lower_bound(const value_type& key) const
        {
            node_const_proxy ret = tree.parent_of_cnroot();
            node_const_proxy tmp = tree.cnroot();
            while (tmp)
            {
                if (comp(tmp->value(), key))
                    tmp.go_right();
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
                    tmp.go_right();
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
