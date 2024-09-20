# VSCode Workspace

This is the tools and the template used for updating //engine.code-workspace.

VSCode uses a custom version of JSONC for their config files, those config files
don't provide any mechanism for reducing redundancy. Since the engine has a lot
of test targets, without that mechanism it can get very unwieldy. YAML does
however support ways to reduce redundancy, namely anchors.

## Updating //engine.code-workspace

```sh
./refresh.sh
```

## Backporting //engine.code-workspace

If something is accidentally introduced into //engine.code-workspace without editing
the YAML file here there are tools that can be used to more easily fix that.

```sh
# Convert VSCode's JSONC to JSON
json5 ../../engine.code-workspace -s 2 -o engine.code-workspace
# Convert JSON to YAML
yq eval -P engine.code-workspace > engine-workspace.yaml
```

`yq` can be used to merge YAML files too, but I haven't need to to do it so it
isn't documented.
