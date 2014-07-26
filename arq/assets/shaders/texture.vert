uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_texMatrix;
uniform vec4 u_colorTransform;

attribute vec2 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoords; 

varying vec4 v_color;
varying vec2 v_texCoords;

void main()
{
	v_color = a_color * u_colorTransform;
	
	vec4 coord = vec4(a_texCoords, 0.0, 1.0);
   coord = u_texMatrix * coord;

	v_texCoords = coord.xy;

	vec4 position = vec4(a_position, 0, 1);	
	gl_Position = u_viewMatrix * (u_modelMatrix * position);
}