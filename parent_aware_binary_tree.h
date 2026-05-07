#pragma once
#include <memory>
#include <type_traits>
#include <functional>
#include <utility>
#include "union_data.h"

namespace Yc
{
    template<class T, class Alloc>
    class parent_aware_binary_tree;
    namespace details
    {
        // 为了规避分配器涉及到不完整类型
        // 二叉树节点继承空类parent_aware_binary_tree_empty_placeholder
        // 存储parent_aware_binary_tree_empty_placeholder分配器对应的pointer
        struct parent_aware_binary_tree_empty_placeholder
        {
        };
        template<class Alloc>
        struct parent_aware_binary_tree_node_base1: parent_aware_binary_tree_empty_placeholder
        {
            static_assert(std::is_same_v<typename std::allocator_traits<Alloc>::value_type,
                parent_aware_binary_tree_empty_placeholder>);
            using pointer = std::allocator_traits<Alloc>::pointer;
            pointer left{};
        };

        template<class Alloc>
        struct parent_aware_binary_tree_node_base : parent_aware_binary_tree_node_base1<Alloc>
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            pointer right{};
            pointer parent{};
        };

        template<class T, class Alloc>
        struct parent_aware_binary_tree_node : parent_aware_binary_tree_node_base<Alloc>
        {
            data_t<T> data;
        };
        
        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_const_proxy;
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_proxy;
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_const_proxy;

        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            bool left{};
            pointer parent{};
            pointer& child()const noexcept
            {
                if (left) {
                    return static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left;
                } 
                return static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).right;
            }
        public:
            parent_aware_binary_tree_edge_proxy() = default;
            parent_aware_binary_tree_edge_proxy(const parent_aware_binary_tree_edge_proxy&) = default;
            parent_aware_binary_tree_edge_proxy(pointer ptr, bool left)noexcept:parent{ptr}, left{left}
            { }
            parent_aware_binary_tree_edge_proxy& operator=(const parent_aware_binary_tree_edge_proxy&) = default;
            bool valid()const noexcept
            {
                return bool{ parent };
            }
            bool null()const noexcept
            {
                return !bool{ child() };
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            parent_aware_binary_tree_edge_proxy& go_left()noexcept
            {
                parent = child();
                left = true;
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_right()noexcept
            {
                parent = child();
                left = false;
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_up()noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                left = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent;
                parent = parent_of_parent;
                return *this;
            }
            T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data);
            }
            std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(parent_aware_binary_tree_edge_proxy l, parent_aware_binary_tree_edge_proxy r)noexcept
            {
                return std::addressof(l.child()) == std::addressof(r.child());
            }
            friend struct std::hash<parent_aware_binary_tree_edge_proxy>;
            friend class parent_aware_binary_tree_edge_const_proxy<T, Alloc>;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_proxy<T, Alloc>;
            using value_type = T;
        };
    }
}

template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_const_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            bool left{};
            pointer parent{};
            pointer& child()const noexcept
            {
                if (left) {
                    return static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left;
                }
                return static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).right;
            }
        public:
            parent_aware_binary_tree_edge_const_proxy() = default;
            parent_aware_binary_tree_edge_const_proxy(const parent_aware_binary_tree_edge_const_proxy&) = default;
            parent_aware_binary_tree_edge_const_proxy(pointer ptr, bool left)noexcept :parent{ ptr }, left{ left }
            {
            }
            parent_aware_binary_tree_edge_const_proxy(parent_aware_binary_tree_edge_proxy<T, Alloc> p)noexcept :
                parent_aware_binary_tree_edge_const_proxy{p.parent, p.left}
            {}
            parent_aware_binary_tree_edge_const_proxy& operator=(const parent_aware_binary_tree_edge_const_proxy&) = default;
            bool valid()const noexcept
            {
                return bool( parent );
            }
            bool null()const noexcept
            {
                return !bool( child() );
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            parent_aware_binary_tree_edge_const_proxy& go_left()noexcept
            {
                parent = child();
                left = true;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_right()noexcept
            {
                parent = child();
                left = false;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_up()noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                left = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent;
                parent = parent_of_parent;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data);
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(parent_aware_binary_tree_edge_const_proxy l, parent_aware_binary_tree_edge_const_proxy r)noexcept
            {
                return std::addressof(l.child()) == std::addressof(r.child());
            }
            friend struct std::hash<parent_aware_binary_tree_edge_const_proxy>;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_const_proxy<T, Alloc>;
            using value_type = T;
        };
    }
}

