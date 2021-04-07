# SPIR-V Transpiler

A dart library for transpiling a subset of SPIR-V to the shader languages used by Flutter internally.

- [SkSL](https://skia.org/docs/user/sksl/)
- [GLSL ES 100](https://www.khronos.org/files/opengles_shading_language.pdf)
- [GLSL ES 300](https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf)

All exported symbols are documented in `lib/spirv.dart`.

The supported subset of SPIR-V is specified in `lib/src/constants.dart`.

If you're using GLSL to generate SPIR-V with `glslangValidator` or `shaderc`,
the code will need to adhere to the following rules.

- There must be a single vec4 output at location 0.
- The output can only be written to from the main function.
- `gl_FragCoord` can only be read from the main function, and its z and w components
  have no meaning.
- Control flow is prohibited aside from function calls and `return`.
  `if`, `while`, `for`, `switch`, etc.
- No inputs from other shader stages.
- Only float, float-vector types, and square float-matrix types.
- Only square matrices are supported.
- Only built-in functions present in GLSL ES 100 are used.
- Debug symbols must be stripped, you can use the `spirv-opt` `--strip_debug` flag.

These rules may become less strict in future versions. Confirmant SPIR-V should succesfully transpile from the current version onwards.  In other words, a spir-v shader you use now that meets these rules should keep working, but the output of the transpiler may change for that shader.

Support for textures, control flow, and structured types is planned, but not currently included.

## Testing

Tests are based on golden files contained at `tests/goldens`. SPIR-V binary files
ending in `.spv` are read and transpiled to each supported language. The results
are compared against the `.golden` files. If you add or change `.spv` files, or if
you modify the transpiler in a way that changes its output, you can run `dart run tool/regenerate_goldens.dart` to generate new `.golden` files.

The current `.spv` files are generated from `.glsl` files under `tests/goldens/src`, by running `./tool/compile_to_spirv.sh`. That requires `glslangValidator` and `spirv-opt` to
be on your `PATH`. If you compile new `.spv` files, be sure to generate new golden files as well with `dart run tool/regenerate_goldens.dart`.

Golden files can be validated locally during development using tools like `glslangValidator`. They will also be compiled and tested for compilation errors as part of Flutter's engine unit tests.
