#version 410 core

in vec2 fragTexCoords;
in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;

uniform mat3 lightDirMatrix;

uniform sampler2D shadowMap;

uniform	vec3 lightDir;
uniform	vec3 lightColor;
//uniform	vec3 lightPosEye;

uniform samplerCube skybox;

uniform	sampler2D diffuseTexture;
uniform	sampler2D specularTexture;

uniform int fog;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

vec3 ambient;
vec3 reflection;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeFog()
{
 float fogDensity = 0.0f;
 float fragmentDistance = length(fragPosEye.xyz);
 if(fog==1)
 {
	fogDensity = 0.1f;
 }
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

void main() 
{	computeLightComponents();
	
	float shadow = computeShadow();
	
	//modulate with diffuse map
	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	specular *= vec3(texture(specularTexture, fragTexCoords));
	vec4 text = texture(diffuseTexture, fragTexCoords);
	if(text.a < 0.1)
		discard;
	//modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    		
	float fogFactor = computeFog();
	vec3 fogColor = vec3(0.5f, 0.5f, 0.5f);
	//fColor = mix(fogColor, color, fogFactor);
    //fColor = vec4(color, 1.0f);
	fColor = vec4(fogColor * (1 - fogFactor) + color * fogFactor, 1.0f);
}
