
function dl(){

    dl = this;

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
