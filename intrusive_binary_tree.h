// ===========================================================================
// intrusive_binary_tree.h
//
// 侵入式二叉树。与 intrusive_list.h 同一套设计思路：
//   * 节点钩子（node base）只保存结构指针，不保存用户数据、不管理生命周期；
//   * 用户把节点钩子作为自己类型的基类（或用 intrusive_binary_tree_node
//     就地存放一个 T），节点地址一经分配就不再改变；
//   * 树对象（basic_*_tree）只持有一个根槽位 / 哨兵，不拥有任何节点；
//     销毁树对象不会销毁节点，销毁节点前必须先把节点从树上摘下来。
//
// 公开建模只使用 proxy：
//   * edge_proxy 代表一个"树位置"（父节点到子节点之间的槽位）；
//   * node_proxy 代表一个"节点"；
//   * 节点指针类型、槽位指针类型都是实现细节，不出现在公开签名中。
//   * 侵入式层不知道 T；node_proxy 的 operator* 返回 node_base&，用户
//     static_cast 到自己的节点类型。非侵入式容器在外层 proxy 里补上 T。
//
// 本文件提供两类设施：
//   1. 无父指针：intrusive_binary_tree_node_base / basic_intrusive_binary_tree
//      节点只有 left、right 两个钩子，最轻量；一条边就是"某个槽位"。
//   2. 带父指针：intrusive_parent_aware_binary_tree_node_base /
//      basic_intrusive_parent_aware_binary_tree
//      节点额外保存 parent，容器内部有一个只含 left 的哨兵；一条边由
//      （父节点，左/右）表示，可以在常数时间内 go_up，并且所有结构操作
//      都维护 parent 链。
//
// 约定：
//   * vptr 是节点指针的"种子"类型：默认 void*；上层也可以传入分配器的
//     pointer（例如缀饰指针的 fancy<T>）。所有节点链接都由
//     pointer_traits<vptr>::rebind 得到。
//     不要传 fancy<void>：某些缀饰指针的 pointer_to 需要 void&，无法形成；
//     传 fancy<T> 同样可以 rebind 出 fancy<node_base>。
//   * 节点钩子禁止拷贝：逐位复制会让两份钩子指向同一组邻居，结构会静默
//     损坏；节点本来就是"就地构造、挂在树上"的对象，不支持搬迁语义。
//   * 结构函数都假定调用方满足前置条件（例如旋转要求对应孩子存在）。
//     检查版旋转 checked_* 会把前置条件变成返回值。
// ===========================================================================
#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace Yc
{
    template<class vptr = void*>
    class basic_intrusive_binary_tree;
    template<class vptr = void*>
    class basic_intrusive_parent_aware_binary_tree;

    // ===================================================================
    // 无父指针：节点钩子与两个 proxy
    // ===================================================================
    template<class vptr = void*>
    class intrusive_binary_tree_node_base;
    template<class vptr = void*>
    class intrusive_binary_tree_node_proxy;
    template<class vptr = void*>
    class intrusive_binary_tree_edge_proxy;

    template<class vptr>
    class intrusive_binary_tree_node_base
    {
        using node_pointer = typename std::pointer_traits<vptr>::template
            rebind<intrusive_binary_tree_node_base>;
        static_assert(
            std::is_same_v<typename std::pointer_traits<node_pointer>::element_type,
                intrusive_binary_tree_node_base>,
            "pointer_traits<vptr>::rebind must yield a pointer to the node base");

        node_pointer left_{};
        node_pointer right_{};

        template<class>
        friend class basic_intrusive_binary_tree;
        template<class>
        friend class intrusive_binary_tree_node_proxy;
        template<class>
        friend class intrusive_binary_tree_edge_proxy;

        static node_pointer to_pointer(intrusive_binary_tree_node_base& n) noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to(n);
        }

    public:
        intrusive_binary_tree_node_base() = default;
        intrusive_binary_tree_node_base(const intrusive_binary_tree_node_base&) = delete;
        intrusive_binary_tree_node_base& operator=(const intrusive_binary_tree_node_base&) = delete;
    };

    // 节点 proxy：只暴露节点句柄与导航，不暴露内部指针。
    template<class vptr>
    class intrusive_binary_tree_node_proxy
    {
        using node_base = intrusive_binary_tree_node_base<vptr>;
        using node_pointer = typename std::pointer_traits<vptr>::template
            rebind<node_base>;

        node_pointer node_{};

        template<class>
        friend class basic_intrusive_binary_tree;
        template<class>
        friend class intrusive_binary_tree_edge_proxy;

        explicit intrusive_binary_tree_node_proxy(node_pointer p) noexcept
            : node_{ p }
        {
        }

    public:
        intrusive_binary_tree_node_proxy() = default;
        explicit intrusive_binary_tree_node_proxy(node_base& n) noexcept
            : node_{ node_base::to_pointer(n) }
        {
        }
        bool null()const noexcept
        {
            return !node_;
        }
        explicit operator bool()const noexcept
        {
            return !null();
        }
        // 只返回节点基类引用：侵入式层不知道 T
        node_base& operator*()const noexcept
        {
            return *node_;
        }
        intrusive_binary_tree_node_proxy& go_left()noexcept
        {
            node_ = node_->left_;
            return *this;
        }
        intrusive_binary_tree_node_proxy& go_right()noexcept
        {
            node_ = node_->right_;
            return *this;
        }
        intrusive_binary_tree_node_proxy get_left()const noexcept
        {
            return intrusive_binary_tree_node_proxy{ node_->left_ };
        }
        intrusive_binary_tree_node_proxy get_right()const noexcept
        {
            return intrusive_binary_tree_node_proxy{ node_->right_ };
        }
        bool is_leaf()const noexcept
        {
            return !node_ || (!node_->left_ && !node_->right_);
        }
        std::pair<intrusive_binary_tree_edge_proxy<vptr>,
            intrusive_binary_tree_edge_proxy<vptr>> get_children()const noexcept;

        friend bool operator==(intrusive_binary_tree_node_proxy l,
            intrusive_binary_tree_node_proxy r)noexcept
        {
            return l.node_ == r.node_;
        }
        friend struct std::hash<intrusive_binary_tree_node_proxy>;
    };

    // 边 proxy：代表父节点到子节点的槽位。
    template<class vptr>
    class intrusive_binary_tree_edge_proxy
    {
        using node_base = intrusive_binary_tree_node_base<vptr>;
        using node_proxy = intrusive_binary_tree_node_proxy<vptr>;
        using node_pointer = typename std::pointer_traits<vptr>::template
            rebind<node_base>;
        using pointer_to_pointer = typename std::pointer_traits<node_pointer>::
            template rebind<node_pointer>;

        pointer_to_pointer ptr_{};

        template<class>
        friend class basic_intrusive_binary_tree;

        explicit intrusive_binary_tree_edge_proxy(pointer_to_pointer p)noexcept
            : ptr_{ p }
        {
        }

    public:
        intrusive_binary_tree_edge_proxy() = default;
        // 由父节点与方向构造；父节点可以是树里的真实节点
        intrusive_binary_tree_edge_proxy(node_base& parent, bool left_child)noexcept
            : ptr_{ std::pointer_traits<pointer_to_pointer>::pointer_to(
                left_child ? parent.left_ : parent.right_) }
        {
        }
        // 由节点 proxy 与方向构造子边
        intrusive_binary_tree_edge_proxy(node_proxy parent, bool left_child)noexcept
            : ptr_{ std::pointer_traits<pointer_to_pointer>::pointer_to(
                left_child ? parent.node_->left_ : parent.node_->right_) }
        {
        }
        bool valid()const noexcept
        {
            return static_cast<bool>(ptr_);
        }
        bool null()const noexcept
        {
            return !*ptr_;
        }
        explicit operator bool()const noexcept
        {
            return valid() && (!null());
        }
        intrusive_binary_tree_edge_proxy& go_left()noexcept
        {
            ptr_ = std::pointer_traits<pointer_to_pointer>::pointer_to(
                (*ptr_)->left_);
            return *this;
        }
        intrusive_binary_tree_edge_proxy& go_right()noexcept
        {
            ptr_ = std::pointer_traits<pointer_to_pointer>::pointer_to(
                (*ptr_)->right_);
            return *this;
        }
        node_base& operator*()const noexcept
        {
            return **ptr_;
        }
        intrusive_binary_tree_edge_proxy get_left()const noexcept
        {
            return intrusive_binary_tree_edge_proxy{
                std::pointer_traits<pointer_to_pointer>::pointer_to(
                    (*ptr_)->left_) };
        }
        intrusive_binary_tree_edge_proxy get_right()const noexcept
        {
            return intrusive_binary_tree_edge_proxy{
                std::pointer_traits<pointer_to_pointer>::pointer_to(
                    (*ptr_)->right_) };
        }
        std::pair<intrusive_binary_tree_edge_proxy,
            intrusive_binary_tree_edge_proxy> get_children()const noexcept
        {
            return { get_left(), get_right() };
        }
        node_proxy get_node()const noexcept
        {
            return valid() ? node_proxy{ *ptr_ } : node_proxy{};
        }
        friend bool operator==(intrusive_binary_tree_edge_proxy l,
            intrusive_binary_tree_edge_proxy r)noexcept
        {
            return l.ptr_ == r.ptr_;
        }
        friend struct std::hash<intrusive_binary_tree_edge_proxy>;
    };

    template<class vptr>
    std::pair<intrusive_binary_tree_edge_proxy<vptr>,
        intrusive_binary_tree_edge_proxy<vptr>>
        intrusive_binary_tree_node_proxy<vptr>::get_children()const noexcept
    {
        return { intrusive_binary_tree_edge_proxy<vptr>{*this, true},
                 intrusive_binary_tree_edge_proxy<vptr>{*this, false} };
    }

    // 无父指针的侵入式二叉树：只持有一个根槽位，不拥有节点。
    template<class vptr>
    class basic_intrusive_binary_tree
    {
    public:
        using node_base = intrusive_binary_tree_node_base<vptr>;
        using node_proxy = intrusive_binary_tree_node_proxy<vptr>;
        using edge_proxy = intrusive_binary_tree_edge_proxy<vptr>;

    private:
        using node_pointer = typename std::pointer_traits<vptr>::template
            rebind<node_base>;
        using pointer_to_pointer = typename std::pointer_traits<node_pointer>::
            template rebind<node_pointer>;

        node_pointer root_{};

        static node_pointer& slot(edge_proxy e)noexcept
        {
            return *e.ptr_;
        }

        template<class F>
        static void for_each_impl(node_pointer p, F& f)
            noexcept(std::is_nothrow_invocable_v<F&, node_base&>)
        {
        derecur:
            if (!p)
            {
                return;
            }
            const node_pointer l = p->left_;
            const node_pointer r = p->right_;
            if (l)
            {
                for_each_impl(l, f);
            }
            std::invoke(f, *p);
            if (r)
            {
                p = r;
                goto derecur;
            }
        }

    public:
        basic_intrusive_binary_tree() = default;
        basic_intrusive_binary_tree(const basic_intrusive_binary_tree&) = delete;
        basic_intrusive_binary_tree& operator=(const basic_intrusive_binary_tree&) = delete;
        basic_intrusive_binary_tree(basic_intrusive_binary_tree&& other)noexcept
            : root_{ std::exchange(other.root_, node_pointer{}) }
        {
        }
        basic_intrusive_binary_tree& operator=(basic_intrusive_binary_tree&& other)noexcept
        {
            if (this != &other)
            {
                root_ = std::exchange(other.root_, node_pointer{});
            }
            return *this;
        }
        ~basic_intrusive_binary_tree() = default;

        bool empty()const noexcept
        {
            return !root_;
        }
        // 根边 / 根节点（只暴露 proxy）
        edge_proxy root()noexcept
        {
            return edge_proxy{ std::pointer_traits<pointer_to_pointer>::pointer_to(root_) };
        }
        edge_proxy root()const noexcept
        {
            return const_cast<basic_intrusive_binary_tree*>(this)->root();
        }
        node_proxy nroot()const noexcept
        {
            return node_proxy{ root_ };
        }
        // 由节点基类引用构造节点 proxy（供上层容器包装自己分配的节点）
        node_proxy node(node_base& n)const noexcept
        {
            return node_proxy{ n };
        }
        // 接管一棵已经与旧树断开的子树
        void set_root(node_proxy n)noexcept
        {
            root_ = n.node_;
        }
        // 摘下整棵树并交还给调用方
        node_proxy release_root()noexcept
        {
            return node_proxy{ std::exchange(root_, node_pointer{}) };
        }
        void swap(basic_intrusive_binary_tree& other)noexcept
        {
            std::swap(root_, other.root_);
        }

        // ---- 纯结构原语（全部使用 proxy） -----------------------------
        static void link(edge_proxy to, node_proxy child)noexcept
        {
            slot(to) = child.node_;
        }
        static node_proxy cut(edge_proxy slot_)noexcept
        {
            return node_proxy{ std::exchange(slot(slot_), node_pointer{}) };
        }
        static void splice(edge_proxy to, edge_proxy from)noexcept
        {
            slot(to) = std::exchange(slot(from), node_pointer{});
        }
        static void swap_sub_tree(edge_proxy l, edge_proxy r)noexcept
        {
            std::swap(slot(l), slot(r));
        }
        // 交换两个位置上的节点，并成对交换两个节点的孩子槽位：
        // 净效果是"两个位置上的值互换、子树留在原位、树形不变"。
        static void swap_node(edge_proxy l, edge_proxy r)noexcept
        {
            swap_sub_tree(l, r);
            node_pointer a = slot(l);
            node_pointer b = slot(r);
            if (a && b)
            {
                node_pointer al = a->left_;
                node_pointer ar = a->right_;
                node_pointer bl = b->left_;
                node_pointer br = b->right_;
                a->left_ = bl;
                b->left_ = al;
                a->right_ = br;
                b->right_ = ar;
            }
        }
        static void left_rotate(edge_proxy e)noexcept
        {
            node_pointer& p = slot(e);
            node_pointer a = p;
            node_pointer c = a->right_;
            node_pointer d = c->left_;
            a->right_ = d;
            c->left_ = a;
            p = c;
        }
        static void right_rotate(edge_proxy e)noexcept
        {
            node_pointer& p = slot(e);
            node_pointer a = p;
            node_pointer c = a->left_;
            node_pointer d = c->right_;
            a->left_ = d;
            c->right_ = a;
            p = c;
        }
        static bool checked_left_rotate(edge_proxy e)noexcept
        {
            node_pointer p = slot(e);
            if (!p || !(p->right_))
            {
                return false;
            }
            left_rotate(e);
            return true;
        }
        static bool checked_right_rotate(edge_proxy e)noexcept
        {
            node_pointer p = slot(e);
            if (!p || !(p->left_))
            {
                return false;
            }
            right_rotate(e);
            return true;
        }
        // 逐节点拆掉 e 上的子树，并对每个节点调用 del(node_proxy)。
        template<class Deleter>
        static void erase(edge_proxy e, Deleter del)noexcept
        {
            node_pointer& place = slot(e);
            if (!place)
            {
                return;
            }
            while (true)
            {
                node_base& n = *place;
                const bool has_l = static_cast<bool>(n.left_);
                const bool has_r = static_cast<bool>(n.right_);
                if (!has_l && !has_r)
                {
                    node_pointer victim = std::exchange(place, node_pointer{});
                    del(node_proxy{ victim });
                    return;
                }
                if (has_r)
                {
                    erase(edge_proxy{ n, true }, del);
                    node_pointer right = std::exchange(n.right_, node_pointer{});
                    node_pointer victim = std::exchange(place, right);
                    del(node_proxy{ victim });
                    continue;
                }
                node_pointer left_child = std::exchange(n.left_, node_pointer{});
                node_pointer victim = std::exchange(place, left_child);
                del(node_proxy{ victim });
            }
        }
        // 只拆结构、不销毁节点
        void clear()noexcept
        {
            struct noop
            {
                void operator()(node_proxy)const noexcept {}
            };
            erase(root(), noop{});
        }

        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, node_base&>&&
            std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl(root_, f);
            return f;
        }
    };

    // ===================================================================
    // 带父指针：节点钩子与两个 proxy
    // ===================================================================
    template<class vptr = void*>
    class intrusive_parent_aware_binary_tree_node_base;
    template<class vptr = void*>
    class intrusive_parent_aware_binary_tree_virtual_node_base;
    template<class vptr = void*>
    class intrusive_parent_aware_binary_tree_node_proxy;
    template<class vptr = void*>
    class intrusive_parent_aware_binary_tree_edge_proxy;

    // 第一层只含 left：哨兵只需要这一层，树对象因此只占一个指针。
    template<class vptr>
    class intrusive_parent_aware_binary_tree_virtual_node_base
    {
        using node_base = intrusive_parent_aware_binary_tree_node_base<vptr>;
        using pointer = typename std::pointer_traits<vptr>::template
            rebind<intrusive_parent_aware_binary_tree_virtual_node_base>;
        static_assert(
            std::is_same_v<typename std::pointer_traits<pointer>::element_type,
                intrusive_parent_aware_binary_tree_virtual_node_base>,
            "pointer_traits<vptr>::rebind must yield a pointer to the node hook base");

        pointer left_{};

        template<class>
        friend class basic_intrusive_parent_aware_binary_tree;
        template<class>
        friend class intrusive_parent_aware_binary_tree_node_base;
        template<class>
        friend class intrusive_parent_aware_binary_tree_node_proxy;
        template<class>
        friend class intrusive_parent_aware_binary_tree_edge_proxy;

        static pointer to_pointer(
            intrusive_parent_aware_binary_tree_virtual_node_base& n)noexcept
        {
            return std::pointer_traits<pointer>::pointer_to(n);
        }

    public:
        intrusive_parent_aware_binary_tree_virtual_node_base() = default;
        intrusive_parent_aware_binary_tree_virtual_node_base(
            const intrusive_parent_aware_binary_tree_virtual_node_base&) = delete;
        intrusive_parent_aware_binary_tree_virtual_node_base& operator=(
            const intrusive_parent_aware_binary_tree_virtual_node_base&) = delete;
    };

    template<class vptr>
    class intrusive_parent_aware_binary_tree_node_base
        : public intrusive_parent_aware_binary_tree_virtual_node_base<vptr>
    {
        using virtual_base = intrusive_parent_aware_binary_tree_virtual_node_base<vptr>;
        using pointer = typename virtual_base::pointer;

        pointer right_{};
        pointer parent_{};

        template<class>
        friend class basic_intrusive_parent_aware_binary_tree;
        template<class>
        friend class intrusive_parent_aware_binary_tree_node_proxy;
        template<class>
        friend class intrusive_parent_aware_binary_tree_edge_proxy;

    public:
        intrusive_parent_aware_binary_tree_node_base() = default;
        intrusive_parent_aware_binary_tree_node_base(
            const intrusive_parent_aware_binary_tree_node_base&) = delete;
        intrusive_parent_aware_binary_tree_node_base& operator=(
            const intrusive_parent_aware_binary_tree_node_base&) = delete;
    };

    template<class vptr>
    class intrusive_parent_aware_binary_tree_node_proxy
    {
        using virtual_base = intrusive_parent_aware_binary_tree_virtual_node_base<vptr>;
        using node_base = intrusive_parent_aware_binary_tree_node_base<vptr>;
        using pointer = typename virtual_base::pointer;

        pointer node_{};

        template<class>
        friend class basic_intrusive_parent_aware_binary_tree;
        template<class>
        friend class intrusive_parent_aware_binary_tree_edge_proxy;

        explicit intrusive_parent_aware_binary_tree_node_proxy(pointer p)noexcept
            : node_{ p }
        {
        }

    public:
        intrusive_parent_aware_binary_tree_node_proxy() = default;
        // 可以接受真实节点，也可以接受哨兵（哨兵只有 left，不能解引用）
        explicit intrusive_parent_aware_binary_tree_node_proxy(virtual_base& n)noexcept
            : node_{ virtual_base::to_pointer(n) }
        {
        }
        bool null()const noexcept
        {
            return !node_;
        }
        explicit operator bool()const noexcept
        {
            return !null();
        }
        // 真实节点的引用；哨兵不能解引用
        node_base& operator*()const noexcept
        {
            return static_cast<node_base&>(*node_);
        }
        intrusive_parent_aware_binary_tree_node_proxy& go_left()noexcept
        {
            node_ = node_->left_;
            return *this;
        }
        intrusive_parent_aware_binary_tree_node_proxy& go_right()noexcept
        {
            node_ = static_cast<node_base&>(*node_).right_;
            return *this;
        }
        intrusive_parent_aware_binary_tree_node_proxy& go_up()noexcept
        {
            node_ = static_cast<node_base&>(*node_).parent_;
            return *this;
        }
        intrusive_parent_aware_binary_tree_node_proxy get_left()const noexcept
        {
            return intrusive_parent_aware_binary_tree_node_proxy{ node_->left_ };
        }
        intrusive_parent_aware_binary_tree_node_proxy get_right()const noexcept
        {
            return intrusive_parent_aware_binary_tree_node_proxy{
                static_cast<node_base&>(*node_).right_ };
        }
        intrusive_parent_aware_binary_tree_node_proxy get_parent()const noexcept
        {
            return intrusive_parent_aware_binary_tree_node_proxy{
                static_cast<node_base&>(*node_).parent_ };
        }
        bool is_leaf()const noexcept
        {
            return !node_ || (!node_->left_ && !static_cast<node_base&>(*node_).right_);
        }
        // 指向该节点的边（要求 node_ 是真实节点，不能是哨兵）
        intrusive_parent_aware_binary_tree_edge_proxy<vptr> get_edge()const noexcept;
        std::pair<intrusive_parent_aware_binary_tree_edge_proxy<vptr>,
            intrusive_parent_aware_binary_tree_edge_proxy<vptr>> get_children()const noexcept;

        friend bool operator==(intrusive_parent_aware_binary_tree_node_proxy l,
            intrusive_parent_aware_binary_tree_node_proxy r)noexcept
        {
            return l.node_ == r.node_;
        }
        friend struct std::hash<intrusive_parent_aware_binary_tree_node_proxy>;
    };

    template<class vptr>
    class intrusive_parent_aware_binary_tree_edge_proxy
    {
        using virtual_base = intrusive_parent_aware_binary_tree_virtual_node_base<vptr>;
        using node_base = intrusive_parent_aware_binary_tree_node_base<vptr>;
        using node_proxy = intrusive_parent_aware_binary_tree_node_proxy<vptr>;
        using pointer = typename virtual_base::pointer;

        pointer parent_{};
        bool left_{ true };

        template<class>
        friend class basic_intrusive_parent_aware_binary_tree;

        pointer child()const noexcept
        {
            return left_ ? parent_->left_
                         : static_cast<node_base&>(*parent_).right_;
        }
        pointer& child_slot()const noexcept
        {
            return left_ ? parent_->left_
                         : static_cast<node_base&>(*parent_).right_;
        }

    public:
        intrusive_parent_aware_binary_tree_edge_proxy() = default;
        intrusive_parent_aware_binary_tree_edge_proxy(virtual_base& parent,
            bool left_child)noexcept
            : parent_{ virtual_base::to_pointer(parent) }, left_{ left_child }
        {
        }
        intrusive_parent_aware_binary_tree_edge_proxy(node_proxy parent,
            bool left_child)noexcept;
        bool valid()const noexcept
        {
            return static_cast<bool>(parent_);
        }
        bool null()const noexcept
        {
            return !child();
        }
        explicit operator bool()const noexcept
        {
            return valid() && (!null());
        }
        intrusive_parent_aware_binary_tree_edge_proxy& go_left()noexcept
        {
            parent_ = child();
            left_ = true;
            return *this;
        }
        intrusive_parent_aware_binary_tree_edge_proxy& go_right()noexcept
        {
            parent_ = child();
            left_ = false;
            return *this;
        }
        // 从子边走到父边；根边（父节点是哨兵）不能再向上
        intrusive_parent_aware_binary_tree_edge_proxy& go_up()noexcept
        {
            node_base& p = static_cast<node_base&>(*parent_);
            pointer up = p.parent_;
            left_ = static_cast<virtual_base&>(*up).left_ == parent_;
            parent_ = up;
            return *this;
        }
        node_base& operator*()const noexcept
        {
            return static_cast<node_base&>(*child());
        }
        intrusive_parent_aware_binary_tree_edge_proxy get_left()const noexcept
        {
            return intrusive_parent_aware_binary_tree_edge_proxy{
                static_cast<virtual_base&>(*child()), true };
        }
        intrusive_parent_aware_binary_tree_edge_proxy get_right()const noexcept
        {
            return intrusive_parent_aware_binary_tree_edge_proxy{
                static_cast<virtual_base&>(*child()), false };
        }
        intrusive_parent_aware_binary_tree_edge_proxy get_parent()const noexcept
        {
            node_base& p = static_cast<node_base&>(*parent_);
            pointer up = p.parent_;
            return intrusive_parent_aware_binary_tree_edge_proxy{
                static_cast<virtual_base&>(*up),
                static_cast<virtual_base&>(*up).left_ == parent_ };
        }
        std::pair<intrusive_parent_aware_binary_tree_edge_proxy,
            intrusive_parent_aware_binary_tree_edge_proxy> get_children()const noexcept
        {
            return { get_left(), get_right() };
        }
        node_proxy get_node()const noexcept
        {
            return valid() ? node_proxy{ child() } : node_proxy{};
        }
        bool is_left()const noexcept
        {
            return left_;
        }
        friend bool operator==(intrusive_parent_aware_binary_tree_edge_proxy l,
            intrusive_parent_aware_binary_tree_edge_proxy r)noexcept
        {
            return l.parent_ == r.parent_ && l.left_ == r.left_;
        }
        friend struct std::hash<intrusive_parent_aware_binary_tree_edge_proxy>;
    };

    // 由节点 proxy 构造子边；parent 为空时得到无效边
    template<class vptr>
    intrusive_parent_aware_binary_tree_edge_proxy<vptr>::
        intrusive_parent_aware_binary_tree_edge_proxy(
            intrusive_parent_aware_binary_tree_node_proxy<vptr> parent,
            bool left_child)noexcept
        : parent_{ parent.node_ }, left_{ left_child }
    {
    }

    template<class vptr>
    intrusive_parent_aware_binary_tree_edge_proxy<vptr>
        intrusive_parent_aware_binary_tree_node_proxy<vptr>::get_edge()const noexcept
    {
        auto p = static_cast<node_base&>(*node_).parent_;
        return intrusive_parent_aware_binary_tree_edge_proxy<vptr>{
            static_cast<virtual_base&>(*p),
            static_cast<virtual_base&>(*p).left_ == node_ };
    }

    template<class vptr>
    std::pair<intrusive_parent_aware_binary_tree_edge_proxy<vptr>,
        intrusive_parent_aware_binary_tree_edge_proxy<vptr>>
        intrusive_parent_aware_binary_tree_node_proxy<vptr>::get_children()const noexcept
    {
        return { intrusive_parent_aware_binary_tree_edge_proxy<vptr>{*this, true},
                 intrusive_parent_aware_binary_tree_edge_proxy<vptr>{*this, false} };
    }

    // 带父指针的侵入式二叉树：内部只持有 virtual_base 哨兵。
    template<class vptr>
    class basic_intrusive_parent_aware_binary_tree
    {
    public:
        using virtual_base = intrusive_parent_aware_binary_tree_virtual_node_base<vptr>;
        using node_base = intrusive_parent_aware_binary_tree_node_base<vptr>;
        using node_proxy = intrusive_parent_aware_binary_tree_node_proxy<vptr>;
        using edge_proxy = intrusive_parent_aware_binary_tree_edge_proxy<vptr>;

    private:
        using pointer = typename virtual_base::pointer;

        virtual_base sentinel_{};

        static node_base& as_node(pointer p)noexcept
        {
            return static_cast<node_base&>(*p);
        }
        pointer sentinel_pointer()noexcept
        {
            return virtual_base::to_pointer(sentinel_);
        }
        template<class F>
        static void for_each_impl(pointer p, F& f)
            noexcept(std::is_nothrow_invocable_v<F&, node_base&>)
        {
        derecur:
            if (!p)
            {
                return;
            }
            const pointer l = p->left_;
            const pointer r = as_node(p).right_;
            if (l)
            {
                for_each_impl(l, f);
            }
            std::invoke(f, as_node(p));
            if (r)
            {
                p = r;
                goto derecur;
            }
        }

    public:
        basic_intrusive_parent_aware_binary_tree()noexcept = default;
        basic_intrusive_parent_aware_binary_tree(
            const basic_intrusive_parent_aware_binary_tree&) = delete;
        basic_intrusive_parent_aware_binary_tree& operator=(
            const basic_intrusive_parent_aware_binary_tree&) = delete;
        basic_intrusive_parent_aware_binary_tree(
            basic_intrusive_parent_aware_binary_tree&& other)noexcept
        {
            swap(other);
        }
        basic_intrusive_parent_aware_binary_tree& operator=(
            basic_intrusive_parent_aware_binary_tree&& other)noexcept
        {
            if (this != &other)
            {
                clear();
                swap(other);
            }
            return *this;
        }
        ~basic_intrusive_parent_aware_binary_tree() = default;

        bool empty()const noexcept
        {
            return !sentinel_.left_;
        }
        edge_proxy root()noexcept
        {
            return edge_proxy{ sentinel_, true };
        }
        edge_proxy root()const noexcept
        {
            return const_cast<basic_intrusive_parent_aware_binary_tree*>(this)->root();
        }
        node_proxy nroot()const noexcept
        {
            return node_proxy{ sentinel_.left_ };
        }
        // 哨兵对应的节点 proxy（用于 parent_of_nroot）
        node_proxy parent_of_nroot()const noexcept
        {
            return node_proxy{ const_cast<virtual_base&>(sentinel_) };
        }
        node_proxy node(virtual_base& n)const noexcept
        {
            return node_proxy{ n };
        }
        void set_root(node_proxy n)noexcept
        {
            sentinel_.left_ = n.node_;
            if (n)
            {
                as_node(n.node_).parent_ = sentinel_pointer();
            }
        }
        node_proxy release_root()noexcept
        {
            pointer p = std::exchange(sentinel_.left_, pointer{});
            if (p)
            {
                as_node(p).parent_ = pointer{};
            }
            return node_proxy{ p };
        }
        void swap(basic_intrusive_parent_aware_binary_tree& other)noexcept
        {
            std::swap(sentinel_.left_, other.sentinel_.left_);
            if (sentinel_.left_)
            {
                as_node(sentinel_.left_).parent_ = sentinel_pointer();
            }
            if (other.sentinel_.left_)
            {
                as_node(other.sentinel_.left_).parent_ =
                    virtual_base::to_pointer(other.sentinel_);
            }
        }

        // ---- 纯结构原语（全部使用 proxy） -----------------------------
        static void link(edge_proxy to, node_proxy child)noexcept
        {
            to.child_slot() = child.node_;
            if (child)
            {
                as_node(child.node_).parent_ = to.parent_;
            }
        }
        static node_proxy unlink(edge_proxy e)noexcept
        {
            pointer p = std::exchange(e.child_slot(), pointer{});
            if (p)
            {
                as_node(p).parent_ = pointer{};
            }
            return node_proxy{ p };
        }
        static void splice(edge_proxy to, edge_proxy from)noexcept
        {
            if (to == from)
            {
                return;
            }
            node_proxy p = unlink(from);
            link(to, p);
        }
        static void swap_sub_tree(edge_proxy l, edge_proxy r)noexcept
        {
            if (l == r)
            {
                return;
            }
            pointer lp = l.child();
            pointer rp = r.child();
            l.child_slot() = rp;
            if (rp)
            {
                as_node(rp).parent_ = l.parent_;
            }
            r.child_slot() = lp;
            if (lp)
            {
                as_node(lp).parent_ = r.parent_;
            }
        }
        // 交换两个位置上的节点，并成对交换两个节点的孩子槽位。
        static void swap_node(edge_proxy l, edge_proxy r)noexcept
        {
            if (l == r)
            {
                return;
            }
            swap_sub_tree(l, r);
            pointer a = l.child();
            pointer b = r.child();
            if (a && b)
            {
                pointer al = a->left_;
                pointer ar = as_node(a).right_;
                pointer bl = b->left_;
                pointer br = as_node(b).right_;
                link(edge_proxy{ *a, true }, node_proxy{ bl });
                link(edge_proxy{ *b, true }, node_proxy{ al });
                link(edge_proxy{ *a, false }, node_proxy{ br });
                link(edge_proxy{ *b, false }, node_proxy{ ar });
            }
        }
        static void left_rotate(edge_proxy e)noexcept
        {
            pointer& p = e.child_slot();
            pointer a = p;
            pointer c = as_node(a).right_;
            pointer d = c->left_;
            pointer up = as_node(a).parent_;
            link(edge_proxy{ *a, false }, node_proxy{ d });
            link(edge_proxy{ *c, true }, node_proxy{ a });
            as_node(c).parent_ = up;
            p = c;
        }
        static void right_rotate(edge_proxy e)noexcept
        {
            pointer& p = e.child_slot();
            pointer a = p;
            pointer c = a->left_;
            pointer d = as_node(c).right_;
            pointer up = as_node(a).parent_;
            link(edge_proxy{ *a, true }, node_proxy{ d });
            link(edge_proxy{ *c, false }, node_proxy{ a });
            as_node(c).parent_ = up;
            p = c;
        }
        static bool checked_left_rotate(edge_proxy e)noexcept
        {
            pointer p = e.child();
            if (!p || !(as_node(p).right_))
            {
                return false;
            }
            left_rotate(e);
            return true;
        }
        static bool checked_right_rotate(edge_proxy e)noexcept
        {
            pointer p = e.child();
            if (!p || !(p->left_))
            {
                return false;
            }
            right_rotate(e);
            return true;
        }
        // 拆掉 e 上的整棵子树，逐个调用 del(node_proxy)。
        template<class Deleter>
        static void erase(edge_proxy e, Deleter del)noexcept
        {
            pointer& place = e.child_slot();
            if (!place)
            {
                return;
            }
            while (true)
            {
                node_base& n = as_node(place);
                if (!n.left_ && !n.right_)
                {
                    pointer victim = std::exchange(place, pointer{});
                    n.parent_ = pointer{};
                    del(node_proxy{ victim });
                    return;
                }
                if (n.right_)
                {
                    erase(edge_proxy{ n, true }, del);
                    pointer right = std::exchange(n.right_, pointer{});
                    pointer up = n.parent_;
                    pointer victim = std::exchange(place, right);
                    if (right)
                    {
                        as_node(right).parent_ = up;
                    }
                    n.parent_ = pointer{};
                    del(node_proxy{ victim });
                    continue;
                }
                pointer left_child = std::exchange(n.left_, pointer{});
                pointer up = n.parent_;
                pointer victim = std::exchange(place, left_child);
                if (left_child)
                {
                    as_node(left_child).parent_ = up;
                }
                n.parent_ = pointer{};
                del(node_proxy{ victim });
            }
        }
        void clear()noexcept
        {
            struct noop
            {
                void operator()(node_proxy)const noexcept {}
            };
            erase(root(), noop{});
        }

        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, node_base&>&&
            std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl(sentinel_.left_, f);
            return f;
        }
    };

    // ===================================================================
    // 带数据的侵入式节点：把节点钩子 + T 放在同一块存储里。
    // ===================================================================
    template<class vptr, class T>
    class intrusive_binary_tree_node : public intrusive_binary_tree_node_base<vptr>
    {
        union data_holder
        {
            T data;
            data_holder() {}
            ~data_holder() {}
        } d;
    public:
        intrusive_binary_tree_node() = default;
        template<class Alloc, class... Args>
        void construct_value(Alloc& alloc, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        {
            std::allocator_traits<Alloc>::construct(alloc, std::addressof(d.data),
                std::forward<Args>(args)...);
        }
        template<class Alloc>
        void destroy_value(Alloc& alloc) noexcept
        {
            std::allocator_traits<Alloc>::destroy(alloc, std::addressof(d.data));
        }
        T& value() noexcept
        {
            return d.data;
        }
        const T& value() const noexcept
        {
            return d.data;
        }
    };

    template<class vptr, class T>
    class intrusive_parent_aware_binary_tree_node
        : public intrusive_parent_aware_binary_tree_node_base<vptr>
    {
        union data_holder
        {
            T data;
            data_holder() {}
            ~data_holder() {}
        } d;
    public:
        intrusive_parent_aware_binary_tree_node() = default;
        template<class Alloc, class... Args>
        void construct_value(Alloc& alloc, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        {
            std::allocator_traits<Alloc>::construct(alloc, std::addressof(d.data),
                std::forward<Args>(args)...);
        }
        template<class Alloc>
        void destroy_value(Alloc& alloc) noexcept
        {
            std::allocator_traits<Alloc>::destroy(alloc, std::addressof(d.data));
        }
        T& value() noexcept
        {
            return d.data;
        }
        const T& value() const noexcept
        {
            return d.data;
        }
    };
}

// std::hash 特化：只作为 proxy 的实现细节，不暴露内部指针类型。
template<class vptr>
struct std::hash<Yc::intrusive_binary_tree_node_proxy<vptr>>
{
    size_t operator()(const Yc::intrusive_binary_tree_node_proxy<vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        return std::hash<const void*>{}(std::addressof(*p));
    }
};
template<class vptr>
struct std::hash<Yc::intrusive_binary_tree_edge_proxy<vptr>>
{
    size_t operator()(const Yc::intrusive_binary_tree_edge_proxy<vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<const void*>{}(std::addressof(*p.ptr_));
    }
};
template<class vptr>
struct std::hash<Yc::intrusive_parent_aware_binary_tree_node_proxy<vptr>>
{
    size_t operator()(const Yc::intrusive_parent_aware_binary_tree_node_proxy<vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        // 只取钩子对象的地址，哨兵也能安全哈希
        return std::hash<const void*>{}(std::addressof(*p.node_));
    }
};
template<class vptr>
struct std::hash<Yc::intrusive_parent_aware_binary_tree_edge_proxy<vptr>>
{
    size_t operator()(const Yc::intrusive_parent_aware_binary_tree_edge_proxy<vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<const void*>{}(std::addressof(p.child_slot()));
    }
};
