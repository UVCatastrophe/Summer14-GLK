// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This function is called by common.js when the NaCl module is
// loaded.
function moduleDidLoad() {
  dl = dl();
    dl.addFileBrowser($('#loadButtonEntry'), function(url){
	var img = new Image();
	img.onload = function(){ 
	    var array = dl.imgToArray(this)
	    common.naclModule.postMessage(array);
	};
	img.src = url;
    });


    
}

// This function is called by common.js when a message is received from the
// NaCl module.
function handleMessage(message) {
  console.log(message.data);
}
