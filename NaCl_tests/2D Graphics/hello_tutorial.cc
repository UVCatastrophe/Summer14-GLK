// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include "ppapi/c/ppb_image_data.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/utility/completion_callback_factory.h"

namespace {

// The expected string sent by the browser.
const char* const kHelloString = "hello";
// The string sent back to the browser upon receipt of a message
// containing "hello".
const char* const kReplyString = "hello from NaCl";

}  // namespace

class Graphics2DInstance : public pp::Instance {
 public:
  explicit Graphics2DInstance(PP_Instance instance)
    : pp::Instance(instance),
      callback_factory_(this){}
  virtual ~Graphics2DInstance() {}

  virtual void HandleMessage(const pp::Var& var_message) {
    // Ignore the message if it is not a string.
    if (!var_message.is_string())
      return;

    // Get the string message and compare it to "hello".
    std::string message = var_message.AsString();
    console(message.c_str());
    if (message == kHelloString) {
      // If it matches, send our response back to JavaScript.
      pp::Var var_reply(kReplyString);
      PostMessage(var_reply);
    }
}

  virtual void DidChangeView(const pp::View& view){
    size = view.GetRect().size();
    
    context = pp::Graphics2D(this, size, true);
    if(!BindGraphics(context)){
      console("Unable to bind 2d context!\n");
      context = pp::Graphics2D();
      return;
    }
    
    MainLoop(0);
  }
  
  void MainLoop(int32_t){
        PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
    pp::ImageData image_data(this, format, size, true);
    
    uint32_t* data = static_cast<uint32_t*>(image_data.data());
    uint32_t num_pixels = size.width() * size.height();
    for(uint32_t i = 0; i < num_pixels; i++){
      data[i] = static_cast<int32_t>(0xFFFFFF * rand());
    }
    
    context.PaintImageData(image_data, pp::Point(0,0));
    context.Flush(callback_factory_.NewCallback(&Graphics2DInstance::MainLoop));
  }

  void console(const char* s){
    pp::Var var_reply(s);
    PostMessage(s);
  }

  pp::Size size;
  pp::Graphics2D context;
  pp::CompletionCallbackFactory<Graphics2DInstance> callback_factory_;

};

class Graphics2DModule : public pp::Module {
 public:
  Graphics2DModule() : pp::Module() {}
  virtual ~Graphics2DModule() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new Graphics2DInstance(instance);
  }
};

namespace pp {

Module* CreateModule() {
  printf("hello\n");
  return new Graphics2DModule();
}

}  // namespace pp
