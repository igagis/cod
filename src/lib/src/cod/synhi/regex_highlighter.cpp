/*
cod - text editor

Copyright (C) 2021-2024  Ivan Gagis <igagis@gmail.com>

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

#include "regex_highlighter.hpp"

#include <tml/crawler.hpp>
#include <utki/linq.hpp>
#include <utki/string.hpp>

using namespace synhi;

namespace {
std::shared_ptr<font_style> parse_style(const tml::forest& style)
{
	auto ret = std::make_shared<font_style>();
	for (const auto& n : style) {
		if (n.value == "color") {
			ret->color = tml::crawler(n.children).get().value.to_uint32();
		} else if (n.value == "style") {
			const auto& v = tml::crawler(n.children).get().value;
			if (v == "normal") {
				ret->style = ruis::res::font::style::normal;
			} else if (v == "bold") {
				ret->style = ruis::res::font::style::bold;
			} else if (v == "italic") {
				ret->style = ruis::res::font::style::italic;
			} else if (v == "bold_italic") {
				ret->style = ruis::res::font::style::bold_italic;
			} else {
				std::stringstream ss;
				ss << "unknown font style value: " << v;
				throw std::invalid_argument(ss.str());
			}
		} else if (n.value == "underline") {
			ret->underline = tml::crawler(n.children).get().value.to_bool();
		} else if (n.value == "stroke") {
			ret->stroke = tml::crawler(n.children).get().value.to_bool();
		} else {
			std::stringstream ss;
			ss << "unknown font style item: " << n.value;
			throw std::invalid_argument(ss.str());
		}
	}
	return ret;
}
} // namespace

namespace {
struct parsing_context {
	std::map<std::string, std::shared_ptr<font_style>> styles;
	std::map<std::string, regex_highlighter_model::rule::parse_result> rules;

	// needs to preserve order
	std::vector<std::pair<std::string, regex_highlighter_model::state::parse_result>> states;

	std::string initial_state;

	void parse_styles(const tml::forest& styles)
	{
		for (const auto& s : styles) {
			if (this->styles.find(s.value.string) != this->styles.end()) {
				std::stringstream ss;
				ss << "style with name '" << s.value.string << "' already exists";
				throw std::invalid_argument(ss.str());
			}

			this->styles.insert(std::make_pair(s.value.string, parse_style(s.children)));
		}
	}

	void parse_rules(const tml::forest& desc)
	{
		for (const auto& m : desc) {
			if (this->rules.find(m.value.string) != this->rules.end()) {
				std::stringstream ss;
				ss << "rule with name '" << m.value.string << "' already exists";
				throw std::invalid_argument(ss.str());
			}

			this->rules.insert(std::make_pair(m.value.string, regex_highlighter_model::rule::parse(m.children)));
		}
	}

	void parse_states(const tml::forest& desc)
	{
		if (!desc.empty()) {
			this->initial_state = desc.front().value.string;
		}
		for (const auto& s : desc) {
			if (std::find_if(this->states.begin(), this->states.end(), [&](const auto& v) {
					return v.first == s.value.string;
				}) != this->states.end())
			{
				std::stringstream ss;
				ss << "state with name '" << s.value.string << "' already exists";
				throw std::invalid_argument(ss.str());
			}

			this->states.emplace_back(s.value.string, regex_highlighter_model::state::parse(s.children));
		}
	}

	std::shared_ptr<const font_style> get_style(const std::string& name)
	{
		auto i = this->styles.find(name);
		if (i == this->styles.end()) {
			std::stringstream ss;
			ss << "style '" << name << "' not found";
			throw std::invalid_argument(ss.str());
		}
		ASSERT(i->second)
		return i->second;
	}

	const utki::shared_ref<regex_highlighter_model::state>& get_state(const std::string& name)
	{
		auto i = std::find_if(this->states.begin(), this->states.end(), [&](const auto& v) {
			return v.first == name;
		});
		if (i == this->states.end()) {
			std::stringstream ss;
			ss << "state not found: " << name;
			throw std::invalid_argument(ss.str());
		}
		return i->second.state;
	}

	const utki::shared_ref<regex_highlighter_model::rule>& get_rule(const std::string& name)
	{
		auto i = this->rules.find(name);
		if (i == this->rules.end()) {
			std::stringstream ss;
			ss << "rule '" << name << "' not found";
			throw std::invalid_argument(ss.str());
		}
		return i->second.rule;
	}
};
} // namespace

regex_highlighter_model::matcher::match_result regex_highlighter_model::regex_matcher::match(
	std::u32string_view str,
	bool line_begin
) const
{
	srell::match_results<decltype(str)::const_iterator> m;

	auto regex_flags = srell::regex_constants::match_default;
	if (!line_begin) {
		regex_flags |= srell::regex_constants::match_not_bol;
	}

	if (!srell::regex_search(str.begin(), str.end(), m, this->regex, regex_flags)) {
		return match_result{.begin = str.size(), .size = 1};
	}

	ASSERT(!m.empty())

	std::vector<match_result::capture_group> capture_groups;

	ASSERT(m.size() >= 1)
	for (size_t i = 1; i != m.size(); ++i) {
		if (!m[i].matched) {
			// NOLINTNEXTLINE(modernize-use-emplace, "capture_group has no suitable constructor")
			capture_groups.push_back(
				match_result::capture_group{.matched = false, .offset = size_t(std::distance(m[0].first, m[0].second))}
			);
		} else {
			capture_groups.push_back(
				match_result::capture_group{
					.matched = true,
					.offset = size_t(std::distance(m[0].first, m[i].first)),
					.str = std::u32string(m[i].first, m[i].second)
				}
			);
		}
	}

	return match_result{
		.begin = size_t(std::distance(str.cbegin(), m[0].first)),
		.size = size_t(std::distance(m[0].first, m[0].second)),
		.capture_groups = std::move(capture_groups)
	};
}

regex_highlighter_model::rule::parse_result regex_highlighter_model::rule::parse(const tml::forest& desc)
{
	parse_result ret{.rule = utki::make_shared<rule>()};

	for (const auto& n : desc) {
		if (n.value == "styles") {
			ret.styles = utki::linq(n.children)
							 .select([](const auto& p) {
								 return p.value.string;
							 })
							 .get();
		} else if (n.value == "regex") {
			ret.rule.get().matcher_ = std::make_shared<regex_highlighter_model::regex_matcher>(
				utki::to_utf32(tml::crawler(n.children).get().value.string)
			);
		} else if (n.value == "ppregex") {
			ret.rule.get().matcher_ =
				std::make_shared<regex_highlighter_model::ppregex_matcher>(tml::crawler(n.children).get().value.string);
		} else if (n.value == "push") {
			ret.operations.push_back(
				{.type = operation::type::push, .state = tml::crawler(n.children).get().value.string}
			);
		} else if (n.value == "pop") {
			// NOLINTNEXTLINE(modernize-use-emplace, "operation_entry has no suitable constructor")
			ret.operations.push_back({.type = operation::type::pop, .state = {}});
		} else {
			std::stringstream ss;
			ss << "unknown rule keyword: " << n.value;
			throw std::invalid_argument(ss.str());
		}
	}

	if (!ret.rule.get().matcher_) {
		throw std::invalid_argument("rule does not define any mtcher");
	}

	return ret;

	// TODO: remove lint suppression when https://github.com/llvm/llvm-project/issues/60896 is fixed
	// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

regex_highlighter_model::state::parse_result regex_highlighter_model::state::parse(const tml::forest& desc)
{
	parse_result ret{.state = utki::make_shared<state>()};

	for (const auto& n : desc) {
		if (n.value == "style") {
			ret.style = tml::crawler(n.children).get().value.string;
		} else if (n.value == "rules") {
			ret.rules = utki::linq(n.children)
							.select([](const auto& c) {
								return c.value.string;
							})
							.get();
		} else {
			std::stringstream ss;
			ss << "unknown state keyword: " << n.value;
			throw std::invalid_argument(ss.str());
		}
	}

	return ret;

	// TODO: remove lint suppression when https://github.com/llvm/llvm-project/issues/60896 is fixed
	// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

regex_highlighter_model::regex_highlighter_model(const tml::forest& spec)
{
	parsing_context c;

	for (const auto& n : spec) {
		if (n.value == "styles") {
			c.parse_styles(n.children);
		} else if (n.value == "rules") {
			c.parse_rules(n.children);
		} else if (n.value == "states") {
			c.parse_states(n.children);
		} else {
			std::stringstream ss;
			ss << "unknown keyword: " << n.value;
			throw std::invalid_argument(ss.str());
		}
	}

	// set state -> rules and state -> styles references
	for (const auto& n : c.states) {
		this->states.emplace_back(n.second.state);
		auto& state = n.second.state.get();
		const auto& parsed = n.second;
		state.rules = utki::linq(parsed.rules)
						  .select([&](const auto& m) -> utki::shared_ref<const rule> {
							  return c.get_rule(m);
						  })
						  .get();

		state.style = c.get_style(parsed.style);
	}

	// set rule -> style and rule -> state references
	for (const auto& n : c.rules) {
		auto& rule = n.second.rule.get();
		const auto& parsed = n.second;

		// rule can have no style, then it inherits style from pushed state
		rule.styles = utki::linq(parsed.styles)
						  .select([&](const auto& p) {
							  return c.get_style(p);
						  })
						  .get();

		for (const auto& o : parsed.operations) {
			rule.operations.push_back(
				{o.type, o.type == rule::operation::type::push ? &c.get_state(o.state).get() : nullptr}
			);
		}
	}
}

regex_highlighter::regex_highlighter(std::shared_ptr<const regex_highlighter_model> model) :
	model(std::move(model))
{
	ASSERT(this->model)
	this->regex_highlighter::reset();
}

void regex_highlighter::reset()
{
	this->state_stack.clear();
	ASSERT(this->model)
	ASSERT(!this->model->states.empty());
	// NOLINTNEXTLINE(modernize-use-emplace, "state_frame has no appropriate constructor")
	this->state_stack.push_back(state_frame{.state = this->model->states.front().get()});
}

namespace {
class line_span_container
{
	std::vector<line_span> spans;

public:
	void push(const std::shared_ptr<const font_style>& style, size_t length)
	{
		if (length == 0) {
			return;
		}

		if (!this->spans.empty()) {
			auto& prev = this->spans.back();

			ASSERT(prev.length != 0) // we don't push spans of 0 length, so there should be no such spans

			if (prev.style == style) {
				prev.length += length;
				return;
			}
		}

		this->spans.push_back(line_span{.length = length, .style = style});
	}

	std::vector<line_span> reset()
	{
		return std::move(this->spans);
	}
};
} // namespace

std::vector<line_span> regex_highlighter::highlight(std::u32string_view str)
{
	line_span_container spans;

	bool line_begin = true;
	std::u32string_view view(str);

	while (!view.empty()) {
		regex_highlighter_model::matcher::match_result match{.begin = view.size(), .size = 1};
		const regex_highlighter_model::rule* match_rule = nullptr;

		// go through all rules of the current state to find the match closest to current
		// position in the text line
		ASSERT(!this->state_stack.empty())
		for (const auto& r : this->state_stack.back().state.get().rules) {
			auto matcher = r.get().matcher_.get();
			ASSERT(matcher)
			if (matcher->is_preprocessed) {
				auto& cache = this->state_stack.back().preprocessed_rules_cache;
				auto i = std::find_if(cache.begin(), cache.end(), [&](const auto& e) {
					return e.first == matcher;
				});
				if (i == cache.end()) {
					cache.emplace_back(matcher, matcher->preprocess(this->state_stack.back().capture_groups));
					matcher = cache.back().second.get();
				} else {
					matcher = i->second.get();
				}
			}

			auto m = matcher->match(view, line_begin);

			if (m.begin < match.begin) {
				match = std::move(m);
				match_rule = &r.get();
			}

			if (match.begin == 0) {
				// there can be no other match closer than 0 chars away, so exit early
				break;
			}
		}

		const auto& state_style = this->state_stack.back().state.get().style;

		if (!match_rule) {
			// no rule has matched, extend current span to the end of the line and exit early
			spans.push(state_style, view.size());
			break;
		}

		ASSERT(match_rule)

		// extend current span and move the current position to the beginning of the match
		spans.push(state_style, match.begin);
		view = view.substr(match.begin);

		auto size = match.size;

		if (match_rule->styles.empty()) {
			spans.push(state_style, size);
		} else {
			const auto& styles = match_rule->styles;
			ASSERT(!styles.empty())

			const auto& ungrouped_style = styles[0];

			size_t offset = 0;
			for (size_t i = 0; i != match.capture_groups.size(); ++i) {
				const auto& g = match.capture_groups[i];
				if (!g.matched || g.offset < offset) {
					continue;
				}

				ASSERT(g.offset + g.str.size() <= size)

				spans.push(ungrouped_style, g.offset - offset);

				size_t group_num = i + 1;
				if (group_num >= styles.size()) {
					group_num = 0;
				}
				ASSERT(group_num < styles.size())
				spans.push(styles[group_num], g.str.size());

				offset = g.offset + g.str.size();
			}
			ASSERT(offset <= size)

			spans.push(ungrouped_style, size - offset);
		}

		view = view.substr(size);

		// apply rule operations
		for (const auto& op : match_rule->operations) {
			switch (op.type_v) {
				case regex_highlighter_model::rule::operation::type::push:
					ASSERT(op.state_to_push)
					// NOLINTNEXTLINE(modernize-use-emplace, "state_frame has no appropriate constructor")
					this->state_stack.push_back(
						state_frame{.state = *op.state_to_push, .capture_groups = std::move(match.capture_groups)}
					);
					// 'match.capture_groups' is left in undefined state, so need to clear it for next 'push' operation,
					// this means that capture groups are effective only for first push.
					match.capture_groups.clear();
					break;
				case regex_highlighter_model::rule::operation::type::pop:
					// we must not pop the initial state, so check that more than one state is currently in the stack
					if (this->state_stack.size() > 1) {
						this->state_stack.pop_back();
					}
					break;
				default:
					ASSERT(false, [&](auto& o) {
						o << "opeartion = " << unsigned(op.type_v);
					})
					break;
			}
		}

		line_begin = false;
	}

	auto ret = spans.reset();

	if (ret.empty()) {
		ASSERT(!this->state_stack.empty())
		ret.push_back(line_span{.length = 0, .style = this->state_stack.back().state.get().style});
	}

	return ret;
}

regex_highlighter_model::ppregex_matcher::ppregex_matcher(std::string_view regex_str) :
	matcher(true) // true = preprocessed
{
	// prepare preprocessed regex model

	utki::string_parser p(regex_str);

	std::string str;
	while (true) {
		auto w = p.read_chars_until('$');

		str.append(w);

		if (p.empty()) {
			this->regex_tail = utki::to_utf32(str);
			break;
		}

		ASSERT(!p.empty())
		ASSERT(p.peek_char() == '$')
		p.read_char(); // skip '$'

		switch (p.peek_char()) {
			case '$':
				// escaped dollar sign
				p.read_char(); // skip second '$'
				str.append("$");
				continue;
			case '{':
				// capture group reference
				p.read_char(); // skip '{'
				{
					auto num = p.read_number<unsigned>();
					if (num == 0) {
						throw std::invalid_argument("invalid capture group number: 0, numbering starts from 1");
					}
					ASSERT(num >= 1)
					--num;
					if (p.empty() || p.read_char() != '}') {
						throw std::invalid_argument(
							"preprocessed regex capture group reference syntax error: missing closing '}'"
						);
					}
					this->regex_parts.push_back(regex_part{.str = utki::to_utf32(str), .group_num = num});
					str.clear();
				}
				break;
			default:
				// append skipped '$'
				str.append("$");
				continue;
		}
	}
}

std::shared_ptr<const regex_highlighter_model::matcher> regex_highlighter_model::ppregex_matcher::preprocess(
	utki::span<const match_result::capture_group> capture_groups
) const
{
	std::u32string regex_str;
	for (const auto& p : this->regex_parts) {
		regex_str.append(p.str);

		if (p.group_num < capture_groups.size()) {
			regex_str.append(capture_groups[p.group_num].str);
		}
	}

	regex_str.append(this->regex_tail);

	return std::make_shared<regex_matcher>(regex_str);
}
