//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul X - 10_01_Shader.vert |
// =====================================
// 
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei; 
 
 #version 330 core


layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

//  Variabile de iesire;
out vec3 FragPos;
out vec3 Normal;
out vec3 inViewPos;
out vec3 inLightPos;
out vec4 ex_Color;

//  Variabile uniforme;
uniform int nrVertices;
uniform mat4 myMatrix;
uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 projection;
 

void main(void)
  {
    // Transformari in spatiul de modelare
    gl_Position = projection * view * myMatrix * vec4(in_Position, 1.0);

    // Pentru modelul de iluminare 
    // Atribute ale varfurilor
    FragPos = mat3(myMatrix) * in_Position;
    Normal = mat3(myMatrix) * in_Normal;
    // Pozitia observatorului
    inViewPos = viewPos;

    // Pozitia sursei de lumina (coincide cu a observatorului)
    inLightPos = viewPos;
    
    // Pozitia sursei de lumina (diferita de a observatorului)
    // inLightPos = vec3(5.0, 5.0, 7.0);

    // Culoarea varfurilor
 	// ex_Color=vec4(1.2 * gl_VertexID/nrVertices, 0.95 * gl_VertexID/nrVertices, 1.5 * gl_VertexID/nrVertices, 1.0);
	 ex_Color=vec4(0.59, 0.29, 0.0, 1.0);
   } 
 