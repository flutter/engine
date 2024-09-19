# json_injector

## Description

This is a tool to manipulate json files. It's main purpose is to reduce
redundancy in VSCode workspaces.

It can inject missing keys from one json file into another, treat List\<Map\>s as
Maps for the purpose of merging and apply templates.

## Usage

The following invocation will update //engine.code-workspace, injecting the json
data from `injectors/engine.code-workspace` and also applying the templates at
`./injectors/engine-templates.json`.  List\<Map\>s who have the key named
'label' will be treated as Maps whose key is "label".

```shell
dart run bin/main.dart \
  --input ~/dev/engine/src/flutter/engine.code-workspace \
  --injector injectors/engine.code-workspace \
  --output ~/dev/engine/src/flutter/engine.code-workspace \
  --name-key label \
  --templates injectors/engine-templates.json
```