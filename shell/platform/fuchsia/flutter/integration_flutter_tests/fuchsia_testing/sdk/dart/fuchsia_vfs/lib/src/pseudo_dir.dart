// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:collection';
import 'dart:convert';
import 'dart:typed_data';

import 'package:fidl/fidl.dart' as fidl;
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:fidl_fuchsia_io2/fidl_async.dart' as io2_fidl;
import 'package:quiver/collection.dart';
import 'package:zircon/zircon.dart';

import 'internal/_flags.dart';
import 'vnode.dart';

// ignore_for_file: import_of_legacy_library_into_null_safe
// ignore_for_file: public_member_api_docs
// ignore_for_file: unnecessary_null_comparison

/// A [PseudoDir] is a directory-like object whose entries are constructed
/// by a program at runtime.  The client can lookup, enumerate, and watch these
/// directory entries but it cannot create, remove, or rename them.
///
/// This class is designed to allow programs to publish a relatively small number
/// of entries (up to a few hundreds) such as services, file-system roots,
/// debugging [PseudoFile].
///
/// This version doesn't support watchers, should support watchers if needed.
class PseudoDir extends Vnode {
  static const _maxObjectNameLength = 256;

  final HashMap<String, _Entry> _entries = HashMap();
  final AvlTreeSet<_Entry> _treeEntries =
      AvlTreeSet(comparator: (v1, v2) => v1.nodeId.compareTo(v2.nodeId));
  int _nextId = 1;
  final List<_DirConnection> _connections = [];
  bool _isClosed = false;

  static bool _isLegalObjectName(String objectName) {
    const forwardSlashUtf16 = 47;
    return objectName.isNotEmpty &&
        objectName.length < _maxObjectNameLength &&
        objectName != '.' &&
        objectName != '..' &&
        objectName.codeUnits
            .every((unit) => (unit != 0 && unit != forwardSlashUtf16));
  }

  /// Adds a directory entry associating the given [name] with [node].
  /// It is ok to add the same Vnode multiple times with different names.
  ///
  /// Returns `ZX.OK` on success.
  /// Returns `ZX.ERR_INVALID_ARGS` if name is illegal object name.
  /// Returns `ZX.ERR_ALREADY_EXISTS` if there is already a node with the
  /// given name.
  int addNode(String name, Vnode node) {
    if (!_isLegalObjectName(name)) {
      return ZX.ERR_INVALID_ARGS;
    }
    if (_entries.containsKey(name)) {
      return ZX.ERR_ALREADY_EXISTS;
    }
    var id = _nextId++;
    var e = _Entry(node, name, id);
    _entries[name] = e;
    _treeEntries.add(e);
    return ZX.OK;
  }

  @override
  void close() {
    _isClosed = true;
    for (var entry in _entries.entries) {
      entry.value.node!.close();
    }
    removeAllNodes();
    // schedule a task because if user closes this as soon as
    // they open a connection, dart fidl binding throws exception due to
    // event(OnOpen) on this fidl.
    scheduleMicrotask(() {
      for (var c in _connections) {
        c.closeBinding();
      }
      _connections.clear();
    });
  }

  /// Connects to this instance of [PseudoDir] and serves
  /// [fushsia.io.Directory] over fidl.
  @override
  int connect(int flags, int mode, fidl.InterfaceRequest<Node> request,
      [int parentFlags = Flags.fsRights]) {
    if (_isClosed) {
      sendErrorEvent(flags, ZX.ERR_NOT_SUPPORTED, request);
      return ZX.ERR_NOT_SUPPORTED;
    }
    // There should be no modeType* flags set, except for, possibly,
    // modeTypeDirectory when the target is a pseudo dir.
    if ((mode & ~modeProtectionMask) & ~modeTypeDirectory != 0) {
      sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, request);
      return ZX.ERR_INVALID_ARGS;
    }

    var connectFlags = filterForNodeReference(flags);

    if (Flags.isPosix(connectFlags)) {
      // grant POSIX clients additional rights
      var parentRights = parentFlags & Flags.fsRights;
      connectFlags |= parentRights;
      connectFlags &= ~openFlagPosix;
      // Mounting is not supported
      connectFlags &= ~openRightAdmin;
    }

