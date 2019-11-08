// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_CONVERTER_H_
#define FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_CONVERTER_H_

#include <string.h>

#include <map>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"

namespace flutter {

//------------------------------------------------------------------------------
/// The current information about a pointer. This struct is used by
/// PointerDataPacketConverter to fill in necesarry information for raw pointer
/// packet sent from embedding.
///
struct PointerState {
  int64_t pointer;
  bool isDown;
  double physical_x;
  double physical_y;
};

//------------------------------------------------------------------------------
/// Converter to expend the raw pointer data packet from the platforms.
///
/// All pointer data packets that are sent from all the platforms must go
/// through this converter before sending them to framework.
///
class PointerDataPacketConverter {
 public:
  PointerDataPacketConverter();
  ~PointerDataPacketConverter();

  //----------------------------------------------------------------------------
  /// @brief      Expands pointer data packet into a form that framework
  ///             understands. The raw pointer data packet from embedding does
  ///             not have sufficient information and may contain illegal pointer
  ///             events. This method will fill out those information and attempt
  ///             to correct pointer events.
  ///
  /// @param[in]  packet                   The raw pointer packet sent from
  ///                                      embedding.
  ///
  /// @return     A full Expended packet with all the required information filled.
  ///             It may contain synthetic pointer event as the result of converter's
  ///             attempt to correct illegal pointer events.
  ///
  std::unique_ptr<PointerDataPacket> Expand(
      std::unique_ptr<PointerDataPacket> packet);

 private:
  std::map<int64_t, PointerState> states_;

  int64_t pointer_;

  void ExpandPointerData(PointerData pointer_data,
                         std::vector<PointerData>& converted_pointers);

  PointerState EnsurePointerState(PointerData pointer_data);

  void UpdateDeltaAndState(PointerData& pointer_data, PointerState& state);

  void UpdatePointer(PointerData& pointer_data,
                     PointerState& state,
                     bool start_new_pointer);

  bool LocationNeedsUpdate(const PointerData pointer_data,
                           const PointerState state);

  FML_DISALLOW_COPY_AND_ASSIGN(PointerDataPacketConverter);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_POINTER_DATA_PACKET_CONVERTER_H_
