#pragma once
#include "cookie_allocator.h"
#include "parent_aware_binary_tree.h"
#include <functional>
#include <memory>
#include <algorithm>
namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        struct set_iterator
        {
            typename parent_aware_binary_tree<value_with_cookie<T, size_t>, Alloc>::node_const_proxy p;
            set_iterator() = default;
            set_iterator(const set_iterator&) = default;
            set_iterator& operator=(const set_iterator&) = default;
            friend bool operator==(set_iterator, set_iterator) = default;
            set_iterator& operator++()noexcept
            {
                auto [l, r] = p.get_children();
                if (r)
                {
                    p.go_right();
                    while (true)
                    {
                        auto q = p;
                        q.go_left();
                        if (!q)
                        {
                            return *this;
                        }
                        p = q;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p.go_up();
                        auto r = p;
                        r.go_left();
                        if (q == r)
                        {
                            return *this;
                        }
                    }
                }
            }

            set_iterator operator++(int)noexcept
            {
                set_iterator ret = *this;
                ++*this;
                return ret;
            }

            set_iterator& operator--()noexcept
            {
                auto [l, r] = p.get_children();
                if (l)
                {
                    p.go_left();
                    while (true)
                    {
                        auto q = p;
                        q.go_right();
                        if (!q)
                        {
                            return *this;
                        }
                        p = q;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p.go_up();
                        auto r = p;
                        r.go_right();
                        if (q == r)
                        {
                            return *this;
                        }
                    }
                }
            }

            set_iterator operator--(int)noexcept
            {
                set_iterator ret = *this;
                --*this;
                return ret;
            }

            const T& operator*()const noexcept
            {
                return p->value();
            }

            const T* operator&()const noexcept
            {
                return std::addressof(p->value());
            }
        };
    }
    template<class T, class Compare = std::less<T>, class Alloc = std::allocator<T>>
    class avl_set
    {
        parent_aware_binary_tree<value_with_cookie<T, size_t>, Alloc> tree;
        [[no_unique_address]] Compare comp;
        using edge_const_proxy = parent_aware_binary_tree<value_with_cookie<T, size_t>, Alloc>::edge_const_proxy;
    public:
        avl_set() = default;
    private:
        template<class U>
        edge_const_proxy find_impl(U&& u)const noexcept
        {
            edge_const_proxy r = tree.root();
            while (r)
            {
                if (comp(u, *r))
                {
                    r.go_left();
                    continue;
                }
                if (comp(*r, u))
                {
                    r.go_right();
                    continue;
                }
                break;
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

        // p位置必须没有节点，调用者保证
        template<class... Args>
        void emplace_impl(edge_const_proxy p, Args&&... args)
        {
            tree.emplace(p, std::forward<Args>(args)...);
            p->cookie() = 1;
            maintain(p);
        }

        // p位置必须有节点，调用者保证
        void erase_impl(edge_const_proxy p)
        {
            auto [l, r] = p.get_children();
            bool lf = (bool)l, rf = (bool)r;
            if (!lf && !rf)
            {
                tree.erase(p);
                maintain(p);
                return;
            }
            if (lf && rf)
            {
                details::set_iterator<T, Alloc> tmp{ (typename parent_aware_binary_tree<value_with_cookie<T, size_t>, Alloc>::node_const_proxy)p };
                ++tmp;
                edge_const_proxy q{tmp.p};
                size_t old_height = p->cookie();
                tree.swap_node(p, q);
                p->cookie() = old_height;
                tree.erase(q);
                maintain(q);
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
            maintain(p);
            // tmp析构时进行实际的删除
        }

    public:
        using iterator = details::set_iterator<T, Alloc>;
        using const_iterator = iterator;

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
            return { node };
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
            return { node };
        }
    };

}