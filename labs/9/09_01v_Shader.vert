//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IX - 09_01_Shader.vert|
// ======================================
// 
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei; 
//

#version 330 core

//  Variabile de intrare din Vertex Array Object;
layout(location=0) in vec4 in_Position;                 //  Se preia din buffer de pe prima pozitie (0) atributul care contine coordonatele;
layout(location=1) in vec3 in_Color;                    //  Se preia din buffer de pe a doua pozitie (1) atributul care contine culoarea;
layout(location=2) in vec3 in_Normal;					//  Se preia din buffer de pe a treia pozitie (2) atributul care contine normalele;


//  Variabile uniforme;
uniform mat4 viewShader;
uniform mat4 projectionShader;
uniform vec3 obsShader;

// Structura pentru materiale
struct ProprMaterial
{
	vec3 emission;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininessValue;
};

// Structura pentru lumina
struct Light
{
	vec4 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 attenuation;
};	

uniform ProprMaterial materialShader;
uniform Light lightShader;


//  Variabile de iesire;
out vec3 ex_Color;      //  Transmite culoarea (de modificat in Shader.frag); 

// Variabile intermediare
vec4 source_Position;
vec3 positionSource3D, positionVertex3D, s_normal,
	lightDir, viewDir, reflectDir, 
	emission, ambient_model, ambient_term, diffuse_term, specular_term;
float distSV, attenuation_factor, diffCoeff, specCoeff;

// Functia principala a shader-ului de varfuri;
void main(void)
{
	// Geometria scenei;
	// Pozitiile 3D ale varfului si sursei de lumina si distanta dintre ele;
	if (lightShader.position.w != 0.0 && in_Position.w !=0)
	{
		positionVertex3D = vec3(in_Position.x, in_Position.y, in_Position.z)/in_Position.w;
		positionSource3D = vec3(lightShader.position.x, lightShader.position.y, lightShader.position.z)/lightShader.position.w;
		distSV = distance(positionSource3D, positionVertex3D);
	};

	// Pozitia finala a varfului, dupa aplicarea transformarilor de vizualizare si proiectie;
	gl_Position = projectionShader * viewShader *  in_Position;

	// Vectorul normal la suprafata randata;
	s_normal = normalize(in_Normal); 

	// IMPLEMENTAREA MODELULUI DE ILUMINARE

	// Emission 
	emission = materialShader.emission;

	// Ambient model (global), valoarea (0.2, 0.2, 0.2) reprezinta termenul ambiental global
	ambient_model =vec3(0.2, 0.2, 0.2) * materialShader.ambient;

	// Pentru sursa de lumina
 
	// (i) Ambient_term
	ambient_term = lightShader.ambient * materialShader.ambient; 

	// (ii) Diffuse_term
	// Calculul vectorului L(lightDir) (catre sursa de lumina)
	if (lightShader.position.w == 0.0 || in_Position.w == 0.0)
	{
		// Lumina directionala
		lightDir = normalize(vec3(lightShader.position));
	}
	else
	{
		// Sursa de lumina punctuala
		lightDir = normalize(positionSource3D - positionVertex3D); // Vectorul L catre sursa de lumina
	}
   // Vectorul s (s_normal) al normalei la suprafata a fost deja calculat; 
    diffCoeff = max(dot(s_normal, lightDir), 0.0); // 
    diffuse_term = diffCoeff * lightShader.diffuse * materialShader.diffuse;

	// (iii) Specular
    reflectDir = normalize(reflect(-lightDir, s_normal));      // versorul vectorului R; 
	viewDir = normalize(obsShader - positionVertex3D);         // versorul catre observator;
    specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), materialShader.shininessValue); // coeficient specular
    specular_term = specCoeff * lightShader.specular * materialShader.specular;     

	// (iv) Attentuation factor
	if (lightShader.position.w != 0.0 && in_Position.w !=0)
	{
		attenuation_factor = 
			1.0 / (lightShader.attenuation[0] + lightShader.attenuation[1] * distSV + lightShader.attenuation[2] * (distSV * distSV));
	}
	else
	{
		attenuation_factor = 1.0;
	}

    
    ex_Color = emission + 
			   ambient_model +
			   attenuation_factor * (ambient_term + diffuse_term + specular_term);
	ex_Color = ex_Color + in_Color; // Adaugarea culorii initiale a varfului
	ex_Color = clamp(ex_Color, 0.0, 1.0); // Asigurarea ca valorile sunt intre 0 si 1
	//ex_Color = normalize(s_normal);
	//ex_Color = vec3(specCoeff, 0, 0);

	 
} 
 