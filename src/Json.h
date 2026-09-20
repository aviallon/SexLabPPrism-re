#pragma once

#include <cstdio>
#include <string>
#include <string_view>

// Minimal JSON helpers. The original builds its JSON with fmt
// (FUN_18000f3c0 / FUN_18000f650) and escapes strings with FUN_18002bee0.
// We reproduce the same output shape with plain std::string concatenation so
// the plugin does not need fmt as a link-time dependency for the natives.
namespace PrismJson
{
	inline void AppendEscaped(std::string& a_out, std::string_view a_in)
	{
		for (const char c : a_in) {
			switch (c) {
			case '"':  a_out += "\\\""; break;
			case '\\': a_out += "\\\\"; break;
			case '\n': a_out += "\\n"; break;
			case '\r': a_out += "\\r"; break;
			case '\t': a_out += "\\t"; break;
			default:
				if (static_cast<unsigned char>(c) < 0x20) {
					char buf[8];
					std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
					a_out += buf;
				} else {
					a_out += c;
				}
			}
		}
	}

	// "..." with the string escaped. Used for every string field.
	inline void AppendQuoted(std::string& a_out, std::string_view a_in)
	{
		a_out += '"';
		AppendEscaped(a_out, a_in);
		a_out += '"';
	}

	inline std::string Quoted(std::string_view a_in)
	{
		std::string out;
		AppendQuoted(out, a_in);
		return out;
	}

	// The original formats the speed with fmt's "{:.2f}".
	inline void AppendFixed2(std::string& a_out, float a_value)
	{
		char buf[32];
		std::snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(a_value));
		a_out += buf;
	}
}  // namespace PrismJson