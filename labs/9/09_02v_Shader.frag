//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IX - 09_02v_Shader.frag |
// =======================================
// 
//  Shaderul de fragment
//

#version 330 core

in vec4 ex_Color; 
out vec3 out_Color;

void main(void)
  { 
	out_Color = vec3(ex_Color);
  }
 