#pragma once

namespace luxe {

// A higher-priority producer must block when its lower-priority consumer needs
// to drain a full buffer. FreeRTOS can return immediately for an oversized send.
template<typename Write, typename Wait>
inline auto cooperative_write(Write write, Wait wait) {
  auto accepted = write();
  if (accepted == 0) wait();
  return accepted;
}

}  // namespace luxe
