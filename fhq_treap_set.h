#pragma once
#include "cookie_allocator.h"
#include "parent_aware_binary_tree.h"
#include "set_common.h"
#include <memory>
#include <algorithm>
#include <utility>
#include <random>
namespace Yc
{
    template<class T, class Compare = std::less<T>, class Alloc = std::allocator<T>,
        class Rng = std::minstd_rand0, typename Rng::result_type default_seed = Rng::default_seed>
    class fhq_treap_set
    {
        using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
            allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
        tree_type tree;
        [[no_unique_address]] Compare comp;
        [[no_unique_address]] Rng rng{default_seed};
        size_t sz{};
        using edge_const_proxy = tree_type::edge_const_proxy;
        using node_const_proxy = tree_type::node_const_proxy;
    public:
        using key_compare = Compare;
        using value_compare = Compare;
        using allocator_type = Alloc;

        fhq_treap_set() = default;
        fhq_treap_set(const Alloc& a)noexcept :tree{ allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t > {a} }
        {}
        fhq_treap_set(const fhq_treap_set& other) :sz{ other.sz }, tree{
            [](edge_const_proxy p) -> std::pair<cookie_wrapper<size_t>, const T&> { return {cookie_wrapper<size_t>{p->cookie()}, p->value()}; },
            parent_aware_binary_tree_functional::get_children,
            other.tree.root(),
            other.tree.get_allocator()
        }
        {}
        void swap(fhq_treap_set& other)noexcept
        {
            tree.swap(other.tree);
            using std::swap;
            swap(comp, other.comp);
            std::swap(sz, other.sz);
        }

        fhq_treap_set& operator=(const fhq_treap_set& other)
        {
            fhq_treap_set tmp{ other };
            swap(tmp);
        }

        fhq_treap_set(fhq_treap_set&& other) :sz{ other.sz }, tree{
            std::move(other.tree()) }, comp{ std::move(other.comp) }
        {
            other.clear();
        }

        fhq_treap_set& operator=(fhq_treap_set&& other)
        {
            fhq_treap_set tmp{ std::move(other) };
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

        // 使用额外的出参来去重
        template<class U>
        std::pair<tree_type, tree_type> split_impl(tree_type& t, U&& u, bool& eq)noexcept
        {
            if (t.empty())
            {
                return { tree_type{get_allocator()} , tree_type{get_allocator()} };
            }
            std::pair<tree_type, tree_type> ret{ get_allocator() , get_allocator() };
            edge_const_proxy rt = t.root();
            if (comp(rt->value(), u))
            {
                rt.go_right();
                tree_type tmp{ get_allocator() };
                edge_const_proxy retlp = ret.first.root();
                tmp.splice(tmp.root(), rt);
                ret.first.splice(retlp, t);
                auto [ltree, rtree] = split_impl(tmp, u, eq);
                retlp.go_right();
                ret.first.splice(retlp, ltree);
                ret.second.swap(rtree);
                return ret;
            }
            else
            {
                eq = eq || !comp(u, rt->value());
                rt.go_left();
                tree_type tmp{ get_allocator() };
                edge_const_proxy retrp = ret.second.root();
                tmp.splice(tmp.root(), rt);
                ret.second.splice(retrp, t);
                auto [ltree, rtree] = split_impl(tmp, u, eq);
                retrp.go_left();
                ret.second.splice(retrp, rtree);
                ret.first.swap(ltree);
                return ret;
            }
        }

        tree_type merge_impl(tree_type& l, tree_type& r)noexcept
        {
            if (l.empty())
            {
                return std::move(r);
            }
            if (r.empty())
            {
                return std::move(l);
            }
            edge_const_proxy lt = l.root();
            edge_const_proxy rt = r.root();
            if (lt->cookie() < rt->cookie())
            {
                lt.go_right();
                tree_type new_l = l.cut(lt);
                l.splice(lt, merge_impl(new_l, r));
                return std::move(l);
            }
            else
            {
                rt.go_left();
                tree_type new_r = r.cut(rt);
                r.splice(rt, merge_impl(l, new_r));
                return std::move(r);
            }
        }

        node_const_proxy insert_node_impl(tree_type& node)noexcept
        {
            bool eq = false;
            auto [l, r] = split_impl(tree, *node.root(), eq);
            node_const_proxy ret = tree.parent_of_cnroot();
            if (!eq)
            {
                ++sz;
                tree_type tmp = merge_impl(l, node);
                l.swap(tmp);
                ret = node.croot();
            }
            tree_type result = merge_impl(l, r);
            tree.swap(result);
            return ret;
        }

        template<class U>
        node_const_proxy insert_value_impl(U&& u)
        {
            bool eq = false;
            auto [l, r] = split_impl(tree, u, eq);
            node_const_proxy ret = tree.parent_of_cnroot();
            if (!eq)
            {
                tree_type tmp{ get_allocator() };
                tmp.emplace(tmp.root(), std::forward<U>(u));
                ret = tmp.cnroot();
                ret->cookie() = rng();
                ++sz;
                tree_type tmp2 = merge_impl(l, tmp);
                l.swap(tmp2);
            }
            tree_type result = merge_impl(l, r);
            tree.swap(result);
            return ret;
        }

        void erase_impl(edge_const_proxy n)noexcept
        {
            bool eq = true;
            node_const_proxy backup = (node_const_proxy)n;
            auto [l, r] = n.get_children();
            --sz;
            if (l && r)
            {
                tree_type tmp1 = tree.cut(n);
                auto [ltree, rtree] = split_impl(tmp1, *backup, eq);
                edge_const_proxy rr = rtree.root();
                rr.go_right();
                rtree.splice(rtree.root(), rr);
                tree.splice(n, merge_impl(ltree, rtree));
                return;
            }
            if (!l && !r)
            {
                tree.erase(n);
                return;
            }
            if (l)
            {
                tree.splice(n, l);
                return;
            }
            if (r)
            {
                tree.splice(n, r);
                return;
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
            node_const_proxy ret = insert_value_impl(value);
            return std::pair<iterator, bool>{ iterator{ ret }, ret == tree.parent_of_cnroot() };
        }

        std::pair<iterator, bool> insert(value_type&& value)
        {
            node_const_proxy ret = insert_value_impl(std::move(value));
            return std::pair<iterator, bool>{ iterator{ ret }, ret == tree.parent_of_cnroot() };
        }

        template< class... Args >
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            tree_type tmp{ get_allocator() };
            tmp.emplace(tmp.root(), std::forward<Args>(args)...);
            tmp.root()->cookie() = rng();
            node_const_proxy ret = insert_node_impl(tmp);
            return std::pair<iterator, bool>{ iterator{ ret }, ret == tree.parent_of_cnroot() };
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