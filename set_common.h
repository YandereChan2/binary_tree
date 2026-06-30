#pragma once
namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        struct set_iterator
        {
            using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
                allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
            typename tree_type::node_const_proxy p;
            set_iterator() = default;
            set_iterator(const set_iterator&) = default;
            set_iterator(typename tree_type::node_const_proxy p)noexcept :p{ p }
            {}
            set_iterator& operator=(const set_iterator&) = default;
            set_iterator& operator++()noexcept
            {
                auto r = p.get_right();
                if (r)
                {
                    p = r;
                    while (true)
                    {
                        auto left = p.get_left();
                        if (!left)
                            return *this;
                        p = left;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p = p.get_parent();
                        if (q == p.get_left())
                            return *this;
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
                auto l = p.get_left();
                if (l)
                {
                    p = l;
                    while (true)
                    {
                        auto right = p.get_right();
                        if (!right)
                            return *this;
                        p = right;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p = p.get_parent();
                        if (q == p.get_right())
                            return *this;
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
            friend bool operator==(set_iterator, set_iterator)noexcept = default;
        };
    }
}