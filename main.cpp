#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>

// assimilates **all** types into a string
template<typename... T>
std::string 
strf(T... t) {
  std::stringstream s;
  ((s << t), ...); 
  return s.str();
}

// produces a tagged string with signature of type, and optional arguments; e.g. href
std::string 
tag(
    std::string type, 
    std::string content, 
    std::string args=""
  ) {
  if (!args.empty()) args = std::string(" "+args);
  return std::string(strf("<",type,args,">",content,"</",type,">"));
}

// sanitise characters to html-safe
std::string 
__sanitise_html(char c) {
  switch (c) {
    case '&':
      return("&amp;");
    case '>':
      return("&gt;");
    case '<':
      return("&lt;");
    default:
      std::string r; r+=c;
      return r;
  }
}
std::string 
__sanitise_html(std::string s) {
  std::string r;
  for (auto& c: s) {
    if (c == '\\') continue; // skip the escape character
    r.append(__sanitise_html(c));
  }
  return r;
}

std::string 
__substr_next_instance(
    std::string source, 
    char delim, 
    unsigned int i=0
  ) {
  return source.substr(i, source.find(delim, i) - i);
}

std::string 
__parse_html(std::string s) {
  std::string r;
  for (unsigned int i=0; i<s.size(); i++) {
    const char c = s.at(i); 
    if (
        c == '\\' && 
        s.at(i+1) 
          != ('\r' || '\n')
        ) {
      r.append(__sanitise_html(s.at(i+1))); i++; continue; // escape
    }
    // check for special characters
    switch (c) {
      case '`': // Code Block
        {
          size_t j = (s.at(i+2) == c) ? 3 : 1;
          std::string code = __substr_next_instance(s, c, i+j); 
          r.append(tag("code", __sanitise_html(code)));
          i += code.size() + ((j+1) * 2) - 1;
          break;
        }
      case '$': // LaTex
        {
          // (this is similar to our b/i parsing)
          // we determine if this is a multi or single line [^a]
          // - this is represented as `j` by the count of `$`
          // we can use this information to:
          // - get the internal string [^b]
          // - get the corresponding LaTex indicator [^c]
          // - move `i` ahead [^d]
          std::string e; size_t j;
          j = (s.at(i+1) == '$') ? 2 : 1; // [a]
          e = __substr_next_instance(s, c, i+j); // [b]
          r.append(
              strf(
                (j < 2) ? "\\(" : "\\[", // [c]
                e,
                (j < 2) ? "\\)" : "\\]" // [c]
                )
            );
          i += (j*2) + e.size() - 1; // [d]
        }
        break;
      case '\n': // newline      
        r.append("\n<br>\n"); 
        break; 
      case '-': // [TODO] lists
        if (s.at(i+1) == ' ') { // list?
          break;
        }
        if (s.at(i+2) == c) {
          r.append("<hr />");
          i += 2;
          break;
        } continue;
      case '>': // block qoutes
        {
          size_t end = s.find("\n", i+2)-(i+2);
          std::string inner = __parse_html(s.substr(i+2, end)); // parse inter-qoute content
          r.append(tag("qoute", inner));
          i += (end + 1); // account for extra newline
        }
        break;
      case '#': // headers
        {
          size_t h = s.find(' ', i)-i; // determine size of header
          size_t c = i+h+1; // content index, e.g. (#### abcdefg), would be index of 'a'
          std::string e = s.substr(c, s.find("\n", c)-c); // content 
          r.append(tag(strf("h",h), e));
          i += (h+e.size());
        }
        break;
      case '[': // links
        { 
          size_t alias_len = s.find(']', i+1);
          if (s.at(alias_len-1) == '\\') alias_len = s.find(']', alias_len+1); // check for escaped brackets 
  
          if (s.at(i+1) == '^') { // referential endnote
            std::string rurl, alias = s.substr(i+2, (alias_len - 2) - i); // (pos of bracket - size of bracket) - initial index 
            if (
                s.at(alias_len + 1) == ':' // check for source
              ) { // source 
              rurl = strf("id=\"", alias, "\"");
              i = alias_len+1; // account for ':'
            } else { // reference
              rurl = strf("href=\"#", alias, "\""); // link to source
              i = alias_len;
            }
            r.append(tag("sup", tag("a", __sanitise_html(alias), rurl)));
          } else {
            std::string alias = s.substr(i+1, (alias_len - 1) - i);
            std::string link = s.substr(alias_len+2, s.find(')', alias_len)-alias_len-2); // (pos of para - pos bracket) - size of para(s)
            std::string rurl = strf("href=\"", link, "\""); // hyperlink 
            i = (alias_len + link.size()+2); // account for '(' and ')'
            r.append(tag("a", __sanitise_html(alias), rurl));
          }
        } 
        break;
      case '*': // italic (or) bold
        {
          // This is completely over-engineered, but it was bugging me
          // we determine if this is a bold or italicised string
          // - this is represented as `j` by the count of `*` [^a] 
          // we can use this information to:
          // - get the internal string [^b]
          // - get the corresponding html type [^c]
          // - move `i` ahead [^d]
          std::string e; size_t j;
          j = (s.at(i+1) == '*') ? 2 : 1; // [a]
          e = __substr_next_instance(s, s.at(i), i+j); // [b]
          r.append(
              tag(
                (j < 2) ? "i" : "b", // [c]
                __parse_html(e) // enabled nested b/i/*
              )
            );
          i += ((j+1)*2) + e.size() - 1; // [d]
        }
        break;
      default: 
        r.append(__sanitise_html(c));
    }
  }
  return r;
}

int
main(int argc, char** argv) {
  std::string s, l;
  while(getline(std::cin, l)) {s.append(l += '\n');}
  std::cout << __parse_html(s);
  return 0;
}
