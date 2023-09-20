# Caliburn

Caliburn is a free and open-source high-level shader language for Vulkan development.

**DISCLAIMER:** Caliburn is a work-in-progress language written by one person. Language featurs may not work correctly. Shaders compiled may not be correct. Breaking changes are inevitable. And please, report all bugs to the GitHub repo. Thank you :)

## License

Caliburn is distributed under the MIT License. See the [LICENSE](LICENSE) file for more information.

## Contributing

Pull requests and issues are always welcomed.

## Features

Caliburn is a multi-paradigm shader language. Language features include:

* **Functions** via top-level `def`
* **Objects** via `class`
* **Generics**: All function and type declarations support both type and constant generics.
* **Shader Pipeline Objects**: A `shader` doesn't just define one named shader, but rather it can define an entire named pipeline, managing inputs and outputs for you to enable for better maintainability.
* **Immutable Structs**: `record` defines an immutable data struct. There's also `struct` in case you need mutability.
* **Type Aliasing**: `type` lets you define an alias.
* **Type Replacement**: Define a `type` alias as `dynamic`, and the type can be changed at compilation time via the compiler settings. `dynamic` also has a generic component which defines a default type, e.g. `dynamic<int32>`.

Caliburn has plans to support:

* **Cross-Platform Shader Development**: Support for Vulkan is underway; Support for DirectX 12 and Metal are planned.

* **Modules**

* **Package Manager**

* **IDE Support**

## Getting Started

### Installation

```bash
git clone https://github.com/JeffPerando/caliburn.git
```

Caliburn uses C++17, but has no library dependencies.

Caliburn does, however, require CMake to build. The main project is inside the `compiler` directory. Compiling to a DLL requires the `"CBRN_OUT"` CMake property to be set to `"DLL"`.

If you don't want to make a new project to try it out, there's a `main.cpp` in the `compiler` project. To use this, set the `"CBRN_OUT"` CMake property to `"EXE"`.

There is a `CMakePresets.json`, but currently is only designed for Windows. Feel free to write Linux and MacOS presets and send a pull request.

### Usage

The Caliburn API is defined in the standalone `caliburn.h` header. No macros are needed to use it as a DLL:

```cpp
//DLL include
#include <caliburn.h>
```

It's not recommended to use Caliburn as a static library; however, the library itself is a work-in-progress, so using it as a DLL may be prone to more issues. To use statically, define the `CBRN_NO_IMPORT` macro:

```cpp
//Static include
#define CBRN_NO_IMPORT
#include <caliburn.h>
```

Within your program, now you just define your settings and compiler object:

```cpp
caliburn::CompilerSettings settings;
//Edit settings here
caliburn::Compiler compiler(settings);
```

Compile:

```cpp
//Replace "HelloTriangle" with the name of your shader object
caliburn::ShaderResult result = compiler.compileSrcShaders(src, "HelloTriangle");
```

Check for errors:

```cpp
if (!result.success())
{
    for (auto const& errMsg : result.errors)
    {
        std::cout << errMsg << '\n';
    }
}
```

Iterate through `result.shaders` and upload to Vulkan:

```cpp
VkShaderModule uploadShader(const caliburn::Shader& shader) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader.code->size() * 4;
    createInfo.pCode = shader.code->data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::exception("shader upload failed");
    }

    return shaderModule;
}
```

The rest is all standard Vulkan programming.

The `Shader` struct will also have named descriptors and vertex inputs, so bind resources appropriately.

**NOTE:** The Caliburn compiler is single-threaded, so integrating into a multithreaded system is trivial.

### Example Shaders

**NOTE:** Caliburn is a work-in-progress. Example shaders may not compile.

Here's Hello Triangle in Caliburn; This creates 2 shaders, one vertex and one fragment. Data is passed from one to another via user-made IO variables. In this case, `fragColor` is an `out` in the vertex shader, and an `in` in the fragment shader.

```caliburn
shader HelloTriangle
{
    vec3 fragColor;

    def vertex(vec3 inVtx, vec3 inColor): vec4
    {
        fragColor = inColor;
        return inVtx ++ 0f;
    };

    def frag(): vec4
    {
        return fragColor;
    };
};
```

And here's a shader that applies a grayscale filter to an image; This defines a descriptor, `tex`.

```caliburn
import math;

shader GrayscaleShader(tex2d tex)
{
    def frag(vec2 inUV): vec4
    {
        const color = tex.sample(inUV);
        const: fp32 gray = math.dot(color.rgb, vec3(0.299, 0.587, 0.114));
        return vec3(gray) ++ color.a;
    };
};
```

## Documentation

A specification is in the works.