    var status = _validateFlags(connectFlags);
    if (status != ZX.OK) {
      sendErrorEvent(connectFlags, status, request);
      return status;
    }
    var connection = _DirConnection(
        mode, connectFlags, this, fidl.InterfaceRequest(request.passChannel()));
    _connections.add(connection);
    return ZX.OK;
  }

  @override
  int inodeNumber() {
    return inoUnknown;
  }

  /// Checks if directory is empty.
  bool isEmpty() {
    return _entries.isEmpty;
  }

  /// Returns names of the the nodes present in this directory.
  List<String> listNodeNames() {
    return _treeEntries.map((f) => f.name).toList();
  }

  /// Looks up a node for given `name`.
  ///
  /// Returns `null` if no node if found.
  Vnode? lookup(String name) {
    var v = _entries[name];
    if (v != null) {
      return v.node;
    }
    return null;
  }

  @override
  void open(
      int flags, int mode, String path, fidl.InterfaceRequest<Node> request,
      [int parentFlags = Flags.fsRights]) {
    if (path.startsWith('/') || path == '') {
      sendErrorEvent(flags, ZX.ERR_BAD_PATH, request);
      return;
    }
    var p = path;
    // remove all ./, .//, etc
    while (p.startsWith('./')) {
      var index = 2;
      while (index < p.length && p[index] == '/') {
        index++;
      }
      p = p.substring(index);
    }

    if (p == '.' || p == '') {
      connect(flags, mode, request, parentFlags);
      return;
    }
    var index = p.indexOf('/');
    var key = '';
    if (index == -1) {
      key = p;
    } else {
      key = p.substring(0, index);
    }
    if (!_isLegalObjectName(key)) {
      sendErrorEvent(flags, ZX.ERR_BAD_PATH, request);
    } else if (_entries.containsKey(key)) {
      var e = _entries[key];
      // final element, open it
      if (index == -1) {
        e!.node!.connect(flags, mode, request, parentFlags);
        return;
      } else if (index == p.length - 1) {
        // '/' is at end, should be a directory, add flag
        e!.node!.connect(flags | openFlagDirectory, mode, request, parentFlags);
        return;
      } else {
        // forward request to child Vnode and let it handle rest of path.
        return e!.node!
            .open(flags, mode, p.substring(index + 1), request, parentFlags);
      }
    } else {
      sendErrorEvent(flags, ZX.ERR_NOT_FOUND, request);
    }
  }

  /// Removes all directory entries.
  void removeAllNodes() {
    _entries.clear();
    _treeEntries.clear();
  }

  /// Removes a directory entry with the given `name`.
  ///
  /// Returns `ZX.OK` on success.
  /// Returns `ZX.RR_NOT_FOUND` if there is no node with the given name.
  int removeNode(String name) {
    var e = _entries.remove(name);
    if (e == null) {
      return ZX.ERR_NOT_FOUND;
    }
    _treeEntries.remove(e);
    return ZX.OK;
  }

  /// Serves this [request] directory over request channel.
  /// Caller may specify the rights granted to the [request] connection.
  /// If `rights` is omitted, it defaults to readable and writable.
  int serve(fidl.InterfaceRequest<Node> request,
      {int rights = openRightReadable | openRightWritable}) {
    assert((rights & ~Flags.fsRights) == 0);
    return connect(openFlagDirectory | rights, 0, request);
  }

  @override
  int type() {
    return direntTypeDirectory;
  }

  void _onClose(_DirConnection obj) {
    final result = _connections.remove(obj);
    scheduleMicrotask(() {
      obj.closeBinding();
    });
    assert(result);
  }

  int _validateFlags(int flags) {
    var allowedFlags = openRightReadable |
        openRightWritable |
        openFlagDirectory |
        openFlagNodeReference |
        openFlagDescribe |
        openFlagPosix |
        cloneFlagSameRights;
    var prohibitedFlags = openFlagCreate |
        openFlagCreateIfAbsent |
        openFlagTruncate |
        openFlagAppend;

    // TODO(fxbug.dev/33058) : do not allow openRightWritable.

    // Pseudo directories do not allow mounting, at this point.
    if (flags & openRightAdmin != 0) {
      return ZX.ERR_ACCESS_DENIED;
    }
    if (flags & prohibitedFlags != 0) {
      return ZX.ERR_INVALID_ARGS;
    }
    if (flags & ~allowedFlags != 0) {
      return ZX.ERR_NOT_SUPPORTED;
    }
    return ZX.OK;
  }
}

