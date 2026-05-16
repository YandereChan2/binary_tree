#pragma once
#include "cookie_allocator.h"
#include "parent_aware_binary_tree.h"
#include "set_common.h"
#include <memory_resource>
#include <memory>
#include <algorithm>
#include <utility>

namespace Yc
{
    
    template<class T, class Compare = std::less<T>, class Alloc = std::allocator<T>>
    class avl_set
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
        avl_set() = default;
        avl_set(const Alloc& a)noexcept :tree{ allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t > {a} }
        {}
        avl_set(const avl_set& other) :sz{ other.sz }, tree{
            [](edge_const_proxy p) -> std::pair<cookie_wrapper<size_t>, const T&> { return {cookie_wrapper<size_t>{p->cookie()}, p->value()}; },
            parent_aware_binary_tree_functional::get_children,
            other.tree.root(),
            other.tree.get_allocator()
        }
        {}

        void swap(avl_set& other)noexcept
        {
            tree.swap(other.tree);
            using std::swap;
            swap(comp, other.comp);
            std::swap(sz, other.sz);
        }

        avl_set& operator=(const avl_set& other)
        {
            avl_set tmp{ other };
            swap(tmp);
        }

        avl_set(avl_set&& other) :sz{ other.sz }, tree{
            std::move(other.tree())}, comp{std::move(other.comp)}
        {
            other.clear();
        }

        avl_set& operator=(avl_set&& other)
        {
            avl_set tmp{ std::move(other) };
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

        // p指向的位置有节点，调用者保证
        static void update_height(edge_const_proxy p)noexcept
        {
            auto [l, r] = p.get_children();
            size_t l_height = l ? l->cookie() : 0;
            size_t r_height = r ? r->cookie() : 0;
            p->cookie() = std::max(l_height, r_height) + 1;
        }

        // p指向的位置有节点，调用者保证
        static long long factor(edge_const_proxy p)noexcept
        {
            auto [l, r] = p.get_children();
            size_t l_height = l ? l->cookie() : 0;
            size_t r_height = r ? r->cookie() : 0;
            return (long long)r_height - (long long)l_height;
        }

        void fix_balance(edge_const_proxy p)noexcept
        {
            if (factor(p) < -1)
            {
                edge_const_proxy q = p;
                q.go_left();
                if (factor(q) < 0)
                {
                    tree.right_rotate(p);
                    q = p;
                    p.go_right();
                    update_height(p);
                    update_height(q);
                }
                else
                {
                    tree.left_rotate(q);
                    edge_const_proxy r = q;
                    r.go_left();
                    update_height(r);
                    update_height(q);
                    tree.right_rotate(p);
                    q = p;
                    p.go_right();
                    update_height(p);
                    update_height(q);
                }
                return;
            }
            if (factor(p) > 1)
            {
                edge_const_proxy q = p;
                q.go_right();
                if (factor(q) > 0)
                {
                    tree.left_rotate(p);
                    q = p;
                    p.go_left();
                    update_height(p);
                    update_height(q);
                }
                else
                {
                    tree.right_rotate(q);
                    edge_const_proxy r = q;
                    r.go_right();
                    update_height(r);
                    update_height(q);
                    tree.left_rotate(p);
                    q = p;
                    p.go_left();
                    update_height(p);
                    update_height(q);
                }
                return;
            }
        }

        void maintain(edge_const_proxy p)noexcept
        {
            while (p != tree.root())
            {
                p.go_up();
                size_t old_height = p->cookie();
                update_height(p);
                if (p->cookie() == old_height && factor(p) >= -1 && factor(p) <= 1)
                {
                    return;
                }
                fix_balance(p);
            }
        }

        // 对刚刚放到树里面的节点的维护动作
        void insert_post(edge_const_proxy p)noexcept
        {
            p->cookie() = 1;
            
            while (p != tree.root())
            {
                p.go_up();
                size_t old_height = p->cookie();
                update_height(p);
                if (p->cookie() == old_height)
                {
                    return;
                }
                fix_balance(p);
            }
        }

        // p位置必须没有节点，调用者保证
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

        // p位置必须有节点，调用者保证
        void erase_impl(edge_const_proxy p)noexcept
        {
            auto [l, r] = p.get_children();
            bool lf = (bool)l, rf = (bool)r;

            if (lf && rf)
            {
                details::set_iterator<T, Alloc> tmp{ (node_const_proxy)p };
                ++tmp;
                edge_const_proxy q{tmp.p};
                size_t old_cookie = p->cookie();
                tree.swap_node(p, q);
                p->cookie() = old_cookie;
                p = q;
                auto [l1, r1] = p->get_children();
                lf = (bool)l1;
                rf = (bool)r1;
            }

            if (!lf && !rf)
            {
                tree.erase(p);
                --sz;
                maintain(p);
                return;
            }
            
            auto tmp = tree.cut(p);
            auto q = tmp.root();
            if (lf)
            {
                q.go_left();
            }
            else
            {
                q.go_right();
            }
            tree.splice(p, q);
            --sz;
            maintain(p);
            // tmp析构时进行实际的删除
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
                return std::pair<iterator, bool>{ iterator{node_const_proxy{ p }}, false };
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