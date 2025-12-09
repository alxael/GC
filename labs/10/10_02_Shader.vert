//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul X - 10_02_Shader.vert |
// =====================================
// 
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei; 

#version 330 core

layout(location=0) in vec4 in_Position;
layout(location=1) in vec3 in_Color;
layout(location=2) in vec3 in_Normal;
 
out vec3 FragPos;
out vec3 Normal;
out vec3 inLightPos;
out vec3 inViewPos;
out vec3 ex_Color;
out vec3 dir;
 
uniform mat4 matrUmbra;
uniform mat4 myMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform int codCol;

void main(void)
  {
    // Transformari in spatiul de modelare
    // Pozitia varfului (pentru shaderul de fragment)
    if (codCol==0)
		gl_Position = projection * view * myMatrix * in_Position;
	if (codCol==1)
		gl_Position = projection * view * matrUmbra * myMatrix * in_Position;

    // Transformari pentru iluminare
    // In acest exemplu myMatrix este identitatea, dar poate fi modificata dupa caz (daca este necesar pot fi luate in considerare matrice diferite, in functie de miscarea varfurilor/sursei de lumina/a observatorului)
    // Atribute ale varfurilor
    FragPos = vec3(myMatrix * in_Position);
    Normal = mat3(myMatrix) * normalize(in_Normal); 
    // Sursa de lumina
    inLightPos = vec3(myMatrix * vec4(lightPos, 1.0f));
    dir = vec3(myMatrix * vec4(100.0f, 100.0f, 150.0f, 0.0f)); // pentru sursa directionala
    // Pozitia observatorului
    inViewPos = vec3(myMatrix * vec4(viewPos, 1.0f));

    ex_Color=in_Color;
   } 