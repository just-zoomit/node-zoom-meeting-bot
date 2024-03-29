#include "object_wrap_demo.h"

#include <glib.h>
#include <thread>
#include "napi-thread-safe-callback.hpp"

using namespace Napi;

gboolean onTimeout (gpointer data) {
    return TRUE;
}


void PrintThreadName() {
    char name[16];
    if (pthread_getname_np(pthread_self(), name, sizeof(name)) == 0) {
        std::cout << "Thread name: " << name << std::endl;
    } else {
        std::cout << "Thread name not available." << std::endl;
    }
}

class ZoomSDKManager {
public:
    // Function to get the singleton instance
    static ZoomSDKManager& getInstance() {
        static ZoomSDKManager instance; // Static instance ensures it's created only once
        return instance;
    }

    ~ZoomSDKManager() {
        stopZoomSDKThread();
        for (auto& zoomSDK : zoomSDKs) {
            delete zoomSDK;
        }
    }

    void startZoomSDKThread() {
        zoomThread = std::thread([this]() {
            loop = g_main_loop_new(NULL, FALSE);
            g_timeout_add(100, onTimeout, loop);
            g_main_loop_run(loop);
        });
    }

    void stopZoomSDKThread() {
        if (loop) {
            g_main_loop_quit(loop);
            if (zoomThread.joinable()) {
                zoomThread.join();
            }
            g_main_loop_unref(loop);
            loop = nullptr;
        }
    }

    std::vector<Zoom*> getAllZoomSDKs() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]() { return !zoomSDKs.empty(); });
        return zoomSDKs;
    }

    void invokeFunctionOnZoomThread(std::function<void()> func) {
        g_idle_add_full(G_PRIORITY_HIGH_IDLE, [](gpointer userData) -> gboolean {
            auto function = reinterpret_cast<std::function<void()>*>(userData);
            if (function) {
                (*function)();
                delete function;
            }
            return G_SOURCE_REMOVE; // Remove the source after execution
        }, new std::function<void()>(std::move(func)), nullptr);
    }

    // Function to invoke a function on ZoomSDK within the Glib main loop
    void invokeFunctionOnZoomThread(std::function<void()> func, std::function<void()> onComplete) {
        // Create a wrapper function to call onComplete on the same thread
        auto wrapperFunc = [onComplete]() {
            onComplete();
        };

        // Schedule the execution of func within the Glib main loop
        g_idle_add_full(G_PRIORITY_HIGH_IDLE, [](gpointer userData) -> gboolean {
            auto functionPair = reinterpret_cast<std::pair<std::function<void()>, std::function<void()>>*>(userData);
            if (functionPair) {
                // Execute the function within the Glib main loop
                functionPair->first();

                // Schedule the wrapper function to be executed on the same thread
                g_idle_add_full(G_PRIORITY_HIGH_IDLE, [](gpointer wrapperUserData) -> gboolean {
                    auto wrapperFunction = reinterpret_cast<std::function<void()>*>(wrapperUserData);
                    if (wrapperFunction) {
                        (*wrapperFunction)();
                        delete wrapperFunction;
                    }
                    return G_SOURCE_REMOVE;
                }, new std::function<void()>(functionPair->second), nullptr);

                delete functionPair;
            }
            return G_SOURCE_REMOVE;
        }, new std::pair<std::function<void()>, std::function<void()>>(std::move(func), std::move(wrapperFunc)), nullptr);
    }

private:
    GMainLoop* loop;
    std::thread zoomThread;
    std::vector<Zoom*> zoomSDKs;
    std::mutex mutex;
    std::condition_variable cv;

    // Make constructors private to prevent external instantiation
    ZoomSDKManager() = default;
    ZoomSDKManager(const ZoomSDKManager&) = delete;
    ZoomSDKManager& operator=(const ZoomSDKManager&) = delete;
};

Napi::Value StartLoop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  ZoomSDKManager& manager = ZoomSDKManager::getInstance();
  manager.startZoomSDKThread();

  std::cout << ">>>> StartLoop"  << std::endl;
  return Boolean::New(env, true);
}

Napi::Value StopLoop(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.stopZoomSDKThread();

  std::cout << ">>>> StopLoop" << std::endl;
  return Boolean::New(env, true);
}

Napi::Value Config(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::string args = info[0].As<Napi::String>().Utf8Value();

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.invokeFunctionOnZoomThread(
      [args]()
      {
        auto *zoom = &Zoom::getInstance();
        zoom->config(args + " RawAudio --separate-participants");
      });

  return Boolean::New(env, true);
}

