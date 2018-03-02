#pragma once

#include <lua.hpp>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stack>

namespace lml {
	struct lex {
		const char* z;
		int line = 1;

		lex(const char* input)
			: z(input)
		{
			if (z[0] == '\xEF' && z[1] == '\xBB' && z[2] == '\xBF') {
				z += 3;
			}
		}

		void incline() {
			char old = *z;
			assert(*z == '\n' || *z == '\r');
			z++;
			if ((*z == '\n' || *z == '\r') && *z != old) {
				z++;
			}
			++line;
		}

		size_t parse_indent() {
			size_t n = 0;
			for (;;) {
				switch (*z) {
				case '\n':
				case '\r':
					incline();
					n = 0;
					break;
				case ' ':
					n++;
					z++;
					break;
				default:
					return n;
				}
			}
		}

		void parse_whitespace() {
			for (;;) {
				switch (*z) {
				case ' ':
					z++;
					break;
				default:
					return;
				}
			}
		}

		template <class Handler, class ... Args>
		bool error(Handler& h, const char* fmt, const Args& ... args)
		{
			h.accept_error(line, fmt, args...);
			return false;
		}

		template <class Handler>
		bool parse_string(Handler& h, char del)
		{
			assert(*z == del);
			z++;
			for (;;) {
				const char* start = z;
				for (char c = *z++; c != del; c = *z++) {
					switch (*z) {
					case '\n':
					case '\r':
						incline();
						break;
					}
				}
				if (*z != del) {
					h.accept_string(start, z - start - 1, false);
					break;
				}
				h.accept_string(start, z - start, true);
				z++;
			}
			return true;
		}

		template <class Handler>
		bool parse_value(Handler& h)
		{
			if (*z == '\'') {
				return parse_string(h, *z);
			}
			const char* p = z;
			for (;;) {
				switch (*z) {
				case ':':
				case '\'':
				case ' ':
				case '\t':
				case '\v':
				case '\f':
				case '\n':
				case '\r':
				case '\0':
					h.accept_string(p, z - p, false);
					return true;
				default:
					z++;
					break;
				}
			}
		}

		template <class Handler>
		bool parse_table(Handler& h, size_t level)
		{
			if (!h.accept_table_begin(level)) {
				return error(h, "error indentation.");
			}
			if (!parse_value(h)) {
				return false;
			}
			parse_whitespace();
			if (*z == ':') {
				z++;
				parse_whitespace();
				if (!parse_value(h)) {
					return false;
				}
			}
			switch (*z) {
			case '\n':
			case '\r':
			case '\0':
				return true;
			}
			return error(h, "'\n' expected near '%c'", *z);
		}

		template <class Handler>
		bool parse(Handler& h)
		{
			h.accept_begin();
			for (;;) {
				size_t n = parse_indent();
				if (*z == '\0') {
					break;
				}
				if (!parse_table(h, n)) {
					return false;
				}
			}
			h.accept_end();
			return true;
		}
	};

	struct handler
	{
		lua_State* L;
		std::stack<size_t> levels;
		bool concat = false;
		bool first = true;
		luaL_Buffer b;

		handler(lua_State* L)
			: L(L)
		{}

		void accept_begin() {
			lua_newtable(L);
			lua_pushstring(L, "");
			lua_rawseti(L, -2, 1);
			lua_pushboolean(L, 0);
			lua_rawseti(L, -2, 2);
		}
		void accept_end() {
			for (; !levels.empty();) {
				node_close();
			}
		}

		void node_close() {
			levels.pop();
			lua_pop(L, 1);
		}

		void node_create(size_t level) {
			levels.push(level);
			lua_Integer len = luaL_len(L, -1);
			if (len == 1) {
				lua_pushboolean(L, 0);
				lua_rawseti(L, -2, ++len);
			}
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_rawseti(L, -3, len + 1);
		}

		bool accept_table_begin(size_t level) {
			if (first) {
				first = false;
				node_create(level);
				return true;
			}
			size_t current = levels.top();
			if (level == current) {
				node_close();
				node_create(level);
			}
			else if (level > current) {
				node_create(level);
			}
			else {
				for (;;) {
					node_close();
					if (levels.empty()) {
						return false;
					}
					size_t current = levels.top();
					if (level == current) {
						node_close();
						node_create(level);
						return true;
					}
					else if (level > current) {
						return false;
					}
				}
			}
			return true;
		}

		void accept_string(const char* str, size_t len, bool next) {
			if (concat) {
				if (!next) {
					luaL_addlstring(&b, str, len);
					luaL_pushresult(&b);
					lua_rawseti(L, -2, luaL_len(L, -2) + 1);
					concat = false;
				}
				else {
					luaL_addlstring(&b, str, len);
				}
			}
			else {
				if (!next) {
					lua_pushlstring(L, str, len);
					lua_rawseti(L, -2, luaL_len(L, -2) + 1);
				}
				else {
					luaL_buffinit(L, &b);
					luaL_addlstring(&b, str, len);
					concat = true;
				}
			}
		}

		template <class ... Args>
		void accept_error(int line, const char* fmt, const Args& ... args) {
			lua_pushinteger(L, line);
			lua_pushfstring(L, fmt, args...);
		}
	};
}
