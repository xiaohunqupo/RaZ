#include "RaZ/Application.hpp"
#include "RaZ/Script/LuaScript.hpp"
#include "RaZ/Script/LuaWrapper.hpp"
#include "RaZ/Utils/FileUtils.hpp"

#define SOL_SAFE_GETTER 0 // Allowing implicit conversion to bool
#include "sol/sol.hpp"

namespace Raz {

LuaScript::LuaScript(const std::string& code) {
  Raz::LuaWrapper::registerTypes();
  loadCode(code);
}

void LuaScript::loadCode(const std::string& code) {
  const sol::object owningEntity = m_environment.get("this");

  m_environment.clear();

  if (!m_environment.execute(code))
    throw std::invalid_argument("Error: The given Lua script is invalid");

  if (m_environment.get("update").get_type() != sol::type::function)
    throw std::invalid_argument("Error: A Lua script must have an update() function");

  if (owningEntity.is<Entity>()) {
    m_environment.registerEntity(owningEntity.as<Entity>(), "this");
    setup();
  }
}

void LuaScript::loadCodeFromFile(const FilePath& filePath) {
  loadCode(FileUtils::readFile(filePath));
}

bool LuaScript::update(const FrameTimeInfo& timeInfo) const {
  const sol::unsafe_function updateFunc = m_environment.get("update");
  const sol::unsafe_function_result updateRes = updateFunc(timeInfo);
  return (updateRes.get_type() == sol::type::none || updateRes);
}

void LuaScript::setup() const {
  if (!m_environment.exists("setup"))
    return;

  const sol::reference setupRef = m_environment.get("setup");

  if (setupRef.get_type() != sol::type::function)
    return;

  const sol::unsafe_function setupFunc = setupRef;

  if (!setupFunc().valid())
    throw std::runtime_error("Error: The Lua script failed to be setup");
}

} // namespace Raz