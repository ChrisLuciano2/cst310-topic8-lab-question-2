// sph.frag — Blinn-Phong with fixed material (teal, 64 shininess).
#version 410 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uCamPos;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 64.0) * (NdotL > 0.0 ? 1.0 : 0.0);

    vec3 lightColor = vec3(1.0, 0.95, 0.85);
    vec3 objColor   = vec3(0.40, 0.78, 0.74);   // teal
    vec3 ambient    = 0.12 * lightColor;
    vec3 color = objColor * (ambient + NdotL * lightColor) + spec * lightColor;
    FragColor = vec4(color, 1.0);
}
