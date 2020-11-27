#pragma once
#include <string_view>
#include <string>
#include <winmd_reader.h>

struct Program;

struct Formatter
{
  Formatter(Program* p) : program(p) {};

  static std::string MakeMarkdownReference(std::string type, std::string propertyName);

  static std::string MakeXmlReference(std::string type, std::string propertyName);

  using Converter = std::string(*)(std::string, std::string);
  std::string ResolveReferences(std::string sane, Converter converter);

  std::string typeToMarkdown(std::string_view ns, std::string type, bool toCode, std::string urlSuffix = "");

  std::string GetNamespacePrefix(std::string_view ns);

  static std::string_view ToString(winmd::reader::ElementType elementType);
  std::string ToString(const winmd::reader::coded_index<winmd::reader::TypeDefOrRef>& tdr, bool toCode = true);


  std::string GetType(const winmd::reader::TypeSig& type);
  std::string GetType(const winmd::reader::TypeSig::value_type& valueType);

private:
  Program* program;
};

std::string code(std::string_view v);
std::string link(std::string_view n);