Napi::Value InitZoom(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  SDKError err{SDKERR_SUCCESS};

  auto callback = std::make_shared<ThreadSafeCallback>(info[0].As<Function>());

  Zoom::RawAudioRecordingCallback audioCallback = [callback](AudioRawData* data, uint32_t node_id) {
    char* buffer = data->GetBuffer();
    unsigned int bufferLen = data->GetBufferLen();
    unsigned int sampleRate = data->GetSampleRate();
    unsigned int channelNum = data->GetChannelNum();

    char* copiedBuffer = new char[bufferLen];
    std::memcpy(copiedBuffer, buffer, bufferLen);

    callback->call([copiedBuffer, bufferLen, sampleRate, channelNum, node_id](Napi::Env env, std::vector<napi_value>& args)
    {
      Napi::Object result = Napi::Object::New(env);
      result.Set("buffer", Napi::Buffer<char>::New(env, copiedBuffer, bufferLen));
      result.Set("bufferLength", Napi::Number::New(env, bufferLen));
      result.Set("sampleRate", Napi::Number::New(env, sampleRate));
      result.Set("channelNum", Napi::Number::New(env, channelNum));
      result.Set("nodeId", Napi::Number::New(env, node_id));

      args = { result };
    });
  };

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.invokeFunctionOnZoomThread(
      [audioCallback]()
      {
        auto *zoom = &Zoom::getInstance();
        zoom->init(audioCallback);
      });
  return Boolean::New(env, true);
}

Napi::Value Auth(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  SDKError err{SDKERR_SUCCESS};

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.invokeFunctionOnZoomThread(
      []()
      {
        auto *zoom = &Zoom::getInstance();
        zoom->auth();
      });
  return Boolean::New(env, true);
}

Napi::Value Leave(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.invokeFunctionOnZoomThread(
      []()
      {
        auto *zoom = &Zoom::getInstance();
        zoom->leave();
      });

  std::cout << "Left" << std::endl;

  return Boolean::New(env, true);
}

Napi::Value Clean(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  ZoomSDKManager &manager = ZoomSDKManager::getInstance();
  manager.invokeFunctionOnZoomThread(
      []()
      {
        auto *zoom = &Zoom::getInstance();
        zoom->clean();
      });

  std::cout << "Cleaned" << std::endl;

  return Boolean::New(env, true);
}

Napi::Value Test(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  auto callback = std::make_shared<ThreadSafeCallback>(info[0].As<Function>());

  Zoom::RawAudioRecordingCallback audioCallback = [callback](AudioRawData* data, uint32_t node_id) {
    // std::cout << "Received AudioRawData with node ID: " << node_id << " " << data->GetBufferLen() << "b " << " at " << data->GetSampleRate() << "Hz" << data->GetChannelNum() << "ch"  << std::endl;
    // Process the AudioRawData as needed

    char* buffer = data->GetBuffer();
    unsigned int bufferLen = data->GetBufferLen();
    unsigned int sampleRate = data->GetSampleRate();
    unsigned int channelNum = data->GetChannelNum();

    callback->call([buffer, bufferLen, sampleRate, channelNum, node_id](Napi::Env env, std::vector<napi_value>& args)
    {

      Napi::Object result = Napi::Object::New(env);
      result.Set("buffer", Napi::Buffer<char>::Copy(env, buffer, bufferLen));
      result.Set("bufferLength", Napi::Number::New(env, bufferLen));
      result.Set("sampleRate", Napi::Number::New(env, sampleRate));
      result.Set("channelNum", Napi::Number::New(env, channelNum));
      result.Set("nodeId", Napi::Number::New(env, node_id));

        // This will run in main thread and needs to construct the
        // arguments for the call
      args = { result };
    });
  };


    ZoomSDKManager& manager = ZoomSDKManager::getInstance();

    manager.invokeFunctionOnZoomThread(
        [audioCallback]() {
            // You can invoke functions of zoomSDK1 or perform other operations here
            auto *zoom = &Zoom::getInstance();

            zoom->init(audioCallback);
            zoom->auth();
        }
    );
  
  return Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "startLoop"), Napi::Function::New(env, StartLoop));
  exports.Set(Napi::String::New(env, "stopLoop"), Napi::Function::New(env, StopLoop));
  exports.Set(Napi::String::New(env, "config"), Napi::Function::New(env, Config));
  exports.Set(Napi::String::New(env, "init"), Napi::Function::New(env, InitZoom));
  exports.Set(Napi::String::New(env, "auth"), Napi::Function::New(env, Auth));
  exports.Set(Napi::String::New(env, "leave"), Napi::Function::New(env, Leave));
  exports.Set(Napi::String::New(env, "clean"), Napi::Function::New(env, Clean));
  exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, Test));
  return exports;
}

NODE_API_MODULE(addon, Init)
