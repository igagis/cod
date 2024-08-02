/*
cod - text editor

Copyright (C) 2021  Ivan Gagis <igagis@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#pragma once

#include <tml/tree.hpp>

#include "../../../../3rd_party/srell.hpp"

#include "highlighter.hpp"

namespace synhi {

class regex_highlighter_model
{
public:
	regex_highlighter_model(const tml::forest& spec);

	struct state;

	class matcher
	{
	public:
		const bool is_preprocessed;

		matcher(bool is_preprocessed = false) :
			is_preprocessed(is_preprocessed)
		{}

		matcher(const matcher&) = delete;
		matcher& operator=(const matcher&) = delete;

		matcher(matcher&&) = delete;
		matcher& operator=(matcher&&) = delete;

		virtual ~matcher() = default;

		struct match_result {
			size_t begin;
			size_t size;

			struct capture_group {
				bool matched;
				size_t offset;
				std::u32string str;
			};

			std::vector<capture_group> capture_groups;
		};

		virtual match_result match(std::u32string_view str, bool line_begin) const = 0;

		virtual std::shared_ptr<const matcher> preprocess(
			utki::span<const match_result::capture_group> capture_groups
		) const
		{
			return nullptr;
		}
	};

	struct rule {
	public:
		std::shared_ptr<const matcher> matcher_;

		struct operation {
			enum class type {
				push,
				pop
			};
			type type_v = type::push;

			// plain pointer to avoid circular references in case state refers a rule which pushes the same state
			state* state_to_push = nullptr;
		};

		std::vector<operation> operations;

		std::vector<std::shared_ptr<const font_style>> styles;

		struct parse_result {
			utki::shared_ref<regex_highlighter_model::rule> rule;

			struct operation_entry {
				operation::type type;
				std::string state;
			};

			std::vector<operation_entry> operations;
			std::vector<std::string> styles;
			std::string state_to_push;
		};

		static parse_result parse(const tml::forest& spec);
	};

	class regex_matcher : public matcher
	{
		srell::u32regex regex;

	public:
		regex_matcher(std::u32string_view regex_str) :
			regex(regex_str.data(), regex_str.size(), srell::regex_constants::optimize)
		{}

		match_result match(std::u32string_view str, bool line_begin) const override;
	};

	// preprocessed regex matcher
	class ppregex_matcher : public matcher
	{
		struct regex_part {
			std::u32string str;
			unsigned group_num;
		};

		std::vector<regex_part> regex_parts;
		std::u32string regex_tail;

	public:
		ppregex_matcher(std::string_view regex);

		match_result match(std::u32string_view str, bool line_begin) const override
		{
			// this method is not supposed to be ever called
			ASSERT(false)
			return {};
		}

		std::shared_ptr<const matcher> preprocess(
			utki::span<const match_result::capture_group> capture_groups
		) const override;
	};

	struct state {
		std::vector<utki::shared_ref<const rule>> rules;
		std::shared_ptr<const font_style> style;

		struct parse_result {
			utki::shared_ref<regex_highlighter_model::state> state;
			std::vector<std::string> rules;
			std::string style;
		};

		static parse_result parse(const tml::forest& spec);
	};

	// need to keep strong pointers to all states, because rules hold only plain pointer to the state_to_push
	std::vector<utki::shared_ref<const state>> states;
};

class regex_highlighter : public highlighter
{
	const std::shared_ptr<const regex_highlighter_model> model;

public:
	regex_highlighter(std::shared_ptr<const regex_highlighter_model> model);

	void reset() override;

	std::vector<line_span> highlight(std::u32string_view str) override;

private:
	struct state_frame {
		std::reference_wrapper<const regex_highlighter_model::state> state;
		std::vector<regex_highlighter_model::matcher::match_result::capture_group> capture_groups;
		std::vector<
			std::pair<const regex_highlighter_model::matcher*, std::shared_ptr<const regex_highlighter_model::matcher>>>
			preprocessed_rules_cache;
	};

	std::vector<state_frame> state_stack;
};

} // namespace synhi