/// Implementation of fuchsia.io.Directory for pseudo directory.
///
/// This class should not be used directly, but by [fuchsia_vfs.PseudoDir].
class _DirConnection extends Directory {
  final DirectoryBinding _binding = DirectoryBinding();
  bool isNodeRef = false;

  // reference to current Directory object;
  final PseudoDir _dir;
  final int _mode;
  final int _flags;

  /// Position in directory where [#readDirents] should start searching. If less
  /// than 0, means first entry should be dot('.').
  ///
  /// All the entires in [PseudoDir] are greater then 0.
  /// We will get key after `_seek` and traverse in the TreeMap.
  int _seek = -1;

  bool _isClosed = false;

  /// Constructor
  _DirConnection(this._mode, this._flags, this._dir,
      fidl.InterfaceRequest<Directory> request)
      : assert(_dir != null),
        assert(request != null) {
    _binding.bind(this, request);
    _binding.whenClosed.then((_) {
      return close();
    });
    if (_flags & openFlagNodeReference != 0) {
      isNodeRef = true;
    }
  }

  @override
  Stream<Directory$OnOpen$Response> get onOpen {
    Directory$OnOpen$Response d;
    if ((_flags & openFlagDescribe) == 0) {
      d = Directory$OnOpen$Response(ZX.ERR_NOT_DIR, null);
    } else {
      NodeInfo nodeInfo = _describe();
      d = Directory$OnOpen$Response(ZX.OK, nodeInfo);
    }
    return Stream.fromIterable([d]);
  }

