// Copyright 2016 Google, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library dart_objc;

import 'dart:nativewrappers';

Object nil = null;
bool YES = true;
bool NO = false;

class Selector extends NativeWrapper2 {
  String get name native 'sel_getName';
  bool get isMapped native 'sel_isMapped';
  bool isEqual(Selector other) native 'sel_isEqual';

  static Selector registerName(String name) native 'sel_registerName';
  static Selector getUid(String name) native 'sel_getUid';
}

class AssociationPolicy {
  AssociationPolicy._();

  const int assign = 0;
  const int retainNonatomic = 1;
  const int copyNonatomic = 3;
  const int retain = 01401;
  const int copy = 01403;
}

class id extends NativeWrapper2 {
  id copy(int size) native 'object_copy';
  id dispose() native 'object_dispose';
  Class get cls native 'object_getClass';
  void set cls(Class cls) native 'object_setClass';
  bool get isClass native 'object_isClass';
  String get className native 'object_getClassName';
  id getIvar(Ivar ivar) native 'object_getIvar';
  void setIvar(Ivar ivar) native 'object_setIvar';
  void setInstanceVariable(String name, int value) native 'object_setInstanceVariable';
  int getInstanceVariable(String name) native 'object_getInstanceVariable';
  void setAssociatedObject(int key, id value, int policy) native 'objc_setAssociatedObject';
  id getAssociatedObject(int key) native 'objc_getAssociatedObject';
  void removeAssociatedObjects() native 'objc_removeAssociatedObjects';

  id msgSend(Selector selector, List args) native 'objc_msgSend';
  id msgSendSuper(Selector selector, List args) native 'objc_msgSendSuper';
}

class Class extends NativeWrapper2 {
  String get className native 'class_getName';
  bool get isMetaClass native 'class_isMetaClass';
  Class get superclass native 'class_getSuperclass';
  void set superclass(Class cls) native 'class_setSuperclass';
  int get version native 'class_getVersion'
  void set version(int version) native 'class_setVersion';
  int get instanceSize native 'class_getInstanceSize';
  Ivar getInstanceVariable(String name) native 'class_getInstanceVariable';
  Ivar getClassVariable(String name) native 'class_getClassVariable';
  List<Ivar> copyIvarList() native 'class_copyIvarList';
  Method getInstanceMethod(Selector selector) native 'class_getInstanceMethod';
  Method getClassMethod(Selector selector) native 'class_getInstanceMethod';
  bool respondsToSelector(Selector selector) native 'class_respondsToSelector';
  List<Method> copyMethodList() native 'class_copyMethodList';
  bool conformsToProtocol(Protocol protocol) native 'class_conformsToProtocol';
  List<Protocol> copyProtocolList() native 'class_copyProtocolList';
  Property getProperty(String name) native 'class_getProperty';
  List<Property> copyPropertyList() native 'class_copyPropertyList';
  String getIvarLayout() native 'class_getIvarLayout';
  String getWeakIvarLayout() native 'class_getWeakIvarLayout';
  bool addProtocol(Protocol protocol) native 'class_addProtocol';
}

class Protocol extends NativeWrapper2 {
  bool conformsToProtocol(Protocol protocol) native 'protocol_conformsToProtocol';
  bool isEqual(Protocol other) native 'protocol_isEqual';
  String get name native 'protocol_getName';
  MethodDescription getMethodDescription(Selector selector, bool isRequiredMethod, bool isInstanceMethod) native 'protocol_getMethodDescription';
  List<MethodDescription> copyMethodDescriptionList(bool isRequiredMethod, bool isInstanceMethod) native 'protocol_copyMethodDescriptionList';
  Property getProperty(String name, bool isRequiredProperty, bool isInstanceProperty) native 'protocol_getProperty';
  List<Property> copyPropertyList() native 'protocol_copyPropertyList';
  List<Protocol> copyProtocolList() native 'protocol_copyProtocolList';
  void addMethodDescription(Selector selector, String types, bool isRequiredMethod, bool isInstanceMethod) native 'protocol_addMethodDescription';
  void addProtocol(Protocol additional) native 'protocol_addProtocol';
  void addProperty(String name, List<PropertyAttribute> attributes, bool isRequiredProperty, bool isInstanceProperty) native 'protocol_addProperty';
}

class Method extends NativeWrapper2 {
  Selector get name native 'method_getName';
  String get typeEncoding native 'method_getTypeEncoding';
  String get numberOfArguments native 'method_getNumberOfArguments';
  String get returnType native 'method_copyReturnType';
  String getArgumentType(int index) native 'method_copyArgumentType';
}

class Ivar extends NativeWrapper2 {
  String get name native 'ivar_getName';
  String get typeEncoding native 'ivar_getTypeEncoding';
  int get offset native 'ivar_getOffset';
}

class Property extends NativeWrapper2 {
  String get name native 'property_getName';
  String get value native 'property_copyAttributeValue';
  String get attributes native 'property_getAttributes';
  List<PropertyAttribute> copyAttributeList() native 'property_copyAttributeList';
}

class MethodDescription {
  Selector name;
  String types;
}

class PropertyAttribute {
  String name;
  String value;
}

Class getClass(String name) native 'objc_getClass';
Class getMetaClass(String name) native 'objc_getMetaClass';
Class lookUpClass(String name) native 'objc_lookUpClass';
Class getRequiredClass(String name) native 'objc_getRequiredClass';
List<Class> getClassList() native 'objc_getClassList';
Class getFutureClass(String name) native 'objc_getFutureClass';
Class allocateClassPair(Class superclass, String name, int extraBytes) native 'objc_allocateClassPair';
void registerClassPair(Class cls) native 'objc_registerClassPair';
void disposeClassPair(Class cls) native 'objc_disposeClassPair';
Protocol getProtocol(String name) native 'objc_getProtocol';
List<Protocol> copyProtocolList() native 'objc_copyProtocolList';
Protocol allocateProtocol(String name) native 'objc_allocateProtocol';
void registerProtocol(Protocol protocol) native 'objc_registerProtocol';
