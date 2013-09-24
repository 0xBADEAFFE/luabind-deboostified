// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef LUABIND_CONTAINER_POLICY_HPP_INCLUDED
#define LUABIND_CONTAINER_POLICY_HPP_INCLUDED

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>
#include <luabind/detail/decorate_type.hpp>  // for decorated_type
#include <luabind/detail/primitives.hpp>  // for null_type (ptr only), etc

namespace luabind { namespace detail {

	template<class Policies>
	struct container_converter_lua_to_cpp
	{
		enum { consumed_args = 1 };

        template<class T>
		T apply(lua_State* L, by_const_reference<T>, int index)
		{
			typedef typename T::value_type value_type;
			applied_converter_policy<1, Policies, value_type, lua_to_cpp> converter;

			T container;

			lua_pushnil(L);
			while (lua_next(L, index-1))
			{
				container.push_back(converter.apply(L, decorated_type<value_type>(), -1));
				lua_pop(L, 1); // pop value
			}

			return container;
		}

		template<class T>
		T apply(lua_State* L, by_value<T>, int index)
		{
			return apply(L, by_const_reference<T>(), index);
		}

		template<class T>
		static int match(lua_State* L, by_const_reference<T>, int index)
		{
			if (lua_istable(L, index)) return 0; else return -1;
		}

		template<class T>
		static int match(lua_State* L, by_value<T>, int index)
		{
			return match(L, by_const_reference<T>(), index)
		}

		template<class T>
		void converter_postcall(lua_State*, T, int) {}
	};

	template<class Policies>
	struct container_converter_cpp_to_lua
	{
		template<class T>
		void apply(lua_State* L, const T& container)
		{
			typedef typename T::value_type value_type;
			applied_converter_policy<1, Policies, value_type, lua_to_cpp> converter;

			lua_newtable(L);

			int index = 1;

			for (const auto& element : container )
			{
				converter.apply(L, element);
				lua_rawseti(L, -2, index);
				++index;
			}
		}
	};

	template<class Policies = no_injectors>
	struct container_policy : conversion_policy<>
	{
		struct only_accepts_nonconst_pointers {};

		template<class T, class Direction>
		struct apply;

		template<class T>
		struct apply<T, lua_to_cpp> {
			using type = container_converter_lua_to_cpp<Policies>;
		};

		template<class T>
		struct apply<T, cpp_to_lua> {
			using type = container_converter_cpp_to_lua<Policies>;
		};

		template< typename T, typename DirectionTag >
		using applied = typename apply<T, DirectionTag>::type;
	};

}}

namespace luabind
{
	template<int N>
	meta::type_list< converter_policy_injector<N, detail::container_policy<> > >
	container(meta::index<N>) 
	{ 
		return meta::type_list< converter_policy_injector<N, detail::container_policy<> > >();
	}

	template<int N, class Policies>
	meta::type_list< converter_policy_injector<N, detail::container_policy<Policies> > >
	container(meta::index<N>, const Policies&) 
	{ 
		return meta::type_list< converter_policy_injector<N, detail::container_policy<Policies> > >();
	}
}

#endif // LUABIND_CONTAINER_POLICY_HPP_INCLUDED
