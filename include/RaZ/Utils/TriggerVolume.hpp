#pragma once

#ifndef RAZ_TRIGGERVOLUME_HPP
#define RAZ_TRIGGERVOLUME_HPP

#include "RaZ/Component.hpp"
#include "RaZ/Utils/Shape.hpp"

#include <functional>
#include <variant>

namespace Raz {

/// Triggerer component, representing an entity that can interact with triggerable entities.
/// \see TriggerVolume
class Triggerer final : public Component {};

/// TriggerVolume component, holding a volume that can be triggered and actions that can be executed accordingly.
/// \see Triggerer, TriggerSystem
class TriggerVolume final : public Component {
  friend class TriggerSystem;

public:
  template <typename VolumeT>
  explicit TriggerVolume(VolumeT&& volume) : m_volume{ std::forward<VolumeT>(volume) } {
    // TODO: the OBB's point containment check isn't implemented yet
    static_assert(std::is_same_v<std::decay_t<VolumeT>, AABB> || std::is_same_v<std::decay_t<VolumeT>, Sphere>);
  }

  void setEnterAction(std::function<void()> enterAction) { m_enterAction = std::move(enterAction); }
  void setStayAction(std::function<void()> stayAction) { m_stayAction = std::move(stayAction); }
  void setLeaveAction(std::function<void()> leaveAction) { m_leaveAction = std::move(leaveAction); }

  /// Changes the trigger volume's state.
  /// \param enabled True if the trigger volume should be enabled (triggerable), false otherwise.
  void enable(bool enabled = true) noexcept { m_enabled = enabled; }
  /// Disables the trigger volume, making it non-triggerable.
  void disable() noexcept { enable(false); }
  void resetEnterAction() { setEnterAction(nullptr); }
  void resetStayAction() { setStayAction(nullptr); }
  void resetLeaveAction() { setLeaveAction(nullptr); }

private:
  bool m_enabled = true;

  std::variant<AABB, Sphere> m_volume;
  std::function<void()> m_enterAction;
  std::function<void()> m_stayAction;
  std::function<void()> m_leaveAction;

  bool m_isCurrentlyTriggered = false;
};

} // namespace Raz

#endif // RAZ_TRIGGERVOLUME_HPP