template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_const_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            pointer node{};
        public:
            parent_aware_binary_tree_node_proxy() = default;
            parent_aware_binary_tree_node_proxy(const parent_aware_binary_tree_node_proxy&) = default;
            parent_aware_binary_tree_node_proxy(pointer node)noexcept:node{node}
            {
            }
            parent_aware_binary_tree_node_proxy(parent_aware_binary_tree_edge_proxy<T, Alloc> p)noexcept :node{p.child()}
            {
            }
            parent_aware_binary_tree_node_proxy& operator=(const parent_aware_binary_tree_node_proxy&) = default;
            bool null()const noexcept
            {
                return node == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            parent_aware_binary_tree_node_proxy& go_left()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_right()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_up()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                return *this;
            }
            T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data);
            }
            std::pair<parent_aware_binary_tree_edge_proxy<T, Alloc>, parent_aware_binary_tree_edge_proxy<T, Alloc>> get_children()const noexcept
            {
                return { {node, true}, {node, false} };
            }
            operator parent_aware_binary_tree_edge_proxy<T, Alloc>()const noexcept
            {
                pointer parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                if (static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left == node)
                {
                    return { parent, true };
                }
                else
                {
                    return { parent, false };
                }
            }
            friend bool operator==(parent_aware_binary_tree_node_proxy l, parent_aware_binary_tree_node_proxy r)noexcept
            {
                return l.node == r.node;
            }
            friend struct std::hash<parent_aware_binary_tree_node_proxy>;
            friend class parent_aware_binary_tree_node_const_proxy<T, Alloc>;
            using value_type = T;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
        };
    }
}

template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_node_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_const_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            pointer node{};
        public:
            parent_aware_binary_tree_node_const_proxy() = default;
            parent_aware_binary_tree_node_const_proxy(const parent_aware_binary_tree_node_const_proxy&) = default;
            parent_aware_binary_tree_node_const_proxy(pointer node)noexcept :node{ node }
            {}
            parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_edge_const_proxy<T, Alloc> p)noexcept :node{ p.child() }
            {}
            parent_aware_binary_tree_node_const_proxy& operator=(const parent_aware_binary_tree_node_const_proxy&) = default;
            bool null()const noexcept
            {
                return node == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            parent_aware_binary_tree_node_const_proxy& go_left()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_right()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_up()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data);
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy<T, Alloc>, parent_aware_binary_tree_edge_const_proxy<T, Alloc>> get_children()const noexcept
            {
                return { {node, true}, {node, false} };
            }
            operator parent_aware_binary_tree_edge_const_proxy<T, Alloc>()const noexcept
            {
                pointer parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                if (static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left == node)
                {
                    return { parent, true };
                }
                else
                {
                    return { parent, false };
                }
            }
            friend bool operator==(parent_aware_binary_tree_node_const_proxy l, parent_aware_binary_tree_node_const_proxy r)noexcept
            {
                return l.node == r.node;
            }
            friend struct std::hash<parent_aware_binary_tree_node_const_proxy>;
            using value_type = T;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
        };
    }
}

