#version 330 core

layout(location=0) in vec4 in_Position;
layout(location=1) in vec3 in_Color;
layout(location=2) in vec3 in_Normal;
layout(location=3) in mat4 modelMatrix;

uniform mat4 viewShader;
uniform mat4 projectionShader;
uniform vec3 obsShader;

struct MaterialProperties
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininessValue;
};

struct Light
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

uniform MaterialProperties materialShader;
uniform Light lightShader;

out vec3 ex_Color;
flat out int ex_InstanceId;

vec4 source_Position;
vec3 positionSource3D, positionVertex3D, s_normal,
    lightDir, viewDir, reflectDir, 
    emission, ambient_model, ambient_term, diffuse_term, specular_term;
float distSV, attenuation_factor, diffCoeff, specCoeff;

void main(void)
{
    if (lightShader.position.w != 0.0 && in_Position.w !=0)
    {
        positionVertex3D = vec3(in_Position.x, in_Position.y, in_Position.z)/in_Position.w;
        positionSource3D = vec3(lightShader.position.x, lightShader.position.y, lightShader.position.z)/lightShader.position.w;
        distSV = distance(positionSource3D, positionVertex3D);
    };

    gl_Position = projectionShader * viewShader * modelMatrix * in_Position;

    s_normal = normalize(in_Normal);

    emission = materialShader.emission;

    ambient_model =vec3(0.2, 0.2, 0.2) * materialShader.ambient;

    ambient_term = lightShader.ambient * materialShader.ambient;

    if (lightShader.position.w == 0.0 || in_Position.w == 0.0)
    {
        lightDir = normalize(vec3(lightShader.position));
    }
    else
    {
        lightDir = normalize(positionSource3D - positionVertex3D);
    }
    
    diffCoeff = max(dot(s_normal, lightDir), 0.0);
    diffuse_term = diffCoeff * lightShader.diffuse * materialShader.diffuse;

    reflectDir = normalize(reflect(-lightDir, s_normal));
    viewDir = normalize(obsShader - positionVertex3D);
    specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), materialShader.shininessValue);
    specular_term = specCoeff * lightShader.specular * materialShader.specular;

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
    ex_Color = ex_Color + in_Color;
    ex_Color = clamp(ex_Color, 0.0, 1.0);

    ex_InstanceId = gl_InstanceID;
}
