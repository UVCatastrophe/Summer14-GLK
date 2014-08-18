function colorMaps(){

    var cm = this;
    cm.max_pts = 10;

    cm.ctrl_pt_dom = [];
    cm.ctrl_pts = new Array(cm.max_pts);
    cm.ctrl_colors = new Array(cm.max_pts*3);
    cm.pts_used = 0;

    cm.toHexColor = function(index){
	var num = (cm.ctrl_colors[index*3 + 2] * 255);
	num += (cm.ctrl_colors[index*3 + 1] * 255) << 8;
	num += (cm.ctrl_colors[index*3 +2] * 255) << 16;

	var str = num.toString(16);
	while(str.length < 6)
	    str = "0" + str;
	return "#" + str;
    };

    cm.init = function(){
	var elm = $('#ctrlPtLast');
	elm.on('dragover',cm.handleDragOver);
	elm.on('dragenter',cm.handleDragEnter);
	elm.on('dragleave',cm.handleDragLeave);

	$('#addCtrlPtButton').on('click',handleAddCtrlPt);

	dl.addFileBrowser($('#uploadButton'), function(url){
	    cm.initGL(url);
	    cm.init_first_point();
	});

    }

    cm.initGL = function(imageUrl){

	cm.canvas = document.getElementById("colorMapsGLCanvas");
	cm.gl = canvas.getContext("experimental-webgl");
    
	cm.vertexShader = createShaderFromScriptElement(gl,
							 "color-map-vertex-shader");

	cm.fragmentShader = createShaderFromScriptElement(gl,
							   "color-map-fragment-shader");

	cm.program = createProgram(gl,[cm.vertexShader,cm.fragmentShader]);
	gl.useProgram(cm.program);

	cm.positionLoc = gl.getAttribLocation(cm.program, "position");

	cm.buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
	gl.bufferData(gl.ARRAY_BUFFER,
		      new Float32Array([-1.0,-1.0,
					 1.0,-1.0,
					 1.0, 1.0,
					-1.0, 1.0,]),
		      gl.STATIC_DRAW);

	gl.enableVertexAttribArray(cm.positionLoc);
	gl.vertexAttribPointer(cm.positionLoc,2,gl.FLOAT,false,0,0);

	cm.texLoc = gl.getUniformLocation(cm.program, "tex");
	cm.ptsLoc = gl.getUniformLocation(cm.program, "ctrl_pts");
	cm.colorsLoc = gl.getUniformLocation(cm.program, "ctrl_colors");
	cm.ptsUsedLoc = gl.getUniformLocation(cm.program, "max_pt");

	var img = new Image();
	img.onload = function(){ init_texture(img); };
	img.src = imageUrl;

    };

    cm.init_texture = function(img){
	cm.texture = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, texture);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE,img);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	gl.uniform1i(cm.texLoc, 0);

	cm.draw();

    }

    cm.draw = function(){
	gl.uniform1fv(cm.ptsLoc,
		      new Float32Array(cm.ctrl_pts));
	gl.uniform3fv(cm.colorsLoc,
		      new Float32Array(cm.ctrl_colors));
	gl.uniform1i(cm.ptsUsedLoc,cm.pts_used);

	window.requestAnimFrame(cm.draw, cm.canvas);
	gl.drawArrays(gl.TRIANGLE_FAN,0,4);
    };

    cm.handleDragStart = function(e){
	this.style.opacity = '0.4';

	cm.dragged = $(this);

    };

    cm.handleDragOver = function(e){
	if(e.preventDefault)
	    e.preventDefault();

    };

    cm.handleDragEnter = function(e){
	this.classList.add('over');

	cm.over = this;
    };

    cm.handleDragLeave = function(e){
	this.classList.remove('over');
    };


    cm.handleDragEnd = function(e){
	$('.ctrlPt').each(function(){
	    this.classList.remove('over');
	    this.style.opacity = '1.0';
	});
	$(cm.over).before($(this));

	cm.changePosition($(this),$(cm.over));
    };

    cm.init_first_point = function(){
	cm.ctrl_pts[0] = 0.0;
	
	cm.ctrl_colors[0] = 0.0;
	cm.ctrl_colors[1] = 0.0;
	cm.ctrl_colors[2] = 0.0;

	var elm = cm.templateCtrlPt(0);
	cm.ctrl_pt_dom.push(elm);
    }

    cm.handleAddCtrlPt = function(){
	cm.pts_used++;
	cm.ctrl_pts[cm.pts_used] = 1.0;

	cm.ctrl_colors[cm.pts_used*3 + 0] = 1.0;
	cm.ctrl_colors[cm.pts_used*3 + 1] = 1.0;
	cm.ctrl_colors[cm.pts_used*3 + 2] = 1.0;
	
	var elm = cm.templateCtrlPt(cm.pts_used);
	cm.ctrl_pt_dom.push(elm);
    };

    cm.changePosition = function(oldDom,newDom){
	var oldIndex;
	var newIndex;

	console.log(cm.ctrl_pt_dom[0].is(newDom));
	//Find the index corresponding to the new and old positions
	for(var i = 0; i < cm.ctrl_pt_dom.length; i++){
	    var dom = cm.ctrl_pt_dom[i];
	    if(dom.is(oldDom))
		oldIndex = i;
	    if(dom.is(newDom))
		newIndex = i;
	    if(oldIndex && newIndex)
		break;
	}
	if(newIndex == undefined)
	    newIndex = cm.ctrl_pt_dom.length;
	console.log(oldIndex,newIndex);
	if(newIndex >= oldIndex){
	    cm.ctrl_pt_dom.splice(newIndex,0,oldDom);
	    var pt = cm.ctrl_pts[oldIndex];
	    cm.ctrl_pts.splice(newIndex,0,pt);
	    var c1 = cm.ctrl_colors[oldIndex*3];
	    var c2 = cm.ctrl_colors[oldIndex*3 + 1];
	    var c3 = cm.ctrl_colors[oldIndex*3 + 2];
	    cm.ctrl_colors.splice(newIndex*3,0,c1,c2,c3);

	    cm.ctrl_pt_dom.splice(oldIndex,1);
	    cm.ctrl_pts.splice(oldIndex,1);
	    cm.ctrl_colors.splice(oldIndex*3,3);

	}

	else{

	    cm.ctrl_pt_dom.splice(oldIndex,1);
	    var pt = cm.ctrl_pts.splice(oldIndex,1);
	    var cols = cm.ctrl_colors.splice(oldIndex*3,3);

	    cm.ctrl_pt_dom.splice(newIndex,0,oldDom);
	    cm.ctrl_pts.splice(newIndex,0,pt[0]);
	    cm.ctrl_colors.splice(newIndex*3,0,cols[0],cols[1],cols[2]);
	}
	console.log(ctrl_pts);

    };

    cm.getIndex = function(dom){
	for(var i = 0; i < cm.ctrl_pt_dom.length; i++){
	    if(dom.is(cm.ctrl_pt_dom[i]))
		return i;
	}
    };

    cm.templateCtrlPt = function(num){
	var li = '<li id="ctrlPtElm'+num+'" draggable="true" num="'+num+'" class="ctrlPt" />';
	var elm = $(li);
	elm.on('dragstart',cm.handleDragStart);
	elm.on('dragover',cm.handleDragOver);
	elm.on('dragenter',cm.handleDragEnter);
	elm.on('dragleave',cm.handleDragLeave);
	elm.on('dragend',cm.handleDragEnd);

	var c = $('<input type="color">');
	c.val(cm.toHexColor(cm.pts_used));
	c.on('change', function(){
	    var index = cm.getIndex(elm);
	    var hex = c.val();
	    cm.ctrl_colors[3*index] = parseInt(hex.substr(1,2),16) /255.0;
	    cm.ctrl_colors[3*index+1] = parseInt(hex.substr(3,2),16) /255.0;
	    cm.ctrl_colors[3*index+2] = parseInt(hex.substr(5,2),16) /255.0;


	});
	elm.append(c);

	var num = $('<input type="number" min="0.0" max="1.0" step=".001" >');
	num.val(cm.ctrl_pts[cm.pts_used]);
	num.on('change', function(){
	    var index = cm.getIndex(elm);
	    cm.ctrl_pts[index] = num.val();
	});

	elm.append(num);
	
	$('#ctrlPtLast').before(elm);
	return elm;
    };

    cm.removeCtrlPt = function(num){
	var pt = $('ctrlPtElm'+num);
	pt.off();
	pt.remove();
    } 

    return this;


};