  @override
  Future<void> clone(int flags, fidl.InterfaceRequest<Node> object) async {
    if (!Flags.inputPrecondition(flags)) {
      _dir.sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, object);
      return;
    }
    if (Flags.shouldCloneWithSameRights(flags)) {
      if ((flags & Flags.fsRightsSpace) != 0) {
        _dir.sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, object);
        return;
      }
    }

    // If SAME_RIGHTS is requested, cloned connection will inherit the same
    // rights as those from the originating connection.
    var newFlags = flags;
    if (Flags.shouldCloneWithSameRights(flags)) {
      newFlags &= (~Flags.fsRights);
      newFlags |= (_flags & Flags.fsRights);
      newFlags &= ~cloneFlagSameRights;
    }

    if (!Flags.stricterOrSameRights(newFlags, _flags)) {
      _dir.sendErrorEvent(flags, ZX.ERR_ACCESS_DENIED, object);
      return;
    }

    _dir.connect(newFlags, _mode, object);
  }

  @override
  Future<int> close() async {
    if (_isClosed) {
      return ZX.OK;
    }
    scheduleMicrotask(() {
      _dir._onClose(this);
    });
    _isClosed = true;

    return ZX.OK;
  }

  void closeBinding() {
    _binding.close();
    _isClosed = true;
  }

  @override
  Future<NodeInfo> describe() async {
    return _describe();
  }

  @override
  Future<Directory$GetAttr$Response> getAttr() async {
    var n = NodeAttributes(
      mode: modeTypeDirectory | modeProtectionMask,
      id: inoUnknown,
      contentSize: 0,
      storageSize: 0,
      linkCount: 1,
      creationTime: 0,
      modificationTime: 0,
    );
    return Directory$GetAttr$Response(ZX.OK, n);
  }

  @override
  Future<Directory$GetToken$Response> getToken() async {
    return Directory$GetToken$Response(ZX.ERR_NOT_SUPPORTED, null);
  }

  @override
  Future<int> link(String src, Handle dstParentToken, String dst) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<void> addInotifyFilter(String path, io2_fidl.InotifyWatchMask filters,
      int wd, Socket socket) async {
    return;
  }

  @override
  Future<void> open(int flags, int mode, String path,
      fidl.InterfaceRequest<Node> object) async {
    if (!Flags.inputPrecondition(flags)) {
      _dir.sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, object);
      return;
    }
    if (Flags.shouldCloneWithSameRights(flags)) {
      _dir.sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, object);
      return;
    }
    if (Flags.isNodeReference(flags) && ((flags & Flags.fsRights) == 0)) {
      _dir.sendErrorEvent(flags, ZX.ERR_INVALID_ARGS, object);
      return;
    }
    if (!Flags.stricterOrSameRights(flags, _flags)) {
      _dir.sendErrorEvent(flags, ZX.ERR_ACCESS_DENIED, object);
      return;
    }
    _dir.open(flags, mode, path, object, _flags);
  }

  @override
  Future<Directory$ReadDirents$Response> readDirents(int maxBytes) async {
    if (isNodeRef) {
      return Directory$ReadDirents$Response(ZX.ERR_BAD_HANDLE, Uint8List(0));
    }
    var buf = Uint8List(maxBytes);
    var bData = ByteData.view(buf.buffer);
    var firstOne = true;
    var index = 0;

    // add dot
    if (_seek < 0) {
      var bytes = _encodeDirent(
          bData, index, maxBytes, inoUnknown, direntTypeDirectory, '.');
      if (bytes == -1) {
        return Directory$ReadDirents$Response(
            ZX.ERR_BUFFER_TOO_SMALL, Uint8List(0));
      }
      firstOne = false;
      index += bytes;
      _seek = 0;
    }

    var status = ZX.OK;

    // add entries
    var entry = _dir._treeEntries.nearest(_Entry(null, '', _seek),
        nearestOption: TreeSearch.GREATER_THAN);

    if (entry != null) {
      var iterator = _dir._treeEntries.fromIterator(entry);
      while (iterator.moveNext()) {
        entry = iterator.current;
        // we should only send entries > _seek
        if (entry.nodeId <= _seek) {
          continue;
        }
        var bytes = _encodeDirent(bData, index, maxBytes,
            entry.node!.inodeNumber(), entry.node!.type(), entry.name);
        if (bytes == -1) {
          if (firstOne) {
            status = ZX.ERR_BUFFER_TOO_SMALL;
          }
          break;
        }
        firstOne = false;
        index += bytes;
        status = ZX.OK;
        _seek = entry.nodeId;
      }
    }
    return Directory$ReadDirents$Response(
        status, Uint8List.view(buf.buffer, 0, index));
  }

  @override
  Future<int> rename(String src, Handle dstParentToken, String dst) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> rename2(String src, Handle dstParentToken, String dst) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> rewind() async {
    _seek = -1;
    return ZX.OK;
  }

  @override
  Future<int> setAttr(int flags, NodeAttributes attributes) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> sync() async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> unlink(String path) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> unlink2(String name, io2_fidl.UnlinkOptions options) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> watch(int mask, int options, Channel watcher) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  NodeInfo _describe() {
    return NodeInfo.withDirectory(DirectoryObject(reserved: 0));
  }

  /// returns number of bytes written
  int _encodeDirent(ByteData buf, int startIndex, int maxBytes, int inodeNumber,
      int type, String name) {
    List<int> charBytes = utf8.encode(name);
    var len = 8 /*ino*/ + 1 /*size*/ + 1 /*type*/ + charBytes.length;
    // cannot fit in buffer
    if (maxBytes - startIndex < len) {
      return -1;
    }
    var index = startIndex;
    buf.setUint64(index, inodeNumber, Endian.little);
    index += 8;
    buf..setUint8(index++, charBytes.length)..setUint8(index++, type);
    for (int i = 0; i < charBytes.length; i++) {
      buf.setUint8(index++, charBytes[i]);
    }
    return len;
  }
}

/// _Entry class to store in pseudo directory.
class _Entry {
  /// Vnode
  Vnode? node;

  /// node name
  String name;

  /// node id: defines insertion order
  int nodeId;

  /// Constructor
  _Entry(this.node, this.name, this.nodeId);
}
