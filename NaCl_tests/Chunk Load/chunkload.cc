// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
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
      callback_factory_(this){
    imgLoaded = false;
  }
  virtual ~Graphics2DInstance() {}

  virtual void HandleMessage(const pp::Var& var_message) {
    if(var_message.is_array()){
      const pp::VarArray img = pp::VarArray(var_message);
      img_w = img.Get( img.GetLength()-2).AsInt();
      img_h = img.Get( img.GetLength()-1).AsInt();
      image_raw = new uint32_t[img_w*img_h];

      uint32_t len = img.GetLength();
      for(uint32_t i = 0; i < len; i++){
	image_raw[i] = img.Get(i).AsInt();
      }

      imgLoaded = true;
     
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

    if(!imgLoaded){
      for(uint32_t i = 0; i < num_pixels; i++){
	data[i] = static_cast<int32_t>(0xFFFFFF * rand());
      }
    }

    else{
      uint32_t w = size.width();
      uint32_t h = size.height();
      for(uint32_t i = 0; i < h; i++){
	for(uint32_t j = 0; j < w; j++){
	  uint32_t row = i % img_h;
	  uint32_t col = j % img_w;
	  data[i*w+j] = image_raw[row*img_w + col]; 
	}
      }
    }
    
    context.PaintImageData(image_data, pp::Point(0,0));
    context.Flush(callback_factory_.NewCallback(&Graphics2DInstance::MainLoop));
  }

  void console(const char* s){
    pp::Var var_reply(s);
    PostMessage(s);
  }

  bool imgLoaded;
  uint32_t img_w;
  uint32_t img_h;
  uint32_t* image_raw;
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