template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_node_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_const_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        struct parent_aware_binary_tree_get_children_t
        {
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_edge_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_edge_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_node_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_node_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace parent_aware_binary_tree_functional
    {
        constexpr inline Yc::details::parent_aware_binary_tree_get_children_t get_children{};
    }
    template<class T, class Alloc = std::allocator<T>>
    class parent_aware_binary_tree
    {
        using place_holder_allocator = std::allocator_traits<Alloc>::template
            rebind_alloc<details::parent_aware_binary_tree_empty_placeholder>;
        using node_pointer = std::allocator_traits<place_holder_allocator>::pointer;
        using node_type = details::parent_aware_binary_tree_node<T, place_holder_allocator>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
    public:
        using value_type = T;
        using reference = T&;
        using edge_proxy = details::parent_aware_binary_tree_edge_proxy<T, place_holder_allocator>;
        using edge_const_proxy = details::parent_aware_binary_tree_edge_const_proxy<T, place_holder_allocator>;
        using node_proxy = details::parent_aware_binary_tree_node_proxy<T, place_holder_allocator>;
        using node_const_proxy = details::parent_aware_binary_tree_node_const_proxy<T, place_holder_allocator>;
    private:
        [[no_unique_address]] Alloc alloc{};
        // 内部虚拟节点，只能被节点代理指代
        // 只有左子节点
        details::parent_aware_binary_tree_node_base1<place_holder_allocator> root_node{};
        node_pointer pointer_to(const details::parent_aware_binary_tree_empty_placeholder& ref)const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to((details::parent_aware_binary_tree_empty_placeholder&)ref);
        }
        node_pointer allocate_node()
        {
            node_allocator a{ alloc };
            node_pointer p = std::pointer_traits<node_pointer>::pointer_to
                (static_cast<details::parent_aware_binary_tree_empty_placeholder&>(*std::allocator_traits<node_allocator>::allocate(a, 1)));
            std::allocator_traits<node_allocator>::construct(a, (node_type*)std::to_address(p));
            return p;
        }
        void deallocate_node(node_pointer p)noexcept
        {
            node_allocator a{ alloc };
            std::allocator_traits<node_allocator>::destroy(a, (node_type*)std::to_address(p));
            return std::allocator_traits<node_allocator>::deallocate(a,
                std::pointer_traits<typename std::allocator_traits<node_allocator>::pointer>::pointer_to(*(node_type*)std::to_address(p)), 1);
        }
    public:
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        node_proxy parent_of_nroot()noexcept
        {
            return { pointer_to(root_node) };
        }
        node_const_proxy parent_of_nroot()const noexcept
        {
            return { pointer_to(root_node) };
        }
        node_const_proxy parent_of_cnroot()const noexcept
        {
            return { pointer_to(root_node) };
        }
        edge_proxy root()noexcept
        {
            return { pointer_to(root_node), true };
        }
        edge_const_proxy root()const noexcept
        {
            return { pointer_to(root_node), true };
        }
        edge_const_proxy croot()const noexcept
        {
            return { pointer_to(root_node), true };
        }
        node_proxy nroot()noexcept
        {
            return parent_of_nroot().go_left();
        }
        node_const_proxy nroot()const noexcept
        {
            return parent_of_nroot().go_left();
        }
        node_const_proxy cnroot()const noexcept
        {
            return parent_of_cnroot().go_left();
        }
    private:
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        void recur_and_write_impl(
            edge_const_proxy p,
            ValueGetter& vg,
            ChildrenGetter& cg,
            InitializeHandle h
        )
        {
        derecur:
            if (!p.null())
            {
                *((int*)nullptr);
            }
            emplace(p, std::invoke(vg, h));
            auto [lh, rh] = std::invoke(cg, h);
            auto [l, r] = p.get_children();
            bool bl = (bool)lh, br = (bool)rh;
            if (!(bl && br))
            {
                if (bl)
                {
                    p = l;
                    h = lh;
                    goto derecur;
                }
                if (br)
                {
                tmp:
                    p = r;
                    h = rh;
                    goto derecur;
                }
                return;
            }
            recur_and_write_impl(l, vg, cg, lh);
            goto tmp;
        }
    public:
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >requires requires (
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h,
            parent_aware_binary_tree b
            )
        {
            b.emplace(p, std::invoke(vg, h));
            (bool)h;
        }
        parent_aware_binary_tree recur_and_write(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
        {
            parent_aware_binary_tree ret{ cut(p) };
            struct _guard
            {
                parent_aware_binary_tree* t;
                edge_const_proxy p;
                parent_aware_binary_tree* r;
                ~_guard()
                {
                    if (t)
                    {
                        t->splice(p, *r);
                    }
                }
            }guard{ this, p, &ret };
            if (h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;
            return ret;
        }
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        parent_aware_binary_tree(
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h,
            const Alloc& a = {}
        ) :alloc{ a }
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        parent_aware_binary_tree(const parent_aware_binary_tree& b) :parent_aware_binary_tree{ b.alloc }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write
                (root(), copier, parent_aware_binary_tree_functional::get_children, b.croot());
        }
        parent_aware_binary_tree& operator=(const parent_aware_binary_tree& b)
        {
            Alloc tmp{ alloc };
            parent_aware_binary_tree t1{ cut(root()) };
            struct _guard
            {
                parent_aware_binary_tree* t{};
                Alloc* tmp{};
                parent_aware_binary_tree* other{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                        splice(*this, root(), *other);
                    }
                }
            }guard{ this, &tmp, &t1 };
            if constexpr (
                std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
            {
                alloc = b.alloc;
            }
            auto copier = [](edge_const_proxy p)->const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, parent_aware_binary_tree_functional::get_children, b.root());
            guard.t = nullptr;
            return *this;
        }
        parent_aware_binary_tree cut(edge_const_proxy p)noexcept
        {
            typename edge_const_proxy::pointer& place = p.child();
            parent_aware_binary_tree ret{ alloc };
            ret.root_node.left = std::exchange(place, {});
            
            if (ret.root_node.left)
            {
                static_cast<node_type&>(*ret.root_node.left).parent = pointer_to(ret.root_node);
            }
            return ret;
        }
        
        bool empty()const noexcept
        {
            return root_node.left == nullptr;
        }
        void erase(edge_const_proxy p)noexcept
        {
            if (!p.null())
            {
                typename edge_const_proxy::pointer& place = p.child();
                while (true)
                {
                    auto [l, r] = p.get_children();
                    bool l_null = l.null();
                    bool r_null = r.null();
                    if (l_null && r_null)
                    {
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = nullptr;
                        return;
                    }
                    if (!r_null)
                    {
                        erase(l);
                        typename edge_const_proxy::pointer right = r.child();
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = right;
                        continue;
                    }
                    if ((!l_null) && r_null)
                    {
                        typename edge_const_proxy::pointer left = l.child();
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = left;
                        continue;
                    }
                }
            }
        }
        void clear()noexcept
        {
            erase(root());
        }

        template<class... Args>
        parent_aware_binary_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_pointer new_node = allocate_node();
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    parent_aware_binary_tree* t;
                    node_pointer ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate_node(ptr);
                        }
                    }
                }guard{ this,new_node };
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            parent_aware_binary_tree ret{ cut(p) };
            p.child() = new_node;
            static_cast<node_type&>(*new_node).parent = p.parent;
            return ret;
        }
        parent_aware_binary_tree insert(edge_const_proxy p, const T& v)
        {
            return emplace(p, v);
        }
        parent_aware_binary_tree insert(edge_const_proxy p, T&& v)
        {
            return emplace(p, std::move(v));
        }

        parent_aware_binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            parent_aware_binary_tree ret{ cut(to) };
            to.child() = std::exchange(from.child(), {});
            if (to)
            {
                static_cast<node_type&>(*to.child()).parent = to.parent;
            }
            return ret;
        }

        parent_aware_binary_tree splice(edge_const_proxy to, parent_aware_binary_tree&, edge_const_proxy from)noexcept
        {
            return splice(to, from);
        }

        parent_aware_binary_tree splice(edge_const_proxy to, parent_aware_binary_tree&&, edge_const_proxy from)noexcept
        {
            return splice(to, from);
        }

        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }

        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }

        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            typename edge_const_proxy::pointer& place_l = l.child();
            typename edge_const_proxy::pointer& place_r = r.child();
            std::swap(place_l, place_r);
            if (place_l)
            {
                static_cast<node_type&>(*place_l).parent = l.parent;
            }
            if (place_r)
            {
                static_cast<node_type&>(*place_r).parent = r.parent;
            }
        }

        static void swap_node(edge_const_proxy l, edge_const_proxy r)
        {
            swap_sub_tree(l, r);
            auto [ll, lr] = l.get_children();
            auto [rl, rr] = r.get_children();
            swap_sub_tree(ll, rl);
            swap_sub_tree(lr, rr);
        }

        void swap(parent_aware_binary_tree& other)noexcept
        {
            swap_sub_tree(root(), other.root());
        }

        parent_aware_binary_tree() = default;
        explicit parent_aware_binary_tree(const Alloc& a)noexcept:alloc{a}
        {
        }
        

        parent_aware_binary_tree(parent_aware_binary_tree&& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
            }
            swap(other);
        }
        parent_aware_binary_tree& operator=(parent_aware_binary_tree&& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
            }
            swap(other);
            return *this;
        }
        ~parent_aware_binary_tree()
        {
            clear();
        }

        void left_rotate(edge_const_proxy p)noexcept
        {
            //     |                         |
            //    <A>                       <C>
            //   /   \                      / \
            //  <B>  <C>     =====>       <A> <E>
            //       / \                  / \
            //     <D> <E>              <B> <D>
            edge_const_proxy q = p;
            q.go_right();
            parent_aware_binary_tree t1 = cut(q); // <D>-<rC>-<E>
            parent_aware_binary_tree t2 = splice(p, t1.root()); //<B>-<rA>
            //     |
            //    <C>                 <A>
            //   /   \                /
            //  <D>  <E>            <B>
            q = p;
            q.go_left();
            parent_aware_binary_tree t3 = splice(q, t2.root());
            q.go_right();
            splice(q, t3.root());
            if (!t1.empty())
            {
                std::unreachable();
            }
            if (!t2.empty())
            {
                std::unreachable();
            }
            if (!t3.empty())
            {
                std::unreachable();
            }
        }

        void right_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            q.go_left();
            parent_aware_binary_tree t1 = cut(q);
            parent_aware_binary_tree t2 = splice(p, t1.root());
            q = p;
            q.go_right();
            parent_aware_binary_tree t3 = splice(q, t2.root());
            q.go_left();
            splice(q, t3.root());
            if (!t1.empty())
            {
                std::unreachable();
            }
            if (!t2.empty())
            {
                std::unreachable();
            }
            if (!t3.empty())
            {
                std::unreachable();
            }
        }

        bool checked_left_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            if (p.null() || (p.go_right(), p.null()))
            {
                return false;
            }
            left_rotate(q);
            return true;
        }

        bool checked_right_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            if (p.null() || (p.go_left(), p.null()))
            {
                return false;
            }
            right_rotate(q);
            return true;
        }

    };
    template<class T>
    void swap(parent_aware_binary_tree<T>& l, parent_aware_binary_tree<T>& r)noexcept
    {
        l.swap(r);
    }
}