#pragma once
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
    class sbt_set
    {
        using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
            allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
        tree_type tree;
        [[no_unique_address]] Compare comp;
        size_t sz{};
        using edge_const_proxy = tree_type::edge_const_proxy;
        using node_const_proxy = tree_type::node_const_proxy;
    public:
        using key_compare = Compare;
        using value_compare = Compare;
        using allocator_type = Alloc;

        sbt_set() = default;
        sbt_set(const Alloc& a)noexcept :tree{ allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t > {a} }
        {}
        sbt_set(const sbt_set& other) :sz{ other.sz }, tree{
            [](edge_const_proxy p) -> std::pair<cookie_wrapper<size_t>, const T&> { return {cookie_wrapper<size_t>{p->cookie()}, p->value()}; },
            parent_aware_binary_tree_functional::get_children,
            other.tree.root(),
            other.tree.get_allocator()
        }
        {}
        void swap(sbt_set& other)noexcept
        {
            tree.swap(other.tree);
            using std::swap;
            swap(comp, other.comp);
            std::swap(sz, other.sz);
        }

        sbt_set& operator=(const sbt_set& other)
        {
            sbt_set tmp{ other };
            swap(tmp);
        }

        sbt_set(sbt_set&& other) :sz{ other.sz }, tree{
            std::move(other.tree()) }, comp{ std::move(other.comp) }
        {
            other.clear();
        }

        sbt_set& operator=(sbt_set&& other)
        {
            sbt_set tmp{ std::move(other) };
            swap(tmp);
        }

    private:
        template<class U>
        edge_const_proxy find_impl(U&& u)const noexcept
        {
            edge_const_proxy r = tree.root();
            while (r)
            {
                if (comp(u, r->value()))
                {
                    r.go_left();
                    continue;
                }
                if (comp(r->value(), u))
                {
                    r.go_right();
                    continue;
                }
                break;
            }
            return r;
        }

        void update_size(edge_const_proxy p)noexcept
        {
            auto [l, r] = p.get_children();
            size_t l_sz = l ? l->cookie() : 0;
            size_t r_sz = r ? r->cookie() : 0;
            p->cookie() = l_sz + r_sz + 1;
        }

        size_t node_size(edge_const_proxy p)noexcept
        {
            return p ? p->cookie() : 0;
        }

        void fix_balance(edge_const_proxy n, bool flag)noexcept
        {
            if (!n)
            {
                return;
            }
            auto [l, r] = n.get_children();
            if (!flag)
            {
                if (!l)
                {
                    return;
                }
                auto [ll, lr] = l.get_children();
                if (node_size(ll) > node_size(r))
                {
                    tree.right_rotate(n);
                    edge_const_proxy r1 = n;
                    r1.go_right();
                    update_size(r1);
                    update_size(n);
                }
                else if (node_size(lr) > node_size(r))
                {
                    tree.left_rotate(l);
                    edge_const_proxy ll1 = l;
                    ll1.go_left();
                    update_size(ll1);
                    update_size(l);
                    tree.right_rotate(n);
                    edge_const_proxy r1 = n;
                    r1.go_right();
                    update_size(r1);
                    update_size(n);
                }
                else
                {
                    return;
                }
            }
            else
            {
                if (!r)
                {
                    return;
                }
                auto [rl, rr] = r.get_children();
                if (node_size(rr) > node_size(l))
                {
                    tree.left_rotate(n);
                    edge_const_proxy l1 = n;
                    l1.go_left();
                    update_size(l1);
                    update_size(n);
                }
                else if (node_size(rl) > node_size(l))
                {
                    tree.right_rotate(r);
                    edge_const_proxy rr1 = r;
                    rr1.go_right();
                    update_size(rr1);
                    update_size(r);
                    tree.left_rotate(n);
                    edge_const_proxy l1 = n;
                    l1.go_left();
                    update_size(l1);
                    update_size(n);
                }
                else
                {
                    return;
                }
            }
            auto [l1, r1] = n.get_children();
            fix_balance(l1, false);
            fix_balance(r1, true);
            fix_balance(n, true);
            fix_balance(n, false);
        }

        void insert_post(edge_const_proxy n)noexcept
        {
            n->cookie() = 1;
            while (n != tree.root())
            {
                edge_const_proxy tmp = n;
                n.go_up();
                ++n->cookie();
                edge_const_proxy tmp2 = n;
                tmp2.go_left();
                fix_balance(n, tmp != tmp2);
            }
        }

        template<class... Args>
        void emplace_impl(edge_const_proxy p, Args&&... args)
        {
            tree.emplace(p, std::forward<Args>(args)...);
            ++sz;
            insert_post(p);
        }

        void insert_node_impl(edge_const_proxy p, tree_type& node)noexcept
        {
            tree.splice(p, node);
            ++sz;
            insert_post(p);
        }

        void erase_impl(edge_const_proxy n)noexcept
        {
            auto [l, r] = n.get_children();
            bool lf = (bool)l, rf = (bool)r;

            if (lf && rf)
            {
                details::set_iterator<T, Alloc> tmp{ (node_const_proxy)n };
                ++tmp;
                edge_const_proxy q{ tmp.p };
                size_t old_cookie_n = n->cookie();
                tree.swap_node(n, q);
                n->cookie() = old_cookie_n;
                n = q;
                auto [l1, r1] = n->get_children();
                lf = (bool)l1;
                rf = (bool)r1;
            }

            if (!lf && !rf)
            {
                size_t color_n = n->cookie();
                tree.erase(n);
                --sz;
                while (n != tree.root())
                {
                    edge_const_proxy tmp = n;
                    n.go_up();
                    ++n->cookie();
                    edge_const_proxy tmp2 = n;
                    tmp2.go_left();
                    fix_balance(n, tmp == tmp2);
                }
            }

            auto tmp = tree.cut(n);
            auto q = tmp.root();
            if (lf)
            {
                q.go_left();
            }
            else
            {
                q.go_right();
            }
            tree.splice(n, q);
            --sz;
            while (n != tree.root())
            {
                edge_const_proxy tmp = n;
                n.go_up();
                ++n->cookie();
                edge_const_proxy tmp2 = n;
                tmp2.go_left();
                fix_balance(n, tmp == tmp2);
            }
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
            // 找到最左节点
            while (true)
            {
                auto left = node;
                left.go_left();
                if (!left)
                {
                    break;
                }
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
            // 找到最左节点
            while (true)
            {
                auto left = node;
                left.go_left();
                if (!left)
                {
                    break;
                }
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
            edge_const_proxy p = find_impl(*tmp.root());
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
            if (p)
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