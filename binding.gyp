{
  'targets': [
    {
      'target_name': 'object-wrap-demo-native',
      'sources': [
        'src_cpp/object_wrap_demo.cc',
        "zoom-bot/src/Zoom.cpp",
        "zoom-bot/src/Config.cpp",
        # "zoom-bot/src/util/Singleton.h",
        # "zoom-bot/src/util/Log.h",
        "zoom-bot/src/events/AuthServiceEvent.cpp",
        "zoom-bot/src/events/MeetingServiceEvent.cpp",
        "zoom-bot/src/events/MeetingReminderEvent.cpp",
        "zoom-bot/src/events/MeetingRecordingCtrlEvent.cpp",
        "zoom-bot/src/raw-record/ZoomSDKAudioRawDataDelegate.cpp",        
        # "zoom-bot/src/raw-record/ZoomSDKRendererDelegate.cpp"
        ],
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!(pkg-config --cflags glib-2.0)",
        "/usr/include/glib-2.0",
        "zoom-bot/lib/zoomsdk/h",
        "build/vcpkg_installed/x64-linux/include",
        ],
      'libraries': [
        "-L../zoom-bot/lib/zoomsdk",
        "-lmeetingsdk",
        "-L../zoom-bot/lib/zoomsdk/qt_libs/Qt/lib",
        "-L./vcpkg_installed/x64-linux/lib", 
        "-lCLI11",
        "-lada"
      ],
      "ldflags": [
        "-Wl,-rpath,../zoom-bot/lib/zoomsdk"
      ],
      "conditions": [
            ["OS==\"linux\"",
              {
                "link_settings": {
                  "libraries": [
                    "-Wl,-rpath,'$$ORIGIN'",
                    "-Wl,-rpath,'$$ORIGIN'/..",
                    "-Wl,-rpath,'$$ORIGIN'/../../zoom-bot/lib/zoomsdk",
                  ],
                }
              }
            ]
        ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}
