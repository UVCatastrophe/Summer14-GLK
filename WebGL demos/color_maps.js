function colorMaps(){

    var cm = this;

    cm.init = function(){

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
	gl.uniform1i(cm.texLoc, 0);

	var img = new Image();
	img.onload = function(){ init_texture(img); };
	img.src = "test.png";


    };

    cm.init_texture = function(img){
	cm.texture = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, texture);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE,img);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
	gl.bindTexture(gl.TEXTURE_2D,null);

	cm.draw();

    }

    cm.draw = function(){
	window.requestAnimFrame(cm.draw, cm.canvas);
	gl.drawArrays(gl.TRIANGLE_FAN,0,4);
    };


    return this;

};

cm = colorMaps();
