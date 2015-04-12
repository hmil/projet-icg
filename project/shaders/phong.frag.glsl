// Copyright (C) 2014 - LGG EPFL
#version 330 core
uniform vec3 Ia, Id, Is;
uniform vec3 ka, kd, ks;
uniform float p;

in vec3 normal;
in vec3 light_dir;
in vec3 view_dir;

out vec3 color;

void main() {

    vec3 R = normalize(2*normal*dot(normal, light_dir) - light_dir);
    color = Ia*ka + clamp(Id*kd*dot(normal, light_dir), vec3(0,0,0), vec3(1,1,1)) + Is * ks * pow(clamp(dot(view_dir, R), 0, 1), p);
}