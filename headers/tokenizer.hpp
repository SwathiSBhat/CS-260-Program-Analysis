#pragma once

#include <regex>

#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

using std::nullopt;
using std::optional;
using std::set;
using std::string;
using std::vector;
using std::pair;

namespace util {

class Tokenizer {
 public:
  // The input to be parsed; what characters should be considered whitespace
  // (and hence automatically skipped); what strings are considered delimiters
  // (i.e., tokens in their own right, no matter what is around them, and
  // reading them when they aren't expected is an error); what strings are
  // considered reserved words (i.e., reading them when they aren't expected is
  // an error); what two strings (if any) are considered to delimit "raw" parts
  // of the input that should be considered tokens verbatim (i.e., all
  // delimiters and whitespace inside those delimiters are preserved).
  //
  // Note that '\n' is always considered a delimiter, but if it is also included
  // in whitespace then it is ignored when reading tokens. Also, if one
  // delimiter is a prefix of another then the input is tokenized based on the
  // longer delimiter first, then the shorter delimiter.
  Tokenizer(const string& input, const set<char>& whitespace,
            const set<string>& delimiters, const set<string>& reserved_words,
            const optional<std::pair<string, string>>& raw = nullopt)
      : delimiters_(delimiters), reserved_words_(reserved_words) {
    // Is '\n' considered whitespace?
    newline_is_whitespace_ = whitespace.count('\n');

    // For tokenization, '\n' is always considered a delimiter.
    delimiters_.insert("\n");

    // Break the input into raw and non-raw pieces; turn the raw pieces directly
    // into tokens and tokenize the non-raw pieces.
    if (raw) {
      auto [left, right] = *raw;

      // Treat the raw delimiters also as regular delimiters.
      delimiters_.insert(left);
      delimiters_.insert(right);

      size_t start = input.find(left), end = 0;
      while (start != string::npos) {
        Tokenize(input.substr(end, start - end), whitespace);
        tokens_.push_back(left);
        start += left.size();
        end = input.find(right, start);
        /*CHECK_NE(end, string::npos)
            << "Left raw delimiter unmatched by right raw delimiter";*/
        tokens_.push_back(input.substr(start, end - start));
        tokens_.push_back(right);
        end += right.size();
        start = input.find(left, end);
      }
      Tokenize(input.substr(end), whitespace);
    } else {
      Tokenize(input, whitespace);
    }

    // Reverse so that the beginning of the input is at the end of tokens_.
    std::reverse(tokens_.begin(), tokens_.end());
  }

  vector<string> Tokens() const { return tokens_; }

  void Tokenize(const string& str, const set<char>& whitespace) {
    // During tokenization '\n' is not considered
    // whitespace (so that we can keep track of line numbers during parsing).
    string space;
    for (char symbol : whitespace) {
      if (symbol != '\n') space.push_back(symbol);
    }

    // Tokenize based on whitespace and delimiters.
    size_t start = str.find_first_not_of(space), end = 0;
    while (start != string::npos && end != string::npos) {
      end = str.find_first_of(space, start);
      string seq = str.substr(start, end - start);
      DelimitAndAddTokens(seq);
      if (end != string::npos) {
        start = str.find_first_not_of(space, end);
      }
    }
  }

  // Separates 'str' into pieces based on delimiters_ and adds them to tokens_.
  void DelimitAndAddTokens(const string& str) {
    // We sort the delimiters by size to guarantee that if delimiter A is a
    // prefix of delimiter B, then B is processed before A is processed.
    vector<string> delimiters(delimiters_.begin(), delimiters_.end());
    std::sort(delimiters.begin(), delimiters.end(),
              [](string& s1, string& s2) { return s1.size() > s2.size(); });

    // Find minimum position of any delimiter.
    const auto find_min = [&](const string& str) {
      size_t min_pos = string::npos;
      int length = 0;
      for (const auto& delimit : delimiters_) {
        auto pos = str.find(delimit);
        if (pos < min_pos) {
          min_pos = pos;
          length = delimit.size();
        }
      }
      return pair<size_t, int>{min_pos, length};
    };

    string token = str;
    //CHECK_NE(token, "") << "Empty token";

    while (true) {
      auto [pos, length] = find_min(token);
      if (pos == string::npos) {
        if (token != "") tokens_.push_back(token);
        break;
      }
      if (pos != 0) tokens_.push_back(token.substr(0, pos));
      tokens_.push_back(token.substr(pos, length));
      token = token.substr(pos + length);
    }
  }

  static string Consume(vector<std::string> &tokens) {
    if (tokens.empty()) {
        return "";
    }

    string token = tokens.back();
    tokens.pop_back();
    return token;
  }

static void ConsumeToken(vector<std::string> &tokens, std::string str) {
    if (tokens.empty()) {
        return;
    }
    if (tokens.back() != str) {
        std::cout << "Expected " << str << " but got " << tokens.back() << std::endl;
        return;
    }
    Consume(tokens);
}

  // The tokenized input, in reverse (the back of the vector is the front of the
  // input). '\n' is always considered a delimiter even if it's specified as
  // whitespace; '\n' being whitespace is handled specially by ReturnNextToken()
  // and ConsumeNextToken().
  vector<string> tokens_;

  // The current line number within the input being parsed.
  int line_number_ = 1;

  // Special strings: delimiters are always individual tokens; delimiters and
  // reserved words cause an error if they are consumed by ConsumeToken().
  set<string> delimiters_;
  set<string> reserved_words_;

  // Remembers whether '\n' should be considered a whitespace character.
  bool newline_is_whitespace_ = false;
};

}  // namespace util