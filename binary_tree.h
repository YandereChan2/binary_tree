#pragma once
#include <memory>
#include <type_traits>
#include <functional>
#include "union_data.h"

namespace Yc
{
	template<class T, class Alloc>
	class binary_tree;
	namespace details
	{
		struct binary_tree_node_base
		{
			binary_tree_node_base* left{};
			binary_tree_node_base* right{};
		};
		template<class T>
		struct binary_tree_node : binary_tree_node_base
		{
			data_t<T> data;
		};
		template<class T>
		class binary_tree_edge_const_proxy;
		template<class T>
		class binary_tree_node_proxy;
		template<class T>
		class binary_tree_node_const_proxy;
		template<class T>
		class binary_tree_edge_proxy
		{
			binary_tree_node_base** ptr{};
		public:
			binary_tree_edge_proxy() = default;
			binary_tree_edge_proxy(const binary_tree_edge_proxy&) = default;
			binary_tree_edge_proxy(binary_tree_node_base** ptr)noexcept :ptr{ ptr }
			{
			}
			binary_tree_edge_proxy& operator=(const binary_tree_edge_proxy&) = default;
			bool valid()const noexcept
			{
				return (bool)ptr;
			}
			bool null()const noexcept
			{
				return !(bool)*ptr;
			}
			explicit operator bool()const noexcept
			{
				return valid() && (!null());
			}
			binary_tree_edge_proxy& go_left()noexcept
			{
				ptr = &((*ptr)->left);
				return *this;
			}
			binary_tree_edge_proxy& go_right()noexcept
			{
				ptr = &((*ptr)->right);
				return *this;
			}
			T& operator*()const noexcept
			{
				return ((binary_tree_node<T>*)(*ptr))->data.data;
			}
			T* operator->()const noexcept
			{
				return std::addressof(((binary_tree_node<T>*)(*ptr))->data.data);
			}
			std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> get_children()const noexcept
			{
				std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> ret{ *this, *this };
				ret.first.go_left();
				ret.second.go_right();
				return ret;
			}
			friend bool operator==(binary_tree_edge_proxy l, binary_tree_edge_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<binary_tree_edge_proxy>;
			friend class binary_tree_edge_const_proxy<T>;
			template<class T1, class Alloc>
			friend class Yc::binary_tree;
			friend class binary_tree_node_proxy<T>;
			using value_type = T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::binary_tree_edge_proxy<T>>
{
	size_t operator()(const Yc::details::binary_tree_edge_proxy<T>& para)const noexcept
	{
		return std::hash<Yc::details::binary_tree_node_base**>{}(para.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class binary_tree_edge_const_proxy
		{
			binary_tree_node_base** ptr{};
		public:
			binary_tree_edge_const_proxy() = default;
			binary_tree_edge_const_proxy(const binary_tree_edge_const_proxy&) = default;
			binary_tree_edge_const_proxy(const binary_tree_edge_proxy<T>& p)noexcept : ptr{ p.ptr }
			{
			}
			binary_tree_edge_const_proxy(binary_tree_node_base** ptr)noexcept :ptr{ ptr }
			{
			}
			binary_tree_edge_const_proxy& operator=(const binary_tree_edge_const_proxy&) = default;
			bool valid()const noexcept
			{
				return (bool)ptr;
			}
			bool null()const noexcept
			{
				return !(bool)*ptr;
			}
			explicit operator bool()const noexcept
			{
				return valid() && (!null());
			}
			binary_tree_edge_const_proxy& go_left()noexcept
			{
				ptr = &((*ptr)->left);
				return *this;
			}
			binary_tree_edge_const_proxy& go_right()noexcept
			{
				ptr = &((*ptr)->right);
				return *this;
			}
			const T& operator*()const noexcept
			{
				return ((binary_tree_node<T>*)(*ptr))->data.data;
			}
			const T* operator->()const noexcept
			{
				return std::addressof(((binary_tree_node<T>*)(*ptr))->data.data);
			}
			std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> get_children()const noexcept
			{
				std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> ret{ *this, *this };
				ret.first.go_left();
				ret.second.go_right();
				return ret;
			}
			friend bool operator==(binary_tree_edge_const_proxy l, binary_tree_edge_const_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<binary_tree_edge_const_proxy>;
			template<class T1, class Alloc>
			friend class Yc::binary_tree;
			using value_type = const T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::binary_tree_edge_const_proxy<T>>
{
	size_t operator()(const Yc::details::binary_tree_edge_const_proxy<T>& para)const noexcept
	{
		return std::hash<Yc::details::binary_tree_node_base**>{}(para.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class binary_tree_node_proxy
		{
			binary_tree_node<T>* ptr{};
		public:
			binary_tree_node_proxy() = default;
			binary_tree_node_proxy(binary_tree_node<T>* ptr)noexcept :ptr{ ptr }
			{
			}
			binary_tree_node_proxy(binary_tree_edge_proxy<T> p)noexcept :ptr{ *p.ptr }
			{
			}
			binary_tree_node_proxy(const binary_tree_node_proxy&) = default;
			binary_tree_node_proxy& operator=(const binary_tree_node_proxy&) = default;
			bool null()const noexcept
			{
				return ptr == nullptr;
			}
			operator bool()const noexcept
			{
				return !null();
			}
			T& operator*()const noexcept
			{
				return ptr->data.data;
			}
			T* operator->()const noexcept
			{
				return std::addressof(ptr->data.data);
			}
			binary_tree_node_proxy& go_left()noexcept
			{
				ptr = (binary_tree_node<T>*)ptr->left;
				return *this;
			}
			binary_tree_node_proxy& go_right()noexcept
			{
				ptr = (binary_tree_node<T>*)ptr->right;
				return *this;
			}
			std::pair<binary_tree_edge_proxy<T>, binary_tree_edge_proxy<T>> get_children()const
			{
				return { {&ptr->left},{&ptr->right} };
			}
			friend bool operator==(binary_tree_node_proxy l, binary_tree_node_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<binary_tree_node_proxy>;
			friend class binary_tree_node_const_proxy<T>;
			using value_type = T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::binary_tree_node_proxy<T>>
{
	size_t operator()(const Yc::details::binary_tree_node_proxy<T>& p)const noexcept
	{
		return std::hash<Yc::details::binary_tree_node<T>*>{}(p.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class binary_tree_node_const_proxy
		{
			binary_tree_node<T>* ptr{};
		public:
			binary_tree_node_const_proxy() = default;
			binary_tree_node_const_proxy(binary_tree_node<T>* ptr)noexcept :ptr{ ptr }
			{
			}
			binary_tree_node_const_proxy(binary_tree_edge_const_proxy<T> p)noexcept :ptr{ *p.ptr }
			{
			}
			binary_tree_node_const_proxy(binary_tree_node_proxy<T> p)noexcept :ptr{ p.ptr }
			{
			}
			binary_tree_node_const_proxy(const binary_tree_node_const_proxy&) = default;
			binary_tree_node_const_proxy& operator=(const binary_tree_node_const_proxy&) = default;
			bool null()const noexcept
			{
				return ptr == nullptr;
			}
			operator bool()const noexcept
			{
				return !null();
			}
			const T& operator*()const noexcept
			{
				return ptr->data.data;
			}
			const T* operator->()const noexcept
			{
				return std::addressof(ptr->data.data);
			}
			binary_tree_node_const_proxy& go_left()noexcept
			{
				ptr = (binary_tree_node<T>*)ptr->left;
				return *this;
			}
			binary_tree_node_const_proxy& go_right()noexcept
			{
				ptr = (binary_tree_node<T>*)ptr->right;
				return *this;
			}
			std::pair<binary_tree_edge_const_proxy<T>, binary_tree_edge_const_proxy<T>> get_children()const
			{
				return { {&ptr->left},{&ptr->right} };
			}
			friend bool operator==(binary_tree_node_const_proxy l, binary_tree_node_const_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<binary_tree_node_const_proxy>;
			using value_type = const T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::binary_tree_node_const_proxy<T>>
{
	size_t operator()(const Yc::details::binary_tree_node_const_proxy<T>& p)const noexcept
	{
		return std::hash<Yc::details::binary_tree_node<T>*>{}(p.ptr);
	}
};
namespace Yc
{
	template<class T, class Alloc = std::allocator<T>>
	class binary_tree
	{
		using node_ptr = details::binary_tree_node_base*;
		using node_type = details::binary_tree_node<T>;
		using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
		[[no_unique_address]] Alloc alloc{};
		node_ptr root_ptr{};
		node_ptr allocate_node()
		{
			node_allocator a{ alloc };
			node_ptr p = std::allocator_traits<node_allocator>::allocate(a, 1);
			p->left = nullptr;
			p->right = nullptr;
			return p;
		}
		void deallocate_node(node_ptr p)noexcept
		{
			node_allocator a{ alloc };
			return std::allocator_traits<node_allocator>::deallocate(a, (node_type*)p, 1);
		}
	public:
		Alloc get_allocator()const noexcept
		{
			return alloc;
		}
		using value_type = T;
		using reference = T&;
		using edge_proxy = details::binary_tree_edge_proxy<T>;
		using edge_const_proxy = details::binary_tree_edge_const_proxy<T>;
		using node_proxy = details::binary_tree_node_proxy<T>;
		using node_const_proxy = details::binary_tree_node_const_proxy<T>;
	private:
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		> requires requires (
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h,
			binary_tree t
			) {
			t.emplace(p, std::invoke(vg, h));
			//auto&& [lh, rh] = std::invoke(cg, h);
			(bool)h;
		}
		void recur_and_write_impl(
			edge_const_proxy p,
			ValueGetter& vg,
			ChildrenGetter& cg,
			InitializeHandle& h
		)
		{
			if (!p.null())
			{
				*(int*)nullptr;
			}
			if (h)
			{
				emplace(p, std::invoke(vg, h));
				auto&& [lh, rh] = std::invoke(cg, h);
				auto [l, r] = p.get_children();
				recur_and_write_impl(l, vg, cg, lh);
				recur_and_write_impl(r, vg, cg, rh);
			}
		}
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		>
		void recur_and_overwrite_impl(
			edge_const_proxy p,
			ValueGetter& vg,
			ChildrenGetter& cg,
			InitializeHandle& h
		)
		{
			if (!h)
			{
				cut(p);
				return;
			}
			if (p)
			{
				T& t = (T&)*p;
				if constexpr (requires{
					t = std::invoke(vg, h);
				} && noexcept(t = std::invoke(vg, h)))
				{
					t = std::invoke(vg, h);
				}
				else if constexpr (requires{
					t = T{ std::invoke(vg,h) };
				} && noexcept(t = T{ std::invoke(vg,h) }))
				{
					t = T{ std::invoke(vg,h) };
				}
				else
				{
					std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
					static_assert(std::is_nothrow_constructible_v<T, decltype(std::invoke(vg, h))>);
					std::allocator_traits<Alloc>::construct(alloc, (T*)p.operator->(), std::invoke(vg, h));
				}
			}
			else
			{
				emplace(p, std::invoke(vg, h));
			}
			auto&& [lh, rh] = std::invoke(cg, h);
			auto [l, r] = p.get_children();
			recur_and_overwrite_impl(l, vg, cg, lh);
			recur_and_overwrite_impl(r, vg, cg, rh);
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
			binary_tree b
			) {
			b.emplace(p, std::invoke(vg, h));
			//auto&& [lh, rh] = std::invoke(cg, h);
			(bool)h;
		}
		binary_tree recur_and_write(
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
		)
		{
			binary_tree ret{ cut(p) };
			recur_and_write_impl(p, vg, cg, h);
			return ret;
		}
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		>requires requires (
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
			) {
			emplace(p, std::invoke(vg, h));
			//auto&& [lh, rh] = std::invoke(cg, h);
			(bool)h;
		}
		void recur_and_overwrite(
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
		)
		{
			recur_and_overwrite_impl(p, vg, cg, h);
		}
		binary_tree() = default;
		explicit binary_tree(const Alloc& a) :alloc{ a }
		{
		}
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		>
		binary_tree(
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h,
			const Alloc& a = {}
		) :alloc{ a }
		{
			recur_and_write_impl(root(), vg, cg, h);
		}
		binary_tree(const binary_tree& b) :binary_tree{ b.alloc }
		{
			auto copier = [](edge_const_proxy p) -> const T&
				{
					return *p;
				};
			recur_and_write
				(root(), copier, &edge_const_proxy::get_children, b.croot());
		}
		binary_tree& operator=(const binary_tree& b)
		{
			clear();
			if constexpr (
				std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
			{
				alloc = b.alloc;
			}
			auto copier = [](edge_const_proxy p)->const T&
				{
					return *p;
				};
			recur_and_write(root(), copier, &edge_const_proxy::get_children, b.root());
			return *this;
		}
		binary_tree(binary_tree&& b)noexcept :
			root_ptr{ std::exchange(b.root_ptr, {}) }, alloc{ std::move(b.alloc) }
		{
		}
		binary_tree& operator=(binary_tree&& b)noexcept(
			std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
			std::allocator_traits<Alloc>::is_always_equal::value)
		{
			if constexpr
				(std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
			{
				using namespace std;
				swap(alloc, b.alloc);
				std::swap(root_ptr, b.root_ptr);
				return *this;
			}
			else
			{
				if (alloc == b.alloc)
				{
					std::swap(root_ptr, b.root_ptr);
					return *this;
				}
				auto vg = [](edge_proxy p) -> T&&
					{
						return (T&&)*p;
					};
				if constexpr (std::is_nothrow_move_assignable_v<T>)
				{
					recur_and_overwrite(root(), vg, &edge_proxy::get_children, b.root());
				}
				else
				{
					binary_tree tmp{ vg, &edge_proxy::get_children, b.root(), alloc };
					swap(tmp);
				}
				return *this;
			}
		}
		bool empty()const noexcept
		{
			return root_ptr == nullptr;
		}
		edge_proxy root()noexcept
		{
			return { (node_ptr*) & root_ptr };
		}
		edge_const_proxy root()const noexcept
		{
			return { (node_ptr*) & root_ptr };
		}
		edge_const_proxy croot()const noexcept
		{
			return { (node_ptr*) & root_ptr };
		}
		node_proxy nroot()noexcept
		{
			return root();
		}
		node_const_proxy nroot()const noexcept
		{
			return root();
		}
		node_const_proxy cnroot()const noexcept
		{
			return root();
		}
		binary_tree cut(edge_const_proxy p)noexcept
		{
			node_ptr* ptr = p.ptr;
			binary_tree ret{ alloc };
			ret.root_ptr = std::exchange(*ptr, {});
			return ret;
		}
		void erase(edge_const_proxy p)
		{
			if (!p.null())
			{
				auto [l, r] = p.get_children();
				erase(l);
				erase(r);
				node_ptr* ptr = p.ptr;
				std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
				deallocate_node(*ptr);
				*ptr = nullptr;
			}
			return;
		}
		void clear()noexcept
		{
			erase(root());
		}
		template<class... Args>
		binary_tree emplace(edge_const_proxy p, Args&&... args)
		{
			node_ptr new_node = allocate_node();
			node_allocator na{ alloc };
			if constexpr (std::is_nothrow_constructible_v<T, Args...>)
			{
				std::allocator_traits<Alloc>::
					construct(alloc, &((node_type*)new_node)->data.data, std::forward<Args>(args)...);
			}
			else
			{
				struct guard_t
				{
					binary_tree* t;
					node_ptr ptr;
					~guard_t()
					{
						if (ptr != nullptr)
						{
							t->deallocate_node(ptr);
						}
					}
				}guard{ this,new_node };
				std::allocator_traits<Alloc>::
					construct(na, &((node_type*)new_node)->data.data, std::forward<Args>(args)...);
				guard.ptr = nullptr;
			}
			binary_tree ret{ cut(p) };
			node_ptr* ptr = p.ptr;
			*ptr = new_node;
			return ret;
		}
		binary_tree insert(edge_const_proxy p, const T& v)
		{
			return emplace(p, v);
		}
		binary_tree insert(edge_const_proxy p, T&& v)
		{
			return emplace(p, std::move(v));
		}
		binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
		{
			binary_tree ret{ cut(to) };
			*(to.ptr) = std::exchange(*(from.ptr), {});
			return ret;
		}
		binary_tree splice(edge_const_proxy p, binary_tree& tree)noexcept
		{
			return splice(p, tree.root());
		}
		binary_tree splice(edge_const_proxy p, binary_tree&& tree)noexcept
		{
			return splice(p, tree.root());
		}
		static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
		{
			std::swap(*(l.ptr), *(r.ptr));
		}
		void swap(binary_tree& other)noexcept
		{
			if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
			{
				using namespace std;
				swap(alloc, other.alloc);
			}
			swap_sub_tree(root(), other.root());
		}
		~binary_tree()
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
			binary_tree t1 = cut(q); // <D>-<rC>-<E>
			binary_tree t2 = splice(p, t1.root()); //<B>-<rA>
			//     |
			//    <C>                 <A>
			//   /   \                /
			//  <D>  <E>            <B>
			q = p;
			q.go_left();
			binary_tree t3 = splice(q, t2.root());
			q.go_right();
			splice(q, t3.root());
		}
		void right_rotate(edge_const_proxy p)noexcept
		{
			edge_const_proxy q = p;
			q.go_left();
			binary_tree t1 = cut(q);
			binary_tree t2 = splice(p, t1.root());
			q = p;
			q.go_right();
			binary_tree t3 = splice(q, t2.root());
			q.go_left();
			splice(q, t3.root());
		}
		bool checked_left_rotate(edge_const_proxy p)noexcept
		{
			if (p.null() || (p.go_right(), p.null()))
			{
				return false;
			}
			left_rotate(p);
			return true;
		}
		bool checked_right_rotate(edge_const_proxy p)noexcept
		{
			if (p.null() || (p.go_left(), p.null()))
			{
				return false;
			}
			right_rotate(p);
			return true;
		}
	};
	template<class T>
	void swap(binary_tree<T>& l, binary_tree<T>& r)noexcept
	{
		l.swap(r);
	}
	template<class T, class Alloc>
	class parent_aware_binary_tree;
	namespace details
	{
		struct parent_aware_binary_tree_node_base
		{
			parent_aware_binary_tree_node_base* ptr[3]{};
			parent_aware_binary_tree_node_base(parent_aware_binary_tree_node_base* p)
				noexcept :ptr{ nullptr,nullptr,p }
			{
			}
			parent_aware_binary_tree_node_base(
				parent_aware_binary_tree_node_base* p1,
				parent_aware_binary_tree_node_base* p2,
				parent_aware_binary_tree_node_base* p3
			)noexcept :ptr{ p1,p2,p3 }
			{
			}
			parent_aware_binary_tree_node_base() = default;
			parent_aware_binary_tree_node_base(const parent_aware_binary_tree_node_base&) = default;
			parent_aware_binary_tree_node_base*& left()noexcept
			{
				return ptr[0];
			}
			parent_aware_binary_tree_node_base*& right()noexcept
			{
				return ptr[1];
			}
			parent_aware_binary_tree_node_base*& parent()noexcept
			{
				return ptr[2];
			}
			const parent_aware_binary_tree_node_base* const& left()const noexcept
			{
				return ptr[0];
			}
			const parent_aware_binary_tree_node_base* const& right()const noexcept
			{
				return ptr[1];
			}
			const parent_aware_binary_tree_node_base* const& parent()const noexcept
			{
				return ptr[2];
			}
		};
		parent_aware_binary_tree_node_base* find_this(parent_aware_binary_tree_node_base** p)noexcept
		{
			if (*p != nullptr)
			{
				//总之是一个子节点
				return (*p)->parent();
			}
			//先假设p是左节点
			++p;//那么按理说现在是右节点
			if (*p == nullptr)
			{
				//右节点显然可以nullptr
				++p;
				return (parent_aware_binary_tree_node_base*)(p - 2);
			}
			//如果现在是右节点，那么它的parent应该和tmp一致
			parent_aware_binary_tree_node_base* tmp = (parent_aware_binary_tree_node_base*)(p - 1);
			if ((*p)->parent() != tmp)
			{
				//不一致说明原先就是右节点，现在已经偏到父节点了
				return (parent_aware_binary_tree_node_base*)(p - 2);
			}
			++p;//再偏一格
			return (parent_aware_binary_tree_node_base*)(p - 2);
		}
		template<class T>
		struct parent_aware_binary_tree_node : parent_aware_binary_tree_node_base
		{
			data_t<T> data;
		};
		template<class T>
		class parent_aware_binary_tree_edge_const_proxy;
		template<class T>
		class parent_aware_binary_tree_node_proxy;
		template<class T>
		class parent_aware_binary_tree_node_const_proxy;
		template<class T>
		class parent_aware_binary_tree_edge_proxy
		{
			parent_aware_binary_tree_node_base** ptr{};
		public:
			parent_aware_binary_tree_edge_proxy() = default;
			parent_aware_binary_tree_edge_proxy(const parent_aware_binary_tree_edge_proxy&) = default;
			parent_aware_binary_tree_edge_proxy(parent_aware_binary_tree_node_base** ptr)noexcept :ptr{ ptr }
			{
			}
			parent_aware_binary_tree_edge_proxy& operator=(const parent_aware_binary_tree_edge_proxy&) = default;
			bool valid()const noexcept
			{
				return (bool)ptr;
			}
			bool null()const noexcept
			{
				return !(bool)*ptr;
			}
			explicit operator bool()const noexcept
			{
				return valid() && (!null());
			}
			parent_aware_binary_tree_edge_proxy& go_left()noexcept
			{
				ptr = &((*ptr)->left());
				return *this;
			}
			parent_aware_binary_tree_edge_proxy& go_right()noexcept
			{
				ptr = &((*ptr)->right());
				return *this;
			}
			parent_aware_binary_tree_edge_proxy& go_up()noexcept
			{
				parent_aware_binary_tree_node_base* tmp = find_this(ptr);
				if (tmp->parent()->left() == tmp)
				{
					ptr = &tmp->parent()->left();
				}
				else
				{
					ptr = &tmp->parent()->right();
				}
				return *this;
			}
			parent_aware_binary_tree_node_proxy<T> get_parent()const noexcept
			{
				return { (parent_aware_binary_tree_node<T>*)find_this(ptr) };
			}
			T& operator*()const noexcept
			{
				return ((parent_aware_binary_tree_node<T>*)(*ptr))->data.data;
			}
			T* operator->()const noexcept
			{
				return std::addressof(((parent_aware_binary_tree_node<T>*)(*ptr))->data.data);
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
				return l.ptr == r.ptr;
			}
			friend struct std::hash<parent_aware_binary_tree_edge_proxy>;
			friend class parent_aware_binary_tree_edge_const_proxy<T>;
			template<class T1, class Alloc>
			friend class Yc::parent_aware_binary_tree;
			friend class parent_aware_binary_tree_node_proxy<T>;
			using value_type = T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_proxy<T>>
{
	size_t operator()(const Yc::details::parent_aware_binary_tree_edge_proxy<T>& para)const noexcept
	{
		return std::hash<Yc::details::parent_aware_binary_tree_node_base**>{}(para.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class parent_aware_binary_tree_edge_const_proxy
		{
			parent_aware_binary_tree_node_base** ptr{};
		public:
			parent_aware_binary_tree_edge_const_proxy() = default;
			parent_aware_binary_tree_edge_const_proxy(const parent_aware_binary_tree_edge_const_proxy&) = default;
			parent_aware_binary_tree_edge_const_proxy(const parent_aware_binary_tree_edge_proxy<T>& p)noexcept : ptr{ p.ptr }
			{
			}
			parent_aware_binary_tree_edge_const_proxy(parent_aware_binary_tree_node_base** ptr)noexcept :ptr{ ptr }
			{
			}
			parent_aware_binary_tree_edge_const_proxy& operator=(const parent_aware_binary_tree_edge_const_proxy&) = default;
			bool valid()const noexcept
			{
				return (bool)ptr;
			}
			bool null()const noexcept
			{
				return !(bool)*ptr;
			}
			explicit operator bool()const noexcept
			{
				return valid() && (!null());
			}
			parent_aware_binary_tree_edge_const_proxy& go_left()noexcept
			{
				ptr = &((*ptr)->left());
				return *this;
			}
			parent_aware_binary_tree_edge_const_proxy& go_right()noexcept
			{
				ptr = &((*ptr)->right());
				return *this;
			}
			parent_aware_binary_tree_edge_const_proxy& go_up()noexcept
			{
				parent_aware_binary_tree_node_base* tmp = find_this(ptr);
				if (tmp->parent()->left() == tmp)
				{
					ptr = &tmp->parent()->left();
				}
				else
				{
					ptr = &tmp->parent()->right();
				}
				return *this;
			}
			parent_aware_binary_tree_node_const_proxy<T> get_parent()const noexcept
			{
				return { (parent_aware_binary_tree_node<T>*)find_this(ptr) };
			}
			const T& operator*()const noexcept
			{
				return ((parent_aware_binary_tree_node<T>*)(*ptr))->data.data;
			}
			const T* operator->()const noexcept
			{
				return std::addressof(((parent_aware_binary_tree_node<T>*)(*ptr))->data.data);
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
				return l.ptr == r.ptr;
			}
			friend struct std::hash<parent_aware_binary_tree_edge_const_proxy>;
			template<class T1, class Alloc>
			friend class Yc::parent_aware_binary_tree;
			using value_type = const T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_const_proxy<T>>
{
	size_t operator()(const Yc::details::parent_aware_binary_tree_edge_const_proxy<T>& para)const noexcept
	{
		return std::hash<Yc::details::parent_aware_binary_tree_node_base**>{}(para.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class parent_aware_binary_tree_node_proxy
		{
			parent_aware_binary_tree_node<T>* ptr{};
		public:
			parent_aware_binary_tree_node_proxy() = default;
			parent_aware_binary_tree_node_proxy(parent_aware_binary_tree_node<T>* ptr)noexcept :ptr{ ptr }
			{
			}
			parent_aware_binary_tree_node_proxy(parent_aware_binary_tree_edge_proxy<T> p)noexcept :ptr{ *p.ptr }
			{
			}
			parent_aware_binary_tree_node_proxy(const parent_aware_binary_tree_node_proxy&) = default;
			parent_aware_binary_tree_node_proxy& operator=(const parent_aware_binary_tree_node_proxy&) = default;
			bool null()const noexcept
			{
				return ptr == nullptr;
			}
			operator bool()const noexcept
			{
				return !null();
			}
			T& operator*()const noexcept
			{
				return ptr->data.data;
			}
			T* operator->()const noexcept
			{
				return std::addressof(ptr->data.data);
			}
			parent_aware_binary_tree_node_proxy& go_left()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->left;
				return *this;
			}
			parent_aware_binary_tree_node_proxy& go_right()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->right;
				return *this;
			}
			parent_aware_binary_tree_node_proxy& go_up()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->parent;
				return *this;
			}
			std::pair<parent_aware_binary_tree_edge_proxy<T>, parent_aware_binary_tree_edge_proxy<T>> get_children()const
			{
				return { {&ptr->left},{&ptr->right} };
			}
			friend bool operator==(parent_aware_binary_tree_node_proxy l, parent_aware_binary_tree_node_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<parent_aware_binary_tree_node_proxy>;
			friend class parent_aware_binary_tree_node_const_proxy<T>;
			template<class T1, class Alloc>
			friend class parent_aware_binary_tree;
			operator parent_aware_binary_tree_edge_proxy<T>()const noexcept
			{
				parent_aware_binary_tree_node<T>* tmp = (parent_aware_binary_tree_node<T>*)ptr->parent;
				if (tmp->left == ptr)
				{
					return { &tmp->left };
				}
				else
				{
					return { &tmp->right };
				}
			}
			using value_type = T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_node_proxy<T>>
{
	size_t operator()(const Yc::details::parent_aware_binary_tree_node_proxy<T>& p)const noexcept
	{
		return std::hash<Yc::details::parent_aware_binary_tree_node<T>*>{}(p.ptr);
	}
};
namespace Yc
{
	namespace details
	{
		template<class T>
		class parent_aware_binary_tree_node_const_proxy
		{
			parent_aware_binary_tree_node<T>* ptr{};
		public:
			parent_aware_binary_tree_node_const_proxy() = default;
			parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_node<T>* ptr)noexcept :ptr{ ptr }
			{
			}
			parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_edge_const_proxy<T> p)noexcept :ptr{ *p.ptr }
			{
			}
			parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_node_proxy<T> p)noexcept :ptr{ p.ptr }
			{
			}
			parent_aware_binary_tree_node_const_proxy(const parent_aware_binary_tree_node_const_proxy&) = default;
			parent_aware_binary_tree_node_const_proxy& operator=(const parent_aware_binary_tree_node_const_proxy&) = default;
			bool null()const noexcept
			{
				return ptr == nullptr;
			}
			operator bool()const noexcept
			{
				return !null();
			}
			const T& operator*()const noexcept
			{
				return ptr->data.data;
			}
			const T* operator->()const noexcept
			{
				return std::addressof(ptr->data.data);
			}
			parent_aware_binary_tree_node_const_proxy& go_left()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->left;
				return *this;
			}
			parent_aware_binary_tree_node_const_proxy& go_right()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->right;
				return *this;
			}
			parent_aware_binary_tree_node_const_proxy& go_up()noexcept
			{
				ptr = (parent_aware_binary_tree_node<T>*)ptr->parent;
				return *this;
			}
			std::pair<parent_aware_binary_tree_edge_const_proxy<T>, parent_aware_binary_tree_edge_const_proxy<T>> get_children()const
			{
				return { {&ptr->left},{&ptr->right} };
			}
			friend bool operator==(parent_aware_binary_tree_node_const_proxy l, parent_aware_binary_tree_node_const_proxy r)noexcept
			{
				return l.ptr == r.ptr;
			}
			friend struct std::hash<parent_aware_binary_tree_node_const_proxy>;
			template<class T1, class Alloc>
			friend class parent_aware_binary_tree;
			operator parent_aware_binary_tree_edge_const_proxy<T>()const noexcept
			{
				parent_aware_binary_tree_node<T>* tmp = (parent_aware_binary_tree_node<T>*)ptr->parent;
				if (tmp->left == ptr)
				{
					return { &tmp->left };
				}
				else
				{
					return { &tmp->right };
				}
			}
			using value_type = const T;
		};
	}
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_node_const_proxy<T>>
{
	size_t operator()(const Yc::details::parent_aware_binary_tree_node_const_proxy<T>& p)const noexcept
	{
		return std::hash<Yc::details::parent_aware_binary_tree_node<T>*>{}(p.ptr);
	}
};
namespace Yc
{
	template<class T, class Alloc = std::allocator<T>>
	class parent_aware_binary_tree
	{
		using node_ptr = details::parent_aware_binary_tree_node_base*;
		using node_type = details::parent_aware_binary_tree_node<T>;
		using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
		[[no_unique_address]] Alloc alloc{};
		details::parent_aware_binary_tree_node_base parent_of_root{};
		node_ptr allocate_node()
		{
			node_allocator a{ alloc };
			node_ptr p = std::allocator_traits<node_allocator>::allocate(a, 1);
			p->left() = nullptr;
			p->right() = nullptr;
			return p;
		}
		void deallocate_node(node_ptr p)noexcept
		{
			node_allocator a{ alloc };
			return std::allocator_traits<node_allocator>::deallocate(a, (node_type*)p, 1);
		}
	public:
		Alloc get_allocator()const noexcept
		{
			return alloc;
		}
		using value_type = T;
		using reference = T&;
		using edge_proxy = details::parent_aware_binary_tree_edge_proxy<T>;
		using edge_const_proxy = details::parent_aware_binary_tree_edge_const_proxy<T>;
		using node_proxy = details::parent_aware_binary_tree_node_proxy<T>;
		using node_const_proxy = details::parent_aware_binary_tree_node_const_proxy<T>;
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
			InitializeHandle& h
		)
		{
			if (!p.null())
			{
				*(int*)nullptr;
			}
			if (h)
			{
				emplace(p, std::invoke(vg, h));
				auto&& [lh, rh] = std::invoke(cg, h);
				auto [l, r] = p.get_children();
				recur_and_write_impl(l, vg, cg, lh);
				recur_and_write_impl(r, vg, cg, rh);
			}
		}
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		>
		void recur_and_overwrite_impl(
			edge_const_proxy p,
			ValueGetter& vg,
			ChildrenGetter& cg,
			InitializeHandle& h
		)
		{
			if (!h)
			{
				cut(p);
				return;
			}
			if (p)
			{
				T& t = (T&)*p;
				if constexpr (requires{
					t = std::invoke(vg, h);
				} && noexcept(t = std::invoke(vg, h)))
				{
					t = std::invoke(vg, h);
				}
				else if constexpr (requires{
					t = T{ std::invoke(vg,h) };
				} && noexcept(t = T{ std::invoke(vg,h) }))
				{
					t = T{ std::invoke(vg,h) };
				}
				else
				{
					std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
					static_assert(std::is_nothrow_constructible_v<T, decltype(std::invoke(vg, h))>);
					std::allocator_traits<Alloc>::construct(alloc, (T*)p.operator->(), std::invoke(vg, h));
				}
			}
			else
			{
				emplace(p, std::invoke(vg, h));
			}
			auto&& [lh, rh] = std::invoke(cg, h);
			auto [l, r] = p.get_children();
			recur_and_overwrite_impl(l, vg, cg, lh);
			recur_and_overwrite_impl(r, vg, cg, rh);
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
			InitializeHandle h
			) {
			emplace(p, std::invoke(vg, h));
			//auto&& [lh, rh] = std::invoke(cg, h);
		}
		parent_aware_binary_tree recur_and_write(
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
		)
		{
			parent_aware_binary_tree ret{ cut(p) };
			recur_and_write_impl(p, vg, cg, h);
			return ret;
		}
		template<
			class ValueGetter,
			class ChildrenGetter,
			class InitializeHandle
		>requires requires (
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
			) {
			emplace(p, std::invoke(vg, h));
			//auto&& [lh, rh] = std::invoke(cg, h);
		}
		void recur_and_overwrite(
			edge_const_proxy p,
			ValueGetter vg,
			ChildrenGetter cg,
			InitializeHandle h
		)
		{
			recur_and_overwrite_impl(p, vg, cg, h);
		}
		parent_aware_binary_tree() = default;
		explicit parent_aware_binary_tree(const Alloc& a) :alloc{ a }
		{
		}
		parent_aware_binary_tree(const parent_aware_binary_tree& b) :alloc{ b.alloc }
		{
			auto copier = [](edge_const_proxy p) -> const T&
				{
					return *p;
				};
			recur_and_write
			(root(), copier, &edge_const_proxy::get_children, b.root());
		}
		parent_aware_binary_tree& operator=(const parent_aware_binary_tree& b)
		{
			clear();
			if constexpr (
				std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
			{
				alloc = b.alloc;
			}
			auto copier = [](edge_const_proxy p)->const T&
				{
					return *p;
				};
			recur_and_write(root(), copier, &edge_const_proxy::get_children, b.root());
			return *this;
		}
		parent_aware_binary_tree(parent_aware_binary_tree&& b)noexcept :
			parent_of_root{ std::exchange(b.parent_of_root, {}) }, alloc{ std::move(b.alloc) }
		{
			parent_of_root.left()->parent() = &parent_of_root;
		}
		parent_aware_binary_tree& operator=(parent_aware_binary_tree&& b)
		{
			if constexpr
				(std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
			{
				using namespace std;
				swap(alloc, b.alloc);
				swap_sub_tree(root(), b.root());
				return *this;
			}
			else
			{
				if (alloc == b.alloc)
				{
					swap_sub_tree(root(), b.root());
					return *this;
				}
				auto vg = [](edge_proxy p) -> T&&
					{
						return (T&&)*p;
					};
				if constexpr (std::is_nothrow_move_assignable_v<T>)
				{
					recur_and_overwrite(root(), vg, &edge_proxy::get_children, b.root());
				}
				else
				{
					parent_aware_binary_tree tmp{ vg, &edge_proxy::get_children, b.root(), alloc };
					swap(tmp);
				}
				return *this;
			}
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
			recur_and_write_impl(root(), vg, cg, h);
		}
		bool empty()const noexcept
		{
			return parent_of_root.left() == nullptr;
		}
		edge_proxy root()noexcept
		{
			return { &parent_of_root.left() };
		}
		edge_const_proxy root()const noexcept
		{
			return { &parent_of_root.left() };
		}
		edge_const_proxy croot()const noexcept
		{
			return { &parent_of_root.left() };
		}
		node_proxy nroot()noexcept
		{
			return root();
		}
		node_const_proxy nroot()const noexcept
		{
			return root();
		}
		node_const_proxy cnroot()const noexcept
		{
			return root();
		}
		parent_aware_binary_tree cut(edge_const_proxy p)noexcept
		{
			node_ptr* ptr = p.ptr;
			node_ptr tmp = *ptr;
			parent_aware_binary_tree ret{ alloc };
			if (p)
			{
				ret.parent_of_root.left() = std::exchange(*ptr, {});
				tmp->parent() = &ret.parent_of_root;
			}
			return ret;
		}
		void erase(edge_const_proxy p)
		{
			if (!p.null())
			{
				auto [l, r] = p.get_children();
				erase(l);
				erase(r);
				node_ptr* ptr = p.ptr;
				std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
				deallocate_node(*ptr);
				*ptr = nullptr;
			}
			return;
		}
		void clear()noexcept
		{
			erase(root());
		}
		template<class... Args>
		parent_aware_binary_tree emplace(edge_const_proxy p, Args&&... args)
		{
			edge_const_proxy tmp = p;
			node_const_proxy tmp2 = tmp.get_parent();
			node_ptr parent_node = tmp2.ptr;
			node_ptr new_node = allocate_node();
			node_allocator na{ alloc };
			if constexpr (std::is_nothrow_constructible_v<T, Args...>)
			{
				std::allocator_traits<Alloc>::
					construct(alloc, &((node_type*)new_node)->data.data, std::forward<Args>(args)...);
			}
			else
			{
				struct guard_t
				{
					parent_aware_binary_tree* t;
					node_ptr ptr;
					~guard_t()
					{
						if (ptr != nullptr)
						{
							t->deallocate_node(ptr);
						}
					}
				}guard{ this, new_node };
				std::allocator_traits<Alloc>::
					construct(na, &((node_type*)new_node)->data.data, std::forward<Args>(args)...);
				guard.ptr = nullptr;
			}
			parent_aware_binary_tree ret{ cut(p) };
			node_ptr* ptr = p.ptr;
			*ptr = new_node;
			new_node->parent() = parent_node;
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
			if (from)
			{
				edge_const_proxy tmp = to;
				node_const_proxy tmp2 = tmp.get_parent();
				node_ptr new_parent = tmp2.ptr;
				(*(from.ptr))->parent() = new_parent;
			}
			parent_aware_binary_tree ret{ cut(to) };
			*(to.ptr) = std::exchange(*(from.ptr), {});
			return ret;
		}
		static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
		{
			if (l.null() && r.null())
			{
				return;
			}
			if (l.null())
			{
				edge_const_proxy tmp = l;
				tmp.go_up();
				node_ptr new_parent = *(tmp.ptr);
				(*r.ptr)->parent = new_parent;
				*l.ptr = std::exchange(*r.ptr, {});
				return;
			}
			if (r.null())
			{
				edge_const_proxy tmp = r;
				tmp.go_up();
				node_ptr new_parent = *(tmp.ptr);
				(*l.ptr)->parent = new_parent;
				*r.ptr = std::exchange(*l.ptr, {});
				return;
			}
			std::swap((*l.ptr)->parent, (*r.ptr)->parent);
			std::swap(*l.ptr, *r.ptr);
		}
		void swap(parent_aware_binary_tree& other)noexcept
		{
			if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
			{
				using namespace std;
				swap(alloc, other.alloc);
			}
			swap_sub_tree(root(), other.root());
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
		}
		bool checked_left_rotate(edge_const_proxy p)noexcept
		{
			if (p.null() || (p.go_right(), p.null()))
			{
				return false;
			}
			left_rotate(p);
			return true;
		}
		bool checked_right_rotate(edge_const_proxy p)noexcept
		{
			if (p.null() || (p.go_left(), p.null()))
			{
				return false;
			}
			right_rotate(p);
			return true;
		}
	};
}
