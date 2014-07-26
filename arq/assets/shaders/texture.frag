uniform sampler2D u_texture;

varying vec4 v_color;
varying vec2 v_texCoords;

void main()
{
	
	vec4 color = v_color;
	float opacity = color.a;
	
	color *= texture2D(u_texture, v_texCoords);

	gl_FragColor = color;

}