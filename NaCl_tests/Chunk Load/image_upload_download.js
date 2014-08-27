
function dl(){

    dl = this;

    /*consolidate an an array of w*h*4 values into a w*h array 32-bit colors*/
    dl.consolidateImageData = function(imgData,w,h){
	var con = new Array(w*h+2);
	
	for(var i = 0; i < w*h; i++){
	    var r = imgData[i*4];
	    var g = imgData[i*4 + 1];
	    var b = imgData[i*4 + 2];

	    con[i] =  (r << 16) + (g << 8) + b;
	}
	con[w*h] = w;
	con[w*h+1] = h;
	return con;
    };

    /*Turns an image into a byte image using an invisible canvas element*/
    dl.imgToArray = function(img){
	var canvas = $('<canvas width="' + img.width + '" height="' + img.height + '" style="display:none" />')[0];
	var ctx = canvas.getContext('2d');
	ctx.drawImage(img, 0,0, img.width,img.height);
        var imgData = ctx.getImageData(0,0,img.width, img.height).data;
	return dl.consolidateImageData(imgData,img.width,img.height);

    };

    dl.errorHandle = function(e){
	console.log('Error in the filesystem API: ' + e.message);
    };

    /*Append a file upload dialogue to the give anchor
     *Calls filleLoaded(url) when the file has been loaded.
     */
    dl.addFileBrowser = function(anchor,fileLoaded){
	var elm = $('<input type="file" id="fileLoader" multiple />');

	var fileChosen = function(e){
	   var file = this.files[0];
	   var reader = new FileReader();

	    reader.onload = function(e) {
		fileLoaded(reader.result);
	    };
	    reader.readAsDataURL(file);

	};
	
	elm.on('change',fileChosen);
	anchor.append(elm);
    };


    
   return dl;
};
