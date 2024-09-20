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
./merge.sh
```

Since JSON doesn't support anchors some work may be needed to resolve any
conflicts that happen when merging.
