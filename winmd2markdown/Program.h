#pragma once
#include <string>
#include <filesystem>
#include <winmd_reader.h>

#include "Options.h"
#include "output.h"
#include "Format.h"

using property_entry_t = std::pair<std::string_view, const winmd::reader::Property>;
using method_entry_t = std::pair<std::string_view, const winmd::reader::MethodDef>;
using event_entry_t = std::pair<std::string_view, const winmd::reader::Event>;

struct Program {
  std::string currentNamespace;
  static constexpr std::string_view ObjectClassName = "Object"; // corresponds to IInspectable in C++/WinRT
  static constexpr std::string_view ctorName = ".ctor";
  std::unique_ptr<winmd::reader::cache> cache{ nullptr };
  std::unique_ptr<options> opts;

  std::map<std::string, std::vector<winmd::reader::TypeDef>> interfaceImplementations{};


  int Process(std::vector<std::string>);

  template <typename T>
  void AddUniqueReference(const T& type, const winmd::reader::TypeDef& owningType);

  // map of namespaces N -> (map of types T in N -> (list of types that reference T))
  std::map<std::string, std::map<std::string, std::vector<winmd::reader::TypeDef>>> references{};
  output ss;
  friend class UnitTests;

  Program() : ss(this), format(this) {}
private:
  void process_class(output& ss, const winmd::reader::TypeDef& type, std::string kind);
  void process_enum(output& ss, const winmd::reader::TypeDef& type);
  void process_event(output& ss, const winmd::reader::TypeDef& classType, const winmd::reader::Event& evt, bool asTable = false);
  void process_property(output& ss, const winmd::reader::Property& prop, bool asTable = false);
  void process_method(output& ss, const winmd::reader::MethodDef& method, std::string_view realName = "", bool asTable = false);
  void process_field(output& ss, const winmd::reader::Field& field);
  void process_struct(output& ss, const winmd::reader::TypeDef& type);
  void process_delegate(output& ss, const winmd::reader::TypeDef& type);
  void process(std::string_view namespaceName, const winmd::reader::cache::namespace_members& ns);

  void write_index(std::string_view namespaceName, const winmd::reader::cache::namespace_members& ns);

  void AddReference(const winmd::reader::TypeSig& prop, const winmd::reader::TypeDef& owningType);
  void AddReference(const winmd::reader::coded_index<winmd::reader::TypeDefOrRef>& classTypeDefOrRef, const winmd::reader::TypeDef& owningType);

  std::string getWindowsWinMd();
  template<typename IT>
  bool shouldSkipInterface(const IT /*TypeDef*/& interfaceEntry);

  template<typename T, typename F = nullptr_t>
  void PrintOptionalSections(MemberType mt, output& ss, const T& type, std::optional<F> fallback_type = std::nullopt);

  template<typename T, typename Converter>
  std::string GetDeprecated(const T& type, Converter converter);

  Formatter format;

  std::list<property_entry_t> GetSortedProperties(const winmd::reader::TypeDef& type);
  std::list<method_entry_t> GetSortedMethods(const winmd::reader::TypeDef& type);
  std::list<event_entry_t> GetSortedEvents(const winmd::reader::TypeDef& type);
};
