#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "Options.h"

#include "Program.h"

#include "output.h"
#include "Format.h"

using namespace std;

output::output(Program* p) : program(p) {
}

filesystem::path output::GetFileForType(std::string_view name) {
  std::filesystem::path out(program->opts->outputDirectory);
  const string filename = boost::algorithm::to_lower_copy(std::string(name)) + program->opts->fileSuffix + ".md";
  if (!std::filesystem::exists(out)) {
    std::error_code ec;
    std::filesystem::create_directory(out, ec); // ignore ec
  }
  return out / filename;
}

output::type_helper output::StartType(std::string_view name, std::string_view kind, std::string description, std::vector<std::string_view> members) {
  EndType();
  indents = 0;
  currentFile = std::move(GetOutputStream(GetFileForType(name)));
  const auto apiVersionPrefix = (program->opts->apiVersion != "") ? ("version-" + program->opts->apiVersion + "-") : "";
  // Adding ms metadata
  *currentFile << "---\n" <<
    "description: " << description << "\n" <<
    "title: " << name << R"(
ms.date: )" << program->opts->msDate << R"(
keywords: webview2, webview, winrt, win32, edge, CoreWebView2, CoreWebView2Controller, browser control, edge html, )" <<
    name << R"(
topic_type:
- APIRef
api_type:
- Assembly
api_location:
- Microsoft.Web.WebView2.Core.dll
api_name:
- )" << name << "\n";

    if (members.size() > 0) {
      for (const auto& member : members) {
        *currentFile << "- " << name << "." << member << "\n";
      }
    }

  if (program->opts->apiVersion != "") {
    *currentFile << "original_id: " << name << "\n";
  }

  *currentFile << "---\n\n";
  string header = string{ name };
  string kindName = string{ kind };
  kindName[0] = toupper(kindName[0]);
  header += " " + kindName;
  return type_helper(*this, header);
}

output::section_helper output::StartSection(const std::string& a) {
  return section_helper(*this, a);
}

output::section_helper::section_helper(output& out, string s) : o(out) {
  o.indents++;
  if (!s.empty()) {
    string t(o.indents, '#');
    o << t << " " << s << "\n";
  }
}

output::type_helper::type_helper(output& out, std::string kind) : o(out), sh(o.StartSection(kind + "\n")) {};

void output::StartNamespace(std::string_view namespaceName) {
  currentXml = intellisense_xml(namespaceName);
}


void intellisense_xml::AddMember(MemberType mt, std::string shortName, std::string data) {
  auto parsedData = Sanitize(data);
  out << R"(
    <member name=")" << ToString(mt) << ":" << namespaceName << "." << shortName << R"(">
      <summary>)" << parsedData << R"(</summary>)";

  out << R"(
    </member>)";
}

bool isTag(size_t& pos, const std::string& text, const std::string tag) {
  auto ret = text.substr(pos)._Starts_with(tag);
  if (ret) {
    pos += tag.length() - 1;
  }
  return ret;
}

std::string intellisense_xml::Sanitize(std::string text) {
  bool isInCode = false;
  bool isInInlineCode = false;
  stringstream ss;

  for (size_t input = 0; input < text.length(); input++) {
    if (!isInCode && (isTag(input, text, "<br/>") || isTag(input, text, "<br />"))) {
      ss << "\n"; break;
    }
    switch (text[input]) {

      //    case '<': {    
      //      ss << "&lt;"; break;
      //    }
      //    case '>':
      //      ss << "&gt;"; break;
      //      break;
    case '`': {
      if (isTag(input, text, "```")) {
        for (size_t i = input; i < text.length(); i++) {
          if (text[i] == '\r' || text[i] == '\n') {
            input = i;
            break;
          }
        }
        isInCode = !isInCode;
        if (!isInCode) {
          // remove the last newline
          const size_t pos = ss.tellp();
          ss.seekp(pos - 1);
        }
        ss << (isInCode ? "<example><code>" : "</code></example>");
      }
      else {
        isInInlineCode = !isInInlineCode;
        ss << (isInInlineCode ? "<c>" : "</c>");
      }
      break;
    }
    default:
      ss << text[input]; break;
    }
  }
  assert(!isInCode);
  return ss.str();
}

