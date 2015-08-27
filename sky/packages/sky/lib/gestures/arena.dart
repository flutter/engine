// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

enum GestureDisposition {
  accepted,
  rejected
}

abstract class GestureArenaMember {
  void acceptGesture(Object key);
  void rejectGesture(Object key);
}

class GestureArenaEntry {
  GestureArenaEntry._(this._arena, this._key, this._member);

  final GestureArena _arena;
  final Object _key;
  final GestureArenaMember _member;

  void resolve(GestureDisposition disposition) {
    _arena._updateDisposition(_key, _member, disposition);
  }
}

class GestureArena {
  final Map<Object, List<GestureArenaMember>> _arenas = new Map<Object, List<GestureArenaMember>>();

  GestureArenaEntry add(Object key, GestureArenaMember member) {
    List<GestureArenaMember> members = _arenas.putIfAbsent(key, () => new List<GestureArenaMember>());
    members.add(member);
    return new GestureArenaEntry._(this, key, member);
  }

  void _updateDisposition(Object key, GestureArenaMember member, GestureDisposition disposition) {
    List<GestureArenaMember> members = _arenas[key];
    assert(members != null);
    assert(members.contains(member));
    if (disposition == GestureDisposition.rejected) {
      members.remove(member);
      if (members.length == 1) {
        _arenas.remove(key);
        members.first.acceptGesture(key);
      } else if (members.isEmpty) {
        _arenas.remove(key);
      }
    } else {
      assert(disposition == GestureDisposition.accepted);
      _arenas.remove(key);
      member.acceptGesture(key);
    }
  }
}
