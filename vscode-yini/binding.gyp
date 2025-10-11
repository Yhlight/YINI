{
  "targets": [
    {
      "target_name": "yini_validator",
      "sources": [ "native/validator.cpp" ],
      "include_dirs": [
          "<!@(node -p \"require('node-addon-api').include\")",
        "../src"
      ],
      "libraries": [
        "../../build/lib/libLexer.a",
        "../../build/lib/libParser.a",
        "../../build/lib/libResolver.a",
        "../../build/lib/libValidator.a",
        "../../build/lib/libYmetaManager.a"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7"
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 }
      }
    }
  ]
}
